#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include "setting.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_Ports = QSerialPortInfo::availablePorts();
    qDebug() << "Ports" << m_Ports.size();

    foreach( QSerialPortInfo item, m_Ports )
    {
        QString pn = item.portName();
        qDebug() << pn;
        ui->cbPorts->addItem(pn);
        // will items always be processed in numerical order by index?
        // do something with "item";
    }

    counter = 1;

    m_StateCount = 0;
    m_TestCount = 0;
    m_MaxCnt = 65530;  // max 65535 !!

    connect(ui->pbConnect,     SIGNAL (clicked()),this, SLOT (doConnect()));
    connect(ui->pb_Disconnect, SIGNAL (clicked()),this, SLOT (doDisconnect()));
    connect(&m_Seplos, SIGNAL (UpdateCell(int, int)),this, SLOT (UpdateCell(int, int)));
    connect(&m_Seplos, SIGNAL (UpdateDouble(int, double)),this, SLOT (UpdateDouble(int, double)));


    m_settings = settingProvider();
    ui->le_MqttHost->setText(m_settings->getMqttHost());
    ui->le_MqttPort->setText(QString::number(m_settings->getMqttPort()));
    ui->le_Start->setText(QString::number(m_settings->getStartNo()));
    ui->le_End->setText(QString::number(m_settings->getEndNo()));
    ui->le_DelayMs->setText(QString::number(m_settings->getWaitTimeMs()));
    ui->le_Autostart->setText(QString::number(m_settings->getAutoStartDelayMs()));

    // when user changes in gui -> write to class no password and username!
    // also ini is not save... change only for test purpose
    connect(ui->le_MqttHost , SIGNAL(textChanged(const QString &)), this, SLOT(csMqttHost(const QString &)));
    connect(ui->le_MqttPort , SIGNAL(textChanged(const QString &)), this, SLOT(csMqttPort(const QString &)));
    connect(ui->le_Start    , SIGNAL(textChanged(const QString &)), this, SLOT(csStart(const QString &)));
    connect(ui->le_End      , SIGNAL(textChanged(const QString &)), this, SLOT(csEnd(const QString &)));
    connect(ui->le_DelayMs  , SIGNAL(textChanged(const QString &)), this, SLOT(csDelay(const QString &)));
    connect(ui->le_Autostart, SIGNAL(textChanged(const QString &)), this, SLOT(csAuto(const QString &)));

    guiConnected(false);

    uint32_t delay = settingProvider()->getAutoStartDelayMs();

    if (delay >0)
    {
        QString msg = QString("Autostart in %1 sec").arg(delay/1000);
        ui->l_StatusLine->setText(msg);
        QTimer::singleShot(3000, this, [this]{doConnect();});
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doStatusMsg(const char *msg)
{
    qDebug() << "doStatusMsg: " <<msg;
    ui->l_StatusLine->setText(msg);
}

void MainWindow::doDisconnect()
{
    m_Seplos.close();
    doStatusMsg("Disconnected");
}

void MainWindow::doConnect(void)
{
    int pos = ui->cbPorts->currentIndex();
    QSerialPortInfo spi = m_Ports.at(pos);
    QString pn = spi.portName();

    bool open = m_Seplos.doConnect(spi,m_settings);

    if (open) {
        doStatusMsg("Connected!");
        m_StateCount = 0;
        qDebug() << m_Seplos.ts()  << "start timer";
        guiConnected(true);
    } else {
        qDebug() << "FAILED connect" << pn << " open=false ";
        guiConnected(false);
    }
}

void MainWindow::writeTextFile(QString fn,QString data)
{
    QFile file(fn);
      if(file.open(QIODevice::WriteOnly | QIODevice::Text))
      {
          // We're going to streaming text to the file
          QTextStream stream(&file);

          stream << data;
          file.close();
          qDebug() << "file:" << fn << "Writing finished";
      }
}
// START

void MainWindow::guiConnected( bool connected)
{
    ui->pb_Disconnect->setDisabled(!connected);
}

void MainWindow::csMqttHost(const QString &value)
{
    m_settings->setMqttHost(value);
}

void MainWindow::csMqttPort(const QString &value)
{
    m_settings->setMqttPort(value.toInt());
}

void MainWindow::csStart(const QString &value)
{
    m_settings->setStartNo(value.toInt());
}

void MainWindow::csEnd(const QString &value)
{
    m_settings->setEndNo(value.toInt());
}

void MainWindow::csDelay(const QString &value)
{
    m_settings->setWaitTimeMs(value.toInt());
}

void MainWindow::csAuto(const QString &value)
{
    m_settings->setAutoStartDelayMs(value.toInt());
}

void MainWindow::UpdateDouble(int no, double value)
{
   // qDebug() << "UpdateDouble:" << no << " double " << value;
    QLabel *label = nullptr;
    QLCDNumber *lcd = nullptr;
    QString name= "";

    switch(no) {
    case  0: label = ui->l_T01; break;
    case  1: label = ui->l_T02; break;
    case  2: label = ui->l_T03; break;
    case  3: label = ui->l_T04; break;
    case  4: label = ui->l_T05; break;
    case  5: label = ui->l_T06; break;

    case 7:  lcd = ui->lcd_I; break;  // tcurr
    case 8:  lcd = ui->lcd_V; break;

    case 9: label = ui->l_ahmax; name="AhMax"; break;
    case 11: label = ui->l_ahact; name="AhAct"; break;

    case 12:  lcd = ui->lcd_SOC; break;

    case 13: label = ui->l_rcap; name="rcap"; break;
    case 14: label = ui->l_cycle; name="cycle"; break;
    case 15: label = ui->l_soh; name="soh "; break;
    case 16: label = ui->l_pvolt; name="pvolt"; break;

    default:
        label = ui->l_C16; break;

    }
    char buffer[255];

    if (label) {
        if (name.size() > 2)
        {
            sprintf(buffer,"%8s %3.2f",name.toLocal8Bit().constData(),value);
            label->setText(buffer);
            return;
        } else {
            sprintf(buffer,"Temp%02d %3.2f",no,value);
            label->setText(buffer);
            return;
        }
    }


    if (lcd != nullptr) {

        lcd->display(value);
    }

}

void MainWindow::UpdateCell(int no, int value)
{
    if (no == 42) {
        qDebug() << "UpdateCell:" << "MQTT Status" << " -> " << value;
        switch (value) {
        case 0: ui->l_MqttStatus->setText("MQTT disconneced"); break;
        case 1: ui->l_MqttStatus->setText("MQTT connecting"); break;
        case 2: ui->l_MqttStatus->setText("MQTT connected"); break;
        }
        return;
    }
    if (no == 43) {
        qDebug() << "UpdateCell:" << "Status" << " -> " << value;
        char buffer[255];
        sprintf(buffer, "Actual No: %d",value);
        doStatusMsg(buffer);
        return;
    }
//    qDebug() << "UpdateCell:" << no << " mV " << value;
    QLabel *label;
    switch(no) {
    case  1: label = ui->l_C01; break;
    case  2: label = ui->l_C02; break;
    case  3: label = ui->l_C03; break;
    case  4: label = ui->l_C04; break;
    case  5: label = ui->l_C05; break;
    case  6: label = ui->l_C06; break;
    case  7: label = ui->l_C07; break;
    case  8: label = ui->l_C08; break;
    case  9: label = ui->l_C09; break;
    case 10: label = ui->l_C10; break;
    case 11: label = ui->l_C11; break;
    case 12: label = ui->l_C12; break;
    case 13: label = ui->l_C13; break;
    case 14: label = ui->l_C14; break;
    case 15: label = ui->l_C15; break;
    default:
             label = ui->l_C16; break;

    }
    char buffer[255];
    sprintf(buffer,"Cell%02d %4d",no,value);
    label->setText(buffer);


}
