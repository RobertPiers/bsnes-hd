#include "mesh-builder.hpp"

namespace SuperFamicom::HD3D {

auto MeshBuilder::buildBackgroundPlane(const FrameIR& frame) const -> PlaneMesh {
  PlaneMesh mesh;
  mesh.width = frame.width;
  mesh.height = frame.height;

  if(frame.backgrounds.size() != 0) {
    auto& layer = frame.backgrounds.first();
    mesh.scrollX = layer.hscroll;
    mesh.scrollY = layer.vscroll;
  }

  return mesh;
}

}
