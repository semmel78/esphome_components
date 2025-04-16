#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/one_wire/one_wire.h"

namespace esphome {
namespace empty_onewire_sensor {

class EmptyOnewireSensor : public PollingComponent, public sensor::Sensor, public one_wire::OneWireDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
};

}  // namespace empty_onewire_sensor
}  // namespace esphome
