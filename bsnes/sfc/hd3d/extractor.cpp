#include "extractor.hpp"

namespace SuperFamicom::HD3D {

auto Extractor::beginFrame(uint64 frameNumber, uint width, uint height) -> void {
  reset(staging);
  staging.frameNumber = frameNumber;
  staging.width = width;
  staging.height = height;
}

auto Extractor::captureBackgroundLine(const BackgroundLineSample& input) -> void {
  //v1 scope: only consume BG1 so we can prototype one plane in 3D.
  if(input.source != 0) return;

  auto& layer = findOrCreateBackground(staging, input.source);
  layer.tileMode = input.tileMode;
  layer.screenSize = input.screenSize;
  layer.tileSize = input.tileSize;
  layer.hires = input.hires;
  layer.directColor = input.directColor;
  layer.aboveEnable = input.aboveEnable;
  layer.belowEnable = input.belowEnable;
  layer.tiledataAddress = input.tiledataAddress;
  layer.screenAddress = input.screenAddress;
  layer.hscroll = input.hscroll;
  layer.vscroll = input.vscroll;
  layer.priority[0] = input.priority0;
  layer.priority[1] = input.priority1;

  //TODO: replace scanline marker tiles with real tilemap extraction from VRAM.
  TileIR marker;
  marker.x = (int16)(input.hscroll & 0x1ff);
  marker.y = input.y;
  marker.tileNumber = input.y;
  marker.palette = 0;
  marker.priority = input.priority0;
  layer.tiles.append(marker);
}

auto Extractor::captureMode7(const Mode7IR& mode7) -> void {
  staging.mode7 = mode7;
}

auto Extractor::endFrame() -> void {
  committed = staging;
}

auto Extractor::previousFrame() const -> const FrameIR& {
  return committed;
}

}
