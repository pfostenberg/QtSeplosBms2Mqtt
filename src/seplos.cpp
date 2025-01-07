#include "seplos.h"

#include <QDebug>
#include <QDateTime>
#include "setting.h"


Seplos::Seplos(QObject *parent) : QObject(parent)
{
   connect(&m_Rs232,          SIGNAL(readyRead()), this, SLOT(rsReadFunction()));   
   connect(&m_Timer,          SIGNAL(timeout()),   this, SLOT(doTimer()) );
   m_TimerState = 0;
   m_ActAdr = 0;
   m_OpenTx = 0;

   m_V3_Protocol = true;
   int version = settingProvider()->getVersion();
   if (version == 3)
   {
       m_V3_Protocol = true;
   }

}

void Seplos::updateLogStateChange()
{
    int mqttState = m_MqttClient.state();
    const QString content = QDateTime::currentDateTime().toString()
                            + QLatin1String(": State Change")
                            + QString::number(mqttState)
                            + QLatin1Char('\n');
    qDebug() << ts() << "updateLogStateChange" << content;
    emit UpdateCell(42, mqttState);
    if (mqttState == 2) {
        setLastWill();
    }
}

static const char* const MqttNames[] = {
    "cell",              // 0
    "cell_temp",         // 1
    "env_temp",          // 2
    "power_temp",        // 3
    "_4_",               // 4
    "_5_",               // 5
    "_6_",               // 6
    "charge_discharge",  // 7
    "total_voltage",     // 8
    "amp_hours1",        // 9
    "_10_",              // 10
    "residual_capacity", // 11 ahact
    "soc",               // 12
    "amp_hours1",        // 13
    "cycles",            // 14
    "soh",               // 15
    "port_voltage",      // 16
    "power",             // 17
    "diff_mv",           // 18
    "low_mv",
    "high_mv",
    "low_cell",
    "high_cell",
    "void"
};


bool Seplos::sendMqttPublish(int adrX, int no, int subno, double value, int dezimals)
{
    QString prefix = settingProvider()->getMqttPrefix();
    //mqtt.0.BMS1.cell16
    //mqtt.0.BMS1.cell_temp4
    //mqtt.0.BMS1.amp_hours1
    //mqtt.0.BMS1.charge_discharge
    //mqtt.0.BMS1.cycles
    //mqtt.0.BMS1.diff_mv
    //mqtt.0.BMS1.env_temp
    //mqtt.0.BMS1.high_cell
    //mqtt.0.BMS1.high_mv
    //mqtt.0.BMS1.low_cell
    //mqtt.0.BMS1.low_mv
    //mqtt.0.BMS1.port_voltage
    //mqtt.0.BMS1.power
    //mqtt.0.BMS1.power_temp
    //mqtt.0.BMS1.residual_capacity
    //mqtt.0.BMS1.soh
    //mqtt.0.BMS1.total_voltage

    const char * itemname = "xxxx";
    if (no <= 22) {
        itemname = MqttNames[no];
    }

    int mqttAdr = adrX + settingProvider()->getMqttOffset();

    char buffer[256];
    if (subno < 1) {
        sprintf(buffer,"%s%d/%s",prefix.toLocal8Bit().constData(),mqttAdr,itemname);
    } else {
        if (no==1) {
            sprintf(buffer,"%s%d/%s%01d",prefix.toLocal8Bit().constData(),mqttAdr,itemname,subno);
        } else {
            sprintf(buffer,"%s%d/%s%02d",prefix.toLocal8Bit().constData(),mqttAdr,itemname,subno);
        }
    }
    QString topic(buffer);

    char message[255];
    char fmt[30];
    // %3.2g
    sprintf(fmt,"%%5.%df",dezimals);
    sprintf(message,fmt,value);
    QByteArray bAmessage(message);

    qint32 x2 =  m_MqttClient.publish(topic,message);
    qDebug() << ts() << "publish" << topic << message << x2;
    return x2;
}

