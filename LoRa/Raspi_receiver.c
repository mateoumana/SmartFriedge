#include <stdio.h>
#include <pigpio.h>
#include <errno.h>
#include <sqlite3.h>
#include <string.h>
#include <wiringPi.h>
#include "LoRa.h"

#define DIO0 0  // 0= pin 17 para wiringPi
#define DATA_LORA 8//cantidad de datos enviados por lora

// Bandera para indicar que hay datos por procesar
volatile int packetReceivedFlag = 0;

// Función de interrupción personalizada
void LoRa_ISR(void) {
    printf("ISR executed\n");
    packetReceivedFlag = 1;  // Marca que hay un paquete recibido
}

/*void createTables() {
    char *command[];
    sqlite3 *db = NULL;
    
    if(sqlite3_open("~/Desktop/SmartFriedge/dataSmartFriedge.db", &db) != SQLITE_OK){//la direccion del puntero db, abre la db con UTF-8
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    command = "CREATE TABLE IF NOT EXIST Producto ( "; 
    strcat(command, "id_product INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "name TEXT NOT NULL,");
    strcat(command, "fecha_cadu DATE DEFAULT NULL,");
    strcat(command, "codigo_ref TEXT NOT NULL,");
    strcat(command, "tipo TEXT NOT NULL,");//vacuna o medicamento
    strcat(command, "FOREIGGN KEY (id_accion) REFERENCES Accion (id_accion),");
    strcat(command, "FOREIGGN KEY (id_nevera) REFERENCES Nevera (id_nevera));");
    if(sqlite3_exec(db,command, NULL, NULL, NULL) != SQLITE_OK){
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    command = "CREATE TABLE IF NOT EXIST personal ( "; 
    strcat(command, "id_personal INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "name TEXT NOT NULL,");
    strcat(command, "fecha_naci DATE DEFAULT NULL,");
    strcat(command, "edad INTEGER,");
    strcat(command, "code_carnet TEXT NOT NULL,");
    strcat(command, "email TEXT,");
    strcat(command, "cedula TEXT,");
    strcat(command, "cargo TEXT);");
    
    command = "CREATE TABLE IF NOT EXIST Temp_produc ( "; 
    strcat(command, "id_temp INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "fecha DATETIME DEFAULT NULL,");
    strcat(command, "temp INTEGER,");
    strcat(command, "FOREIGGN KEY (id_producto) REFERENCES Producto (id_producto);");
    
    command = "CREATE TABLE IF NOT EXIST Temp_nevera ( "; 
    strcat(command, "id_temp INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "fecha DATETIME DEFAULT NULL,");
    strcat(command, "temp INTEGER,");
    strcat(command, "FOREIGGN KEY (id_nevera) REFERENCES Nevera (id_nevera);");
    
    command = "CREATE TABLE IF NOT EXIST Cant_nevera ( "; 
    strcat(command, "id_cant INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "fecha DATETIME DEFAULT NULL,");
    strcat(command, "cantidad INTEGER,");
    strcat(command, "FOREIGGN KEY (id_nevera) REFERENCES Nevera (id_nevera);");
        
    command = "CREATE TABLE IF NOT EXIST Nevera ( "; 
    strcat(command, "id_nevera INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "temp INTEGER,");
    strcat(command, "cant_actual INTEGER,");
    strcat(command, "sn_nevera TEXT NOT NULL,");
    strcat(command, "no_inventa TEXT NOT NULL);");
    
    command = "CREATE TABLE IF NOT EXIST Accion ( "; 
    strcat(command, "id_accion INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "fecha DATETIME DEFAULT NULL,");
    strcat(command, "accion TEXT NOT NULL,");
    strcat(command, "t_apertura TIME NOT NULL,");
    strcat(command, "FOREIGGN KEY (id_personal) REFERENCES Personal (id_personal),");
    strcat(command, "FOREIGGN KEY (id_producto) REFERENCES Producto (id_producto));");
    
    command = "CREATE TABLE IF NOT EXIST Inspeccion ( "; 
    strcat(command, "id_inspeccion INTEGER AUTOINCREMENT PRIMARY KEY,");
    strcat(command, "fecha DATETIME DEFAULT NULL,");
    strcat(command, "t_apertura TIME NOT NULL,");
    strcat(command, "FOREIGGN KEY (id_personal) REFERENCES Personal (id_personal),");
    strcat(command, "FOREIGGN KEY (id_nevera) REFERENCES Nevera (id_nevera);");
    
    sqlite3_close(db);
}*/

