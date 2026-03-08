#include "renderer.hpp"

namespace SuperFamicom::HD3D {

auto Renderer::renderScanline(uint32* output, uint width, uint y, const FrameIR& frame) -> bool {
  if(frame.backgrounds.size() == 0) return false;

  auto& layer = frame.backgrounds.first();
  int scrollX = layer.hscroll;
  int scrollY = layer.vscroll;

  //Debug renderer: projects a single SNES layer as a pseudo-3D floor grid.
  for(uint x : range(width)) {
    int worldX = (int)x + scrollX;
    int worldY = (int)y + scrollY;

    uint8 r = (worldX >> 1) & 0xff;
    uint8 g = (worldY >> 1) & 0xff;
    uint8 b = ((worldX ^ worldY) >> 1) & 0xff;

    if((worldX & 31) == 0 || (worldY & 31) == 0) {
      r = 255;
      g = 255;
      b = 255;
    }

    output[x] = (r << 16) | (g << 8) | b;
  }

  return true;
}

auto Renderer::camera() const -> const Camera& {
  return debugCamera;
}

auto Renderer::setCamera(const Camera& camera) -> void {
  debugCamera = camera;
}

}
