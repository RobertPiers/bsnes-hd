#pragma once

#include "extractor.hpp"
#include "mesh-builder.hpp"
#include "renderer.hpp"
#include "rules.hpp"

namespace SuperFamicom::HD3D {

auto beginFrame(uint64 frameNumber, uint width, uint height) -> void;
auto endFrame() -> void;
auto captureBackgroundLine(const BackgroundLineSample&) -> void;
auto renderScanline(uint32* output, uint width, uint y) -> bool;

}
