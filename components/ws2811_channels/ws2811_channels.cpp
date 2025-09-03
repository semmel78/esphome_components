#include "esphome/components/ws2811_channels/ws2811_channels.h"

namespace esphome {
namespace ws2811_channels {

WS2811ChannelsController::WS2811ChannelsController(light::AddressableLightState *strip_state, int num_pixels)
    : strip_state_(strip_state), num_pixels_(num_pixels),
      r_(num_pixels, 0), g_(num_pixels, 0), b_(num_pixels, 0) {}

void WS2811ChannelsController::set_channel_value(int pixel, int color, uint8_t value) {
  if (pixel < 0 || pixel >= num_pixels_ || color < 0 || color > 2) return;

  // Cache aktualisieren
  if (color == 0) r_[pixel] = value;
  else if (color == 1) g_[pixel] = value;
  else b_[pixel] = value;

  // Pixel im Ausgabe-Treiber setzen + Show planen
  auto *out = this->strip_state_->get_output();   // AddressableLightOutput*
  if (out != nullptr) {
    out->set_pixel(pixel, r_[pixel], g_[pixel], b_[pixel]);
    this->strip_state_->schedule_show();          // Buffer -> LEDs
  }
}

WS2811ChannelLight::WS2811ChannelLight(WS2811ChannelsController *ctrl, int pixel, int color)
    : ctrl_(ctrl), pixel_(pixel), color_(color) {}

void WS2811ChannelLight::write_state(light::LightState *state) {
  float brightness = 0.0f;
  state->current_values_as_brightness(&brightness);    // 0.0 .. 1.0
  const uint8_t v = static_cast<uint8_t>(brightness * 255.0f);
  this->ctrl_->set_channel_value(this->pixel_, this->color_, v);
}

light::LightTraits WS2811ChannelLight::get_traits() {
  light::LightTraits t;
  t.set_supports_brightness(true);
  t.set_supports_rgb(false);
  t.set_supports_color_temperature(false);
  return t;
}

}  // namespace ws2811_channels
}  // namespace esphome
