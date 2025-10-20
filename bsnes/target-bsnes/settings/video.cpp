auto VideoSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  colorAdjustmentLabel.setFont(Font().setBold()).setText("Color Adjustment");
  colorLayout.setSize({3, 3});
  colorLayout.column(0).setAlignment(1.0);
  luminanceLabel.setText("Luminance:");
  luminanceValue.setAlignment(0.5);
  luminanceSlider.setLength(101).setPosition(settings.video.luminance).onChange([&] {
    string value = {luminanceSlider.position(), "%"};
    settings.video.luminance = value.natural();
    emulator->configure("Video/Luminance", settings.video.luminance);
    luminanceValue.setText(value);
  }).doChange();
  saturationLabel.setText("Saturation:");
  saturationValue.setAlignment(0.5);
  saturationSlider.setLength(201).setPosition(settings.video.saturation).onChange([&] {
    string value = {saturationSlider.position(), "%"};
    settings.video.saturation = value.natural();
    emulator->configure("Video/Saturation", settings.video.saturation);
    saturationValue.setText(value);
  }).doChange();
  gammaLabel.setText("Gamma:");
  gammaValue.setAlignment(0.5);
  gammaSlider.setLength(101).setPosition(settings.video.gamma - 100).onChange([&] {
    string value = {100 + gammaSlider.position(), "%"};
    settings.video.gamma = value.natural();
    emulator->configure("Video/Gamma", settings.video.gamma);
    gammaValue.setText(value);
  }).doChange();

  dimmingOption.setText("Dim video when idle").setToolTip(
    "Darkens the video to indicate that the emulation is not running."
  ).setChecked(settings.video.dimming).onToggle([&] {
    settings.video.dimming = dimmingOption.checked();
  });

  snowOption.setText("Draw snow effect when idle").setChecked(settings.video.snow).onToggle([&] {
    settings.video.snow = snowOption.checked();
    presentation.updateProgramIcon();
  });

  cameraLabel.setFont(Font().setBold()).setText("3D Camera");
  cameraEnable.setText("Enable camera transform").setChecked(settings.video.camera.enabled);
  cameraLayout.setSize({3, 8});
  cameraLayout.column(0).setAlignment(1.0);
  cameraLayout.column(1).setAlignment(0.5);

  auto applyCamera = [&] {
    Video::CameraSettings camera;
    camera.enabled = settings.video.camera.enabled;
    camera.yaw = settings.video.camera.yaw;
    camera.pitch = settings.video.camera.pitch;
    camera.roll = settings.video.camera.roll;
    camera.offsetX = settings.video.camera.offsetX;
    camera.offsetY = settings.video.camera.offsetY;
    camera.offsetZ = settings.video.camera.offsetZ;
    camera.zoom = settings.video.camera.zoom;
    camera.perspective = settings.video.camera.perspective;
    video.setCamera(camera);
  };

  auto refreshCameraControls = [&] {
    bool enabled = cameraEnable.checked();
    cameraLayout.setEnabled(enabled);
  };

  cameraEnable.onToggle([&] {
    settings.video.camera.enabled = cameraEnable.checked();
    refreshCameraControls();
    applyCamera();
  });

  auto setupAngleSlider = [&](HorizontalSlider& slider, Label& valueLabel, int& setting) {
    valueLabel.setAlignment(0.5);
    slider.setLength(361).setPosition(setting + 180).onChange([&] {
      setting = slider.position() - 180;
      valueLabel.setText({setting, "°"});
      applyCamera();
    }).doChange();
  };

  auto setupOffsetSlider = [&](HorizontalSlider& slider, Label& valueLabel, int& setting) {
    valueLabel.setAlignment(0.5);
    slider.setLength(201).setPosition(setting + 100).onChange([&] {
      setting = slider.position() - 100;
      valueLabel.setText({setting, "%"});
      applyCamera();
    }).doChange();
  };

  cameraYawLabel.setText("Yaw:");
  setupAngleSlider(cameraYawSlider, cameraYawValue, settings.video.camera.yaw);

  cameraPitchLabel.setText("Pitch:");
  setupAngleSlider(cameraPitchSlider, cameraPitchValue, settings.video.camera.pitch);

  cameraRollLabel.setText("Roll:");
  setupAngleSlider(cameraRollSlider, cameraRollValue, settings.video.camera.roll);

  cameraOffsetXLabel.setText("Offset X:");
  setupOffsetSlider(cameraOffsetXSlider, cameraOffsetXValue, settings.video.camera.offsetX);

  cameraOffsetYLabel.setText("Offset Y:");
  setupOffsetSlider(cameraOffsetYSlider, cameraOffsetYValue, settings.video.camera.offsetY);

  cameraOffsetZLabel.setText("Offset Z:");
  setupOffsetSlider(cameraOffsetZSlider, cameraOffsetZValue, settings.video.camera.offsetZ);

  cameraZoomLabel.setText("Zoom:");
  cameraZoomValue.setAlignment(0.5);
  cameraZoomSlider.setLength(251).setPosition(settings.video.camera.zoom - 50).onChange([&] {
    settings.video.camera.zoom = cameraZoomSlider.position() + 50;
    cameraZoomValue.setText({settings.video.camera.zoom, "%"});
    applyCamera();
  }).doChange();

  cameraPerspectiveLabel.setText("Perspective:");
  cameraPerspectiveValue.setAlignment(0.5);
  cameraPerspectiveSlider.setLength(201).setPosition(settings.video.camera.perspective).onChange([&] {
    settings.video.camera.perspective = cameraPerspectiveSlider.position();
    cameraPerspectiveValue.setText({settings.video.camera.perspective, "%"});
    applyCamera();
  }).doChange();

  refreshCameraControls();
  applyCamera();
}
