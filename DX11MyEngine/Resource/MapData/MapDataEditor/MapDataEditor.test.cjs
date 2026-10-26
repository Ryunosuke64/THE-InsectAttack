"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const C = require("./MapDataEditor.core.js");
const original = Object.fromEntries(Object.entries(C.definitions).map(([kind, def]) => [kind, JSON.parse(fs.readFileSync(path.join(__dirname, def.file), "utf8"))]));
let count = 0;
function test(name, fn) { fn(); count++; console.log(`PASS ${name}`); }
test("all existing definitions validate together", () => assert.deepEqual(C.validate(original), []));
for (const [kind, data] of Object.entries(original)) {
  test(`${kind}: read/write round trip preserves every value`, () => {
    const result = C.parse(JSON.stringify(data)); assert.equal(result.kind, kind); assert.deepEqual(JSON.parse(JSON.stringify(result.data, null, 4)), data);
  });
}
test("unknown root and nested keys survive a targeted field edit", () => {
  const data = C.clone(original.physics);
  data.future = { version: 9 }; data.physics[0].rigidBody.extra = ["future", { enabled: true }];
  const read = C.parse(JSON.stringify(data)).data; read.physics[0].rigidBody.mass = 42;
  const saved = C.parse(JSON.stringify(read)).data;
  assert.deepEqual(saved.future, data.future); assert.deepEqual(saved.physics[0].rigidBody.extra, data.physics[0].rigidBody.extra);
});
test("physics ID rename updates assets and debris, without touching other presets", () => {
  const docs = C.clone(original); const old = docs.physics.physics[0].presetID;
  const refs = C.references(docs, "physics", old).length;
  assert.equal(C.rename(docs, "physics", 0, "Renamed"), refs);
  assert.equal(C.references(docs, "physics", old).length, 0); assert.equal(C.references(docs, "physics", "Renamed").length, refs);
  assert.deepEqual(C.validate(docs), []);
});
test("asset ID rename updates the corresponding fragment set", () => {
  const docs = C.clone(original); const index = docs.assets.objects.findIndex(e => e.assetID === docs.fragments.fragmentData[0].assetID);
  C.rename(docs, "assets", index, "NewBuilding"); assert.equal(docs.fragments.fragmentData[0].assetID, "NewBuilding"); assert.deepEqual(C.validate(docs), []);
});
test("duplicate-ID rename fails without mutation", () => {
  const docs = C.clone(original), before = C.clone(docs);
  assert.throws(() => C.rename(docs, "physics", 0, docs.physics.physics[1].presetID)); assert.deepEqual(docs, before);
});
test("snapshot restore can restore linked documents", () => {
  const docs = C.clone(original), before = C.clone(docs); C.rename(docs, "physics", 0, "Changed");
  const restored = C.clone(before); assert.deepEqual(restored, original); assert.deepEqual(C.validate(restored), []);
});
test("missing dependencies are warnings when not loaded, errors when loaded", () => {
  assert(C.validate({ assets: original.assets }).some(i => i.level === "warning"));
  const docs = C.clone(original); docs.assets.objects[0].physicsPresetID = "missing";
  assert(C.validate(docs).some(i => i.path === "physicsPresetID" && i.level === "error"));
});
test("multiple colliders, NONE, and zero primitive sizes are blocked", () => {
  for (const colliders of [[C.collider(), C.collider()], [{ ...C.collider(), type: "NONE" }], [{ ...C.collider(), size: [0, 1, 1] }]]) {
    const data = { physics: [C.newEntry("physics", "test")] }; data.physics[0].colliders = colliders;
    assert(C.validate({ physics: data }).some(i => i.level === "error"));
  }
});
test("non-finite, null, invalid scale and duplicate material slots are blocked", () => {
  const docs = C.clone(original); docs.physics.physics[0].rigidBody.mass = null;
  docs.fragments.fragmentData[0].fragments[0].scale = [1, 0, 1];
  docs.assets.objects[0].materials.push(C.clone(docs.assets.objects[0].materials[0]));
  assert(C.validate(docs).some(i => i.path === "rigidBody.mass"));
  assert(C.validate(docs).some(i => i.path.endsWith(".scale")));
  assert(C.validate(docs).some(i => i.path.endsWith(".slot")));
  assert.throws(() => C.parse('{"physics":[],"extra":1e400}'));
});
test("malformed containers and unsupported map layout are rejected", () => {
  for (const value of [null, [], { physics: {} }, { physics: [null] }, { objects: [{ assetID: "bad" }] }, { physics: [], objects: [] }, { fragmentData: [{ assetID: "bad", fragments: [5] }] }]) assert.throws(() => C.parse(JSON.stringify(value)));
});
test("duplicate IDs and unsupported enums are reported", () => {
  const docs = C.clone(original); docs.physics.physics.push(C.clone(docs.physics.physics[0])); docs.physics.physics[0].collisionCategory = "UNKNOWN";
  assert(C.validate(docs).some(i => i.path === "presetID")); assert(C.validate(docs).some(i => i.path === "collisionCategory"));
});
test("new IDs are unique and BOM is supported", () => {
  assert.notEqual(C.uniqueID("assets", original.assets, original.assets.objects[0].assetID), original.assets.objects[0].assetID);
  assert.deepEqual(C.parse("\uFEFF" + JSON.stringify(original.assets)).data, original.assets);
});
console.log(`${count} tests passed.`);
