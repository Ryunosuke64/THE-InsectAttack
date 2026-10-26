/* Editor state utilities; no renderer, DOM, or external dependencies. */
((root, factory) => {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.MapEditorCore = api;
})(typeof globalThis === "object" ? globalThis : this, () => {
  "use strict";
  class History {
    constructor(limit = 100, onChange = () => {}) {
      this.limit = limit; this.onChange = onChange; this.commands = []; this.cursor = 0;
    }
    get canUndo() { return this.cursor > 0; }
    get canRedo() { return this.cursor < this.commands.length; }
    commit(command, alreadyApplied = false) {
      if (!alreadyApplied) command.redo();
      this.commands.splice(this.cursor);
      this.commands.push(command);
      if (this.commands.length > this.limit) this.commands.shift();
      this.cursor = this.commands.length; this.onChange();
    }
    undo() {
      if (!this.canUndo) return;
      this.commands[this.cursor - 1].undo(); this.cursor--; this.onChange();
    }
    redo() {
      if (!this.canRedo) return;
      this.commands[this.cursor].redo(); this.cursor++; this.onChange();
    }
    clear() { this.commands.length = 0; this.cursor = 0; this.onChange(); }
  }
  class TransformCommand {
    constructor(label, before, after, beforePivot, afterPivot, apply) {
      Object.assign(this, { label, before, after, beforePivot, afterPivot, apply });
    }
    undo() { this.apply(this.before, this.beforePivot); }
    redo() { this.apply(this.after, this.afterPivot); }
  }
  class ObjectsCommand {
    constructor(label, entries, adding, beforeSelection, afterSelection, setPresent, select) {
      Object.assign(this, { label, entries, adding, beforeSelection, afterSelection, setPresent, select });
    }
    undo() { this.setPresent(this.entries, !this.adding); this.select(this.beforeSelection); }
    redo() { this.setPresent(this.entries, this.adding); this.select(this.afterSelection); }
  }
  const equal = (a, b) => JSON.stringify(a) === JSON.stringify(b);
  const filterObjects = (models, query) => {
    const term = query.trim().toLowerCase();
    return models.filter(model => model.assetID.toLowerCase().includes(term));
  };
  const snap = (value, interval) => Math.round(value / interval) * interval;
  // Hold the starting cell until the pointer crosses a grid boundary. Enabling
  // snapping or simply grabbing a gizmo must never move an off-grid object.
  function snapDrag(value, start, interval) {
    const next = snap(value, interval);
    return next === snap(start, interval) ? start : next;
  }
  function validateMap(data) {
    if (!data || typeof data !== "object" || Array.isArray(data) || typeof data.mapName !== "string" || typeof data.groundAssetTag !== "string" || !Array.isArray(data.objects)) {
      throw new Error("配置JSONにはmapName、groundAssetTag、objectsが必要です。");
    }
    data.objects.forEach((object, index) => {
      if (!object || typeof object.assetID !== "string" || !object.assetID.trim()) throw new Error(`objects[${index}]のassetIDが不正です。`);
      for (const key of ["position", "rotation", "scale"]) {
        if (!Array.isArray(object[key]) || object[key].length !== 3 || !object[key].every(Number.isFinite)) throw new Error(`objects[${index}].${key}には有限の数値3個が必要です。`);
      }
    });
    return data;
  }
  // The FBX preview is right-handed; the game imports FBX with Assimp's
  // MakeLeftHanded (Z reflection). Apply this basis change only at JSON I/O.
  // DirectX XMQuaternionRotationRollPitchYaw uses the YXZ Euler composition.
  const rotationOrder = "YXZ";
  const negate = value => value === 0 ? 0 : -value;
  function fromGameTransform(object) {
    return {
      position: [object.position[0], object.position[1], negate(object.position[2])],
      rotation: [negate(object.rotation[0]), negate(object.rotation[1]), object.rotation[2], rotationOrder],
      scale: [...object.scale],
    };
  }
  function toGameTransform(root) {
    const position = root.position.toArray();
    return {
      position: [position[0], position[1], negate(position[2])],
      rotation: [negate(root.rotation.x), negate(root.rotation.y), root.rotation.z],
      scale: root.scale.toArray(),
    };
  }
  function serializeMap(template, models) {
    const result = { ...template, objects: models.map(model => ({
      assetID: model.assetID,
      ...toGameTransform(model.root),
    })) };
    return validateMap(result);
  }
  function jsonFileName(name) {
    const safe = String(name).trim().replace(/[<>:"/\\|?*\x00-\x1f]/g, "_").replace(/[. ]+$/, "") || "Map";
    return /\.json$/i.test(safe) ? safe : `${safe}.json`;
  }
  return { History, TransformCommand, ObjectsCommand, equal, filterObjects, snap, snapDrag, validateMap, serializeMap, rotationOrder, fromGameTransform, toGameTransform, jsonFileName };
});
