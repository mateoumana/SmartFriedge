#include <Arduino.h>
#include <ESP32QRCodeReader.h>
//library: https://github.com/alvarowolfx/ESP32QRCodeReader/tree/master

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
struct QRCodeData qrCodeData;

#define FLASH_PIN 4
#define BUTTON_PIN 12

void setup(){
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  Serial.begin(115200);
  Serial.println();

  reader.setup();
  Serial.println("Setup QRCode Reader");
  reader.begin();
  Serial.println("Begin on Core 1");

}

void loop(){
  if (digitalRead(BUTTON_PIN) == HIGH){ // Si el botón se presiona
    Serial.println("Escaneando QR...");
    digitalWrite(FLASH_PIN, HIGH); // Enciende el flash
    if (reader.receiveQrCode(&qrCodeData, 100)){
      Serial.println("Found QRCode");
      if (qrCodeData.valid){
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
      }
      else{
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
    delay(300);
    digitalWrite(FLASH_PIN, LOW); // Apagar el flash
  }
}