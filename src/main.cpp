#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
//#include <stdlib.h>
#include "FS.h"
#include "LittleFS.h"
#include <ctype.h>
#include <U8g2lib.h>

#define FONT u8g2_font_wqy14_t_gb2312b
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 22, 21, U8X8_PIN_NONE);//CLK; DATA; RESET


#define VOCAB_SIZE 150
#define EMB_DIM 4
#define MAX_ENT 3

void read_file(String ,int ,int ,float *);// should start with /
uint64_t hash1(String);
uint64_t hash2(uint64_t);
void encoding_hashing_trick();
void onehot();
void processEnt();
//void embedding();// se traen los coeficientes desde colab, usando en colab mi propia codificación fija, que no varíe en cada entrenamiento
void multMatriz(float *, uint8_t, uint8_t, float *, uint8_t, float *);
void flatten();
float sigmoid(float);

String entrada;
uint8_t ent_encoded[MAX_ENT] = {0};
float ent_encod_onehot[MAX_ENT][VOCAB_SIZE] = {0};
float emb[VOCAB_SIZE][EMB_DIM] = {0};
float output_emb[MAX_ENT][EMB_DIM] = {0};
float flatted[1][MAX_ENT*EMB_DIM] = {0};
float dense_weights[MAX_ENT*EMB_DIM + 1][1] = {0};//+ 1 por el sesgo
float sesgo = 0;
float y = 0;//para tener ya sumado el sesgo una vez empiece a sumar en la capa densa


void setup() {
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(FONT);	// choose a suitable font
  if (!LittleFS.begin(true)) {
    Serial.println("Error to initiate LittleFS");
    return;
    //para cargar el sistema de archivos a la esp, darle en el icono de platformio, luego en PROJECT TASKS;
    //luego en Platform/Buils Filesystem Image y por ultimo Platform/Upload Filesystem Image
  }

  Serial.println("\n=== File system info ===");
  Serial.printf("Total space: %d\n",LittleFS.totalBytes());
  Serial.printf("Total space used: %d\n",LittleFS.usedBytes());
  Serial.println("========================\n");

  read_file("/capa_embedding.txt",EMB_DIM,VOCAB_SIZE,(float *)emb);
  read_file("/capa_dense.txt",1,MAX_ENT*EMB_DIM+1,(float *)dense_weights);
  sesgo = dense_weights[0][0]; //el primer dato es el sesgo
  delay(1000); 
}


void loop() {
  if (Serial.available()){ 
    processEnt();
    u8g2.drawStr(20,45,"I");
    u8g2.setCursor(25,45); u8g2.print(String(ent_encoded[0]));
    u8g2.setCursor(55,45); u8g2.print(String(ent_encoded[1]));
    u8g2.setCursor(85,45); u8g2.print(String(ent_encoded[2]));
    u8g2.drawStr(107,45,"I");
    u8g2.sendBuffer(); 

    //Ejecucion del modelo
    y = 0;
    onehot();
    multMatriz((float *)ent_encod_onehot, VOCAB_SIZE, MAX_ENT, (float *)emb, EMB_DIM, (float *)output_emb);//salida capa embedding
    flatten();//salida capa flaten
    multMatriz((float *)flatted, MAX_ENT*EMB_DIM, 1, (float *)(dense_weights+1), 1, &y);
    y += sesgo;//salida capa densa
    //Serial.printf("Salida capa densa + sesgo: %.8f\n",y);
    y = sigmoid(y); //paso por función de activación
    Serial.printf("Probabilidad: %.8f\n",y);
    u8g2.drawStr(15,55,"Probabilidad:");
    u8g2.setCursor(95, 55); u8g2.print(String(y));
    u8g2.sendBuffer();
  }
}

void read_file(String path, int size_x, int size_y, float *matriz){
  File archivo = LittleFS.open(path, "r");
  String word;
  uint8_t indx = 0;
  if (!archivo) {
    Serial.println("¡No se pudo abrir el archivo!");
    return;
  }
  for(int i = 0; i < size_y; i++){
    word = archivo.readStringUntil('\n');
    for(int j = 0; j < size_x; j++){
      if(word.indexOf(",",indx) != -1){ //=-1 cuando no existe el caracter en la cadena
        *(matriz + i * size_x + j) = word.substring(indx,word.indexOf(',',indx)).toFloat();
        indx = word.indexOf(',',indx) + 1;
      }else{
        *(matriz + i * size_x + j) = word.substring(indx,word.indexOf('\n', indx)).toFloat();
        indx = 0;
      }
      //Serial.printf("%.6f,",*(matriz + i * size_x + j));
    }
    //Serial.println();
  }
}

uint64_t hash1(String str){
  uint64_t hash = 7919;
  int c;
  int i = 0;
  while ((c = str[i])) {
      hash = ((hash << 5) + hash) + c;
      i+=1;
  }
  return hash;
}

uint64_t hash2(uint64_t hash){
  return hash * 65599; //otro numero primo grande se necesita un int de 64bits, si no trunca el resultado
}

void encoding_hashing_trick(){
  int ind_str = 0;
  ent_encoded[0] = 0; ent_encoded[1] = 0; ent_encoded[2] = 0;
  for(int i = 0; i < MAX_ENT; i++){
    if(entrada.indexOf(' ', ind_str) != -1){
      ent_encoded[i] = hash2(hash1(entrada.substring(ind_str,entrada.indexOf(' ', ind_str)))) % VOCAB_SIZE;
      ind_str = entrada.indexOf(' ', ind_str) + 1;
    }else{
      ent_encoded[i] = hash2(hash1(entrada.substring(ind_str,entrada.indexOf('\0')-1))) % VOCAB_SIZE;
      break;//para la ultima substring se le quita el ultimo simbolo que es '\0', para que no afecte el hash
    }
  }
}

void onehot(){
  memset(ent_encod_onehot, 0, sizeof(ent_encod_onehot));//reiniciar la matriz
  for(int i = 0; i < MAX_ENT; i++){
    ent_encod_onehot[i][ent_encoded[i]] = 1;
  }
}

void processEnt(){
  entrada = Serial.readStringUntil('\n');  // Lee todo el String recibido
  entrada.toLowerCase();
  Serial.print("Recibido: ");
  Serial.println(entrada);
  u8g2.clearBuffer();					    // clear the internal memory
  u8g2.drawStr(20,15,"Max. 3 Palabras:");	// write something to the internal memory
  u8g2.setCursor(20, 30);
  u8g2.print(entrada);
  u8g2.sendBuffer();              // transfer internal memory to the display
  encoding_hashing_trick();
}

void multMatriz(float *mat1, uint8_t sizex1_y2, uint8_t sizey1, float *mat2, uint8_t sizex2, float *result) {
  for (int i = 0; i < sizey1; i++) {
    for (int j = 0; j < sizex2; j++) {
      *(result + i*sizex2 + j) = 0; //reiniciar la matriz
      for (int k = 0; k < sizex1_y2; k++) {
        *(result + i*sizex2 + j) += *(mat1 + i*sizex1_y2 + k) * *(mat2 + k*sizex2 + j);  // Producto fila-columna
      }
    }
  }
}

void flatten(){
  for(int i = 0; i < MAX_ENT; i++){
    for(int j = 0; j < EMB_DIM; j++){
      flatted[0][i * EMB_DIM + j] = output_emb[i][j];
    }
  }
}

float sigmoid(float z) {
  return 1.0 / (1.0 + exp(-z));
}