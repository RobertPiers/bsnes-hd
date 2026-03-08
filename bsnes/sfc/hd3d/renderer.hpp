#pragma once

#include "scene.hpp"

namespace SuperFamicom::HD3D {

struct Camera {
  float x = 0.0f;
  float y = 80.0f;
  float z = 180.0f;
  float pitch = -25.0f;
  float yaw = 0.0f;
};

struct Renderer {
  auto renderScanline(uint32* output, uint width, uint y, const FrameIR& frame) -> bool;

  auto camera() const -> const Camera&;
  auto setCamera(const Camera&) -> void;

private:
  Camera debugCamera;
};

}
