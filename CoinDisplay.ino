
#include <WiFiManager.h>
#include <strings_en.h>
#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h> 
#include <WiFiClient.h> 
#include <ArduinoJson.h> 
#include <U8g2lib.h>
#define ARDUINOJSON_USE_DOUBLE 1 
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, D8, D1, D3);

// using code inspired by https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/ and https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/examples/BasicHttpsClient/BasicHttpsClient.ino



unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

#define bitcoin_icon_width 25
#define bitcoin_icon_height 25
static uint8_t bitcoin_icon_bits[] PROGMEM = {
   0x00, 0x38, 0x00, 0x00, 0x80, 0xff, 0x03, 0x00, 0xc0, 0x01, 0x07, 0x00,
   0x70, 0x00, 0x1c, 0x00, 0x18, 0x24, 0x30, 0x00, 0x08, 0x24, 0x20, 0x00,
   0x0c, 0x24, 0x60, 0x00, 0x86, 0xff, 0xc0, 0x00, 0x06, 0xff, 0xc1, 0x00,
   0x02, 0x86, 0x81, 0x00, 0x02, 0x86, 0x81, 0x00, 0x03, 0x86, 0x81, 0x01,
   0x03, 0xfe, 0x81, 0x01, 0x03, 0x06, 0x83, 0x01, 0x02, 0x06, 0x83, 0x00,
   0x02, 0x06, 0x83, 0x00, 0x06, 0xff, 0xc3, 0x00, 0x86, 0xff, 0xc1, 0x00,
   0x0c, 0x24, 0x60, 0x00, 0x08, 0x24, 0x20, 0x00, 0x18, 0x24, 0x30, 0x00,
   0x70, 0x00, 0x1c, 0x00, 0xc0, 0x01, 0x07, 0x00, 0x80, 0xff, 0x03, 0x00,
   0x00, 0x38, 0x00, 0x00 };

#define Dogecoin_width 25
#define Dogecoin_height 25
static uint8_t Dogecoin_bits[] = {
   0x00, 0xfe, 0x00, 0x00, 0x80, 0xff, 0x03, 0x00, 0xe0, 0xff, 0x0f, 0x00,
   0xf0, 0x00, 0x1e, 0x00, 0x78, 0x00, 0x3c, 0x00, 0x1c, 0x00, 0x70, 0x00,
   0x1c, 0x3f, 0x70, 0x00, 0x0e, 0xff, 0xe0, 0x00, 0x06, 0xff, 0xc1, 0x00,
   0x07, 0x87, 0xc3, 0x01, 0x07, 0x87, 0xc3, 0x01, 0x87, 0x1f, 0xc7, 0x01,
   0xc7, 0x1f, 0xc7, 0x01, 0x87, 0x1f, 0xc7, 0x01, 0x07, 0x07, 0xc3, 0x01,
   0x07, 0x87, 0xc3, 0x01, 0x06, 0xe7, 0xc3, 0x00, 0x0e, 0xff, 0xe1, 0x00,
   0x1c, 0x7f, 0x70, 0x00, 0x1c, 0x00, 0x70, 0x00, 0x78, 0x00, 0x3c, 0x00,
   0xf0, 0x00, 0x1e, 0x00, 0xe0, 0xff, 0x0f, 0x00, 0x80, 0xff, 0x03, 0x00,
   0x00, 0xfe, 0x00, 0x00 };


void writeStartUpMessage() {
	u8g2.firstPage();
	do {
		u8g2.setFont(u8g2_font_7x13B_mf);
		u8g2.drawStr(2, 15, "Connect to WiFi");
		u8g2.drawStr(2, 32, "SSID: CoinDisplay");
		u8g2.drawStr(2, 49, "IP: 192.168.4.1");
	} while (u8g2.nextPage());
	delay(1000);
}

void writeBrandMessage() {
	u8g2.firstPage();
	do {
		u8g2.setFont(u8g2_font_VCR_OSD_mu);
		u8g2.drawStr(32, 42, "ADH");
		u8g2.setFont(u8g2_font_lubBI14_te);
		u8g2.drawStr(67, 42, "DiY");
	} while (u8g2.nextPage());
	delay(1000);
}

