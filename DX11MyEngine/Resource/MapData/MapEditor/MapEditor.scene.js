/* Geometry helpers shared by terrain placement and camera framing. */
((root, factory) => {
  if (typeof module === "object" && module.exports) module.exports = factory;
  else root.createMapEditorSceneTools = factory;
})(typeof globalThis === "object" ? globalThis : this, T => {
  "use strict";
  function boundsOf(roots, precise = false) {
    const bounds = new T.Box3();
    for (const root of roots) {
      root.updateWorldMatrix(true, true);
      bounds.union(new T.Box3().setFromObject(root, precise));
    }
    return bounds;
  }
  function focusFrame(bounds, camera, aspect) {
    if (bounds.isEmpty()) return null;
    const center = bounds.getCenter(new T.Vector3());
    const radius = Math.max(bounds.getSize(new T.Vector3()).length() / 2, 1e-7);
    if (![...center.toArray(), radius, aspect].every(Number.isFinite) || aspect <= 0) return null;
    const height = radius * 2 * 1.2 / Math.min(1, aspect);
    let distance = Math.max(radius * 3, 1e-6);
    if (camera.isPerspectiveCamera) {
      const vertical = 2 * Math.atan(Math.tan(camera.fov * Math.PI / 360) / camera.zoom);
      const horizontal = 2 * Math.atan(Math.tan(vertical / 2) * aspect);
      distance = radius / Math.sin(Math.min(vertical, horizontal) / 2) * 1.2;
    }
    return { center, radius, height, distance, near: Math.max(radius / 10000, 1e-10), far: Math.max(distance * 100, radius * 100, 1) };
  }
  function visibleInHierarchy(object) {
    for (let node = object; node; node = node.parent) if (!node.visible) return false;
    return true;
  }
  function rectangleSelection(roots, camera, rect) {
    camera.updateMatrixWorld(true);
    const e = camera.projectionMatrix.clone().multiply(camera.matrixWorldInverse).elements;
    const row = index => [e[index], e[index + 4], e[index + 8], e[index + 12]];
    const x = row(0), y = row(1), z = row(2), w = row(3);
    const plane = (a, factor, b) => a.map((value, i) => value - factor * b[i]);
    // Selection sub-frustum in homogeneous clip space, including near/far.
    // Positive-vertex AABB tests work for perspective and orthographic cameras
    // and reject objects behind the camera without dividing by clip W.
    const planes = [plane(x, rect.left, w), plane(w.map(value => value * rect.right), 1, x), plane(y, rect.bottom, w), plane(w.map(value => value * rect.top), 1, y), plane(z, -1, w), plane(w, 1, z)];
    return roots.filter(root => {
      if (!visibleInHierarchy(root)) return false;
      const box = boundsOf([root]);
      if (box.isEmpty()) return false;
      return planes.every(([a, b, c, d]) => a * (a >= 0 ? box.max.x : box.min.x) + b * (b >= 0 ? box.max.y : box.min.y) + c * (c >= 0 ? box.max.z : box.min.z) + d >= 0);
    });
  }
  function groundY(root, groundRoots) {
    const grounds = groundRoots.filter(ground => ground !== root && visibleInHierarchy(ground));
    if (!grounds.length) return null;
    const objectBounds = boundsOf([root], true), terrainBounds = boundsOf(grounds);
    if (objectBounds.isEmpty() || terrainBounds.isEmpty()) return null;
    const extent = Math.max(objectBounds.max.y - objectBounds.min.y, terrainBounds.max.y - terrainBounds.min.y, 1);
    const originY = Math.max(objectBounds.max.y, terrainBounds.max.y) + extent * 0.01 + 1;
    const ray = new T.Raycaster(new T.Vector3(), new T.Vector3(0, -1, 0), 0, originY - terrainBounds.min.y + 1);
    const xs = [objectBounds.min.x, (objectBounds.min.x + objectBounds.max.x) / 2, objectBounds.max.x];
    const zs = [objectBounds.min.z, (objectBounds.min.z + objectBounds.max.z) / 2, objectBounds.max.z];
    const materials = new Map();
    let height = -Infinity;
    try {
      // Intersect both sides of terrain triangles, independent of render winding.
      // This is synchronous; restore shared materials before the next render.
      for (const ground of grounds) ground.traverse(node => {
        for (const material of node.material ? Array.isArray(node.material) ? node.material : [node.material] : []) {
          if (!materials.has(material)) { materials.set(material, material.side); material.side = 2; }
        }
      });
      for (const x of xs) for (const z of zs) {
        ray.ray.origin.set(x, originY, z);
        const hit = ray.intersectObjects(grounds, true).find(hit => hit.object.isMesh && visibleInHierarchy(hit.object));
        if (hit && Number.isFinite(hit.point.y)) height = Math.max(height, hit.point.y);
      }
    } finally { for (const [material, side] of materials) material.side = side; }
    return height === -Infinity ? null : root.position.y + height - objectBounds.min.y;
  }
  return { boundsOf, focusFrame, groundY, rectangleSelection };
});
