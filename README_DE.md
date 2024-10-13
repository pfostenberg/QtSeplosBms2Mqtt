# QtSeplosBms2Mqtt

English Version see README.md

## Motivation
Dieses Projekt wurde entwickelt, um Seplos 280Ah DIY Akkupacks stabil und zuverlässig auf einem Raspberry Pi zu überwachen und die Daten via MQTT an einen Server zu senden. Das Programm läuft mit Qt5 oder Qt6 und verwendet einen RS485-isolierten USB-Transceiver für die Kommunikation mit den Akkus. 

## Voraussetzungen

Bevor du mit der Installation beginnst, stelle sicher, dass dein Raspberry Pi folgende Anforderungen erfüllt:

- **Raspberry Pi OS** (getestet auf Raspberry Pi OS Lite, sollte auch auf Desktop-Versionen laufen)
- **Qt5 oder Qt6**: Qt-Bibliotheken für die grafische Benutzeroberfläche
- **RS485 isolierter USB-Transceiver**: Für die Verbindung zu den Seplos-Akkus
- **MQTT Broker**: Das Programm sendet Daten über MQTT, daher benötigst du einen MQTT-Broker (z.B. Mosquitto)

### Abhängigkeiten
Installiere die notwendigen Pakete und Abhängigkeiten auf deinem Raspberry Pi:

```bash
sudo apt-get update
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5serialport5-dev libmosquitto-dev mosquitto-clients qt5-default
```

### Kompilieren und Starten des Programms


1. **Qt mqtt lib muss installiert sein / werden.
Wir müssen erst raus bekommen welches Qt5 genau installiert ist.

```bash
dpkg -l qt5-default
```
Bei mir 5.11.3

install QtMqtt

```bash
git clone --branch v5.11.3 git://code.qt.io/qt/qtmqtt.git
cd qtmqtt
qmake qtmqtt.pro
make install
```

see http://www.diy.ind.in/linux/31-install-mqtt-module-in-qt

2. **Klone das Repository auf deinem Raspberry Pi:**

```bash
git clone https://github.com/pfostenberg/QtSeplosBms2Mqtt.git
cd QtSeplosBms2Mqtt
```

3. **Erstelle das Programm mit qmake und make:**

```bash
qmake QtSeplosBms2Mqtt.pro
make
```
Nach der erfolgreichen Kompilierung wird die ausführbare Datei im Verzeichnis erstellt.

### Konfiguration

1. **Kopiere Sample INI:**

Das Programm verwendet eine INI-basierte Konfiguration. 
Du musst die Datei config.ini bearbeiten, um deine spezifischen Einstellungen zu konfigurieren.
Die INI muss bei PI unter /etc/seplos/QtSeplosMqtt.ini speichern.
Ein Beispiel ist unter src\QtSeplosMqtt_sample.ini

2. **MQTT-Einstellungen: Trage die MQTT-Serverdaten ein:**

```bash
MqttHost=dein-mqtt-server
MqttPort=1883
MqttUser=dein-username
MqttPassword=dein-passwort
```
3. **RS485-Einstellungen: Konfiguriere den RS485-Port:**

```bash
Rs485Dev=/dev/ttyUSB0
or under windows..
Rs485Dev = COM12
```
Stelle sicher, dass der angegebene Port korrekt ist und der Transceiver verbunden ist.

### Programm ausführen

Um das Programm auszuführen, verwende den folgenden Befehl:

```bash
./QtSeplosBms2Mqtt
```

Das Programm beginnt damit, Daten von den Seplos-Akkus zu sammeln und sie via MQTT zu versenden. Über ein VNC-Tool kannst du die Benutzeroberfläche auf deinem Pi anzeigen lassen.

### Troubleshooting

*    **MQTT-Verbindung schlägt fehl:** Überprüfe die Netzwerkverbindung und die MQTT-Konfiguration.
*    **RS485-Verbindung funktioniert nicht:** Stelle sicher, dass der richtige Port konfiguriert ist und der Transceiver korrekt angeschlossen ist.
*    **Fehlende Abhängigkeiten:** Achte darauf, dass alle erforderlichen Pakete installiert sind, insbesondere die Qt- und MQTT-Bibliotheken.

### Lizenz

Dieses Projekt steht unter der MIT-Lizenz. Weitere Details findest du in der LICENSE-Datei.