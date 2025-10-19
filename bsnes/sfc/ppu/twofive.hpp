struct TwoFiveD {
  struct Settings {
    bool enable = false;
    bool overridePriority = false;
    bool clampDepth = true;
    uint farDepth = 0xffff;
    struct Layer {
      uint base = 0;
      uint paletteScale = 0;
      uint priorityScale = 0x10;
    } bg[4], obj;
  };

  template<typename Source> static auto makeSettings(const Source& source) -> Settings {
    Settings settings;
    settings.enable = source.enable;
    settings.overridePriority = source.overridePriority;
    settings.clampDepth = source.clampDepth;
    settings.farDepth = source.farDepth;
    for(uint index : range(4)) {
      settings.bg[index].base = source.bg[index].base;
      settings.bg[index].paletteScale = source.bg[index].paletteScale;
      settings.bg[index].priorityScale = source.bg[index].priorityScale;
    }
    settings.obj.base = source.obj.base;
    settings.obj.paletteScale = source.obj.paletteScale;
    settings.obj.priorityScale = source.obj.priorityScale;
    return settings;
  }

  auto power(const Settings& settings) -> void {
    configure(settings);
    output.lineA = nullptr;
    output.lineB = nullptr;
    memory::fill<uint16>(output.buffer, io.farDepth);
  }

  auto configure(const Settings& settings) -> void {
    io.enable = settings.enable;
    io.overridePriority = settings.overridePriority;
    io.clampDepth = settings.clampDepth;

    auto clamp16 = [](uint value) -> uint16 { return (uint16)std::min(value, 0xffffu); };
    auto clamp8  = [](uint value) -> uint8 { return (uint8)std::min(value, 0xffu); };

    uint16 farDepth = clamp16(settings.farDepth);
    bool farChanged = io.farDepth != farDepth;
    io.farDepth = farDepth;

    for(uint index : range(4)) {
      io.bg[index].base = clamp16(settings.bg[index].base);
      io.bg[index].paletteScale = clamp8(settings.bg[index].paletteScale);
      io.bg[index].priorityScale = clamp8(settings.bg[index].priorityScale);
    }

    io.obj.base = clamp16(settings.obj.base);
    io.obj.paletteScale = clamp8(settings.obj.paletteScale);
    io.obj.priorityScale = clamp8(settings.obj.priorityScale);

    if(farChanged || !io.enable) {
      memory::fill<uint16>(output.buffer, io.farDepth);
    }
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
