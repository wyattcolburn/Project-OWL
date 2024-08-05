# Project OWL LORA

Hello,

The work in this project is for Project-OWL Summer Research. The goal of this repo is to interface the raspberry pi with a SX1262 to send/receive LoRa packets.

The project is written in C and leverages the LG library for GPIO control. The LG library is based of the linux kernel driver so will work with the new raspberry pi5.

To install LG indepent to this project:

wget http://abyz.me.uk/lg/lg.zip  
unzip lg.zip  
cd lg  
make  
sudo make install

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
