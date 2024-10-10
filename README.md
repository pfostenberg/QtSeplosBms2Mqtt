# QtSeplosBms2Mqtt

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
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5serialport5-dev libmosquitto-dev mosquitto-clients
```

### Kompilieren und Starten des Programms

1. **Klone das Repository auf deinem Raspberry Pi:**

```bash
git clone https://github.com/dein-git-repo-url/QtSeplosBms2Mqtt.git
cd QtSeplosBms2Mqtt
```
2. **Erstelle das Programm mit qmake und make:**

```bash
qmake
make
```
Nach der erfolgreichen Kompilierung wird die ausführbare Datei im Verzeichnis erstellt.
Konfiguration

Das Programm verwendet eine INI-basierte Konfiguration. Du musst die Datei config.ini bearbeiten, um deine spezifischen Einstellungen zu konfigurieren.

1. **MQTT-Einstellungen: Trage die MQTT-Serverdaten ein:**

```bash
[MQTT]
Host=dein-mqtt-server
Port=1883
Username=dein-username
Password=dein-passwort
```
2. **RS485-Einstellungen: Konfiguriere den RS485-Port:**

```bash

    [RS485]
    Port=/dev/ttyUSB0
    BaudRate=9600
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