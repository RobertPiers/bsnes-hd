#pragma once

#include "scene.hpp"

namespace SuperFamicom::HD3D {

struct PlaneMesh {
  int width = 0;
  int height = 0;
  int scrollX = 0;
  int scrollY = 0;
};

struct MeshBuilder {
  auto buildBackgroundPlane(const FrameIR&) const -> PlaneMesh;
};

}
