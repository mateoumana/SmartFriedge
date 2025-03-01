#include <SPI.h>
#include <LoRa.h>

int counter = 0;
// Pines LoRa
#define LORA_SS 17
#define LORA_RST 16
#define LORA_DIO0 23

// Pin del LED incorporado
#define LED_BUILTIN 2  // El LED incorporado en la mayoría de las ESP32 está en el pin 2

void sendLoRaMessage(const char* message);

void setup() {
  // Inicializar el LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Apagado inicialmente
  
  Serial.begin(115200);
  Serial.println("Inicializando LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
  if (!LoRa.begin(433E6)) {
    Serial.println("Error al inicializar LoRa.");
    while (1);
  }

  //LoRa.setPreambleLength(6);
  Serial.println("LoRa inicializado correctamente.");
  // Configuración de LoRa
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.enableCrc();

  LoRa.setSyncWord(0xAA);

  Serial.println("LoRa inicializado con éxito.");
  
  // Parpadeo inicial para indicar inicio correcto
  for(int i=0; i<3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void loop() {
  // Verificar si hay algún paquete recibido
  int packetSize = LoRa.parsePacket();
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