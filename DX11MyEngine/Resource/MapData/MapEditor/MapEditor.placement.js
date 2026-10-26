/* Pure placement calculations. Preview and scene mutation live in the editor. */
((root, factory) => {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.MapEditorPlacement = api;
})(typeof globalThis === "object" ? globalThis : this, () => {
  "use strict";
  function axisIndex(axis) {
    const index = ["x", "y", "z"].indexOf(axis);
    if (index < 0) throw new Error("X / Y / Z軸を指定してください。");
    return index;
  }
  function alignmentPlan(items, axis, mode) {
    const index = axisIndex(axis);
    if (items.length < 2) throw new Error("編集可能な対象を2個以上選択してください。");
    const values = items.map(item => item.position[index]);
    const min = Math.min(...values), max = Math.max(...values);
    const target = { min, center: min + (max - min) / 2, max }[mode];
    if (!Number.isFinite(target)) throw new Error("整列方法または座標が不正です。");
    return items.map(item => { const position = [...item.position]; position[index] = target; return { id: item.id, position }; });
  }
  function spacingPlan(items, axis, mode, gap) {
    const index = axisIndex(axis);
    if (items.length < (mode === "ends" ? 3 : 2)) throw new Error(mode === "ends" ? "両端固定には編集可能な対象が3個以上必要です。" : "編集可能な対象を2個以上選択してください。");
    if (!["step", "ends"].includes(mode) || !Number.isFinite(gap) || gap < 0) throw new Error("配置間隔には0以上の有限の数値を指定してください。");
    const sorted = items.map((item, order) => ({ ...item, order })).sort((a, b) => a.position[index] - b.position[index] || a.order - b.order);
    const first = sorted[0].position[index], last = sorted[sorted.length - 1].position[index];
    return sorted.map((item, order) => {
      const position = [...item.position];
      if (mode === "step") position[index] = first + gap * order;
      else if (order !== 0 && order !== sorted.length - 1) position[index] = first + (last - first) * order / (sorted.length - 1);
      if (!position.every(Number.isFinite)) throw new Error("配置後の座標が数値の範囲を超えます。");
      return { id: item.id, position };
    });
  }
  function seededRandom(seed) {
    let state = 2166136261;
    for (const character of String(seed)) state = Math.imul(state ^ character.codePointAt(0), 16777619) >>> 0;
    return () => {
      state = (state + 0x6D2B79F5) >>> 0;
      let value = Math.imul(state ^ state >>> 15, 1 | state);
      value ^= value + Math.imul(value ^ value >>> 7, 61 | value);
      return ((value ^ value >>> 14) >>> 0) / 4294967296;
    };
  }
  function validateScatter(config) {
    if (!Number.isInteger(config.count) || config.count < 1 || config.count > 2000) throw new Error("配置数は1〜2000の整数で指定してください。");
    const numeric = ["centerX", "centerY", "centerZ", "radius", "width", "depth", "minDistance", "yawMin", "yawMax", "scaleMin", "scaleMax"];
    if (numeric.some(key => !Number.isFinite(config[key]))) throw new Error("配置設定には有限の数値を指定してください。");
    if (!["circle", "rectangle"].includes(config.shape) || config.radius <= 0 || config.width <= 0 || config.depth <= 0 || config.minDistance < 0) throw new Error("範囲の大きさは正の数値、最小間隔は0以上にしてください。");
    if (config.yawMin > config.yawMax || !Number.isFinite(config.yawMax - config.yawMin) || config.scaleMin < 0.001 || config.scaleMin > config.scaleMax) throw new Error("角度・スケールの最小値と最大値を確認してください（スケールは0.001以上）。");
    if (![config.centerX + config.radius, config.centerX - config.radius, config.centerX - config.width / 2, config.centerX + config.width / 2, config.centerZ + config.radius, config.centerZ - config.radius, config.centerZ - config.depth / 2, config.centerZ + config.depth / 2].every(Number.isFinite)) throw new Error("配置範囲が数値の上限を超えます。");
    return config;
  }
  function createScatter(config, existing = []) {
    validateScatter(config);
    const random = seededRandom(config.seed), cells = new Map(), distance = config.minDistance;
    let attempts = 0, placed = 0;
    const limit = Math.min(100000, Math.max(100, config.count * 50));
    const cellKey = (x, z) => `${x},${z}`;
    function add(position) {
      if (!distance) return;
      const key = cellKey(Math.floor(position[0] / distance), Math.floor(position[2] / distance));
      if (!cells.has(key)) cells.set(key, []);
      cells.get(key).push([position[0], position[2]]);
    }
    existing.forEach(add);
    function free(position) {
      if (!distance) return true;
      const x = Math.floor(position[0] / distance), z = Math.floor(position[2] / distance);
      for (let dx = -1; dx <= 1; dx++) for (let dz = -1; dz <= 1; dz++) {
        for (const point of cells.get(cellKey(x + dx, z + dz)) || []) if (Math.hypot(position[0] - point[0], position[2] - point[1]) < distance) return false;
      }
      return true;
    }
    return {
      get attempts() { return attempts; }, get placed() { return placed; }, get limit() { return limit; },
      next() {
        if (placed >= config.count || attempts >= limit) return { done: true };
        attempts++;
        const u = random(), v = random(), yaw = random(), size = random();
        const r = Math.sqrt(u) * config.radius, angle = v * Math.PI * 2;
        const position = config.shape === "circle" ? [config.centerX + r * Math.cos(angle), config.centerY, config.centerZ + r * Math.sin(angle)] : [config.centerX + (u - 0.5) * config.width, config.centerY, config.centerZ + (v - 0.5) * config.depth];
        if (!free(position)) return { done: false, candidate: null };
        const scale = config.randomScale ? config.scaleMin + (config.scaleMax - config.scaleMin) * size : 1;
        return { done: false, candidate: { position, rotation: [0, config.randomYaw ? (config.yawMin + (config.yawMax - config.yawMin) * yaw) * (Math.PI / 180) : 0, 0, "YXZ"], scale: [scale, scale, scale] } };
      },
      accept(candidate) { add(candidate.position); placed++; },
    };
  }
  return { alignmentPlan, spacingPlan, seededRandom, validateScatter, createScatter };
});