void drawPrice(String bprice, String dprice) {

	u8g2.firstPage();
	do {
		u8g2.drawXBMP(2, 10, bitcoin_icon_width, bitcoin_icon_height, bitcoin_icon_bits);
		u8g2.setFont(u8g2_font_ncenB14_tr);
		u8g2.drawStr(29, 30, bprice.c_str());

		u8g2.drawXBMP(2, 36, Dogecoin_width, Dogecoin_height, Dogecoin_bits);
		u8g2.setFont(u8g2_font_ncenB14_tr);
		u8g2.drawStr(30, 54, dprice.c_str());

	} while (u8g2.nextPage());
	delay(1000);

}


String ParsePriceDoge(String rawdata) {
	String result = "";

	StaticJsonDocument<1000> doc;
	DeserializationError error = deserializeJson(doc, rawdata);
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
	}
	else {
		const char* tresult = doc["data"]["prices"][0]["price"];
		Serial.print("Current Doge Price: ");
		Serial.println(tresult);
		result = tresult;
	}



	return result;
}

String ParsePriceBitCoin(String rawdata) {
	String result = "";

	StaticJsonDocument<1000> doc;
	DeserializationError error = deserializeJson(doc, rawdata);
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
	}
	else {
		const char* tresult = doc["data"]["prices"][0]["price"];
		Serial.print("Current BTC Price: ");
		Serial.println(tresult);
		result = tresult;
	}

	return result;
}

void setup() {


	Serial.begin(115200);
	u8g2.begin();
	//u8g2.setDisplayRotation(U8G2_R2);
	Serial.println("");

	WiFi.mode(WIFI_STA);
	WiFiManager wm;
	wm.resetSettings();
	writeBrandMessage();
	delay(4000);
	writeStartUpMessage();

	bool res;
	res = wm.autoConnect("CoinDisplay", "password"); // password protected ap

	if (!res) {
		Serial.println("Connection Failed. Rebooting. ");
		ESP.restart();
	}
	else {

		Serial.println("Connected to" + WiFi.localIP().toString());
	}

	Serial.println("");
	Serial.print("Connected to Wifi netwwork with IP Address: ");
	Serial.println("Timer set to 10000 ms. First update in 10000 ms. ");
	//StaticJsonDocument<1000> doc;

}

void loop() {
	String bitPayload = "";
	String dogePayload = "";
	String bitPrice = "";
	String dogePrice = "";
	if ((millis() - lastTime) > timerDelay) {
		if (WiFi.status() == WL_CONNECTED) {

			std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
			client->setInsecure();
			HTTPClient https;
			Serial.print("[HTTPS] begin DOGE...\n");
			if (https.begin(*client, "https://sochain.com//api/v2/get_price/DOGE/USD")) {  // HTTPS
				Serial.print("[HTTPS] GET...\n");
				// start connection and send HTTP header
				int httpCode = https.GET();
				// httpCode will be negative on error
				if (httpCode > 0) {
					// HTTP header has been send and Server response header has been handled
					Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

					// file found at server
					if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
						String payload = https.getString();
						Serial.println(payload);
						dogePrice = ParsePriceDoge(payload);
					}
				}
				else {
					Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
				}

				Serial.print("[HTTPS] begin BTC...\n");
				if (https.begin(*client, "https://sochain.com//api/v2/get_price/BTC/USD")) {  // HTTPS
					Serial.print("[HTTPS] GET...\n");
					// start connection and send HTTP header
					int httpCode = https.GET();
					// httpCode will be negative on error
					if (httpCode > 0) {
						// HTTP header has been send and Server response header has been handled
						Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

						// file found at server
						if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
							String payload = https.getString();
							Serial.println(payload);
							bitPrice = ParsePriceBitCoin(payload);
						}
					}
					else {
						Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
					}

					https.end();
				}

			}
			else {
				Serial.printf("[HTTPS] Unable to connect\n");
			}

			drawPrice(bitPrice, dogePrice);


		}
		else {
			Serial.println("WiFi Disconnected");
		}
		lastTime = millis();
	}

}