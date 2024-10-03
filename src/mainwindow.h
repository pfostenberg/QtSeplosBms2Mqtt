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
    void doStatusMsg(const char *msg);
    void guiConnected( bool connected);
    void writeTextFile(QString fn,QString data);


public slots:
    void doConnect();
    void doDisconnect();
    void UpdateCell(int no, int value);
    void UpdateDouble(int no, double value);

    void csMqttHost(const QString &value);
    void csMqttPort(const QString &value);
    void csStart(const QString &value);
    void csEnd(const QString &value);
    void csDelay(const QString &value);
    void csAuto(const QString &value);
private:
    Ui::MainWindow *ui;
    setting* m_settings;
    Seplos                m_Seplos;
    QList<QSerialPortInfo> m_Ports;

    int     m_StateCount;
    int     m_TestCount;
    QString m_CsvData;
    int     m_CsvTimeout;

    int counter;
    int m_MaxCnt;
};
#endif // MAINWINDOW_H
