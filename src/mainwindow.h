#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include <QRandomGenerator64>



#include "usb2can.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void RsSend(QByteArray data);
    void doStatusMsg(const char *msg);
    void doTx(QByteArray tx);
    void guiConnected( bool connected);
    void writeTextFile(QString fn,QString data);


public slots:
    void doConnect();
    void doDisconnect();
    void doStartTest();
    void doStopTest();
    void doTimer();
    void CanRx(long id,QByteArray data);
    void CanReady(void);
    void rpmChanged(const QString &text);
    void rpmIntChanged(int );

private:
    Ui::MainWindow *ui;
    QRandomGenerator m_Rnd;

    Usb2Can                m_Can2Usb;
    QList<QSerialPortInfo> m_Ports;

    QTimer *m_Timer;
    int     m_StateCount;
    int     m_TestCount;
    QString m_CsvData;
    int     m_CsvTimeout;


    int counter;
    int m_DriveMode;
    int m_DriveRpm;      // *250 4=1000  60=15k

    int m_RndAn;
    int m_RndAus;


    int m_MaxCnt;





};
#endif // MAINWINDOW_H
