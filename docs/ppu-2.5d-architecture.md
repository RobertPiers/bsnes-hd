# PPU 2.5D Architecture

The 2.5D pipeline augments the classic Super Famicom pixel priority rules with a 16-bit depth buffer that can be consumed by the HD renderer.  It exposes programmable parameters so that homebrew software and ROM hacks can express simple height-map style effects without replacing the original rendering hardware.

The feature set implemented by this change set comprises the following pieces:

* A dedicated 16-bit depth buffer with the same 512×480 layout as the colour buffer.  The buffer is reset every time the PPU powers on and is populated while the slow (cycle-accurate) PPU renders each pixel.  The fast PPU remains unchanged and therefore does not emit depth information.
* Per-layer depth configuration registers that allow software to derive depth from the layer priority and from the palette index.  Each layer exposes independent base depth, palette scale and priority scale controls.
* Optional depth-aware compositing.  When the feature is enabled, ties between layers with the same SNES priority are broken using depth, and an optional override flag lets foreground elements overtake higher-priority layers if they are closer to the camera.  When the feature is disabled, the original priority-only behaviour is preserved.
* A programmable "far plane" depth value that is written when no layer produces a visible pixel.  This matches the default depth returned for transparent pixels and lets tooling interpret untouched regions reliably.

## Register Map

All registers are write/read accessible through the $21C0–$21D7 range.  Reads return the last value written so tools can query the current configuration.

| Address | Name        | Description |
|---------|-------------|-------------|
| $21C0   | P25DCTRL    | Bit 0 – enable depth buffering; bit 1 – allow nearer pixels to override higher SNES priorities; bit 2 – clamp arithmetic to 16 bits. |
| $21C1   | P25DFARL    | Low byte of the far plane depth value written for transparent pixels. |
| $21C2   | P25DFARH    | High byte of the far plane depth value. |
| $21C4–$21C7 | P25DBG1* | BG1 base depth (low/high), palette scale and priority scale. |
| $21C8–$21CB | P25DBG2* | BG2 configuration fields, ordered as BG1. |
| $21CC–$21CF | P25DBG3* | BG3 configuration fields, ordered as BG1. |
| $21D0–$21D3 | P25DBG4* | BG4 configuration fields, ordered as BG1. |
| $21D4–$21D7 | P25DOBJ* | Object layer configuration fields, ordered as BG1. |

The “*” rows share the same layout: the first byte sets the low 8 bits of the base depth, the second byte sets the upper 8 bits, the third byte sets the palette-derived depth scale, and the fourth byte sets the priority-derived depth scale.

## Depth Equation

Whenever depth is enabled the renderer computes the depth for each non-transparent pixel as:

```
depth = base + palette_scale × palette_index + priority_scale × priority
```

*Palette index* is the colour number delivered by the layer (0–255 for backgrounds, 0–31 for sprites).  *Priority* uses the already expanded SNES priority value (1–12 for backgrounds, 1–15 for sprites).  Arithmetic is 32-bit internally and optionally clamped to 16 bits when the clamp flag is asserted; without clamping the results wrap.

Transparent pixels keep the configurable far plane value and therefore never influence the compositor.  When depth-aware compositing is disabled the renderer writes the far plane for every pixel and layer ordering is identical to upstream bsnes-hd.

## Buffer Access

The buffer is exposed through the `PPU::depthBuffer()` helper which returns a pointer to the 512×480 array alongside a fixed 1024 element pitch.  The helper always returns the storage address so front-ends can sample the last depth image even if depth writes were disabled mid-frame.  Callers are expected to gate their logic with `PPU::depthBufferEnabled()` to avoid interpreting stale data.

> **Note:** The fast PPU implementation currently ignores the new registers.  Depth data is therefore only available when the cycle-accurate renderer is active.
