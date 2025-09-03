#pragma once
#include "esphome.h"

namespace esphome {
namespace ws2811_channels {

class WS2811ChannelLight : public light::LightOutput {
 public:
  WS2811ChannelLight(light::AddressableLight *strip, int pixel, int color);

  void write_state(light::LightState *state) override;
  light::LightTraits get_traits() override;

 protected:
  light::AddressableLight *strip_;
  int pixel_;   // 0..num_pixels-1
  int color_;   // 0=R, 1=G, 2=B
};

}  // namespace ws2811_channels
}  // namespace esphome
