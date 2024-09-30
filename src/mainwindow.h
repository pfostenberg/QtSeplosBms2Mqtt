#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include <QRandomGenerator64>



#include "seplos.h"

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
    void doTimer();
    void UpdateCell(int no, int value);
    void UpdateDouble(int no, double value);

private:
    Ui::MainWindow *ui;

    Seplos                m_Seplos;
    QList<QSerialPortInfo> m_Ports;

    QTimer *m_Timer;
    int     m_StateCount;
    int     m_TestCount;
    QString m_CsvData;
    int     m_CsvTimeout;

    int counter;
    int m_RndAn;
    int m_RndAus;
    int m_MaxCnt;
};
#endif // MAINWINDOW_H
