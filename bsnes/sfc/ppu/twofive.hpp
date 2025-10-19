struct TwoFiveD {
  auto power() -> void;
  auto serialize(serializer&) -> void;
  auto readIO(uint16 address) -> uint8;
  auto writeIO(uint16 address, uint8 data) -> void;

  auto defaultDepth() const -> uint16 { return io.farDepth; }
  auto depthForBackground(uint layer, uint priority, uint color) const -> uint16;
  auto depthForObject(uint priority, uint color) const -> uint16;
  auto beginScanline(uint y, bool interlace, bool field) -> void;
  auto write(uint16 depth, bool hires) -> void;
  auto frontDepth(uint16 aboveDepth, bool aboveEnable, uint16 belowDepth, bool belowEnable) const -> uint16;

  struct Layer {
    uint16 base;
    uint8 paletteScale;
    uint8 priorityScale;
  };

  struct IO {
    uint1 enable;
    uint1 overridePriority;
    uint1 clampDepth;
    uint16 farDepth;
    Layer bg[4];
    Layer obj;
  } io;

  struct Output {
    uint16 buffer[512 * 480];
    uint16* lineA = nullptr;
    uint16* lineB = nullptr;
  } output;

  auto clamp(uint32 value) const -> uint16 {
    if(!io.clampDepth) return (uint16)value;
    return value > 0xffff ? 0xffff : (uint16)value;
  }

  friend class PPU;
};