void Seplos::setStatusOnline(bool online) {
    qDebug() << ts() << "Seplos::setLastWill";
    QString prefix = settingProvider()->getMqttPrefix();
    int snoX = settingProvider()->getStartNo();
    int mqttAdr = snoX + settingProvider()->getMqttOffset();

    // _ let it be the first -:)
    QString statusTopic = QString("%1%2/_connected").arg(prefix).arg(mqttAdr);

    QByteArray bAmessageDis("0");
    m_MqttClient.setWillTopic(statusTopic);
    m_MqttClient.setWillMessage(bAmessageDis);
    m_MqttClient.setWillRetain(true);
    m_MqttClient.setWillQoS(2);

    QByteArray bAmessageCon("1");
    if (!online) {
        bAmessageCon="0";   // we are disconnected
    }
    m_MqttClient.publish(statusTopic,bAmessageCon);      // now we are connected.
    qDebug() << ts() << "QMqttClient::publish" << statusTopic << " -> " << bAmessageCon;
}

void Seplos::setLastWill()
{
    setStatusOnline(true);
}

bool Seplos::doConnect(QSerialPortInfo spi, setting *si)
{
    m_MqttClient.setHostname(si->getMqttHost());
    m_MqttClient.setPort(si->getMqttPort());
    m_MqttClient.setUsername(si->getmMqttUser());
    m_MqttClient.setPassword(si->getMqttPassword());
    m_MqttClient.connectToHost();

    m_ActAdr = si->getEndNo();   // on next timer we start from it

    connect(&m_MqttClient, &QMqttClient::stateChanged, this, &Seplos::updateLogStateChange);
  //  connect(m_MqttClient, &QMqttClient::disconnected, this, &Seplos::brokerDisconnected);

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

    m_OpenTx = 0;
    m_Rs232.setBaudRate(QSerialPort::Baud19200);
    m_Rs232.setDataBits(QSerialPort::Data8);
    m_Rs232.setParity(QSerialPort::NoParity);
    m_Rs232.setPort(spi);
    bool isOpen = m_Rs232.open(QIODevice::ReadWrite);

    if (isOpen)
    {
        m_TimerState = si->getWaitTimeMs()-30;  // start after 3 sec...
        m_Timer.start(100);
        return true;
    } else {

        return false;
    }
}

void Seplos::close()
{
    qDebug() << ts() << "Seplos::close";
    setStatusOnline(false);// disconnected
    m_Timer.stop();
    m_Rs232.close();
    m_OpenTx = 0;
//    m_MqttClient.disconnectFromHost();  // simulate disconnecting
}

void Seplos::doTx(QByteArray data)
{
    if (m_V3_Protocol) {
        m_RxData.clear();
    }

    if (data.at(0)!='t') {

        m_OpenTx++;
        qDebug() << ts() << "TX: " << data << " m_OpenTx" << m_OpenTx;


        if (m_OpenTx > 2)
        {
            qDebug() << ts() << "doTx m_OpenTx: " << m_OpenTx;

            if (m_OpenTx > 4) {
                qDebug() << ts() << "doTx RequestPortReopen() ";
                emit RequestPortReopen();
                m_OpenTx = 0;
            return;
            }
        }
    }
    m_Rs232.write(data);
}

void Seplos::rsReadFunction()
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

void Seplos::processRx(void)
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


        if (m_V3_Protocol)
        {
            qDebug() << ts() << "\t V3 \tlen\t" << rxLen ;
            // min MODUBS RTU ADR + TYPE + LEN + x + CR
            if (rxLen > 5)
            {
                int adr = m_RxData.at(0);
                if (m_RxData.at(1)== 0x04)   // INPUT Register
                {
                    int len = m_RxData.at(2);
                    if ( (len == 0x24) || (len == 0x34) )
                    {
                        int minLen = 3 + len + 2; // 3 head + data + CRC
                        if (rxLen >= minLen) {
                        qDebug() << ts() << "\t V3 OK";
                        QByteArray data = m_RxData.mid(3,len*2);

                        processProV30(adr, data);
                        } else {
                            qDebug() << ts() << "\t V3 min length < len " << rxLen << " < " << minLen;
                        }
                    }
                }
            }
        } else {
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
                                qDebug() << ts() << "crcCalcBa(dez): " << crcCalcBa.toHex();
                                char * data = crcCalcBa.data();
                                int balen = crcCalcBa.size();
                                uint16_t crcCalc = Seplos_CRC(data,balen);
                                qDebug() << ts() << "crcCalc: " << crcCalc;
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
                                    m_OpenTx = 0; // was ok.

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
        }

    }
}

uint32_t getUintFromString(QString &str, int &start,int len ) {
    bool ok = true;
    uint32_t data = str.mid(start,len).toUInt(&ok,16);
    start +=len;
    return data;
}

