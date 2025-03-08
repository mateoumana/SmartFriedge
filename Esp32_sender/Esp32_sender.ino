#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>

//Slave Address
#define I2C_SLAVE_ADDR 0x08
#define SDA 21
#define SCL 22

uint8_t counter = 0;
// Pines LoRa
#define LORA_SS 17
#define LORA_RST 16
#define LORA_DIO0 4

// Pin del LED incorporado
#define LED_BUILTIN 2  // El LED incorporado en la mayoría de las ESP32 está en el pin 2

void sendLoRaMessage(const char* message);
void receiveEvent(int bytes);//INTERRUPT I2C

void setup() {
  // Inicializar el LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Apagado inicialmente
  
  // Inicializar el I2C, por defecto sda es 21 y scl 22, si se definen en el constructor no funciona
  Wire.begin(I2C_SLAVE_ADDR); // Configura ESP32 DevKit como esclavo en I2C
  Wire.setClock(100000); // 100 kHz
  Wire.onReceive(receiveEvent); // Llama a la función cuando recibe datos

  // Inicializar el LoRa
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
  
  /*// Enviar mensaje periódicamente
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 2000) {  // Enviar cada 5 segundos
    Serial.println("Paquete enviado");
    sendLoRaMessage("Hola Raspberry pi\0");
    Serial.println("====================================");
    lastSendTime = millis();
  }*/
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
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
}

void receiveEvent(int bytes) {
  uint8_t i = 0;
  char mssg[30];
  Serial.print("Recibido: ");
  while (Wire.available()) {
    mssg[i] = Wire.read();
    if(mssg[i] == '\0') break;
    i++;
  }
  mssg[i] = '\0'; //asegurar el final del string
  Serial.println("Paquete enviado");
  sendLoRaMessage(mssg);
  Serial.println("====================================");
}