// Función para escribir la base de datos
void writeDataBase(uint8_t *buffer) {
    const char *comm_person ="INSERT INTO dataSmartFriedge.Personal (name, fecha_naci, edad, code_carnet, email, cedula, cargo) VALUES (?, ?, ?, ?, ?, ?, ?);";
    const char *comm_produc ="INSERT INTO dataSmartFriedge.Producto (name, fecha_cadu, codigo_ref, tipo, id_accion, id_nevera) VALUES (?, ?, ?, ?, ?, ?);";
    const char *comm_accion ="INSERT INTO dataSmartFriedge.Accion (fecha, accion, id_personal, id_producto) VALUES (CURRENT_TIMESTAMP, ?, ?, ?);";
    const char *comm_nevera ="INSERT INTO dataSmartFriedge.Nevera (temp, cant_actual, sn_nevera, no_inventa) VALUES (?, ?, ?, ?);";
    const char *comm_inspec ="INSERT INTO dataSmartFriedge.Inspeccion (fecha, id_personal, id_nevera) VALUES (CURRENT_TIMESTAMP, ?, ?);";
    const char *comm_temper ="INSERT INTO dataSmartFriedge.Temperatura (fecha, accion, id_personal, id_producto) VALUES (CURRENT_TIMESTAMP, ?, ?, ?);";
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt;
    char *tokens[DATA_LORA];
    
    tokens[0] = strtok(buffer,",");
    for(int i=1; i < DATA_LORA; i++){
        printf("%s\n",tokens[i]);
        tokens[i] = strtok(NULL,",");
    }
        
    if(sqlite3_open("~/Desktop/SmartFriedge/dataSmartFriedge.db", &db) != SQLITE_OK){//la direccion del puntero db, abre la db con UTF-8
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    
    //prepara la sentencia. -1 indica que el comando termina en '\0'
    if(sqlite3_prepare_v2(db, comm_accion, -1, &stmt, NULL) != SQLITE_OK){//la direccion del puntero db, abre la db con UTF-8
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    //vincula la variable con el parametro ? numero x 
    sqlite3_bind_text(stmt,1,tokens[1],-1,SQLITE_STATIC);//1=primer ?
    //sqlite3_bind_text(stmt,2,);//...
    if(sqlite3_step(stmt) != SQLITE_DONE){//ejecuta la sentencia sql preparada
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);//finaizar la sentencia para liberar recursos
    
    
    sqlite3_close(db);
}

// Función para leer el paquete directamente del SX1278
void readPacketDirectly(LoRa_ctl *modem) {
    uint8_t irqFlags = lora_reg_read_byte(modem->spid, REG_IRQ_FLAGS);
    
    // Verifica si hay un paquete recibido
    if (irqFlags & 0x40) {  // RxDone
        printf("Paquete recibido!\n");

        // Lee el número de bytes recibidos
        uint8_t packetSize = lora_reg_read_byte(modem->spid, REG_RX_NB_BYTES);
        printf("Tamaño del paquete: %d bytes\n", packetSize);

        // Configura el puntero al inicio del FIFO
        lora_reg_write_byte(modem->spid, REG_FIFO_ADDR_PTR, lora_reg_read_byte(modem->spid, REG_FIFO_RX_CURRENT_ADDR));

        // Lee el paquete desde el FIFO
        uint8_t buffer[256] = {0};  // Buffer para el paquete
        for (int i = 0; i < packetSize; i++) {
            buffer[i] = lora_reg_read_byte(modem->spid, REG_FIFO);
        }

        printf("Datos recibidos: ");
        for (int i = 0; i < packetSize; i++) {
            printf("%c ", buffer[i]);
        }
        printf("\n");

        // Limpia las banderas de IRQ
        lora_reg_write_byte(modem->spid, REG_IRQ_FLAGS, 0xFF);

        // Rehabilita recepción (modo continuo)
        LoRa_receive(modem);
        
        //estructura del mensaje QR
        //"name_prod:pfizer,tipo:vacuna,date_cad:12/12/2025,cod_ref:142AS15"
        //estructura del mensaje LoRa (los datos se enviaran sin leyendas, solo separados por coma)
        //buffer="id_person:123546,accion:ingreso,temp_nevera:10,temp_prod:5" + mensaje QR
        //temperatura del producto, en ingreso se calcula con respecto a la anterior, en retiro se toma la ambiente de la nevera
        writeDataBase(buffer);//se tokeniza el buffer en la funcion        
    }
}

// Función principal
int main() {
    LoRa_ctl modem;

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    //modem.rx.callback = rx_f;
    modem.rx.callback = NULL;
    modem.eth.preambleLen=6;
    modem.eth.bw = BW125;//Bandwidth 62KHz
    modem.eth.sf = SF12;//Spreading Factor 12
    modem.eth.ecr = CR8;//Error coding rate CR4/8
    modem.eth.CRC = 1;//Turn on CRC checking
    modem.eth.freq = 433000000;// 434.8MHz
    modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pi
    modem.eth.dio0GpioN = 27;//27=GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = 0;//Explicit header mode
    modem.eth.syncWord = 0xAA;
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

    // sets up the wiringPi library
    if (wiringPiSetup () < 0) {
        fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
        printf("error\n");
        return 1;
    }

    // set Pin 17/0 generate an interrupt on high-to-low transitions and attach myInterrupt() to the interrupt
    if ( wiringPiISR (DIO0, INT_EDGE_RISING, &LoRa_ISR) < 0 ) {
        fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
        printf("error\n");
        return 1;
    }
    
    // Inicializa LoRa
    LoRa_begin(&modem);
    // Coloca al módulo en modo recepción continua
    gpioSetMode(modem.eth.dio0GpioN, PI_INPUT);
    gpioSetPullUpDown(modem.eth.dio0GpioN, PI_PUD_DOWN);
    LoRa_receive(&modem);
    if(LoRa_check_conn(&modem)){
        printf("si\n");
    }else{
        printf("no\n");
    }

    // Bucle principal
    while (1) {
        if (packetReceivedFlag) {
            printf("Recivió algo\n");
            printf("Current Mode: %d\n\n",LoRa_get_op_mode(&modem));
            packetReceivedFlag = 0;  // Limpia la bandera de la ISR
            readPacketDirectly(&modem);  // Procesa el paquete directamente
        }

        delay(100);  // Pequeña pausa para no saturar la CPU
    }

    // Apaga LoRa antes de salir
    LoRa_end(&modem);

    return 0;
}