double Seplos::getUintFromBa(QByteArray &ba, int len, double mult, double offset ) {
    int pos = len * 2;
    uint32_t hb = ba.at(pos)&0xFF;
    uint32_t lb = ba.at(pos+1)&0xFF;
    uint32_t data = (hb << 8) + lb;
    double  dData = data;
    double  dRes = (dData * mult) + offset;

    //char msg[255];
    //sprintf(msg," %02X %02X %04X", hb, lb, data);

    qDebug() << ts() << "getUintFromBa: " << len << " data-> " << dRes;
    return dRes;
}

double Seplos::getIntFromBa(QByteArray &ba, int len, double mult ) {
    int pos = len * 2;
    uint32_t hb = ba.at(pos)&0xFF;
    uint32_t lb = ba.at(pos+1)&0xFF;
    int16_t data = (hb << 8) + lb;
    double  dData = data;
    double  dRes = dData * mult;

    qDebug() << ts() << "getUintFromBa: " << len << " data-> " << dRes;
    return dRes;
}

void Seplos::pollV3(int no, int baseAdr)
{
    m_V3_ActAdr = baseAdr;
    QByteArray myHexArray = QByteArray::fromHex("0004100000127516");  // 0x1000

    if ( m_V3_ActAdr == 0x1100) {
      myHexArray = QByteArray::fromHex("00041100001A752C");   // 0x1100
    }
    m_OpenTx = 0;  // we got one.

    doTx(myHexArray);  // read telemetrie
}

/**
 * @brief Seplos::processProV30
 * @param ba
 * from https://github.com/syssi/esphome-seplos-bms/blob/main/esp32-seplos-v3-example.yaml
 */
