#include <algorithm>
#include <cmath>

auto OpenGL::setShader(const string& pathname) -> void {
  for(auto& program : programs) program.release();
  programs.reset();

  settings.reset();

  format = inputFormat;
  filter = GL_LINEAR;
  wrap = GL_CLAMP_TO_BORDER;
  absoluteWidth = 0, absoluteHeight = 0;
  relativeWidth = 0, relativeHeight = 0;

  uint historySize = 0;
  if(pathname == "None") {
    filter = GL_NEAREST;
  } else if(pathname == "Blur") {
    filter = GL_LINEAR;
  } else if(directory::exists(pathname)) {
    auto document = BML::unserialize(file::read({pathname, "manifest.bml"}));

    for(auto node : document["settings"]) {
      settings.insert({node.name(), node.text()});
    }

    for(auto node : document["input"]) {
      if(node.name() == "history") historySize = node.natural();
      if(node.name() == "format") format = glrFormat(node.text());
      if(node.name() == "filter") filter = glrFilter(node.text());
      if(node.name() == "wrap") wrap = glrWrap(node.text());
    }

    for(auto node : document["output"]) {
      string text = node.text();
      if(node.name() == "width") {
        if(text.endsWith("%")) relativeWidth = toReal(text.trimRight("%", 1L)) / 100.0;
        else absoluteWidth = text.natural();
      }
      if(node.name() == "height") {
        if(text.endsWith("%")) relativeHeight = toReal(text.trimRight("%", 1L)) / 100.0;
        else absoluteHeight = text.natural();
      }
    }

    for(auto node : document.find("program")) {
      uint n = programs.size();
      programs(n).bind(this, node, pathname);
    }
  }

  //changing shaders may change input format, which requires the input texture to be recreated
  if(texture) { glDeleteTextures(1, &texture); texture = 0; }
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, getFormat(), getType(), buffer);
  allocateHistory(historySize);
}

auto OpenGL::allocateHistory(uint size) -> void {
  for(auto& frame : history) glDeleteTextures(1, &frame.texture);
  history.reset();
  while(size--) {
    OpenGLTexture frame;
    frame.filter = filter;
    frame.wrap = wrap;
    glGenTextures(1, &frame.texture);
    glBindTexture(GL_TEXTURE_2D, frame.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, frame.width = width, frame.height = height, 0, getFormat(), getType(), buffer);
    history.append(frame);
  }
}

