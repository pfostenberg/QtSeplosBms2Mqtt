# QtSeplosBms2Mqtt

German Version see README_DE.md 

## Motivation
This project was developed to monitor Seplos 280Ah DIY battery packs reliably and stably on a Raspberry Pi and to send the data via MQTT to a server. The program runs with Qt5 or Qt6 and uses an RS485-isolated USB transceiver for communication with the batteries.

## Prerequisites

Before starting the installation, ensure that your Raspberry Pi meets the following requirements:

- **Raspberry Pi OS** (tested on Raspberry Pi OS Lite, should also work on desktop versions)
- **Qt5 or Qt6**: Qt libraries for the graphical user interface
  on the PI only Qt5 works easily at this time with the open source version.
  Unter Windows or IOS both will work.
- **RS485 isolated USB transceiver**: For connecting to the Seplos batteries
- **MQTT Broker**: The program sends data via MQTT, so you need an MQTT broker (e.g., Mosquitto)

### Dependencies
Install the necessary packages and dependencies on your Raspberry Pi:

```bash
sudo apt-get update
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5serialport5 libqt5serialport5-dev libmosquitto-dev mosquitto-clients qt5-default qtbase5-private-dev
```

### Compiling and Running the Program

1. **QtMqtt lib must be installed
We need to find out what version is installed exactly.
```bash
dpkg -l qt5-default
```
in my case 5.11.3

install QtMqtt

```bash
git clone --branch v5.11.3 git://code.qt.io/qt/qtmqtt.git
cd qtmqtt
qmake qtmqtt.pro
make install
```

see http://www.diy.ind.in/linux/31-install-mqtt-module-in-qt


2. **Clone the repository to your Raspberry Pi:**

```bash
git clone https://github.com/pfostenberg/QtSeplosBms2Mqtt.git
cd QtSeplosBms2Mqtt
```

3. **Build the program with qmake and make:**

```bash
qmake QtSeplosBms2Mqtt.pro
make
```

After successful compilation, the executable file will be created in the directory.

### Configuration

1. **MQTT Settings: create INI file:**

The program uses an INI-based configuration.
You need to edit the config.ini file to configure your specific settings.
Die INI must be saved unter /etc/seplos/QtSeplosMqtt.ini.
A sample is under src\QtSeplosMqtt_sample.ini

2. **MQTT Settings: Enter your MQTT server details:**

```bash
MqttHost=dein-mqtt-server
MqttPort=1883
MqttUser=dein-username
MqttPassword=dein-passwort
```

3. **RS485 Settings: Configure the RS485 port:**

```bash
Rs485Dev=/dev/ttyUSB0
or under windows..
Rs485Dev = COM12
```

Ensure that the specified port is correct and the transceiver is connected.

### Running the Program

To run the program, use the following command:

```bash
./QtSeplosBms2Mqtt
```

The program will start collecting data from the Seplos batteries and sending it via MQTT. You can display the user interface on your Pi using a VNC tool.

### Troubleshooting

* **MQTT connection fails:** Check the network connection and MQTT configuration.
* **RS485 connection doesn't work:** Ensure the correct port is configured and the transceiver is properly connected.
* **Missing dependencies:** Make sure all necessary packages are installed, particularly the Qt and MQTT libraries.

### License

This project is licensed under the MIT License. See the LICENSE file for more details.
