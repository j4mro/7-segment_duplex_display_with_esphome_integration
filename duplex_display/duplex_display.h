#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/i2c/i2c.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome {
namespace duplex_display {

class DuplexDisplay : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_writer(std::function<void(DuplexDisplay &)> &&writer) { this->writer_ = std::move(writer); }
  void setup() override;
  float get_setup_priority() const override;
  void update() override;
  void dump_config() override;
  
  /// Print the given text
  void print(const char *str);
  /// Print the given string
  void print(const std::string &str);
  /// Evaluate the printf-format and print the text
  void printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

  // Set setting from config
  void set_animation(bool animation) { this->animation = animation; }
  void set_colon_blink(bool colon_blink) { this->colon_blink = colon_blink; }

  //LEDs controll
  void control_led(short int led, bool state);
  void PMLed(bool state) { this -> control_led(1,state); };
  void AMLed(bool state) { this -> control_led(2,state); };
  void AlarmLed(bool state) { this -> control_led(3,state); };

#ifdef USE_TIME
  /// Evaluate the strftime-format and print the text
  void strftime(const char *format, time::ESPTime time) __attribute__((format(strftime, 2, 0)));
#endif

 protected:
  void command_(uint8_t value);
  void call_writer() { this->writer_(*this); }
  void display_();
  bool animation {false};
  bool colon_blink {false};

  enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};

  std::function<void(DuplexDisplay &)> writer_;
  uint8_t buffer_[5] = {1};
};

}  // namespace duplex_display
}  // namespace esphome