auto OpenGL::clear() -> void {
  for(auto& p : programs) {
    glUseProgram(p.program);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.framebuffer);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  glUseProgram(0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
}

auto OpenGL::lock(uint32_t*& data, uint& pitch) -> bool {
  pitch = width * sizeof(uint32_t);
  return data = buffer;
}

auto OpenGL::output() -> void {
  clear();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, getFormat(), getType(), buffer);

  struct Source {
    GLuint texture;
    uint width, height;
    GLuint filter, wrap;
  };
  vector<Source> sources;
  sources.prepend({texture, width, height, filter, wrap});

  for(auto& p : programs) {
    uint targetWidth = p.absoluteWidth ? p.absoluteWidth : outputWidth;
    uint targetHeight = p.absoluteHeight ? p.absoluteHeight : outputHeight;
    if(p.relativeWidth) targetWidth = sources[0].width * p.relativeWidth;
    if(p.relativeHeight) targetHeight = sources[0].height * p.relativeHeight;

    p.size(targetWidth, targetHeight);
    glUseProgram(p.program);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.framebuffer);

    glrUniform1i("phase", p.phase);
    glrUniform1i("historyLength", history.size());
    glrUniform1i("sourceLength", sources.size());
    glrUniform1i("pixmapLength", p.pixmaps.size());
    glrUniform4f("targetSize", targetWidth, targetHeight, 1.0 / targetWidth, 1.0 / targetHeight);
    glrUniform4f("outputSize", outputWidth, outputHeight, 1.0 / outputWidth, 1.0 / outputHeight);

    uint aid = 0;
    for(auto& frame : history) {
      glrUniform1i({"history[", aid, "]"}, aid);
      glrUniform4f({"historySize[", aid, "]"}, frame.width, frame.height, 1.0 / frame.width, 1.0 / frame.height);
      glActiveTexture(GL_TEXTURE0 + (aid++));
      glBindTexture(GL_TEXTURE_2D, frame.texture);
      glrParameters(frame.filter, frame.wrap);
    }

    uint bid = 0;
    for(auto& source : sources) {
      glrUniform1i({"source[", bid, "]"}, aid + bid);
      glrUniform4f({"sourceSize[", bid, "]"}, source.width, source.height, 1.0 / source.width, 1.0 / source.height);
      glActiveTexture(GL_TEXTURE0 + aid + (bid++));
      glBindTexture(GL_TEXTURE_2D, source.texture);
      glrParameters(source.filter, source.wrap);
    }

    uint cid = 0;
    for(auto& pixmap : p.pixmaps) {
      glrUniform1i({"pixmap[", cid, "]"}, aid + bid + cid);
      glrUniform4f({"pixmapSize[", bid, "]"}, pixmap.width, pixmap.height, 1.0 / pixmap.width, 1.0 / pixmap.height);
      glActiveTexture(GL_TEXTURE0 + aid + bid + (cid++));
      glBindTexture(GL_TEXTURE_2D, pixmap.texture);
      glrParameters(pixmap.filter, pixmap.wrap);
    }

    glActiveTexture(GL_TEXTURE0);
    glrParameters(sources[0].filter, sources[0].wrap);
    p.render(sources[0].width, sources[0].height, 0, 0, targetWidth, targetHeight);
    glBindTexture(GL_TEXTURE_2D, p.texture);

    p.phase = (p.phase + 1) % p.modulo;
    sources.prepend({p.texture, p.width, p.height, p.filter, p.wrap});
  }

  uint targetWidth = absoluteWidth ? absoluteWidth : outputWidth;
  uint targetHeight = absoluteHeight ? absoluteHeight : outputHeight;
  if(relativeWidth) targetWidth = sources[0].width * relativeWidth;
  if(relativeHeight) targetHeight = sources[0].height * relativeHeight;

  updateCamera(targetWidth, targetHeight);

  glUseProgram(program);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glrUniform1i("source[0]", 0);
  glrUniform4f("targetSize", targetWidth, targetHeight, 1.0 / targetWidth, 1.0 / targetHeight);
  glrUniform4f("outputSize", outputWidth, outputHeight, 1.0 / outputWidth, 1.0 / outputHeight);
  glrUniform1i("cameraEnabled", camera.enabled);
  glrUniformMatrix4fv("cameraModelViewProjection", camera.matrix);

  glrParameters(sources[0].filter, sources[0].wrap);
  render(sources[0].width, sources[0].height, outputX, outputY, outputWidth, outputHeight);

  if(history.size() > 0) {
    OpenGLTexture frame = history.takeRight();

    glBindTexture(GL_TEXTURE_2D, frame.texture);
    if(width == frame.width && height == frame.height) {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, getFormat(), getType(), buffer);
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, format, frame.width = width, frame.height = height, 0, getFormat(), getType(), buffer);
    }

    history.prepend(frame);
  }
}

auto OpenGL::initialize(const string& shader) -> bool {
  if(!OpenGLBind()) return false;

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_STENCIL_TEST);
  glEnable(GL_DITHER);

  program = glCreateProgram();
  vertex = glrCreateShader(program, GL_VERTEX_SHADER, OpenGLOutputVertexShader);
//geometry = glrCreateShader(program, GL_GEOMETRY_SHADER, OpenGLGeometryShader);
  fragment = glrCreateShader(program, GL_FRAGMENT_SHADER, OpenGLFragmentShader);
  OpenGLSurface::allocate();
  glrLinkProgram(program);

  setShader(shader);
  return initialized = true;
}

auto OpenGL::terminate() -> void {
  if(!initialized) return;
  setShader("");  //release shader resources (eg frame[] history)
  OpenGLSurface::release();
  if(buffer) { delete[] buffer; buffer = nullptr; }
  initialized = false;
}
auto OpenGL::setCamera(const Video::CameraSettings& settings) -> void {
  static constexpr float degrees = 3.14159265358979323846f / 180.0f;
  camera.enabled = settings.enabled;
  camera.yaw = settings.yaw * degrees;
  camera.pitch = settings.pitch * degrees;
  camera.roll = settings.roll * degrees;
  camera.offsetX = settings.offsetX / 100.0f;
  camera.offsetY = settings.offsetY / 100.0f;
  camera.offsetZ = settings.offsetZ / 100.0f;
  camera.zoom = std::max(0.01f, settings.zoom / 100.0f);
  camera.perspective = settings.perspective / 100.0f;
  camera.dirty = true;
}

