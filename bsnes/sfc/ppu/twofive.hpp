struct TwoFiveD {
  auto power() -> void {
    io.enable = false;
    io.overridePriority = false;
    io.clampDepth = true;
    io.farDepth = 0xffff;
    for(auto index : range(4)) {
      io.bg[index].base = 0;
      io.bg[index].paletteScale = 0;
      io.bg[index].priorityScale = 0x10;
    }
    io.obj.base = 0;
    io.obj.paletteScale = 0;
    io.obj.priorityScale = 0x10;

    output.lineA = nullptr;
    output.lineB = nullptr;
    memory::fill<uint16>(output.buffer, io.farDepth);
  }

  auto serialize(serializer& s) -> void {
    s.integer(io.enable);
    s.integer(io.overridePriority);
    s.integer(io.clampDepth);
    s.integer(io.farDepth);
    for(auto& layer : io.bg) {
      s.integer(layer.base);
      s.integer(layer.paletteScale);
      s.integer(layer.priorityScale);
    }
    s.integer(io.obj.base);
    s.integer(io.obj.paletteScale);
    s.integer(io.obj.priorityScale);
  }

  auto readIO(uint16 address) -> uint8 {
    switch(address) {
    case 0x21c0: return (uint8)io.enable << 0 | (uint8)io.overridePriority << 1 | (uint8)io.clampDepth << 2;
    case 0x21c1: return io.farDepth >> 0;
    case 0x21c2: return io.farDepth >> 8;
    }

    if(address >= 0x21c4 && address <= 0x21d7) {
      uint index = (address - 0x21c4) >> 2;
      uint offset = (address - 0x21c4) & 3;
      const Layer& layer = index < 4 ? io.bg[index] : io.obj;
      switch(offset) {
      case 0: return layer.base >> 0;  //low byte
      case 1: return layer.base >> 8;  //high byte
      case 2: return layer.paletteScale;
      case 3: return layer.priorityScale;
      }
    }

    return 0x00;
  }

  auto writeIO(uint16 address, uint8 data) -> void {
    switch(address) {
    case 0x21c0:
      io.enable = data & 1;
      io.overridePriority = data >> 1 & 1;
      io.clampDepth = data >> 2 & 1;
      return;
    case 0x21c1:
      io.farDepth = io.farDepth & 0xff00 | data << 0;
      return;
    case 0x21c2:
      io.farDepth = io.farDepth & 0x00ff | data << 8;
      return;
    }

    if(address >= 0x21c4 && address <= 0x21d7) {
      uint index = (address - 0x21c4) >> 2;
      uint offset = (address - 0x21c4) & 3;
      Layer& layer = index < 4 ? io.bg[index] : io.obj;
      switch(offset) {
      case 0: layer.base = layer.base & 0xff00 | data << 0; break;
      case 1: layer.base = layer.base & 0x00ff | data << 8; break;
      case 2: layer.paletteScale = data; break;
      case 3: layer.priorityScale = data; break;
      }
      return;
    }
  }

  auto defaultDepth() const -> uint16 { return io.farDepth; }

  auto depthForBackground(uint layer, uint priority, uint color) const -> uint16 {
    if(!io.enable) return io.farDepth;
    layer &= 3;
    const auto& config = io.bg[layer];
    uint32 depth = config.base;
    depth += (uint32)config.priorityScale * priority;
    depth += (uint32)config.paletteScale * color;
    return clamp(depth);
  }

  auto depthForObject(uint priority, uint color) const -> uint16 {
    if(!io.enable) return io.farDepth;
    uint32 depth = io.obj.base;
    depth += (uint32)io.obj.priorityScale * priority;
    depth += (uint32)io.obj.paletteScale * color;
    return clamp(depth);
  }

  auto beginScanline(uint y, bool interlace, bool field) -> void {
    auto offset = y * 1024u;
    output.lineA = output.buffer + offset;
    output.lineB = output.lineA + (interlace ? 0 : 512);
    if(interlace && field) output.lineA += 512, output.lineB += 512;
  }

  auto write(uint16 belowDepth, bool belowVisible, uint16 aboveDepth, bool aboveVisible, uint16 frontDepth, bool hires) -> void {
    if(!output.lineA || !output.lineB) return;

    uint16 first = hires ? (belowVisible ? belowDepth : io.farDepth) : frontDepth;
    uint16 second = hires ? (aboveVisible ? aboveDepth : io.farDepth) : frontDepth;

    if(!io.enable) first = second = io.farDepth;

    *output.lineA++ = first;
    *output.lineB++ = first;
    *output.lineA++ = second;
    *output.lineB++ = second;
  }

  auto frontDepth(uint16 aboveDepth, bool aboveEnable, uint16 belowDepth, bool belowEnable) const -> uint16 {
    if(!io.enable) return io.farDepth;
    if(aboveEnable) return aboveDepth;
    if(belowEnable) return belowDepth;
    return io.farDepth;
  }

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
    return (uint16)std::clamp<uint32>(value, 0u, 0xffffu);
  }

  friend class PPU;
};
