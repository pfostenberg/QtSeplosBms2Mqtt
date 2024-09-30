#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>

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
    m_DriveMode = 0;
    m_DriveRpm=60;      // *250 4=1000  60=15k
#ifdef LONG_TERM
   m_DriveRpm=24;      // 3k
#endif

    ui->le_Rpm->setText(QString::number(m_DriveRpm*250));
    m_StateCount = 0;
    m_TestCount = 0;
    m_MaxCnt = 65530;  // max 65535 !!

    connect(ui->pbConnect,     SIGNAL (clicked()),this, SLOT (doConnect()));
    connect(ui->pb_Disconnect, SIGNAL (clicked()),this, SLOT (doDisconnect()));
    connect(ui->pb_test,       SIGNAL (clicked()),this, SLOT (doStartTest()));
    connect(ui->pb_Stop,       SIGNAL (clicked()),this, SLOT (doStopTest()));
    connect(ui->le_Rpm,       SIGNAL (textChanged(const QString &)),this, SLOT (rpmChanged(const QString &)));
    connect(ui->sliderRPM,       SIGNAL (valueChanged(int)),this, SLOT (rpmIntChanged(int)));




    connect(&m_Can2Usb,        SIGNAL (CanRx(long ,QByteArray )),this, SLOT (CanRx(long ,QByteArray)));
    connect(&m_Can2Usb,        SIGNAL (CanReady()),this, SLOT (CanReady()));

    guiConnected(false);

    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL (timeout()), this, SLOT(doTimer()) );

    m_Rnd =  QRandomGenerator::securelySeeded();
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

void MainWindow::doStartTest()
{
    //m_DriveMode = 1;
    m_TestCount = 0;
    m_StateCount=0;
}

void MainWindow::doStopTest()
{
    m_DriveMode = 0;
}


void MainWindow::rpmChanged(const QString &text)
{

    m_DriveRpm = text.toInt()/250;
   qDebug() << "rpm_Changed: " << text << " -> " << m_DriveRpm;
}

void MainWindow::rpmIntChanged(int newRpm)
{
    char buffer[255];
    sprintf(buffer,"%d",newRpm);
    ui->le_Rpm->setText(buffer);

    //m_DriveRpm = newRpm/250;
   //qDebug() << "rpmIntChanged: " << newRpm << " -> " << m_DriveRpm;
}

void MainWindow::doDisconnect()
{
    m_Timer->stop();
    m_Can2Usb.close();

    doStatusMsg("Disconnected");
}

void MainWindow::doTx(QByteArray data)
{
    m_Can2Usb.doTx(data);
}

void MainWindow::doTimer()
{

    char buffer[255];
    int maxWatt = 120;   // was 5E
//  doTx("t7368000C5A0014005E00\r");   // poll each time 100ms
    sprintf(buffer,"t7368%02X%02X%02X0014005E00\r",m_DriveMode,m_DriveRpm,maxWatt );
    doTx(buffer);   // poll each time 100ms

    // 2 min 1200 2*60*10
    if (m_StateCount < 2000)
    {
        m_StateCount++;
 //       qDebug() << m_Can2Usb.ts()  << "cnd" << m_StateCount;
#ifndef LONG_TERM


        int max =  m_RndAn +  m_RndAus;
        if (m_StateCount==max ) {
            m_TestCount++;
            ui->l_Tc->setText(QString("m_TestCount: %1").arg(m_TestCount));
            qDebug() << m_Can2Usb.ts()  << "Timer new m_TestCount;" << m_TestCount;
            m_StateCount=0;

            m_RndAn = 2 + m_Rnd.bounded(600); // 60 sec an.
            m_RndAus = 2 + m_Rnd.bounded(900);  // nach 90 sec ... da 60 nachlauf.
            qDebug() << m_Can2Usb.ts()  << "Random on/off " << m_RndAn << " " << m_RndAus;


        } else {
            if (m_StateCount == 1) {
                m_DriveMode = 1;
                qDebug() << m_Can2Usb.ts()  << "Timer start motor" << m_StateCount << " for " << m_RndAn << " ticks";
            }
            if (m_StateCount == m_RndAn) {
                m_DriveMode = 0;
                qDebug() << m_Can2Usb.ts()  << "Timer stop motor" << m_StateCount<< " for " << m_RndAus << " ticks";
            }

        }

        /*
        case 2:
            m_DriveMode = 1;
            qDebug() << m_Can2Usb.ts()  << "Timer start motor" << m_StateCount;
            break;

        case 300:
            m_DriveMode = 0;
            qDebug() << m_Can2Usb.ts()  << "Timer stop motor" << m_StateCount;
            break;


         case 900:

            break;
*/
#else
        // nur an..
        if (m_StateCount == 1) {
            m_DriveMode = 1;
            qDebug() << m_Can2Usb.ts()  << "Timer start motor" << m_StateCount;
        }

#endif     
    } else {
        qDebug() << "State Overshoot";
    }

}

