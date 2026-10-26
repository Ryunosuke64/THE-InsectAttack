/* Left-button click/marquee gestures; gizmos keep priority. No scene mutation. */
window.createMapEditorSelection = api => {
  "use strict";
  const { canvas, viewport, overlay } = api;
  let gesture = null;
  function point(event) {
    const rect = canvas.getBoundingClientRect();
    return { x: Math.max(0, Math.min(rect.width, event.clientX - rect.left)), y: Math.max(0, Math.min(rect.height, event.clientY - rect.top)) };
  }
  function rectangle(start, end) {
    return { left: Math.min(start.x, end.x), top: Math.min(start.y, end.y), right: Math.max(start.x, end.x), bottom: Math.max(start.y, end.y) };
  }
  function finish(event, cancelled = false) {
    if (!gesture) return false;
    const state = gesture; gesture = null;
    overlay.hidden = true; document.body.classList.remove("marquee-selecting");
    if (canvas.hasPointerCapture(state.id)) canvas.releasePointerCapture(state.id);
    api.interactionChanged(false);
    if (!cancelled && event) {
      if (state.dragging) api.selectRectangle(rectangle(state.start, point(event)), state.modifiers, state.ids);
      else api.click(event);
    }
    return true;
  }
  canvas.addEventListener("pointerdown", event => {
    if (gesture) { if (event.button !== 0) finish(null, true); return; }
    if (event.button !== 0 || event.pointerType === "touch" || event.isPrimary === false || event.buttons !== 1) return;
    viewport.focus({ preventScroll: true });
    if (!api.canStart() || api.isGizmo()) return;
    gesture = { id: event.pointerId, start: point(event), dragging: false, ids: api.selectedIDs(), modifiers: { ctrlKey: event.ctrlKey || event.metaKey, shiftKey: event.shiftKey } };
    canvas.setPointerCapture(event.pointerId); api.interactionChanged(true);
  });
  document.addEventListener("pointermove", event => {
    if (!gesture || event.pointerId !== gesture.id) return;
    if (!(event.buttons & 1)) { finish(null, true); return; }
    const end = point(event);
    if (!gesture.dragging && Math.hypot(end.x - gesture.start.x, end.y - gesture.start.y) < 5) return;
    gesture.dragging = true; event.preventDefault(); document.body.classList.add("marquee-selecting");
    const rect = rectangle(gesture.start, end);
    overlay.hidden = false;
    Object.assign(overlay.style, { left: `${rect.left}px`, top: `${rect.top}px`, width: `${rect.right - rect.left}px`, height: `${rect.bottom - rect.top}px` });
  }, { passive: false });
  document.addEventListener("pointerup", event => { if (gesture?.id === event.pointerId && event.button === 0) finish(event); });
  for (const type of ["pointercancel", "lostpointercapture"]) canvas.addEventListener(type, event => { if (gesture?.id === event.pointerId) finish(null, true); });
  document.addEventListener("keydown", event => {
    if (!gesture || event.key !== "Escape") return;
    event.preventDefault(); event.stopPropagation(); finish(null, true);
  }, true);
  window.addEventListener("blur", () => finish(null, true));
  new ResizeObserver(() => finish(null, true)).observe(canvas);
  return { cancel: () => finish(null, true), get interacting() { return !!gesture; } };
};
