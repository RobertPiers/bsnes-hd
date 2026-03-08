#include "rules.hpp"

namespace SuperFamicom::HD3D {

auto Rules::apply(Profile& profile) const -> void {
  if(!allowMode7) {
    //TODO: once Mode7 IR support exists, downgrade mode7-specific profile entries here.
  }
  if(!preferTileGrouping) {
    //TODO: add extraction mode switch for per-tile meshes.
  }

  if(profile.cameraHeight < 16) profile.cameraHeight = 16;
  if(profile.cameraDistance < 32) profile.cameraDistance = 32;
}

}
