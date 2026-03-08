#include <sfc/sfc.hpp>
#include "hd3d.hpp"

#include "scene.cpp"
#include "extractor.cpp"
#include "renderer.cpp"
#include "profile.cpp"
#include "rules.cpp"
#include "mesh-builder.cpp"

namespace SuperFamicom::HD3D {

static Extractor extractor;
static Renderer renderer;
static MeshBuilder meshBuilder;

auto beginFrame(uint64 frameNumber, uint width, uint height) -> void {
  extractor.beginFrame(frameNumber, width, height);
}

auto endFrame() -> void {
  extractor.endFrame();
}

auto captureBackgroundLine(const BackgroundLineSample& sample) -> void {
  extractor.captureBackgroundLine(sample);
}

auto renderScanline(uint32* output, uint width, uint y) -> bool {
  auto& frame = extractor.previousFrame();
  if(frame.backgrounds.size() == 0) return false;

  //TODO: meshBuilder output is currently not consumed by the debug software renderer.
  (void)meshBuilder.buildBackgroundPlane(frame);

  return renderer.renderScanline(output, width, y, frame);
}

}
