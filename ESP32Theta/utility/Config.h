#ifndef _CONFIG_H_
#define _CONFIG_H_

#define EMULATE
//#define DEBUG
//#define DUMPJSON
//#define INITNVS

// SD card
#define TFCARD_CS_PIN 4

// Buttons
#define BTN_A 0
#define BTN_B 1
#define BTN_C 2
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_C 2
#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 35
#define BUTTON_C_PIN 34

// BEEP PIN
#define SPEAKER_PIN 25
#define TONE_PIN_CHANNEL 0

// UART
#define USE_SERIAL Serial

#define TFT_MISO_PIN 19
#define TFT_MOSI_PIN 23
#define TFT_CLK_PIN 5
#define TFT_CS_PIN    16  // Chip select control pin
#define TFT_DC_PIN    17  // Data Command control pin
#define TFT_RST_PIN   9  // Reset pin (could connect to RST pin)

#endif /* SETTINGS_C */
