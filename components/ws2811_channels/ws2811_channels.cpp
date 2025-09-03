#include "ws2811_channels.h"

WS2811ChannelLight::WS2811ChannelLight(esphome::light::AddressableLight *strip, int pixel, int color)
  : strip_(strip), pixel_(pixel), color_(color), value_(0.0f) {}

void WS2811ChannelLight::write_state(esphome::light::LightState *state) {
  float brightness;
  state->current_values_as_brightness(&brightness);
  value_ = brightness;

  auto c = strip_->get(pixel_);
  uint8_t r = c.red, g = c.green, b = c.blue;

  switch (color_) {
    case 0: r = static_cast<uint8_t>(brightness * 255); break;
    case 1: g = static_cast<uint8_t>(brightness * 255); break;
    case 2: b = static_cast<uint8_t>(brightness * 255); break;
  }

  strip_->get_output()->set_pixel(pixel_, r, g, b);
  strip_->schedule_show();
}

esphome::light::LightTraits WS2811ChannelLight::get_traits() {
  auto traits = esphome::light::LightTraits();
  traits.set_supports_brightness(true);
  traits.set_supports_rgb(false);
  return traits;
}

WS2811Channels::WS2811Channels(esphome::light::AddressableLight *strip) : strip_(strip) {
  int num_pixels = strip_->size();
  for (int i = 0; i < num_pixels; i++) {
    for (int c = 0; c < 3; c++) {
      channels_.push_back(new WS2811ChannelLight(strip_, i, c));
    }
  }
}

WS2811ChannelLight *WS2811Channels::get_channel(int i) {
  return channels_[i];
}

int WS2811Channels::channel_count() const {
  return channels_.size();
}
