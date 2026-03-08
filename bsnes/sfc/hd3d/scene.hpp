#pragma once

namespace SuperFamicom::HD3D {

struct TileIR {
  uint16 tileNumber = 0;
  uint8 palette = 0;
  uint8 priority = 0;
  bool hflip = false;
  bool vflip = false;
  int16 x = 0;
  int16 y = 0;
};

struct BackgroundLayerIR {
  uint8 source = 0;
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
  uint8 priority[2] = {};

  vector<TileIR> tiles;
};

struct SpriteIR {
  uint16 x = 0;
  uint8 y = 0;
  uint8 character = 0;
  uint8 palette = 0;
  uint8 priority = 0;
  bool hflip = false;
  bool vflip = false;
};

struct Mode7IR {
  bool enabled = false;
  bool hflip = false;
  bool vflip = false;
  uint8 repeat = 0;
  int16 a = 0;
  int16 b = 0;
  int16 c = 0;
  int16 d = 0;
  int16 x = 0;
  int16 y = 0;
  int16 hoffset = 0;
  int16 voffset = 0;
};

struct FrameIR {
  uint64 frameNumber = 0;
  uint16 width = 256;
  uint16 height = 224;

  vector<BackgroundLayerIR> backgrounds;
  vector<SpriteIR> sprites;
  Mode7IR mode7;
};

auto reset(FrameIR& frame) -> void;
auto findOrCreateBackground(FrameIR& frame, uint8 source) -> BackgroundLayerIR&;

}
