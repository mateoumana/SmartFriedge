#include <stdio.h>
#include <pigpio.h>
#include <errno.h>
#include <wiringPi.h>//yo agregué
#include "LoRa.h"

#define DIO0 0  // 0= pin 17 para wiringPi

// Bandera para indicar que hay datos por procesar
volatile int packetReceivedFlag = 0;

// Función de interrupción personalizada
void LoRa_ISR(void) {
    printf("ISR executed\n");
    packetReceivedFlag = 1;  // Marca que hay un paquete recibido
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

    // set Pin 17/0 generate an interrupt on high-to-low transitions
    // and attach myInterrupt() to the interrupt
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
