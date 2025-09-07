#include "esphome/components/ws2811_channels/ws2811_channels.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ws2811_channels {

static const char *const TAG = "ws2811_channels";

WS2811ChannelsController::WS2811ChannelsController(light::AddressableLightState *strip_state, int num_pixels)
    : strip_state_(strip_state),
      num_pixels_(0),
      initial_num_pixels_(num_pixels) {}

void WS2811ChannelsController::setup() {
  // Echte Pixelzahl ermitteln (falls nicht vorgegeben)
  int resolved = this->initial_num_pixels_;
  auto *out_base = this->strip_state_ ? this->strip_state_->get_output() : nullptr;
  auto *addr = out_base ? static_cast<light::AddressableLight *>(out_base) : nullptr;

  if (resolved <= 0) {
    if (addr != nullptr) {
      resolved = addr->size();
    } else {
      ESP_LOGE(TAG, "Addressable output not available in setup()");
      resolved = 0;
    }
  }

  this->num_pixels_ = resolved;

  if (this->num_pixels_ > 0) {
    this->r_.assign(this->num_pixels_, 0);
    this->g_.assign(this->num_pixels_, 0);
    this->b_.assign(this->num_pixels_, 0);
    this->dirty_ = true;  // beim ersten loop komplett flushen
    ESP_LOGI(TAG, "Initialized with %d pixels", this->num_pixels_);
  } else {
    ESP_LOGE(TAG, "No pixels resolved (num_pixels_=0) – nothing will be shown");
  }
}

void WS2811ChannelsController::set_channel_value(int pixel, int color, uint8_t value) {
  if (pixel < 0 || pixel >= num_pixels_ || color < 0 || color > 2) {
    ESP_LOGW(TAG, "set_channel_value out of range (pixel=%d, color=%d, num_pixels=%d)", pixel, color, num_pixels_);
    return;
  }

  if (color == 0) r_[pixel] = value;
  else if (color == 1) g_[pixel] = value;
  else b_[pixel] = value;

  dirty_ = true;  // tatsächliches Schreiben im loop()
}

void WS2811ChannelsController::loop() {
  if (!dirty_ || num_pixels_ <= 0) return;

  auto *out_base = this->strip_state_->get_output();
  auto *addr = out_base ? static_cast<light::AddressableLight *>(out_base) : nullptr;
  if (addr == nullptr) return;

  for (int p = 0; p < num_pixels_; p++) {
    auto rng = addr->range(p, p + 1);
    rng.set_red(r_[p]);
    rng.set_green(g_[p]);
    rng.set_blue(b_[p]);
  }

  addr->schedule_show();
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
