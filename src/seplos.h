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
    void RequestPortReopen(void);

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
    void processProV30(int adr ,QByteArray ba);
    void pollV3(int no, int baseAdr);
    bool sendMqttPublish(int adr,int no, int subno, double value, int dezimals);
    double getUintFromBa(QByteArray &ba, int len, double mult , double offset = 0.0);
    double getIntFromBa(QByteArray &ba, int len, double mult );

    QSerialPort            m_Rs232;
    bool                   m_V3_Protocol;
    int                    m_V3_ActAdr;

    QByteArray             m_RxData;
    QTimer                 m_Timer;
    uint32_t               m_TimerState;
    uint32_t               m_ActAdr;
    QMqttClient            m_MqttClient;  // https://doc.qt.io/qt-6/qtmqtt-simpleclient-example.html
    uint32_t               m_OpenTx;

};

#endif // SEPLOS_H
