#pragma once

namespace SuperFamicom::HD3D {

struct Profile {
  bool enable = false;
  bool forceBg1Plane = true;
  bool captureSprites = false;
  int planeDepth = 0;
  int cameraHeight = 80;
  int cameraDistance = 180;
};

struct ProfileLoader {
  auto parse(const string& document) -> Profile;
};

}
