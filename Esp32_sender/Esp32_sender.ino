#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>

//Slave Address
#define I2C_SLAVE_ADDR 0x08
#define SDA 21
#define SCL 22
char* message = (char*) malloc(50);  // Reserva 50 bytes en RAM para recibir el mensaje del ESP32CAM

uint8_t counter = 0;
// Pines LoRa
#define LORA_SS 17
#define LORA_RST 16
#define LORA_DIO0 4

// Pin del LED incorporado
#define LED_BUILTIN 2  // El LED incorporado en la mayoría de las ESP32 está en el pin 2

void sendLoRaMessage(const char* message);
void receiveEvent(int bytes);

void setup() {
  // Inicializar el LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Apagado inicialmente
  
  Wire.begin(SDA, SCL, I2C_SLAVE_ADDR); // Configura ESP32 DevKit como esclavo en I2C
  Wire.onReceive(receiveEvent); // Ejecuta cuando recibe datos
  strcpy(message, " ");

  Serial.begin(115200);
  Serial.println("Inicializando LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
  if (!LoRa.begin(433E6)) {
    Serial.println("Error al inicializar LoRa.");
    while (1);
  }
  Serial.println("LoRa inicializado correctamente.");

  // Configuración de LoRa
  //LoRa.setPreambleLength(6);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.enableCrc();
  LoRa.setSyncWord(0xAA);
  Serial.println("LoRa inicializado con éxito.");
  
  // Parpadeo inicial para indicar inicio correcto
  for(uint8_t i=0; i<3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void loop() {
  // Verificar si hay algún paquete recibido
  uint8_t packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Encender LED para indicar recepción
    digitalWrite(LED_BUILTIN, HIGH);
    
    // Procesar los datos recibidos
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.print("Mensaje recibido: ");
    Serial.println(incoming);
    
    // Mantener el LED encendido brevemente
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  // Enviar mensaje periódicamente
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 500) {  // Enviar cada 5 segundos
    Serial.println("Paquete enviado");
    sendLoRaMessage("Hola\0");
    Serial.println("====================================");
    lastSendTime = millis();
  }
}

// send packet
void sendLoRaMessage(const char* message) {
  // Encender LED para indicar transmisión
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.print("Enviando mensaje por LoRa: ");
  Serial.println(message);

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  
  // Mantener el LED encendido brevemente
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
}

void receiveEvent(int bytes) {
  uint8_t i = 0;
  Serial.print("Recibido: ");
  while (Wire.available()) {
    message[i++] = Wire.read();
    Serial.print(message[i - 1]); // Imprime carácter por carácter
  }
  Serial.println(); 
}