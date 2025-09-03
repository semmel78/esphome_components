#pragma once
#include "esphome.h"
#include <vector>

class WS2811ChannelLight : public esphome::light::LightOutput {
 public:
  WS2811ChannelLight(esphome::light::AddressableLight *strip, int pixel, int color);

  void write_state(esphome::light::LightState *state) override;
  esphome::light::LightTraits get_traits() override;

 protected:
  esphome::light::AddressableLight *strip_;
  int pixel_;
  int color_; // 0=Red, 1=Green, 2=Blue
  float value_;
};

class WS2811Channels : public esphome::Component {
 public:
  WS2811Channels(esphome::light::AddressableLight *strip);
  WS2811ChannelLight *get_channel(int i);
  int channel_count() const;

 protected:
  esphome::light::AddressableLight *strip_;
  std::vector<WS2811ChannelLight *> channels_;
};
