#ifndef SEPLOS_H
#define SEPLOS_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>
#include <QTimer>
#include <QMqttClient>
#include "setting.h"

class Seplos : public QObject
{
    Q_OBJECT
public:
    explicit Seplos(QObject *parent = nullptr);
    bool doConnect(QSerialPortInfo spi, setting *si);
    void close();
    void doTx(QByteArray data);
    void modbusBuildCrcAndCrThenSend(QString data);
    QString ts(void);
    void setLastWill();
    void setStatusOnline(bool online);
signals:
    void UpdateCell(int no, int value);
    void UpdateDouble(int no, double value);

public slots:
    void rsReadFunction();
    void  processRx(void);
    void doTimer();
    void updateLogStateChange();
    void pollTelemetrie(int adr);
signals:


private:
    void oneLineRx(QString);
    void processProV20(QString);
    bool sendMqttPublish(int adr,int no, int subno, double value, int dezimals);

    QSerialPort            m_Rs232;
    QByteArray             m_RxData;
    QTimer                 m_Timer;
    uint32_t               m_TimerState;
    uint32_t               m_ActAdr;
    QMqttClient            m_MqttClient;  // https://doc.qt.io/qt-6/qtmqtt-simpleclient-example.html

};

#endif // SEPLOS_H
