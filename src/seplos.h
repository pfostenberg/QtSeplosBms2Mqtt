#ifndef SEPLOS_H
#define SEPLOS_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>
#include <QTimer>
#include <QMqttClient>

class Seplos : public QObject
{
    Q_OBJECT
public:
    explicit Seplos(QObject *parent = nullptr);
    bool doConnect(QSerialPortInfo spi);
    void close();
    void doTx(QByteArray data);
    void modbusBuildCrcAndCrThenSend(QString data);
    QString ts(void);
signals:
    void UpdateCell(int no, int value);
    void UpdateDouble(int no, double value);

public slots:
    void rsReadFunction();
    void  processRx(void);
    void doTimer();
    void updateLogStateChange();

signals:


private:
    void oneLineRx(QString);
    void processProV20(QString);

    QSerialPort            m_Rs232;
    QByteArray             m_RxData;
    QTimer                 m_Timer;
    int                    m_TimerState;
    QMqttClient            m_MqttClient;  // https://doc.qt.io/qt-6/qtmqtt-simpleclient-example.html

};

#endif // SEPLOS_H
