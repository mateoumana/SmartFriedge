//#include <stdlib.h>
//#include <stdio.h>
//#include <stdint.h>
//#include <string.h>
//#include <WiringPi.h>
#include "LoRa.h"

void * rx_f(void *p){
    rxData *rx = (rxData *)p;
    printf("rx done \n");
    printf("CRC error: %d\n", rx->CRC);
    printf("Data size: %d\n", rx->size);
    printf("string: %s\n", rx->buf);//Data we'v received
    printf("RSSI: %d\n", rx->RSSI);
    printf("SNR: %f\n", rx->SNR);
    free(p);
    for(int i = 0; i < rx->size; i++){
        printf("%02X", rx->buf[i]);
    }
    printf("\n");
    return NULL;
}

int main(){
    LoRa_ctl modem;

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    modem.rx.callback = rx_f;
    modem.eth.preambleLen=6;
    modem.eth.bw = BW125;//Bandwidth 62KHz
    modem.eth.sf = SF12;//Spreading Factor 12
    modem.eth.ecr = CR8;//Error coding rate CR4/8
    modem.eth.CRC = 1;//Turn on CRC checking
    modem.eth.freq = 433000000;// 434.8MHz
    modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pi
    modem.eth.dio0GpioN = 17;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = 0;//Explicit header mode
    modem.eth.syncWord = 0xAA;
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf
    
    LoRa_begin(&modem);
    LoRa_receive(&modem);
    if(LoRa_check_conn(&modem)){
        printf("si\n");
    }else{
        printf("no\n");
    }
    printf("Current Mode: %d\n",LoRa_get_op_mode(&modem));
    sleep(60);
    printf("Current Mode: %d\n",LoRa_get_op_mode(&modem));
    printf("rx done \n");
    printf("CRC error: %d\n", modem.rx.data.CRC);
    printf("Data size: %d\n", modem.rx.data.size);
    printf("string: %s\n", modem.rx.data.buf);
    printf("RSSI: %d\n", modem.rx.data.RSSI);
    printf("SNR: %f\n", modem.rx.data.SNR);
    printf("end\n");
    LoRa_end(&modem);
}