void MainWindow::doConnect(void)
{

    m_RndAn = 600; // 60 sec an.
    m_RndAus = 300; // 30 stop

    int pos = ui->cbPorts->currentIndex();
    QSerialPortInfo spi = m_Ports.at(pos);
    QString pn = spi.portName();

    bool open = m_Can2Usb.doConnect(spi);

    if (open) {
        doStatusMsg("Connected");
        m_StateCount = 0;
        qDebug() << m_Can2Usb.ts()  << "start timer";
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
    ui->pb_Start->setDisabled(!connected);
    ui->pb_Disconnect->setDisabled(!connected);
}

void MainWindow::CanReady(void)
{
    m_Timer->start(100);
    qDebug() << "CanReady ! Start timer ";

}

void MainWindow::CanRx(long id,QByteArray data)
{
    int ErrorMode = data.at(0);
    if (ErrorMode == 0) {
        // Live data
        int gCanDebug = data.at(1);
        int StateMachine = data.at(2);

        ui->l_Error->setText(QString("SM: %1 DEBUG: %2").arg(StateMachine).arg(gCanDebug));
      //  qDebug() << "CanRx: Turbine RUN: " << gCanDebug << " StateMachine: " << StateMachine;

    } else {
        // Error stop
        int ErrorValue1 = data.at(1);
        int ErrorValue2 = data.at(2);
        int ErrorValue= (ErrorValue1<<8)+ErrorValue2;
        ui->l_Error->setText(QString("ErrorMode: %1 / %2").arg(ErrorMode).arg(ErrorValue));
        qDebug() << "CanRx: Turbine Error:" << ErrorMode << " Par: " << ErrorValue;
        m_Timer->stop();
    }


     int rpm = (data.at(3)*250)&0xffff;     // int end at 32767!

    unsigned int cur = data.at(4)& 0xff;

    int strom = cur*10;
    int volt = data.at(5);   // 1:1 im device *20
    int druck = (data.at(6) & 0xff);  // 0..255 ( not -!)
    int version = (data.at(7) & 0xff);
   // int verion = data.at(7);
    //qDebug() << "CanRx: Turbine data RPM:" << rpm << " strom: " << strom << " Druck" << druck;


    ui->l_Rpm->setText(QString("rpm: %1").arg(rpm));
    ui->l_Strom->setText(QString("strom: %1").arg(strom));
    ui->l_Volt->setText(QString("volt: %1").arg(volt));
    ui->l_Druck->setText(QString("druck: %1").arg(druck));
    ui->l_Version->setText(QString("version: %1").arg(version));


    //doTx("t73780000320200300027\r");

/*
 *           testmessage.data[3]=bldc_getCurrentSpeed()/250;  // 4 = 1000RPM ist *250
            testmessage.data[4]=bldc_getMotorCurrent()/10;   // Strom in 10mA steps,
            testmessage.data[5]=s44_getAdc(AD_V_BUS)/20;     // VOLT  gSecCount; // FIXME !!!MZA???
            {
                bdruck_get(&gDruck,&gTemp);   // CAN response
                int mbar = gDruck/100;

                testmessage.data[6]=mbar;            // druck normiert in mbarbzw. hpa = 100Pa
            }
            testmessage.data[7]=I_VERSION;                           // Software version
            */
}


    /*
     *
testmessage.data[0]=modebyte;              // an / aus Motor , Lampe
    if (turbine < 12)
    {
        turbine = 12;  // min 3000 RPM
    }
    //testmessage.data[0]=gdata[0].oper.data;
    // 4 = 1000
    // 120 = 30000
    // Zur turbine
    testmessage.data[1]=turbine;               // RPM Turbine! in 250 RPM steps
    testmessage.data[2]=gMD.turbine_Watt_max;  // Turbine leistung
    testmessage.data[3]=gInfoMode;             // what are the nex 4 bytes.
    switch (gInfoMode)
    {
        case 0:
            testmessage.data[4]=gdata[0].motor_rpm;                 // handstuck soll
            testmessage.data[5]=bldc_getCurrentSpeed()/250;         // handstuck ist
            testmessage.data[6]=g_S35_Data.v_bus/10;                // 24 = 24 V Spannung
            testmessage.data[7]=((g_S35_Data.i_motor/2)/ISOFT_FAKTOR); // Strom
            break;
        case 1:
            testmessage.data[4]=gI2cButtons1;
            testmessage.data[5]=gI2cButtons2;
            testmessage.data[6]=gI2cSlider;
            testmessage.data[7]=gI2cVersion; //gCanDebug;           // send debug info
            break;
        default:

            break;
    }
    testmessage.data_length=8;

     *
    gCanRx = message->data[0];

               gTurbineRpm        = message->data[1];   // in 250rpm steps.

               // mza version h20 gTurbineRpm = gTurbineRpm * 0.7; // 0..10kRPM

               gTurbinePowerLimit = message->data[2];   // max leistung in Watt TODO: Error erzeugen
               if (gTurbineRpm < 12)
               {
                   gTurbineRpm = 12;  // min 3000 RPM
               }
               else
               {
                   if (gTurbineRpm > 140)
                   {
                       gTurbineRpm = 140;  // auf 35k begrenzen
                   }
               }
              */
