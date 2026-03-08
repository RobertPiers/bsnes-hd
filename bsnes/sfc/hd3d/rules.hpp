#pragma once

#include "profile.hpp"

namespace SuperFamicom::HD3D {

struct Rules {
  auto apply(Profile&) const -> void;

  bool allowMode7 = false;
  bool preferTileGrouping = true;
};

}
