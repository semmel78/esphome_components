// Implementation based on:

#include "DS2438.h"
#include "esphome/core/log.h"

#define CONVERT_DELAY_MS    20
#define DS2438_CONVERSION_DELAY 20
#define RECALL_DELAY_MS      2

namespace esphome {
namespace ds2438 {

static const char *TAG = "ds2438";

//      OneWire commands
static const uint8_t DS2438_CONVERT_TEMPERATURE = 0x44;
static const uint8_t DS2438_CONVERT_VOLTAGE     = 0xB4;

static const uint8_t DS2438_RECALL_SCRATCH   = 0xB8;
static const uint8_t DS2438_READ_SCRATCH     = 0xBE;
static const uint8_t DS2438_WRITE_SCRATCH    = 0x4E;
static const uint8_t DS2438_COPY_SCRATCH     = 0x48;

//  bits configuration register 
static const uint8_t DS2438_CFG_IAD = 0;
static const uint8_t DS2438_CFG_CA  = 1;
static const uint8_t DS2438_CFG_EE  = 2;
static const uint8_t DS2438_CFG_AD  = 3;

// OneWire commands
static const uint8_t CMD_CONVERT_T    = 0x44;
static const uint8_t CMD_CONVERT_V    = 0xB4;
static const uint8_t CMD_RECALL       = 0xB8;
static const uint8_t CMD_READ_SCRATCH = 0xBE;

// Config bit for AD (0=VAD,1=VDD)
static const uint8_t CFG_BIT_AD = 3;

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

void DS2438Sensor::setup(){
	ESP_LOGCONFIG(TAG, "Setting up DS2438 sensor...");
	if (!this->check_address_())
		return;
  
  if (!this->set_scratch_pad_page_(0))
    return;

  this->state_ = State::IDLE;

  ESP_LOGCONFIG(TAG, "... Setup done!");
}


void DS2438Sensor::update(){
   // start new cycle only if idle
   if (this->state_ == State::IDLE) {
    this->status_clear_warning();
    this->temp_ = this->Vad_ = this->Vdd_ = NAN;
    this->start_ms_ = millis();
    this->state_ = State::CONVERT_TEMP;
  }
}

void DS2438Sensor::loop() {
  auto now = millis();
  switch (this->state_) {
    case State::CONVERT_TEMP:
      // issue temperature convert
      this->send_command_(DS2438_CONVERT_TEMPERATURE);
      this->next_ms_ = now + CONVERT_DELAY_MS;
      this->state_ = State::RECALL_TEMP;
      break;

    case State::RECALL_TEMP:
      if (now < this->next_ms_) return;
      // recall page0
      this->send_command_(DS2438_RECALL_SCRATCH); this->bus_->write8(0);
      this->next_ms_ = now + RECALL_DELAY_MS;
      this->state_ = State::READ_TEMP;
      break;

    case State::READ_TEMP:
      if (now < this->next_ms_) return;
      // read scratch pad
      this->read_scratch_pad_();
      if (this->check_scratch_pad_()) {
        int16_t raw = (int16_t(this->scratch_pad_[2]) << 8) | this->scratch_pad_[1];
        this->temp_ = raw * 0.00390625f;
      } else this->temp_ = NAN;
      this->state_ = State::CONVERT_VAD;
      break;

    case State::CONVERT_VAD:
      // set VAD mode
      this->clear_config_bit_(CFG_BIT_AD);
      this->send_command_(DS2438_CONVERT_VOLTAGE);
      this->next_ms_ = now + CONVERT_DELAY_MS;
      this->state_ = State::RECALL_VAD;
      break;

    case State::RECALL_VAD:
      if (now < this->next_ms_) return;
      this->send_command_(DS2438_RECALL_SCRATCH); this->bus_->write8(0);
      this->next_ms_ = now + RECALL_DELAY_MS;
      this->state_ = State::READ_VAD;
      break;

    case State::READ_VAD:
      if (now < this->next_ms_) return;
      this->read_scratch_pad_();
      if (this->check_scratch_pad_()) {
        uint16_t raw = uint16_t((this->scratch_pad_[4] & 0x03) << 8 | this->scratch_pad_[3]);
        this->Vad_ = raw * 0.01f;
      } else this->Vad_ = NAN;
      this->state_ = State::CONVERT_VDD;
      break;

    case State::CONVERT_VDD:
      this->set_config_bit_(CFG_BIT_AD);
      this->send_command_(DS2438_CONVERT_VOLTAGE);
      this->next_ms_ = now + CONVERT_DELAY_MS;
      this->state_ = State::RECALL_VDD;
      break;

    case State::RECALL_VDD:
      if (now < this->next_ms_) return;
      this->send_command_(DS2438_RECALL_SCRATCH); this->bus_->write8(0);
      this->next_ms_ = now + RECALL_DELAY_MS;
      this->state_ = State::READ_VDD;
      break;

    case State::READ_VDD:
      if (now < this->next_ms_) return;
      this->read_scratch_pad_();
      if (this->check_scratch_pad_()) {
        uint16_t raw = uint16_t((this->scratch_pad_[4] & 0x03) << 8 | this->scratch_pad_[3]);
        this->Vdd_ = raw * 0.01f;
      } else this->Vdd_ = NAN;
      this->state_ = State::PUBLISH;
      break;

    case State::PUBLISH:
      // publish all
      if (this->temperature_sensor_) this->temperature_sensor_->publish_state(this->temp_);
      if (this->vad_sensor_)         this->vad_sensor_->publish_state(this->Vad_);
      if (this->vdd_sensor_)         this->vdd_sensor_->publish_state(this->Vdd_);
      if (this->humidity_sensor_ && !isnan(this->temp_) && this->Vdd_ > 0) {
        float temp_corr = 1.0546f - 0.00216f * this->temp_;
        if (temp_corr <= 0.0f) temp_corr = 0.0001f;  // not expected, just for safety
        float hum = (this->Vad_ / this->Vdd_ - 0.16f) / (0.0062f * temp_corr);
        this->humidity_sensor_->publish_state(hum);
      }
      // reset state
      this->state_ = State::IDLE;
      break;

    case State::IDLE:
    default:
      break;
  }
}

void DS2438Sensor::dump_config(){
  ESP_LOGCONFIG(TAG, "DS2438 OneWire Sensor:");
  if (this->address_ == 0) {
    ESP_LOGW(TAG, "'%s': Unable to select an address", this->get_name().c_str());
    return;
  }
  LOG_ONE_WIRE_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
}

uint16_t DS2438Sensor::millis_to_wait_for_conversion_() const {
      return DS2438_CONVERSION_DELAY;
}

void IRAM_ATTR DS2438Sensor::read_scratch_pad_int_() {
  for (uint8_t &i : this->scratch_pad_) {
    i = this->bus_->read8();
  }
}

uint8_t DS2438Sensor::get_scratch_pad_page_() {
  return this->scratch_pad_page_;
}

bool DS2438Sensor::set_scratch_pad_page_(uint8_t page) {
  // DS2438 has 8 pages to be read
  if (page > 7)
    return false;

  this->scratch_pad_page_ = page;
  return true;
}

bool DS2438Sensor::read_scratch_pad_() {
  bool success = true;
  
  {
    InterruptLock lock;
    success = this->bus_->select(this->address_);
    this->bus_->write8(DS2438_READ_SCRATCH);
    this->bus_->write8(0);
    if (success) this->read_scratch_pad_int_();
    //for (uint8_t i = 0; i < 8; i++) this->scratch_pad_[i] = this->bus_->read8();
  }
  
  if (!success) {
    ESP_LOGW(TAG, "'%s': reading scratch pad page %i failed bus reset", this->get_name().c_str(), this->scratch_pad_page_);
    this->status_set_warning("read bus reset failed");
  }
  
  return success;
}

bool DS2438Sensor::write_scratch_pad_() {
  bool success = true;

  {
    InterruptLock lock;
    if (!this->bus_->select(this->address_))
      success = false;
    this->bus_->write8(DS2438_WRITE_SCRATCH);
    this->bus_->write8(0);
    if (success)
      for (uint8_t i = 0; i < 8; i++) this->bus_->write8(this->scratch_pad_[i]);

    if (!this->bus_->select(this->address_))
      success = false;
    this->bus_->write8(DS2438_COPY_SCRATCH);
    this->bus_->write8(0);
  }

  if (!success) {
    ESP_LOGW(TAG, "'%s': writing scratch pad page %i failed bus reset", this->get_name().c_str(), this->scratch_pad_page_);
    this->status_set_warning("write bus reset failed");
  }
  return success;
}


float DS2438Sensor::getTemperature() {
  // Temperature value is stored on memory page 0 byte 1 (LSB) and 2 (MSB)
  if (this->scratch_pad_page_ != 0)
    return -999;

	//             sign    MSB                 LSB               step      >> 3
	float tempc = (int(this->scratch_pad_[2]) * 256 + this->scratch_pad_[1]) * 0.03125 * 0.125;	
	return tempc;
}

float DS2438Sensor::getVoltage() {
  // Temperature value is stored on memory page 0 byte 1 (LSB) and 2 (MSB)
  if (this->scratch_pad_page_ != 0)
    return -999;
  //  10 mV resolution
	float voltage = ((this->scratch_pad_[4] & 0x03) * 256 + this->scratch_pad_[3]) * 0.01;

	return voltage;
}

bool DS2438Sensor::check_scratch_pad_() {
  bool chksum_validity = (crc8(this->scratch_pad_, 8) == this->scratch_pad_[8]);

#ifdef ESPHOME_LOG_LEVEL_VERY_VERBOSE
  ESP_LOGVV(TAG, "Scratch pad: %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X (%02X)", this->scratch_pad_[0],
            this->scratch_pad_[1], this->scratch_pad_[2], this->scratch_pad_[3], this->scratch_pad_[4],
            this->scratch_pad_[5], this->scratch_pad_[6], this->scratch_pad_[7], this->scratch_pad_[8],
            crc8(this->scratch_pad_, 8));
#endif
  if (!chksum_validity) {
    ESP_LOGW(TAG, "'%s': Scratch pad checksum invalid!", this->get_name().c_str());
    this->status_set_warning("Scratch pad checksum invalid");
  }
  return chksum_validity;
}

///////////////////////////////////////////////////////////
//
//  CONFIG REGISTER
//
void DS2438Sensor::set_config_bit_(uint8_t bit)
{
  uint8_t mask = (0x01 << bit);
  uint8_t page = this->get_scratch_pad_page_(); // remember page before reading config register
  this->set_scratch_pad_page_(0);         // config register is on page 0
  
  this->read_scratch_pad_();
  this->scratch_pad_[0] |= mask;
  this->write_scratch_pad_();

  this->set_scratch_pad_page_(page);     // restore previously used page
  this->read_scratch_pad_();
}


void DS2438Sensor::clear_config_bit_(uint8_t bit)
{
  uint8_t page;
  page = this->get_scratch_pad_page_(); // remember page before reading config register
  this->set_scratch_pad_page_(0);         // config register is on page 0

  this->read_scratch_pad_();
  uint8_t mask = (0x01 << bit);
  this->scratch_pad_[0] &= ~mask;
  this->write_scratch_pad_();
  
  this->set_scratch_pad_page_(page);     // restore previously used page
  this->read_scratch_pad_();
}


uint8_t DS2438Sensor::get_config_register_()
{
  uint8_t page;
  page = this->get_scratch_pad_page_(); // remember page before reading config register
  
  this->set_scratch_pad_page_(0);         // config register is on page 0
  this->read_scratch_pad_();            // read register
  this->set_scratch_pad_page_(page);     // restore previously used page
  return this->scratch_pad_[0];         // return config register value
}



}  // namespace ds2438
}  // namespace esphome
