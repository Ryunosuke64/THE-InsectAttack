/* View-only materials. Restore imported materials before picking/terrain queries. */
window.createMapEditorAppearance = () => {
  "use strict";
  let mode = "material", baseColor = "#b8bbc1";
  const colors = new WeakMap(), caches = new Map();
  const assets = new WeakMap(), assetColors = new Map();
  const colorOf = root => colors.get(root) || assetColors.get(assets.get(root)) || baseColor;
  function release(root) {
    const cache = caches.get(root);
    if (cache) { for (const material of cache.values()) material.dispose(); caches.delete(root); }
    colors.delete(root); assets.delete(root);
  }
  function render(renderer, scene, camera, roots) {
    if (mode === "material") { renderer.render(scene, camera); return; }
    const restored = [];
    try {
      for (const root of roots) {
        if (!root.visible) continue;
        let cache = caches.get(root);
        const color = colorOf(root);
        const override = original => {
          if (!original) return original;
          if (!cache) { cache = new Map(); caches.set(root, cache); }
          let material = cache.get(original);
          if (!material) {
            material = original.clone();
            // Cloning retains texture references. Detach them without disposing
            // shared source images, including normal/alpha/environment maps.
            for (const [key, value] of Object.entries(material)) if (value?.isTexture) material[key] = null;
            material.vertexColors = false; material.transparent = false; material.opacity = 1;
            material.alphaTest = 0; material.depthWrite = true;
            material.emissive?.set(0); material.specular?.set(0x333333);
            if ("metalness" in material) material.metalness = 0;
            if ("roughness" in material) material.roughness = 1;
            cache.set(original, material);
          }
          const wireframe = mode === "wireframe";
          if (material.wireframe !== wireframe) { material.wireframe = wireframe; material.needsUpdate = true; }
          material.color?.set(color);
          return material;
        };
        root.traverse(node => {
          if (!node.isMesh) return;
          const original = node.material; restored.push([node, original]);
          node.material = Array.isArray(original) ? original.map(override) : override(original);
        });
      }
      renderer.render(scene, camera);
    } finally { for (const [node, material] of restored) node.material = material; }
  }
  return {
    render, release,
    setMode(value) { if (["material", "solid", "wireframe"].includes(value)) mode = value; },
    setBaseColor(value) { baseColor = value; },
    setColor(root, value) { colors.set(root, value); },
    clearColor(root) { colors.delete(root); },
    colorOf,
    setAsset(root, assetID) { assets.set(root, assetID); },
    setAssetColor(assetID, value) { assetColors.set(assetID, value); },
    clearAssetColor(assetID) { assetColors.delete(assetID); },
    hasAssetColor(assetID) { return assetColors.has(assetID); },
    hasColor(root) { return colors.has(root); },
    copyColor(from, to) { if (colors.has(from)) colors.set(to, colors.get(from)); },
    get mode() { return mode; },
  };
};
