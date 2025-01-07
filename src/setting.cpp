#include "setting.h"

#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QDir>

setting*  settingProvider()
{
    static setting * g_Setting = NULL;

    if (g_Setting == NULL) {
        g_Setting = new setting();
    }
    return g_Setting;
}


setting::setting()
{
    QString fn = "QtSeplosMqtt.ini";
    QString fqfn = "/etc/seplos/" + fn;
#ifdef WIN32
    fqfn = QDir::homePath() + QDir::separator() + "qt" + QDir::separator() + fn;
#endif
    qDebug() <<  "setting reading from fqfn: " << fqfn;

    QSettings qs(fqfn, QSettings::IniFormat);

    m_StartNo          = qs.value("StartNo","1").toInt();
    m_EndNo            = qs.value("EndNo","1").toInt();
    m_AutoStartDelayMs = qs.value("AutoStartDelayMs","0").toInt();
    m_WaitTimeMs       = qs.value("WaitTimeMs","10000").toInt();
    m_MqttPrefix       = qs.value("MqttPrefix","BMS").toString();
    m_MqttOffset       = qs.value("MqttOffset","0").toInt();
    m_MqttHost         = qs.value("MqttHost","BMS").toString(); // "192.168.6.128";
    m_MqttPort         = qs.value("MqttPort","1883").toInt();
    m_MqttUser         = qs.value("MqttUser","user1").toString();
    m_MqttPassword     = qs.value("MqttPassword","void").toString();       // TODO will be overwritten from /etc/seplos/seplos_qt.ini
    m_Rs485Dev         = qs.value("Rs485Dev","COM12").toString();
    m_Version          = qs.value("Version","2").toInt();

    qDebug() <<  "setting m_StartNo: " << m_StartNo << " m_EndNo: " << m_EndNo << " offset: " << m_MqttOffset;
    qDebug() <<  "setting m_AutoStartDelayMs: " << m_AutoStartDelayMs;
    qDebug() <<  "setting m_Rs485Dev: " << m_Rs485Dev;
    
    QFile sl(m_Rs485Dev);
    if (sl.exists()) {
		QString xx = sl.symLinkTarget();
		//qDebug() <<  "setting m_Rs485Dev xx: " << xx;
		xx = xx.replace("/dev/","");
        qDebug() <<  "setting m_Rs485Dev smylink resolved: " << xx;
        m_Rs485Dev = xx;
    } else {
		qDebug() <<  "setting m_Rs485Dev not found: " << m_Rs485Dev;
	}
}

QString setting::getRs485Dev()
{
    return m_Rs485Dev;
}

void setting::setRs485Dev(QString value)
{
    m_Rs485Dev = value;
}

QString setting::getMqttPrefix()
{
    return m_MqttPrefix;
}

QString setting::getMqttHost()
{
    return m_MqttHost;
}

void setting::setMqttHost(QString value)
{
    qDebug() << "setting::setMqttHost" << value;
    m_MqttHost = value;
}


QString setting::getmMqttUser()
{
    return m_MqttUser;
}

QString setting::getMqttPassword()
{
    return m_MqttPassword;
}

uint32_t setting::getMqttPort()
{
    return m_MqttPort;
}

uint32_t setting::getMqttOffset()
{
    return m_MqttOffset;
}



void setting::setMqttPort(uint32_t value)
{
    m_MqttPort = value;
}


uint32_t setting::getStartNo()
{
    return m_StartNo;
}

uint32_t setting::getEndNo()
{
    return m_EndNo;
}


uint32_t setting::getAutoStartDelayMs()
{
    return m_AutoStartDelayMs;
}

uint32_t setting::getWaitTimeMs()
{
    return m_WaitTimeMs;
}


void setting::setStartNo(uint32_t value)
{
    m_StartNo = value;
}

void setting::setEndNo(uint32_t value)
{
    m_EndNo = value;
}

void setting::setAutoStartDelayMs(uint32_t value)
{
    m_AutoStartDelayMs  = value;
}

void setting::setWaitTimeMs(uint32_t value)
{
    m_WaitTimeMs = value;
}

uint32_t setting::getVersion() {
    return m_Version;
}