void Seplos::processProV30(int addr, QByteArray ba)
{
    qDebug() << ts() << "processProV30: " << ba.size() << " : " << ba.toHex() << " adr: " << m_V3_ActAdr;
    double od = -273.1;  // offsetDegrees

    switch(m_V3_ActAdr) {
    case 0x1000:
    {
        double tbatt = getUintFromBa(ba, 0, 0.01);     // 1000    Pack Voltage                R    UINT16    2    10mV
        double tcurr = getIntFromBa (ba, 1, 0.01);     // 1001    Current                     R     INT16    2    10mA
        double ahact = getUintFromBa(ba, 2, 0.01);     // 1002    Remaining capacity          R    UINT16    2    10mAH
        double ahmax = getUintFromBa(ba, 3, 0.01);     // 1003    Total Capacity              R    UINT16    2    10mAH
        double  rcap = getUintFromBa(ba, 4, 0.01);     // 1004    Total Discharge Capacity    R    UINT16    2    10AH
        double   soc = getUintFromBa(ba, 5, 0.1);      // 1005    SOC                         R    UINT16    2    0.1%
        double   soh = getUintFromBa(ba, 6, 0.1);      // 1006    SOH                         R    UINT16    2    0.1%
        double  cycle = getUintFromBa(ba, 7, 1.0);      // 1007    Cycle                       R    UINT16    2    1

        double    pvolt = tbatt; // !!!MZA???
      //  getUintFromBa(ba, 8, 0.001);    // 1008    Averag of Cell Votage       R    UINT16    2    1mV
      //  getUintFromBa(ba, 9, 0.1,od);   // 1009    Averag of Cell Temperature  R    UINT16    2    0.1K
        double minVolt   = getUintFromBa(ba,10, 0.001);    // 100A    Max Cell Voltage            R    UINT16    2    1mV
        double maxVolt   = getUintFromBa(ba,11, 0.001);    // 100B    Min Cell Voltage            R    UINT16    2    1mV
        getUintFromBa(ba,12, 0.1,od);   // 100C    Max Cell Temperature        R    UINT16    2    0.1K
        getUintFromBa(ba,13, 0.1,od);   // 100D    Min Cell Temperature        R    UINT16    2    0.1K
        getUintFromBa(ba,14, 1.0);      // 100F    MaxDisCurt                  R    UINT16    2    1A

        emit UpdateDouble(7, tcurr);
        emit UpdateDouble(8, tbatt);
        emit UpdateDouble(9, ahact);
        emit UpdateDouble(11, ahmax);
        emit UpdateDouble(12, soc);
        emit UpdateDouble(13, rcap);
        emit UpdateDouble(14, cycle);
        emit UpdateDouble(15, soh);
        emit UpdateDouble(16, pvolt);

        sendMqttPublish(addr, 7, 0, tcurr, 3);
        sendMqttPublish(addr, 8, 0, tbatt, 1);
        sendMqttPublish(addr, 9, 0, ahact, 0);
        sendMqttPublish(addr, 11, 0, ahmax, 0);
        sendMqttPublish(addr, 12, 0, soc, 0);
        sendMqttPublish(addr, 13, 0, rcap, 0);
        sendMqttPublish(addr, 14, 0, cycle, 0);
        sendMqttPublish(addr, 15, 0, soh, 0);
        sendMqttPublish(addr, 16, 0, pvolt, 1);

        //17 power W
        double power = tbatt * tcurr;
        sendMqttPublish(addr, 17, 0, power, 0);

        //18 dx
        int maxDx = maxVolt-minVolt;
        sendMqttPublish(addr, 18, 0, maxDx, 0);

        sendMqttPublish(addr, 19, 0, minVolt, 0);
        sendMqttPublish(addr, 20, 0, maxVolt, 0);
        int minNo = 0;
        int maxNo = 0; // TODO !!!MZA???
        sendMqttPublish(addr, 21, 0, minNo, 0);
        sendMqttPublish(addr, 22, 0, maxNo, 0);

        pollV3(0,0x1100);
    }

        break;

    case 0x1100:
        // # 1100    Cell1 Voltage               R    UINT16    2    1mV
        // # 110F    Cell16 Voltage              R    UINT16    2    1mV
        for (int i=0;i<16;i++) {
            double volt = getUintFromBa(ba, i, 0.001);
            double milliVolt = volt * 1000.0f;
            emit UpdateCell(i, milliVolt);    // my display
            sendMqttPublish(addr, 0, i +1, volt, 4);  // mqtt.
        }

        {
            double ct1 = getUintFromBa(ba, 16, 0.1,od); // 1110    Cell temperature 1          R    UINT16    2    0.1K
            double ct2 = getUintFromBa(ba, 17, 0.1,od); // 1111    Cell temperature 2          R    UINT16    2    0.1K
            double ct3 = getUintFromBa(ba, 18, 0.1,od); // 1112    Cell temperature 3          R    UINT16    2    0.1K
            double ct4 = getUintFromBa(ba, 19, 0.1,od); // 1113    Cell temperature 4          R    UINT16    2    0.1K
            // 14 -> 20
            // 15 -> 21
            // 16 -> 22
            // 17 -> 23
            double ev1 = getUintFromBa(ba, 24, 0.1,od); // 1118    Environment Temperature     R    UINT16    2    0.1K
            double pt1 = getUintFromBa(ba, 25, 0.1,od); // 1119    Power temperature           R    UINT16    2    0.1K

            emit UpdateDouble(0, ct1);  // own display
            emit UpdateDouble(1, ct2);
            emit UpdateDouble(2, ct3);
            emit UpdateDouble(3, ct4);
            emit UpdateDouble(4, ev1);
            emit UpdateDouble(5, pt1);

            sendMqttPublish(addr, 1, 1, ct1, 1);  // MQTT
            sendMqttPublish(addr, 1, 2, ct2, 1);
            sendMqttPublish(addr, 1, 3, ct3, 1);
            sendMqttPublish(addr, 1, 4, ct4, 1);
            sendMqttPublish(addr, 2, 0, ev1, 1);
            sendMqttPublish(addr, 3, 0, pt1, 1);
        }

        break;

    case 0x1200:
        break;

        /*
         * # Pack Info C
  #
  # 1200    Cells voltage 08-01low alarm state       R    HEX    1    1:alarm
  # 1208    Cells voltage 16-09low alarm state       R    HEX    1    1:alarm
  # 1210    Cells voltage 08-01high alarm state      R    HEX    1    1:alarm
  # 1218    Cells voltage 16-09high alarm state      R    HEX    1    1:alarm
  # 1220    Cell 08-01 temperature Tlow alarm state  R    HEX    1    1:alarm
  # 1228    Cell 08-01 temperature high alarm state  R    HEX    1    1:alarm
  # 1230    Cell 08-01 equalization event code       R    HEX    1    1:on 0:off
  # 1238    Cell 16-09 equalization event code       R    HEX    1    1:on 0:off
  # 1240    System state code                        R    HEX    1    See TB09
  # 1248    Voltage event code                       R    HEX    1    See TB02
  # 1250    Cells Temperature event code             R    HEX    1    See TB03
  # 1258    Environment and power Temperature event code          R    HEX    1    See TB04
  # 1260    Current event code1                      R    HEX    1    See TB05
  # 1268    Current event code2                      R    HEX    1    See TB16
  # 1270     The residual capacity code              R    HEX    1    See TB06

  # System Parameter    reg. 1300...1367
  # System Function     reg. 1400...1448
  # System Control      reg. 1500...1519
  # History Info        reg. 1600...1627
  # Version Info        reg. 1700...1724
  # PCS Control         reg. 1800...1823
  # EMS Info A          reg. 2000...2019
  # EMS Info B          reg. 2100...2115
  # EMS Info C          reg. 2200...2248
  #
  # See docs/XZH BMS Modbus-RTU Protocol.pdf
         */
        break;
    }



m_OpenTx = 0;


}


