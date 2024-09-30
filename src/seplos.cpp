#include "usb2can.h"

#include <QDebug>
#include <QDateTime>
/*
t10021133[CR]
Sends an 11bit CAN frame with ID=0x100, 2 bytes
with the value 0x11 and 0x33.
*/
Usb2Can::Usb2Can(QObject *parent) : QObject(parent)
{
   connect(&m_Rs232,          SIGNAL(readyRead()), this, SLOT(rsReadFunction()));   
   connect(&m_Timer,          SIGNAL(timeout()),   this, SLOT(doTimer()) );
   m_TimerState = 0;
}

void Usb2Can::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                            + QLatin1String(": State Change")
                            + QString::number(m_MqttClient.state())
                            + QLatin1Char('\n');
    qDebug() << ts() << "updateLogStateChange" << content;

    //ui->editLog->insertPlainText(content);
}


bool Usb2Can::doConnect(QSerialPortInfo spi)
{

  //  m_MqttClient->setHostname(ui->lineEditHost->text());
    m_MqttClient.setHostname("192.168.6.128");
    m_MqttClient.setPort(1883);

    m_MqttClient.setUsername("mps");
    m_MqttClient.setPassword("gehheim#mpS");

    m_MqttClient.connectToHost();

    // mqtt.0.BMS1.amp_hours1
    // mqtt.0.BMS1.cell16
    // mqtt.0.BMS1.cell_temp4
    // mqtt.0.BMS1.charge_discharge
    // mqtt.0.BMS1.cycles
    // mqtt.0.BMS1.cycles
    // mqtt.0.BMS1.diff_mv
    // mqtt.0.BMS1.env_temp
    // mqtt.0.BMS1.high_cell
    // mqtt.0.BMS1.high_mv
    // mqtt.0.BMS1.low_cell
    // mqtt.0.BMS1.low_mv
    // mqtt.0.BMS1.port_voltage
    // mqtt.0.BMS1.power
    // mqtt.0.BMS1.power_temp
    // mqtt.0.BMS1.residual_capacity
    // mqtt.0.BMS1.residual_capacity
    //mqtt.0.BMS1.soh
        // mqtt.0.BMS1.total_voltage


    connect(&m_MqttClient, &QMqttClient::stateChanged, this, &Usb2Can::updateLogStateChange);
  //  connect(m_MqttClient, &QMqttClient::disconnected, this, &Usb2Can::brokerDisconnected);

    connect(&m_MqttClient, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString()
                                + QLatin1String(" Received Topic: ")
                                + topic.name()
                                + QLatin1String(" Message: ")
                                + message
                                + QLatin1Char('\n');
        qDebug() << ts() << "QMqttClient::messageReceived" << content;
     //   ui->editLog->insertPlainText(content);
    });

    connect(&m_MqttClient, &QMqttClient::pingResponseReceived, this, [this]() {
        const QString content = QDateTime::currentDateTime().toString()
                                + QLatin1String(" PingResponse")
                                + QLatin1Char('\n');
        qDebug() << ts() << "QMqttClient::pingResponseReceived" << content;
    //    ui->editLog->insertPlainText(content);
    });



    m_Rs232.setBaudRate(QSerialPort::Baud19200);
    m_Rs232.setDataBits(QSerialPort::Data8);
    m_Rs232.setParity(QSerialPort::NoParity);
    m_Rs232.setPort(spi);
    bool isOpen = m_Rs232.open(QIODevice::ReadWrite);


    if (isOpen)
    {
        m_TimerState = 0;
        m_Timer.start(100);
        return true;
    } else {

        return false;
    }
}

void Usb2Can::close()
{
    //oneLineRx("t73780000320200300027\r");
    //return false;
    m_Timer.stop();
    m_Rs232.close();
}

void Usb2Can::doTx(QByteArray data)
{
    if (data.at(0)!='t') {
        qDebug() << ts() << "TX: " << data;
    }
    m_Rs232.write(data);
}


void Usb2Can::rsReadFunction()
{

    QByteArray arrivedMsg = m_Rs232.readAll();
    qDebug() << "RX(junk): " << arrivedMsg.toHex() + " ASC: "+ arrivedMsg;
    m_RxData.append(arrivedMsg);               // add Data...we may have already some
    qDebug() << "RX(all_): " << m_RxData.toHex()+ " ASC: "+ m_RxData;
    processRx();
}

