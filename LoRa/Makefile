all: Raspi_receiver receive_implicit receive_explicit transmit_implicit transmit_explicit ping pong

LoRa.o: LoRa.c
	gcc -c LoRa.c -o LoRa.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3

tx_implicit_example.o: tx_implicit_example.c
	gcc -c tx_implicit_example.c -o tx_implicit_example.o -lpigpio -lrt -pthread -lm

rx_implicit_example.o: rx_implicit_example.c
	gcc -c rx_implicit_example.c -o rx_implicit_example.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3
	
Raspi_receiver.o: Raspi_receiver.c
	gcc -c Raspi_receiver.c -o Raspi_receiver.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3

tx_explicit_example.o: tx_explicit_example.c
	gcc -c tx_explicit_example.c -o tx_explicit_example.o -lpigpio -lrt -pthread -lm

rx_explicit_example.o: rx_explicit_example.c
	gcc -c rx_explicit_example.c -o rx_explicit_example.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3
	
ping.o: ping.c
	gcc -c ping.c -o ping.o -lpigpio -lrt -pthread -lm
	
pong.o: pong.c
	gcc -c pong.c -o pong.o -lpigpio -lrt -pthread -lm

ping: LoRa.o ping.o
	gcc -o ping ping.o LoRa.o -lpigpio -lrt -pthread -lm

pong: LoRa.o pong.o
	gcc -o pong pong.o LoRa.o -lpigpio -lrt -pthread -lm
	
transmit_explicit: LoRa.o tx_explicit_example.o
	gcc -o transmit_explicit tx_explicit_example.o LoRa.o -lpigpio -lrt -pthread -lm

transmit_implicit: LoRa.o tx_implicit_example.o
	gcc -o transmit_implicit tx_implicit_example.o LoRa.o -lpigpio -lrt -pthread -lm

receive_explicit: LoRa.o rx_explicit_example.o
	gcc -o receive_explicit rx_explicit_example.o LoRa.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3
	
Raspi_receiver: LoRa.o Raspi_receiver.o
	gcc -o Raspi_receiver Raspi_receiver.o LoRa.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3

receive_implicit: LoRa.o rx_implicit_example.o
	gcc -o receive_implicit rx_implicit_example.o LoRa.o -lpigpio -lrt -pthread -lm -lwiringPi -lsqlite3

clean:
	rm *.o receive_*plicit transmit_*plicit ping pong Raspi_receiver
