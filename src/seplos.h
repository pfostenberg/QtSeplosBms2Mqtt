#ifndef USB2CAN_H
#define USB2CAN_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>
#include <QTimer>
#include <QMqttClient>

class Usb2Can : public QObject
{
    Q_OBJECT
public:
    explicit Usb2Can(QObject *parent = nullptr);
    bool doConnect(QSerialPortInfo spi);
    void close();
    void doTx(QByteArray data);
    void modbusBuildCrcAndCrThenSend(QString data);
    QString ts(void);


public slots:
    void rsReadFunction();
    void  processRx(void);
    void doTimer();
    void updateLogStateChange();

signals:
    void CanRx(long id,QByteArray data);
    void CanReady();

private:
    void oneLineRx(QString);
    void processProV20(QString);

    QSerialPort            m_Rs232;
    QByteArray             m_RxData;
    QTimer                 m_Timer;
    int                    m_TimerState;
    QMqttClient            m_MqttClient;  // https://doc.qt.io/qt-6/qtmqtt-simpleclient-example.html

};

#endif // USB2CAN_H
