# Project OWL LORA

Hello,

The work in this project is for Project-OWL Summer Research. The goal of this repo is to interface the raspberry pi with a SX1262 to send/receive LoRa packets.

The project is written in C and leverages the LG library for GPIO control. The LG library is based of the linux kernel driver so will work with the new raspberry pi5.

## Cloning the Repo
```
git clone https://github.com/wyattcolburn/Project-OWL.git
chmod +X install.sh
bash install.sh
```
### Running Spi Loopback Test
 The first program you should run is spi_loopback_test
 Make sure to short MOSI to MISO (GPIO PINS 10 --> GPIO 9)
 ```
 ./spi_loopback_test
```
### File Transfer Test
To simulate the message broker, first the receive and transmit functionality will be tested using files to read messages and write messages. 
This is done in main. Two sx1262 are required, a wiring diagram is linked below, one will run file_test_rx.c and one will run file_test_tx.c. 
The two .txt files simulate the webserver, with input.txt being the user input (transmitted message) and output.txt being what is sent to the user interface (received message). 

```
cd Project-OWL
cd src
cd main
```
### First Run file_test_rx.c in order to not have to worry about missing the transmission.The program will receive a LoRa packet, decode the CDP packet, and write to output.txt
```
./writeFile
```
### The run file_test_tx.c to transmit. The program will read the contents of input.txt, generate a CDP packet and then transmit. 
```
./readFile
```
## Editing files
If you wish to transmit a different message: change the input.txt file. 
If you wish to change the wiring, edit sx1262.h
If you wish to change LORA paramaters, edit sx1262.h

After making changes compile the code with the following commands: the first for tx, and the second for rx portion.
```
gcc file_test_tx.c helpFunctions.c cdp.c sx1262.c -o readFile -llgpio
```
```
gcc file_test_rx.c helpFunctions.c cdp.c sx1262.c -o writeFile -llgpio
```

### Running Example Code 
#### RX
Naviagate to the rx directory
For one-shot mode, in which you would receive only one message
```
cd files
cd rx
./receive
```
To test continous mode where the program will run until cancelled and receive messages
```
cd files
cd rx
./receiveCont
```
#### TX
Navaiagte to tx directory
```
cd files
cd tx
./transmit "your message"
```

## Install GPIO DRIVER
To install LG indepent to this project:

``````wget http://abyz.me.uk/lg/lg.zip  
unzip lg.zip  
cd lg  
make  
sudo make install
``````
If you are cloning this repo lg is already installed:
:
cd lg
make 
sudo make install

Source Files are found in /Project-OWL/lg/src

There is a spi_loopback test to ensure spi operations are working, the received data
should be 0x11, 0x12, 0x13

There are rx and tx directories with basic examples. cd into the respective folder and run the file. ./transmit and ./receive 

If you wish to send different messages just edit the transmit.c, line 66 and 68 are the only lines required to edit

After editting a file you must recompile:

Use this command to compile:

gcc filename.c helpFunctions.c -o exeFilename -llgpio

Run the program with:

./exeFilename

If any issues arise please create an issue on the github repo and email me at 
wdcolbur@calpoly.edu
