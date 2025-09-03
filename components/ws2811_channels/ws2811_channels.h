#pragma once
#include "esphome.h"
#include <vector>

namespace esphome {
namespace ws2811_channels {

// Controller hält Pointer auf den Strip-State und einen RGB-Cache pro Pixel.
class WS2811ChannelsController : public Component {
 public:
  WS2811ChannelsController(light::AddressableLightState *strip_state, int num_pixels);

  // vom Channel-Light aufgerufen
  void set_channel_value(int pixel, int color, uint8_t value);

 protected:
  light::AddressableLightState *strip_state_{nullptr};
  int num_pixels_{0};
  std::vector<uint8_t> r_, g_, b_;
};

// Ein einzelner Mono-Channel (R oder G oder B) als eigenes LightOutput.
class WS2811ChannelLight : public light::LightOutput {
 public:
  WS2811ChannelLight(WS2811ChannelsController *ctrl, int pixel, int color);
  void write_state(light::LightState *state) override;
  light::LightTraits get_traits() override;

 protected:
  WS2811ChannelsController *ctrl_{nullptr};
  int pixel_{0};   // 0..num_pixels-1
  int color_{0};   // 0=R, 1=G, 2=B
};

}  // namespace ws2811_channels
}  // namespace esphome
