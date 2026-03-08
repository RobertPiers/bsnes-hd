#include "profile.hpp"

namespace SuperFamicom::HD3D {

auto ProfileLoader::parse(const string& document) -> Profile {
  Profile profile;

  //Minimal key:value profile parser for early experimentation.
  for(auto line : document.split("\n")) {
    line = line.strip();
    if(!line || line.beginsWith("#") || line.beginsWith("%")) continue;

    auto pair = line.split(":", 1L);
    if(pair.size() != 2) continue;

    auto key = pair[0].strip().downcase();
    auto value = pair[1].strip();

    if(key == "enable") profile.enable = value.integer() != 0;
    if(key == "force_bg1_plane") profile.forceBg1Plane = value.integer() != 0;
    if(key == "capture_sprites") profile.captureSprites = value.integer() != 0;
    if(key == "plane_depth") profile.planeDepth = value.integer();
    if(key == "camera_height") profile.cameraHeight = value.integer();
    if(key == "camera_distance") profile.cameraDistance = value.integer();
  }

  return profile;
}

}
