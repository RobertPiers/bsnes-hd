#pragma once

#include "scene.hpp"

namespace SuperFamicom::HD3D {

struct BackgroundLineSample {
  uint8 source = 0;
  uint8 y = 0;
  uint8 tileMode = 0;
  uint8 screenSize = 0;
  bool tileSize = false;
  bool hires = false;
  bool directColor = false;
  bool aboveEnable = false;
  bool belowEnable = false;
  uint16 tiledataAddress = 0;
  uint16 screenAddress = 0;
  uint16 hscroll = 0;
  uint16 vscroll = 0;
  uint8 priority0 = 0;
  uint8 priority1 = 0;
};

struct Extractor {
  auto beginFrame(uint64 frameNumber, uint width, uint height) -> void;
  auto captureBackgroundLine(const BackgroundLineSample&) -> void;
  auto captureMode7(const Mode7IR&) -> void;
  auto endFrame() -> void;

  auto previousFrame() const -> const FrameIR&;

private:
  FrameIR staging;
  FrameIR committed;
};

}