void Seplos::processProV20(QString line)
{
    // ~200146001096000110
    int start = 3;
    int addr = getUintFromString(line,start,2);
    int func = getUintFromString(line,start,2);
    int code = getUintFromString(line,start,2);
    int banz = getUintFromString(line,start,2);
    getUintFromString(line,start,2);
    getUintFromString(line,start,2);
    getUintFromString(line,start,2);
    getUintFromString(line,start,2);

    // 16 -> Seplos 51V(16S)
    // 48 -> Seplos 29V (8s)
    if (banz != 16)
    {
        if (banz != 48)
        {
            return;
        }
    }
    if (banz == 48) {
        banz = 8;  // 48 -> 8 why?! but it works...
    }

    qDebug() << ts() << "XX: " << addr << " " << func << " " << code << " " << banz;

    int minVolt = 10000;
    int maxVolt = 0;
    int minNo = 0;
    int maxNo = 0;
    int maxDx = 0;

    //"2024.09.29 23:34:01,748" XX:  1   70   66   224
    // "2024.09.29 23:34:01,886" XX:  1   70   0   16
    for (int bno = 0; bno < banz; bno++)
    {
        int milliVolt = getUintFromString(line,start,4);
        if ((milliVolt) > maxVolt ) {
            maxVolt = milliVolt;
            maxNo = bno +1;
            qDebug() << ts() << "maxVolt: " << maxVolt << maxNo;
        }
        if ((milliVolt) < minVolt ) {
            minVolt = milliVolt;
            minNo = bno +1;
            qDebug() << ts() << "minVolt: " << minVolt << minNo;
        }
        emit UpdateCell(bno, milliVolt);    // my display
        double dv = milliVolt * 0.001f;
        sendMqttPublish(addr, 0, bno +1, dv, 4);  // mqtt.

        //qDebug() << ts() << "Cell: " << bno << " mv: " << miliVolt;
    }

    maxDx = maxVolt - minVolt;

    int tanz = getUintFromString(line,start,2);
    qDebug() << ts() << "temp anz: " << tanz;
    for (int tno = 0; tno < tanz; tno++)
    {
        int zentelGrad = getUintFromString(line,start,4);
        double grad = zentelGrad * 0.01f;
        emit UpdateDouble(tno, grad);
        //qDebug() << ts() << "Temp: " << tno << " grad: " << grad;
        switch(tno)
        {
        case 0: sendMqttPublish(addr, 1, 1, grad, 1); break;
        case 1: sendMqttPublish(addr, 1, 2, grad, 1); break;
        case 2: sendMqttPublish(addr, 1, 3, grad, 1); break;
        case 3: sendMqttPublish(addr, 1, 4, grad, 1); break;

        case 4: sendMqttPublish(addr, 2, 0, grad, 1); break;
        case 5: sendMqttPublish(addr, 3, 0, grad, 1); break;


        }
    }

    //7      Charge/discharge current (0.01A)      2
    uint32_t ucur = getUintFromString(line,start,4);
    int32_t  scur = 0;
    if (ucur > 32767) {
        uint32_t inv = ucur ^  0xffff;  // 65535 -> 1
        scur = inv * -1;
    } else {
        scur = ucur;
    }

    double tcurr = scur * 0.01f;
    emit UpdateDouble(7, tcurr);
    sendMqttPublish(addr, 7, 0, tcurr, 3);
    //8      Total battery voltage (0.01V)      2
    double tbatt =  getUintFromString(line,start,4) * 0.01f;
    emit UpdateDouble(8, tbatt);
    sendMqttPublish(addr, 8, 0, tbatt, 1);
    //9      Residual capacity (0.01Ah)      2
    double ahact =  getUintFromString(line,start,4) * 0.01f;
    sendMqttPublish(addr, 9, 0, ahact, 0);
    emit UpdateDouble(9, ahact);
    //10      Custom number P=10    1
    //double cno1 =
    getUintFromString(line,start,2);
    //11    Battery capacity (0.01Ah)    2
    double ahmax =  getUintFromString(line,start,4) * 0.01f;
    emit UpdateDouble(11, ahmax);
    sendMqttPublish(addr, 11, 0, ahmax, 0);
    //12    SOC (1‰ )    2
    double    soc =  getUintFromString(line,start,4) * 0.1f;
    sendMqttPublish(addr, 12, 0, soc, 0);
    emit UpdateDouble(12, soc);
    //13    Rated capacity (0.01Ah)    2
    double    rcap =  getUintFromString(line,start,4) * 0.01f;
    emit UpdateDouble(13, rcap);
    sendMqttPublish(addr, 13, 0, rcap, 0);
    //14    Number of cycles    2
    double    cycle =  getUintFromString(line,start,4);
    emit UpdateDouble(14, cycle);
    sendMqttPublish(addr, 14, 0, cycle, 0);
    //15    SOH (1‰ )    2
    double    soh =  getUintFromString(line,start,4) * 0.1f;
    emit UpdateDouble(15, soh);
    sendMqttPublish(addr, 15, 0, soh, 0);
    //16    Port voltage (0.01V)    2
    double    pvolt =  getUintFromString(line,start,4) * 0.01f;
    emit UpdateDouble(16, pvolt);
    sendMqttPublish(addr, 16, 0, pvolt, 1);
    //17    Reservation    2
    //18    Reservation    2
    //19    Reservation    2
    //20    Reservation    2
    //qDebug() << ts() << "7-12\t" << tcurr << "\t" << tbatt << "\t" << ahmax << "\t" << ahact << "\t" << soc ;
    //qDebug() << ts() << "13 "<< rcap << "\t" << cycle << "\t" << soh  << "\t" << pvolt;


    //17 power W
    double power = tbatt * tcurr;
    sendMqttPublish(addr, 17, 0, power, 0);

    //18 dx

    sendMqttPublish(addr, 18, 0, maxDx, 0);

    sendMqttPublish(addr, 19, 0, minVolt, 0);
    sendMqttPublish(addr, 20, 0, maxVolt, 0);
    sendMqttPublish(addr, 21, 0, minNo, 0);
    sendMqttPublish(addr, 22, 0, maxNo, 0);
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

void Seplos::oneLineRx(QString line)
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
        int mqttState = m_MqttClient.state();
        if (mqttState == 2) {
            processProV20(line);
        } else {
            m_MqttClient.disconnectFromHost();
            qDebug() << ts() << "reconnet to host";
            m_MqttClient.connectToHost();
            return;    // wait for next connect
        }
    } else {
        qDebug() << ts() << "Start not with @20";
    }


}

