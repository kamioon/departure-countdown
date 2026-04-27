#ifndef PINS_H
#define PINS_H

// ESP32-S3-DevKitC-1 Pin Configuration
// Note: ESP32-S3 uses different GPIO numbers than classic ESP32

// MAX7219 Display (SPI)
// ESP32-S3 default SPI pins (FSPI)
#define MAX7219_DIN    11   // MOSI (Data In) - GPIO11
#define MAX7219_CLK    12   // SCK (Clock) - GPIO12
#define MAX7219_CS     10   // CS (Chip Select/Load) - GPIO10
#define MAX_DEVICES    8    // Number of 8x8 matrices chained

// Rotary Encoder
#define ENC_CLK     4   // Clock pin - GPIO4
#define ENC_DT      5   // Data pin - GPIO5
#define ENC_SW      6   // Switch/button pin - GPIO6

// Buttons
#define BTN_START   7   // Start/Pause button - GPIO7
#define BTN_RESET   15  // Reset button - GPIO15
#define BTN_MODE    16  // Mode/Menu button - GPIO16

// Alerts
#define BUZZER_PIN  17  // PWM buzzer - GPIO17
#define LED_R       18  // RGB LED Red - GPIO18
#define LED_G       8   // RGB LED Green - GPIO8
#define LED_B       3   // RGB LED Blue - GPIO3

// RTC (I2C - different pins than SPI display)
// ESP32-S3 default I2C pins
#define RTC_SDA     1   // I2C Data - GPIO1
#define RTC_SCL     2   // I2C Clock - GPIO2

// Pin Notes for ESP32-S3-DevKitC-1:
// - GPIO0: Boot button (avoid using)
// - GPIO19, GPIO20: USB D-, D+ (avoid using)
// - GPIO26-32: Not exposed on DevKitC-1
// - GPIO33-37: Used for flash/PSRAM (avoid using)
// - GPIO43, GPIO44: UART TX/RX (can use if not using Serial)

#endif // PINS_H
