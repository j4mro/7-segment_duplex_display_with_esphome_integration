#include "duplex_display.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace duplex_display {

static const char *TAG = "duplex_display";

// First set bit determines command, bits after that are the data.
static const uint8_t DISPLAY_COMMAND_PRINT = 0x00;
static const uint8_t DISPLAY_COMMAND_LED_CONTROLL = 0x01;
static const uint8_t DISPLAY_COMMAND_ANIMATION_CONTROLL = 0x02;
static const uint8_t DISPLAY_COMMAND_COLON_BLINK_CONTROLL = 0x03;


void DuplexDisplay::setup() {
    ESP_LOGCONFIG(TAG, "Setting up I2C DuplexDisplay");

    auto err = this->write(nullptr, 0);
    if (err != i2c::ERROR_OK) {
        this->error_code_ = COMMUNICATION_FAILED;
        this->mark_failed();
        return;
    }

    this->write_byte(DISPLAY_COMMAND_ANIMATION_CONTROLL, this->animation);
    this->write_byte(DISPLAY_COMMAND_COLON_BLINK_CONTROLL, this->colon_blink);
}

void DuplexDisplay::dump_config(){
    ESP_LOGCONFIG(TAG, "I2C Duplex Display: ");
    LOG_I2C_DEVICE(this);
    ESP_LOGCONFIG(TAG, "  Animation: %s", this->animation ? "True" : "False");
    ESP_LOGCONFIG(TAG, "  Colon blink: %s", this->colon_blink ? "True" : "False");

    if (this->error_code_ == COMMUNICATION_FAILED) {
        ESP_LOGE(TAG, "Communication with Duplex Display failed!");
    }
}

float DuplexDisplay::get_setup_priority() const { return setup_priority::PROCESSOR; }

void DuplexDisplay::display_() {
    this->write_bytes(DISPLAY_COMMAND_PRINT, this->buffer_,5);
}

void DuplexDisplay::update() {
    this->call_writer();
    this->display_();
}

void DuplexDisplay::print(const char *str) {
    String chars=" 0123456789abcdefhilnopruy`'-_:";
    String chars_allowed_in_first_segment=" 1237dinoru`-_";
    uint8_t pos = 0;
    for (; *str != '\0'; str++) {
        if (pos >= 5) {
            ESP_LOGE(TAG, "String is too long for the display!");
            break;
        }

        uint8_t index = (uint8_t)chars.indexOf(*str);

        if(index != 255){
            if(pos == 0 && chars_allowed_in_first_segment.indexOf(*str) < 0)
                this->buffer_[pos] = 0;
            else
                this->buffer_[pos] = index;
            pos++;
        }
    }
}

void DuplexDisplay::print(const std::string &str) { this->print(str.c_str()); }

void DuplexDisplay::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    this->print(buffer);
}

void DuplexDisplay::control_led(short int led, bool state) { this->write_byte(DISPLAY_COMMAND_LED_CONTROLL, led<<1 | state); };

#ifdef USE_TIME
void DuplexDisplay::strftime(const char *format, time::ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);
  if (ret > 0)
    this->print(buffer);
}
#endif

}  // namespace duplex_display
}  // namespace esphome