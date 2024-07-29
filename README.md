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

cd lg
make 
sudo make install

Source Files are found in /Project-OWL/lg/src

transmit_mode.c is the most current file with functions for rx and tx.

Use this command to compile:

gcc -transmit_mode.c helpFunctions.c -o transmitMode -llgpio

Run the program with:

./transmitMode
