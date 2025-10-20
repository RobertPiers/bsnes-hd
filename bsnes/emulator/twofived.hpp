#pragma once

#include <nall/memory.hpp>
#include <nall/stdint.hpp>
#include <nall/vector.hpp>

#include <cmath>

namespace Emulator::TwoFiveD {

struct ReprojectionBuffers {
  vector<uint32_t> color;
  vector<uint16_t> depth;
};

inline auto reproject(
    const uint32_t* source, uint sourcePitch,
    uint width, uint height,
    const uint16_t* depth, uint depthPitch,
    uint16_t farDepth,
    ReprojectionBuffers& buffers,
    uint& outputPitch
) -> const uint32_t* {
  if(!source || !depth) return nullptr;
  if(width == 0 || height == 0) return nullptr;

  auto pixelCount = width * height;
  buffers.color.resize(pixelCount);
  buffers.depth.resize(pixelCount);

  auto* destColor = buffers.color.data();
  auto* destDepth = buffers.depth.data();

  memory::fill<uint32_t>(destColor, pixelCount, 0u);
  memory::fill<uint16_t>(destDepth, pixelCount, farDepth);

  float invRange = farDepth ? 1.0f / (float)farDepth : 0.0f;
  constexpr float heightScale = 24.0f;
  constexpr float spreadScale = 0.5f;
  float centerX = width > 1 ? (float)(width - 1) * 0.5f : 0.0f;

  for(uint y = 0; y < height; y++) {
    const uint32_t* sourceRow = source + y * sourcePitch;
    const uint16_t* depthRow = depth + y * depthPitch;

    for(uint x = 0; x < width; x++) {
      uint16_t z = depthRow[x];
      int32 diff = (int32)farDepth - (int32)z;
      if(diff < 0) diff = 0;

      float normalized = diff * invRange;
      if(normalized < 0.0f) normalized = 0.0f;

      float heightPixels = normalized * heightScale;
      float destYf = (float)y - heightPixels;

      float destXf = (float)x;
      if(centerX > 0.0f) {
        float centerOffset = ((float)x - centerX) / centerX;
        destXf += centerOffset * heightPixels * spreadScale;
      }

      int destXi = (int)std::lround(destXf);
      int destYi = (int)std::lround(destYf);

      if(destXi < 0) destXi = 0;
      if(destXi >= (int)width) destXi = width - 1;
      if(destYi < 0) destYi = 0;
      if(destYi >= (int)height) destYi = height - 1;

      size_t destIndex = (size_t)destYi * width + destXi;
      if(z <= destDepth[destIndex]) {
        destDepth[destIndex] = z;
        destColor[destIndex] = sourceRow[x];
      }
    }
  }

  outputPitch = width;
  return destColor;
}

}  // namespace Emulator::TwoFiveD
