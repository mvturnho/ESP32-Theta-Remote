// Do not remove the include below

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <Preferences.h>
#include "ESP32Theta.h"
#include "Theta.h"
#include "utility/music_8bit.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "utility/Config.h"
#include "utility/Button.h"
#include "utility/Speaker.h"


void thetaTask(void *pvParameters);
void buttonTask(void *pvParameters);
void displayTask(void *pvParameters);
void soundTask(void *pvParameters);

int ConnectTHETA(void);
void drawText(int x, int y, const char *text, uint16_t color);

#define TFT_CS 16
#define TFT_RST 9
#define TFT_DC 17
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
#define TFT_SCLK 5
#define TFT_MOSI 23
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_CLK_PIN, TFT_RST_PIN);

Preferences preferences;
Theta theta;
xQueueHandle event_queue;

#ifdef EMULATE
#define DEFAUT_SSID "YOURSSID"
#define DEFAULT_PWD "Password"
#else
#define DEFAUT_SSID "THETAXS00130913.OSC"
#define DEFAULT_PWD "00130913"
#endif
String strThetaSSID;
String strThetaPWD;

#define DEBOUNCE_MS 5
Button BtnA = Button(BUTTON_A_PIN, true, DEBOUNCE_MS);
Button BtnB = Button(BUTTON_B_PIN, true, DEBOUNCE_MS);
Button BtnC = Button(BUTTON_C_PIN, true, DEBOUNCE_MS);
// SPEAKER
SPEAKER Speaker;
bool buttonApressed = false;
bool play_music = false;

void setup() {
	Serial.begin(115200);

	pinMode(BUTTON_A_PIN, INPUT);
	pinMode(BUTTON_B_PIN, INPUT);
	pinMode(BUTTON_C_PIN, INPUT);

	// TONE
	Speaker.begin();
	Speaker.setVolume(5);

#ifdef INITNVS
	int err;
	err=nvs_flash_init();
	Serial.println("nvs_flash_init: " + err);
	err=nvs_flash_erase();
	Serial.println("nvs_flash_erase: " + err);
#endif INITNVS

	tft.initR(INITR_18GREENTAB);   // initialize a ST7735S chip, black tab
	tft.fillScreen(ST7735_BLACK);

	preferences.begin("theta-config");

	strThetaSSID = preferences.getString("WIFI_SSID", DEFAUT_SSID);
	strThetaPWD = preferences.getString("WIFI_PASSWD", DEFAULT_PWD);

	Serial.println("");
	Serial.println("");
	Serial.println("-----------------------------------------");
	Serial.println("  RICOH THETA S Remote Control Software  ");
	Serial.println("           Full Control Edition          ");
	Serial.print("WIFI-SSID: ");
	Serial.println(strThetaSSID);

	event_queue = xQueueCreate(10, sizeof(uint32_t));

	TaskHandle_t xHandle = NULL;
	xTaskCreatePinnedToCore(thetaTask, "thetaTask", 4096, (void *) 1, 1, NULL, 0);
	xTaskCreatePinnedToCore(buttonTask, "buttonTask", 4096, (void *) 1, tskIDLE_PRIORITY, NULL, 1);
	xTaskCreatePinnedToCore(displayTask, "displayTask", 4096, (void *) 1, 1, NULL, 1);
	xTaskCreatePinnedToCore(soundTask, "soundTask", 4096, (void *) 1, 1, &xHandle, 1);

	configASSERT(xHandle);
	//Speaker.playMusic(m5stack_startup_music, 25000);
	play_music = true;
}

void thetaTask(void *pvParameters) {
	bool pic = true;
	int taskno = (int) pvParameters;
	while (1) {
		if (!WiFi.isConnected()) {
			ConnectTHETA();
		} else {
			theta.getInfo();
			theta.postGetState();
			if (pic) {
				theta.postStartSession();
				theta.postGetOptions();
				pic = false;
			}
			if (buttonApressed) {
				theta.postTakePicture();
				buttonApressed = false;
			}

		}
		vTaskDelay(500);
	}
}

void buttonTask(void *pvParameters) {
	int taskno = (int) pvParameters;
	while (1) {
		BtnA.read();
		if (BtnA.wasPressed()) {
			Serial.print("button");
			buttonApressed = true;
		}
		vTaskDelay(50);
	}
}

void displayTask(void *pvParameters) {
	int taskno = (int) pvParameters;
	while (1) {
		tft.fillScreen(ST7735_BLACK);
		if (WiFi.isConnected()) {
			drawText(0, 0, String(theta.bateryLevel).c_str(), ST7735_WHITE);
			drawText(0, 20, String(theta.fwVersion).c_str(), ST7735_WHITE);
		} else {
			drawText(10, 20, String("NOT CONNECTED").c_str(), ST7735_BLUE);
		}

		vTaskDelay(500);
	}
}

void soundTask(void *pvParameters) {
	bool pic = true;
	int taskno = (int) pvParameters;
	while (1) {
		if (play_music) {
			Speaker.playMusic(m5stack_startup_music, 30000);
			play_music = false;
		}
		vTaskDelay(500);
	}
}

void loop() {
	vTaskDelay(portMAX_DELAY);
}

//-------------------------------------------
// Wi-Fi Connect functions
//-------------------------------------------
int ConnectTHETA(void) {
	WiFi.disconnect(true);
//	WiFi.enableSTA(true);
	int retval = WiFi.begin(strThetaSSID.c_str(), strThetaPWD.c_str());
	Serial.print("result WiFi.begin : ");
	Serial.println(retval);

	Serial.print(WiFi.status());
	Serial.print("WIFI-SSID: ");
	Serial.println(WiFi.SSID());
	while (!WiFi.isConnected()) {
		Serial.print(WiFi.status());
		Serial.print("WIFI-SSID: ");
		Serial.println(WiFi.SSID());
		if (WiFi.status() == WL_NO_SSID_AVAIL) {
			WiFi.disconnect(true);
			WiFi.begin(strThetaSSID.c_str(), strThetaPWD.c_str());
		}

		vTaskDelay(500);
	}

	Serial.println("");
	Serial.println("WiFi connected : ");
	Serial.println("Mac            : " + WiFi.macAddress());
	Serial.print("IP address     : ");
	Serial.println(WiFi.localIP());
	Serial.print("GW address     : ");
	Serial.println(WiFi.gatewayIP());
	//set the gateway as connection host
	//theta.strHost = WiFi.gatewayIP().toString();
#ifdef EMULATE
	theta.iHttpPort = 5000;
	theta.strHost = "192.168.2.27";
#endif
	return 0;
}

void drawText(int x, int y, const char *text, uint16_t color) {
	tft.setCursor(x, y);
	tft.setTextColor(color);
	tft.setTextWrap(true);
	tft.print(text);
}
