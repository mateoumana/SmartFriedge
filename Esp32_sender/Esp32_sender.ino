#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//OLED
#define SCREEN_WIDTH 128  // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64  // Alto de la pantalla OLED
#define OLED_ADDR 0x3C  // Dirección I2C de la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);

//Slave I2C Configuration
#define I2C_SLAVE_ADDR 0x08
#define SDA 21
#define SCL 22

//Master I2C Configuration
#define FREQ_I2C 100000
#define SDA_2 27
#define SCL_2 26

uint8_t counter = 0;
// Pines LoRa
#define LORA_SS 17
#define LORA_RST 16
#define LORA_DIO0 4

//botones para la accion
#define BTN1 32
#define BTN2 33
#define BTN3 35
bool lastState1 = LOW, lastState2 = LOW, lastState3 = LOW;

// Pin del LED incorporado
#define LED_BUILTIN 2  // El LED incorporado en la mayoría de las ESP32 está en el pin 2

void oled_init();
void oled_show_message(const char* message, uint8_t x, uint8_t y);
void sendLoRaMessage(const char* message);
void receiveEvent(int bytes);//INTERRUPT I2C

void setup() {
  // Inicializar el LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Apagado inicialmente
  
  // Inicializar el I2C esclavo (ESP32CAM), por defecto sda es 21 y scl 22, si se definen en el constructor no funciona
  Wire.begin(I2C_SLAVE_ADDR); // Configura ESP32 DevKit como esclavo en I2C
  Wire.setClock(FREQ_I2C); // 100 kHz
  Wire.onReceive(receiveEvent); // Llama a la función cuando recibe datos
  // Inicializar el I2C maestro (OLED), por defecto sda es 21 y scl 22, si se definen en el constructor no funciona
  Wire1.begin(SDA_2, SCL_2, FREQ_I2C);
  oled_init();

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
  oled_show_message("Seleccione",50,15);
  oled_show_message("una accion",50,30);
  //lectura de botones
  bool currentState1 = digitalRead(BTN1);
  bool currentState2 = digitalRead(BTN2);
  bool currentState3 = digitalRead(BTN3);
  if (lastState1 == LOW && currentState1 == HIGH) {
      Serial.println("Botón 1 presionado");
  }
  if (lastState2 == LOW && currentState2 == HIGH) {
      Serial.println("Botón 2 presionado");
  }
  if (lastState3 == LOW && currentState3 == HIGH) {
      Serial.println("Botón 3 presionado");
  }
  // Actualizar estados anteriores
  lastState1 = currentState1; lastState2 = currentState2; lastState3 = currentState3;

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

void oled_init() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("Error al iniciar pantalla OLED");
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("OLED Lista!");
    display.display();
    delay(2000);
}

void oled_show_message(const char* message, uint8_t x, uint8_t y) {
    display.clearDisplay();
    display.setCursor(60, 30);
    display.println(message);
    display.display();
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
  char mssg[256];
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