QString Seplos::ts(void)
{
    return QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss,zzz");
}

void Seplos::modbusBuildCrcAndCrThenSend(QString hex)
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

void Seplos::pollTelemetrie(int adr)
{
    qDebug() << ts() << "pollTelemetrie: " << adr;
    if (m_V3_Protocol) {
        pollV3(adr,0x1000);


    } else {

        emit UpdateCell(43,adr); // update status line
        // "~20014642E00201FD35\r"
        // set number and calculate CRC
        char buffer[255];
        char xbuffx[275];
        sprintf(buffer,"20%02d4642E00201",adr);


        int tlen = strlen(buffer);
        uint32_t tcrc = Seplos_CRC(buffer, tlen);
        qDebug() << ts() << "tcrc(dez): " << tcrc;

        sprintf(xbuffx,"~%s%04X\r",buffer,tcrc);
        doTx(xbuffx);  // read telemetrie
    }
}


// all 100ms
void Seplos::doTimer()
{
    m_TimerState++;
    //qDebug() << ts() << "m_TimerState: " << m_TimerState;

    setting* s = settingProvider();

    uint32_t start = s->getStartNo();
    uint32_t end = s->getEndNo();
    uint32_t ms = s->getWaitTimeMs();
    uint32_t ticks = ms/100;

    if (m_TimerState >= ticks) {
        m_TimerState = 0;

        // next adr
        m_ActAdr++;
        if (m_ActAdr > end) {
            m_ActAdr = start;
        }

        pollTelemetrie(m_ActAdr);
    }
}

