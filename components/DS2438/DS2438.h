#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/one_wire/one_wire.h"

namespace esphome {
namespace ds2438 {

class DS2438Sensor : public PollingComponent, public one_wire::OneWireDevice, public EntityBase {
 public:
  
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { humidity_sensor_ = humidity_sensor; }
  void set_vad_sensor(sensor::Sensor *vad_sensor) { vad_sensor_ = vad_sensor; }
  void set_vdd_sensor(sensor::Sensor *vdd_sensor) { vdd_sensor_ = vdd_sensor; }

  protected:

  enum class State {
    IDLE,
    CONVERT_TEMP,
    RECALL_TEMP,
    READ_TEMP,
    CONVERT_VAD,
    RECALL_VAD,
    READ_VAD,
    CONVERT_VDD,
    RECALL_VDD,
    READ_VDD,
    PUBLISH
  };
  
    // *** Zustands- und Timing-Feldern für die State-Machine ***
    State     state_{State::IDLE};
    unsigned long next_ms_{0};
    unsigned long start_ms_ = 0;
    std::string name_;
    float     temp_{NAN}, Vad_{NAN}, Vdd_{NAN};

   uint8_t scratch_pad_[9] = {0};
   uint8_t scratch_pad_page_ = 0;


   
   sensor::Sensor *temperature_sensor_{nullptr};
   sensor::Sensor *humidity_sensor_{nullptr};
   sensor::Sensor *vad_sensor_{nullptr};
   sensor::Sensor *vdd_sensor_{nullptr};

   //bool one_wire::send_command_(uint8_t cmd1, uint8_t cmd2);

   /// Get the number of milliseconds we have to wait for the conversion phase.
   uint16_t millis_to_wait_for_conversion_() const;
   bool read_scratch_pad_();
   bool write_scratch_pad_();
   void read_scratch_pad_int_();
   bool check_scratch_pad_();
   
   float getTemperature();
   float getVoltage();

   void set_config_bit_(uint8_t bit);
   void clear_config_bit_(uint8_t bit);
   uint8_t get_config_register_();

   uint8_t get_scratch_pad_page_();
   bool set_scratch_pad_page_(uint8_t page);
  
  //float Vad_;
  //float Vdd_;
  //float Temperature_;
  //float Humidity_;

};

}  // namespace ds2438
}  // namespace esphome