//“~1203400456ABCEFEFC72CR”,
//    CHKSUM=‘1’+‘2’+‘0’+...+‘F’+‘E’=31H+32H+30H+...+46H+45H=038EH,
//    mod
//    65536, remainder=038EH, do a bitwise invert and plus 1= FC72H.
//    For CHKSUM transmission, HIGH byte first, then LOW byte, and it is divided into 4
//     ASCII codes.

uint16_t Seplos_CRC(const char *buf, int len)
{
    uint16_t crc = 0;

    for (int pos = 0; pos < len; pos++) {
        uint16_t one = buf[pos];
        crc += one;          // XOR byte into least sig. byte of crc
    }
   crc = crc ^0xffff;
    crc = crc +1;
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}

// Compute the MODBUS RTU CRC
uint16_t ModRTU_CRC(char *buf, int len)
{
    uint16_t crc = 0xFFFF;

    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                            // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}


void Usb2Can::processRx(void)
{

    bool more = true;
    while(more)
    {
        // search modbus RTU
        //int len = m_RxData.size();
        //      "00041000000134db0100004b00fff002"
        // RX:  "00041000000134db"
        //       0011223344556677
        //       00                 adress 1
        //         11               mode   1
        //           22             len    1
        //
        //                   mmnn   crc    2
        /*
        if (len >=5 ) {
            // can read len.
            uint16_t dataLen = m_RxData.at(2);
            uint16_t full = 5 + dataLen;
            qDebug() << ts() << "\t modbus\trxLen=" << len  << "\tdataLen\t" << dataLen << " full " << full ;
            if (len >= full ) {
                // one frame
                // m_RxData.mid(0,full);
            }
        }
*/
        /*
        Table 1 List of information structures
            Items   OFS LEN
            SOI     0   1
            VER     1   2 32 30 / 20
            ADR     3   2 30 3x
            CID1    5   1 34 / 4
            CID2    6   1 36 / 5
            LENGTH  7   4
            INFO    11  0-n
            CHKSUM  4
            EOI     1 0x0d
min len 16
        */

        int indexSOF = m_RxData.indexOf('~');
        int indexEOF = m_RxData.indexOf('\r');
        if (indexEOF < 1) {
            indexEOF = m_RxData.indexOf('\n');
        }
        int rxLen = m_RxData.size();

        more = false;
        qDebug() << ts() << "\t V2\tsof=" << indexSOF  << "\tEOF\t" << indexEOF <<  "\tlen\t" << rxLen ;

        //  V2	sof= 23 	EOF	 42 	NL	 43
        if ( (indexSOF >= 0) && ( rxLen >=16 ) && (indexEOF>=0) )
        {

            qDebug() << ts() << "SOF at " << indexSOF << " len= " << rxLen;
            int startPos = indexSOF;
            if (m_RxData.at(startPos+1) == '2')
            {
                if (m_RxData.at(startPos+2) == '0')
                {
                    bool ok= false;

                    uint16_t iAdr = m_RxData.mid(startPos+3,2).toUInt(&ok,16);
                    qDebug() << ts() << "ADR: " << iAdr;
                    if (m_RxData.at(startPos+5) == '4')
                    {
                        if (m_RxData.at(startPos+6) == '6')
                        {
                            QByteArray fc = m_RxData.mid(startPos+7,2);
                            uint16_t dFC = fc.toUInt(&ok,16);
                            qDebug() << ts() << "FC_: " << fc << " dez: " << dFC;

                            uint32_t lenData = m_RxData.mid(startPos+9,4).toUInt(&ok,16);

                            qDebug() << ts() << "LEN: " << lenData;

                            // ~200146030000FDB0\r"
                            // ~20014642E00201FD35$\r
                            if (dFC != 0) {
                                lenData = 0; // on TX len = 2 only 16 bit !!! je 2 char
                            }

                            // must calucalte from 46 based.(6) and allways 3 bytes ( 6 )

                            int absPosCR = indexEOF;
                            int absPosCRC = absPosCR-4;

                            if (m_RxData.at(absPosCR-1) == '$')
                            {
                                absPosCRC--; // do we have a $ at the end... if yes CRC one down
                            }

                            int crcLen  = absPosCRC - startPos;
                            qDebug() << ts() << "posCRC: " << absPosCRC << " CRpos: " << absPosCR << " CRCLEN: " << crcLen;

                            // read CRC what it should be
                            uint32_t crcRx = 0xBAD1;
                            if ( rxLen >= absPosCR)
                            {
                                QByteArray crcBaRx = m_RxData.mid(absPosCRC,4);
                                crcRx = crcBaRx.toUInt(&ok,16);
                                qDebug() << ts() << "CRC: " << crcBaRx << " xx: " << crcRx << " XXXX: " << crcBaRx.toHex();

                            } else {
                                qDebug() << ts() << "CRC: too less data!!!!";
                            }

                            // calulate CRC
                            int offsetBA = 1;
                            QByteArray crcCalcBa = m_RxData.mid(startPos+offsetBA,crcLen-offsetBA);
                            qDebug() << ts() << Qt::hex << "crcCalcBa: " << crcCalcBa.toHex();
                            char * data = crcCalcBa.data();
                            int balen = crcCalcBa.size();
                            uint16_t crcCalc = Seplos_CRC(data,balen);
                            qDebug() << ts() << "crcCalc: " << Qt::hex << crcCalc;
/*
                            const char * test = "1203400456ABCEFE";
                            int tlen = strlen(test);
                            uint16_t tcrc = Seplos_CRC(test, tlen);
                            qDebug() << ts() << Qt::hex << "tcrc: " << tcrc;
*/

                            if (crcRx != crcCalc ) {
                                qDebug() << ts() << "CRC ERROR" << crcRx << "!= " << crcCalc;
                            } else {
                                qDebug() << ts() << "CRC matched";

                                int leftLen = absPosCR-startPos;
                                QByteArray left = m_RxData.mid(startPos,leftLen);
                                oneLineRx(left);
                                QByteArray rest = m_RxData.mid(absPosCR+1);
                                m_RxData = rest;
                                qDebug() << "m_RxData(RX): " << m_RxData.toHex();
                                more = true;
                            }
                        }
                    }


                }
            }


        }
/*
 *         // check ~20014644E00201FD33$ search
        int indexSOF = m_RxData.indexOf('~');
        int indexEOF = m_RxData.indexOf('$');
        int indexNL = m_RxData.indexOf('\r');
        qDebug() << ts() << "\t V2\tsof=" << indexSOF  << "\tEOF\t" << indexEOF <<  "\tNL\t" << indexNL ;
        //  V2	sof= 23 	EOF	 42 	NL	 43

        //if ( (indexNL>0) && (indexEOF == (indexNL-1) ) && (indexSOF>=0)  )
        if (indexSOF>=0)
        {
            // we start after SOF

            if ( (indexNL>0) && (indexNL == (indexSOF-1) )) {
                // CR + SOF
                QByteArray left = m_RxData.mid(0,indexNL);
                oneLineRx(left);
                QByteArray rest = m_RxData.mid(indexSOF+1);
                m_RxData = rest;
                qDebug() << "m_RxData(NL+): " << m_RxData.toHex();
            } else {
                QByteArray rest = m_RxData.mid(indexSOF+1);
                m_RxData = rest;
                qDebug() << "m_RxData(SOF): " << m_RxData.toHex() + " ASC: "+ m_RxData;
            }
        } else {
            if ( (indexNL>0) && (indexEOF == (indexNL-1) ) )
            {
                // EOF + newline
                int xlen = indexEOF - indexNL -1;
                QByteArray line = m_RxData.mid(indexNL+1,xlen);
                QByteArray rest = m_RxData.mid(indexNL+1);
                oneLineRx(line);

                m_RxData = rest;
                qDebug() << "m_RxData/EOF: " << m_RxData.toHex();

            } else {
                if ( (indexNL>0) && (indexSOF>=0)  )
                {
                    // NL + SOF ... das weg schneiden.
                    QByteArray left = m_RxData.mid(0,indexNL-1);
                    QByteArray right = m_RxData.mid(indexSOF+1);

                    m_RxData = left;
                    m_RxData.append(right);
                    qDebug() << "m_RxData/CUT: " << m_RxData.toHex();


                } else {
                    more = false;
                }
            }
        }

        */
        /*
        if ( (indexNL>0) && (indexEOF == (indexNL-1) ) && (indexSOF>=0)  )
        {
            int xlen = indexEOF - indexNL -1;


            QByteArray line = m_RxData.mid(indexNL+1,xlen);
            QByteArray rest = m_RxData.mid(indexNL+1);

            //qDebug() << ts() << "\tcount\t" << counter  << "\tact\t" << duration ;

            oneLineRx(line);

            m_RxData = rest;
            qDebug() << "m_RxData/JUNK: " << m_RxData.toHex();

        } else {
          more = false;
        }
        */
        // more = false;

        // 7E fängt an.
        // 240d -> eine fertig.
        // nur 0d stop.... bis 7E

    }
}


uint32_t getUintFromString(QString &str, int &start,int len ) {
    bool ok = true;
    uint32_t data = str.mid(start,len).toUInt(&ok,16);
    start +=len;
    return data;
}

void Usb2Can::processProV20(QString line)
{
    // ~200146001096000110
    int start = 3;
    int addr = getUintFromString(line,start,2);
    int func = getUintFromString(line,start,2);
    int code = getUintFromString(line,start,2);
    int banz = getUintFromString(line,start,2);
    int xxx1 = getUintFromString(line,start,2);
    int xxx2 = getUintFromString(line,start,2);
    int xxx3 = getUintFromString(line,start,2);
    int xxx4 = getUintFromString(line,start,2);

    if (banz != 16)
    {
        return;
    }
    qDebug() << ts() << "XX: " << addr << " " << func << " " << code << " " << banz;
    //"2024.09.29 23:34:01,748" XX:  1   70   66   224
    // "2024.09.29 23:34:01,886" XX:  1   70   0   16
    for (int bno = 0; bno < banz; bno++)
    {
        int miliVolt = getUintFromString(line,start,4);
        qDebug() << ts() << "Cell: " << bno << " mv: " << miliVolt;
    }
    int tanz = getUintFromString(line,start,2);
    qDebug() << ts() << "temp anz: " << tanz;
    for (int tno = 0; tno < tanz; tno++)
    {
        int zentelGrad = getUintFromString(line,start,4);
        double grad = zentelGrad * 0.01f;
        qDebug() << ts() << "Temp: " << tno << " grad: " << grad;
    }

    //7      Charge/discharge current (0.01A)      2
    double tcurr =  getUintFromString(line,start,4) * 0.01f;
    //8      Total battery voltage (0.01V)      2
    double tbatt =  getUintFromString(line,start,4) * 0.01f;
    //9      Residual capacity (0.01Ah)      2
    double ahmax =  getUintFromString(line,start,4) * 0.01f;
    //10      Custom number P=10    1
    double cno1 =  getUintFromString(line,start,2);
    //11    Battery capacity (0.01Ah)    2
    double ahact =  getUintFromString(line,start,4) * 0.01f;
    //12    SOC (1‰ )    2
    double    soc =  getUintFromString(line,start,4) * 0.1f;
    //13    Rated capacity (0.01Ah)    2
    double    rcap =  getUintFromString(line,start,4) * 0.01f;
    //14    Number of cycles    2
    double    cycle =  getUintFromString(line,start,4);
    //15    SOH (1‰ )    2
    double    soh =  getUintFromString(line,start,4) * 0.1f;
    //16    Port voltage (0.01V)    2
    double    pvolt =  getUintFromString(line,start,4) * 0.01f;
    //17    Reservation    2
    //18    Reservation    2
    //19    Reservation    2
    //20    Reservation    2
    qDebug() << ts() << "7-12\t" << tcurr << "\t" << tbatt << "\t" << ahmax << "\t" << ahact << "\t" << soc ;
    qDebug() << ts() << "13 "<< rcap << "\t" << cycle << "\t" << soh  << "\t" << pvolt;

}

/*

    DATA FLAG                       1    2
    COMMAND GROUP                   1    3
    Number of cells M=16            1    4
    Voltage of Cell 1 (mV)          2
    Voltage of Cell 2 (mV)          2    ...
    Voltage of Cell M (mV)          2    5
    Number of temperatures N=6      1      6
      Cell temperature 1 (0.1℃)
      2
      Cell temperature 2 (0.1℃)
      2
      Cell temperature 3 (0.1℃)
      2
      Cell temperature 4 (0.1℃)
      2
      Environment temperature (0.1℃)
      2
      Power temperature (0.1℃)
      2

    */

void Usb2Can::oneLineRx(QString line)
{
    if (line.size() == 0)
    {
        qDebug() << ts() << "oneLineRx: ACK";
        return;
    }

    qDebug() << ts() << "oneLineRx(ASC): " << line;
   // QString res = line.toLocal8Bit().toHex();
//    qDebug() << ts() << "oneLineRx(HEX): " << res;

    if (line.startsWith("~20"))
    {
        processProV20(line);


    } else {
        qDebug() << ts() << "Start not with @20";
    }


}

QString Usb2Can::ts(void)
{
    return QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss,zzz");
}

void Usb2Can::modbusBuildCrcAndCrThenSend(QString hex)
{
    // sample
    //QString MyHexString1 ="000410000012";
    //QByteArray send = addModbusCRC(MyHexString1);
    //QString MyHexString ="00041000001275160d";

    QByteArray cmd1 = QByteArray::fromHex(hex.toUtf8());

    char * data = cmd1.data();
    int len = cmd1.size();
    uint16_t crc = ModRTU_CRC(data,len);

    uint16_t lb = crc & 0x00ff;
    uint16_t hb = crc >> 8;
    char buffer[255];
    sprintf(buffer,"%04X %02X %02X",crc,lb,hb);
    //qDebug()  << "yy: " << buffer << " x  " << len;

    cmd1.append(lb);
    cmd1.append(hb);
//    cmd1.append(0x0d);

    qDebug()  << "send hex: " << cmd1.toHex();

    //return cmd1;
    m_RxData.clear(); // remove alll when we send new...
     m_Rs232.write(cmd1);
}


void Usb2Can::doTimer()
{
    m_TimerState++;
    qDebug() << ts() << "m_TimerState: " << m_TimerState;

    switch(m_TimerState)
    {



//    pi@raspberrypi:~/pi_mpp/bms $ ./query_seplos_ha.sh 4201 /dev/ttyUSB_bms 19200 01
// Sending "~20014642E00201FD35\r"
// pi@raspberrypi:~/pi_mpp/bms $ ./query_seplos_ha.sh 4201 /dev/ttyUSB_bms 19200 02
// Sending "~20024642E00201FD34\r"





    case 10:



        QMqttTopicName topic("seplos/mytest");
        QByteArray message ("info2");

        //              quint8 qos = 0, bool retain = false);

        qint32 x2 =  m_MqttClient.publish(topic,message);
        qDebug() << ts() << "XXXXXXXXX" << " 2 " << x2;



        doTx("~20014642E00201FD35\r");  // read telemetrie
        //doTx("~2001464F0000FD99$\r");  // read telemetrie
        break;
/*
    case 15:
        doTx("~200146510000FDAD$\r");  // read telemetrie
        break;

    case 20:
        doTx("~20014642E00201FD35\r");  // read telemetrie
        break;

    case 25:
        doTx("~20014644E00201FD33$\r");  // read telemetrie
        break;

    case 30:
        doTx("~20014642E00201FD35$\r");  // read telemetrie
        break;

    case 44:
    {

//        QString MyHexString1 ="000410000012";


//        QString MyHexString1 ="000410000001";
//        modbusBuildCrcAndCrThenSend(MyHexString1);


        //QString MyHexString ="00041000001275160d";
        //QByteArray cmd = QByteArray::fromHex(MyHexString.toUtf8());
        //doTx(cmd);  // version

        doTx("~20014644E00201FD33$\r");  // read telemetrie

    }
        break;





    case 4:
        doTx("~2001464F0000FD99$\r");  // version
        break;

    case 6:
        doTx("~2001464F0000FD99$\r");  // version
        break;

    case 8:
        doTx("~20014644E00201FD33$\r");  // read telemetrie
        break;


    case 10:
        doTx("~20014642E00201FD35$\r");  // Close in case of restart
        break;

    case 6:
        doTx("S6\r");  // 500kbits
        break;

    case 7:
        doTx("M0000E000\r");  // MASK
        break;

    case 8:
        doTx("m00001FF0\r");  // MASK
        break;

    case 9:
        doTx("O\r");  // open
        break;

    case 10:
        doTx("F\r");  // Flag
        break;
    case 11:
        m_Timer.stop();
        CanReady();
        break;
*/
   }
}

