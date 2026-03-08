#include "scene.hpp"

namespace SuperFamicom::HD3D {

auto reset(FrameIR& frame) -> void {
  frame.backgrounds.reset();
  frame.sprites.reset();
  frame.mode7 = {};
}

auto findOrCreateBackground(FrameIR& frame, uint8 source) -> BackgroundLayerIR& {
  for(auto& layer : frame.backgrounds) {
    if(layer.source == source) return layer;
  }
  frame.backgrounds.append(BackgroundLayerIR{});
  frame.backgrounds.last().source = source;
  return frame.backgrounds.last();
}

}
