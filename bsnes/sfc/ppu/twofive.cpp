auto TwoFiveD::power() -> void {
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

auto TwoFiveD::serialize(serializer& s) -> void {
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

auto TwoFiveD::readIO(uint16 address) -> uint8 {
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

auto TwoFiveD::writeIO(uint16 address, uint8 data) -> void {
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

auto TwoFiveD::depthForBackground(uint layer, uint priority, uint color) const -> uint16 {
  if(!io.enable) return io.farDepth;
  layer &= 3;
  const auto& config = io.bg[layer];
  uint32 depth = config.base;
  depth += (uint32)config.priorityScale * priority;
  depth += (uint32)config.paletteScale * color;
  return clamp(depth);
}

auto TwoFiveD::depthForObject(uint priority, uint color) const -> uint16 {
  if(!io.enable) return io.farDepth;
  uint32 depth = io.obj.base;
  depth += (uint32)io.obj.priorityScale * priority;
  depth += (uint32)io.obj.paletteScale * color;
  return clamp(depth);
}

auto TwoFiveD::beginScanline(uint y, bool interlace, bool field) -> void {
  if(!io.enable) {
    output.lineA = nullptr;
    output.lineB = nullptr;
    return;
  }

  auto offset = y * 1024u;
  output.lineA = output.buffer + offset;
  output.lineB = output.lineA + (interlace ? 0 : 512);
  if(interlace && field) output.lineA += 512, output.lineB += 512;
}

auto TwoFiveD::write(uint16 depth, bool hires) -> void {
  if(!io.enable || !output.lineA || !output.lineB) return;
  (void)hires;

  *output.lineA++ = depth;
  *output.lineB++ = depth;
  *output.lineA++ = depth;
  *output.lineB++ = depth;
}

auto TwoFiveD::frontDepth(uint16 aboveDepth, bool aboveEnable, uint16 belowDepth, bool belowEnable) const -> uint16 {
  if(!io.enable) return io.farDepth;
  if(aboveEnable) return aboveDepth;
  if(belowEnable) return belowDepth;
  return io.farDepth;
}
