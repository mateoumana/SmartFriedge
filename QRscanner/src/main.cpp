#include <Arduino.h>
#include <Wire.h>
#include <ESP32QRCodeReader.h>

#define FLASH_PIN 4
#define BUTTON_PIN 16
#define I2C_SDA 14
#define I2C_SCL 15
#define SLAVE_ADDR 0x08 // Dirección I2C del esclavo

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
String scanning = " "; // Variable para controlar el escaneo
String temp = " ";

//library: https://github.com/alvarowolfx/ESP32QRCodeReader/tree/master
//ESP32 CAM Feed Voltage 5V, Input Voltage GPIO 3,3V

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
        scanning = (const char *)qrCodeData.payload;
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Wire.begin(I2C_SDA, I2C_SCL); // Configura ESP32-CAM como maestro
  Serial.begin(115200);
  Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop()
{
  delay(100);
  if(scanning != temp){
    Serial.println(scanning);
    temp = scanning;
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(scanning.c_str());
    Wire.endTransmission();
    Serial.println("Mensaje enviado");
  }
}