#include "esphome/components/ws2811_channels/ws2811_channels.h"

namespace esphome {
namespace ws2811_channels {

WS2811ChannelsController::WS2811ChannelsController(light::AddressableLightState *strip_state, int num_pixels)
    : strip_state_(strip_state), num_pixels_(num_pixels),
      r_(num_pixels, 0), g_(num_pixels, 0), b_(num_pixels, 0) {}

void WS2811ChannelsController::set_channel_value(int pixel, int color, uint8_t value) {
  if (pixel < 0 || pixel >= num_pixels_ || color < 0 || color > 2) return;

  if (color == 0) r_[pixel] = value;
  else if (color == 1) g_[pixel] = value;
  else b_[pixel] = value;

  dirty_ = true;  // <<< Nur markieren; tatsächliches Schreiben im loop()
}

void WS2811ChannelsController::loop() {
  if (!dirty_) return;

  auto *out = static_cast<light::AddressableLight *>(this->strip_state_->get_output());
  if (out == nullptr) return;

  // Alle Pixel aus Cache setzen
  for (int p = 0; p < num_pixels_; p++) {
    auto rng = out->range(p, p + 1);
    rng.set_red(r_[p]);
    rng.set_green(g_[p]);
    rng.set_blue(b_[p]);
  }

  out->schedule_show();   // jetzt einmalig senden
  dirty_ = false;
}

WS2811ChannelLight::WS2811ChannelLight(WS2811ChannelsController *ctrl, int pixel, int color)
    : ctrl_(ctrl), pixel_(pixel), color_(color) {}

void WS2811ChannelLight::write_state(light::LightState *state) {
  float brightness = 0.0f;
  state->current_values_as_brightness(&brightness);
  const uint8_t v = static_cast<uint8_t>(brightness * 255.0f);
  this->ctrl_->set_channel_value(this->pixel_, this->color_, v);
}

light::LightTraits WS2811ChannelLight::get_traits() {
  light::LightTraits t;
  t.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return t;
}

}  // namespace ws2811_channels
}  // namespace esphome
