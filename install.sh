# install script for Project-OWL quad pro
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install unzip
sudo apt-get install libhiredis-dev

sudo apt-get update
sudo apt-get install redis-server

wget http://abyz.me.uk/lg/lg.zip
unzip lg.zip
cd lg
make
sudo make install


git clone https://github.com/wyattcolburn/Project-OWL.git
