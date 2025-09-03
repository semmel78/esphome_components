#include "esphome/components/ws2811_channels/ws2811_channels.h"

namespace esphome {
namespace ws2811_channels {

WS2811ChannelLight::WS2811ChannelLight(light::AddressableLight *strip, int pixel, int color)
    : strip_(strip), pixel_(pixel), color_(color) {}

void WS2811ChannelLight::write_state(light::LightState *state) {
  // Helligkeit (0.0..1.0) holen
  float brightness = 0.0f;
  state->current_values_as_brightness(&brightness);

  // Aktuelle RGB-Werte des Pixels lesen
  auto c = this->strip_->get(this->pixel_);
  uint8_t r = c.red, g = c.green, b = c.blue;

  // Nur den gewünschten Kanal überschreiben
  const uint8_t v = static_cast<uint8_t>(brightness * 255.0f);
  if (this->color_ == 0) r = v;
  else if (this->color_ == 1) g = v;
  else b = v;

  // Pixel setzen + Ausgabe planen
  this->strip_->get_output()->set_pixel(this->pixel_, r, g, b);
  this->strip_->schedule_show();
}

light::LightTraits WS2811ChannelLight::get_traits() {
  light::LightTraits t;
  t.set_supports_brightness(true);
  // Mono-Channel: keine Farbe / kein CCT
  t.set_supports_rgb(false);
  t.set_supports_color_temperature(false);
  return t;
}

}  // namespace ws2811_channels
}  // namespace esphome
