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


    setting* sp = settingProvider();
    ui->le_MqttHost->setText(sp->getMqttHost());
    ui->le_MqttPort->setText(QString::number(sp->getMqttPort()));
    ui->le_Start->setText(QString::number(sp->getStartNo()));
    ui->le_End->setText(QString::number(sp->getEndNo()));
    ui->le_DelayMs->setText(QString::number(sp->getAutoStartDelayMs()));
    ui->le_Autostart->setText(QString::number(sp->getWaitTimeMs()));

    guiConnected(false);

    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL (timeout()), this, SLOT(doTimer()) );

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RsSend(QByteArray data)
{

}

void MainWindow::doStatusMsg(const char *msg)
{
    qDebug() << msg;
    ui->l_StatusLine->setText(msg);
}



void MainWindow::doDisconnect()
{
    m_Timer->stop();
    m_Seplos.close();

    doStatusMsg("Disconnected");
}

void MainWindow::doTx(QByteArray data)
{
    m_Seplos.doTx(data);
}

void MainWindow::doTimer()
{
    /*
    if (m_StateCount < 2000)
    {
        m_StateCount++;
    } else {
        qDebug() << "State Overshoot";
    }
*/
}

void MainWindow::doConnect(void)
{

    m_RndAn = 600; // 60 sec an.
    m_RndAus = 300; // 30 stop

    int pos = ui->cbPorts->currentIndex();
    QSerialPortInfo spi = m_Ports.at(pos);
    QString pn = spi.portName();

    bool open = m_Seplos.doConnect(spi);

    if (open) {
        doStatusMsg("Connected");
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

void MainWindow::UpdateDouble(int no, double value)
{
    qDebug() << "UpdateDouble:" << no << " double " << value;
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
    case 15: label = ui->l_soh; name="soh"; break;
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
    qDebug() << "UpdateCell:" << no << " mV " << value;
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
