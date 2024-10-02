#include "setting.h"

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
    m_StartNo      = 1;
    m_EndNo        = 2;
    m_AutoStartDelayMs = 2000;  // 2 sec
    m_WaitTimeMs = 10000;       // 30 sec.
    m_MqttPrefix   = "BMS";
    m_MqttHost     = "192.168.6.128";
    m_MqttPort     = 1883;
    m_MqttUser     = "mps";
    m_MqttPassword = "void";      // TODO will be overwritten from /etc/seplos/seplos_qt.ini
    m_Rs485Dev     = "COM12";
}

QString setting::getPort()
{
    return m_Rs485Dev;
}

QString setting::getMqttPrefix()
{
    return m_MqttPrefix;
}

QString setting::getMqttHost()
{
    return m_MqttHost;
}

uint32_t setting::getMqttPort()
{
    return m_MqttPort;
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
