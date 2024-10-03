# QtSeplosBms2Mqtt

#Motivation
I have a couple of Seplos 280Ah DIY accu packs.
And monitoring with bash script (query_seplos_ha.sh) does not work stable on new devices.
So I create this QT APP ( should work with QT5 and QT6) on my side 
on a PI with RS485 iso isulated USB transeiver.

with there app i can run this on the PI on VNC.
see the values and the send to mqtt server with following grafana system.

The system config ist ini based 


# compile on PI 

sudo apt-get install  qtbase5-dev  qt5-qmake qtbase5-dev-tools libqt5serialport5 libqt5serialport5-dev qtbase5-private-dev

install qtmqtt ( on my side 5.15.1)
to check use 
  dpkg -l qt5-qmake-bin 

see http://www.diy.ind.in/linux/31-install-mqtt-module-in-qt

git clone --branch v5.15.1 git://code.qt.io/qt/qtmqtt.git

cd qtmqtt
qmake qtmqtt.pro
make install