auto OpenGL::updateCamera(uint targetWidth, uint targetHeight) -> void {
  static constexpr float identity[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  auto setIdentity = [&](float (&matrix)[16]) {
    std::copy(identity, identity + 16, matrix);
  };

  if(!camera.enabled) {
    setIdentity(camera.matrix);
    camera.lastWidth = targetWidth;
    camera.lastHeight = targetHeight;
    camera.dirty = false;
    return;
  }

  if(!camera.dirty && camera.lastWidth == targetWidth && camera.lastHeight == targetHeight) return;

  auto multiply = [&](float (&matrix)[16], const float (&transform)[16]) {
    float result[16];
    MatrixMultiply(result, matrix, 4, 4, transform, 4, 4);
    std::copy(result, result + 16, matrix);
  };

  float model[16];
  setIdentity(model);

  //scale for zoom
  float scale[16] = {
    camera.zoom, 0, 0, 0,
    0, camera.zoom, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };
  multiply(model, scale);

  //rotate around X (pitch)
  if(camera.pitch != 0.0f) {
    float c = std::cos(camera.pitch), s = std::sin(camera.pitch);
    float rotate[16] = {
      1, 0, 0, 0,
      0,  c,  s, 0,
      0, -s,  c, 0,
      0, 0, 0, 1,
    };
    multiply(model, rotate);
  }

  //rotate around Y (yaw)
  if(camera.yaw != 0.0f) {
    float c = std::cos(camera.yaw), s = std::sin(camera.yaw);
    float rotate[16] = {
       c, 0, -s, 0,
       0, 1,  0, 0,
       s, 0,  c, 0,
       0, 0,  0, 1,
    };
    multiply(model, rotate);
  }

  //rotate around Z (roll)
  if(camera.roll != 0.0f) {
    float c = std::cos(camera.roll), s = std::sin(camera.roll);
    float rotate[16] = {
       c,  s, 0, 0,
      -s,  c, 0, 0,
       0,  0, 1, 0,
       0,  0, 0, 1,
    };
    multiply(model, rotate);
  }

  //translation offsets (percent based)
  float outputWidthF = outputWidth ? (float)outputWidth : (float)(targetWidth ? targetWidth : 1);
  float outputHeightF = outputHeight ? (float)outputHeight : (float)(targetHeight ? targetHeight : 1);
  float offsetScaleX = targetWidth ? (float)targetWidth / outputWidthF : 1.0f;
  float offsetScaleY = targetHeight ? (float)targetHeight / outputHeightF : 1.0f;
  float translate[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    camera.offsetX * offsetScaleX, camera.offsetY * offsetScaleY, camera.offsetZ, 1,
  };
  multiply(model, translate);

  //perspective adjustment
  float perspectiveStrength = 0.0f;
  if(camera.perspective > 0.0f) {
    //map [0,+inf) to [0,1) so that perspective never flips the quad
    perspectiveStrength = std::tanh(camera.perspective);

    float planeWidth = targetWidth ? (float)targetWidth : outputWidthF;
    float planeHeight = targetHeight ? (float)targetHeight : outputHeightF;
    if(outputWidthF != 0.0f) planeWidth /= outputWidthF;
    if(outputHeightF != 0.0f) planeHeight /= outputHeightF;

    const float corners[4][4] = {
      {-planeWidth,  planeHeight, 0.0f, 1.0f},
      { planeWidth,  planeHeight, 0.0f, 1.0f},
      {-planeWidth, -planeHeight, 0.0f, 1.0f},
      { planeWidth, -planeHeight, 0.0f, 1.0f},
    };

    float maxPositiveZ = 0.0f;
    for(const auto& corner : corners) {
      float transformed[4];
      MatrixMultiply(transformed, corner, 1, 4, model, 4, 4);
      maxPositiveZ = std::max(maxPositiveZ, transformed[2]);
    }

    if(maxPositiveZ > 0.0f) {
      constexpr float epsilon = 0.05f;
      float limit = (1.0f - epsilon) / maxPositiveZ;
      if(limit < 0.0f) limit = 0.0f;
      perspectiveStrength = std::min(perspectiveStrength, limit);
    }
  }

  if(perspectiveStrength > 0.0f) {
    float perspective[16] = {
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, -perspectiveStrength, 1,
    };
    multiply(model, perspective);
  }

  float alignX = std::fmod(((outputWidthF + (float)targetWidth) * 0.5f), 1.0f) * 2.0f;
  float alignY = std::fmod(((outputHeightF + (float)targetHeight) * 0.5f), 1.0f) * 2.0f;
  float alignTranslate[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    -alignX / outputWidthF, -alignY / outputHeightF, 0, 1,
  };
  multiply(model, alignTranslate);

  std::copy(model, model + 16, camera.matrix);
  camera.lastWidth = targetWidth;
  camera.lastHeight = targetHeight;
  camera.dirty = false;
}
