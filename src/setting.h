#ifndef SETTING_H
#define SETTING_H

//#include <QObject>
#include <QString>

class setting
{
  //  Q_OBJECT
public:
    setting();
    QString getRs485Dev();
    QString getMqttPrefix();
    QString getMqttHost();
    QString getmMqttUser();
    QString getMqttPassword();
    void setMqttHost(QString value);
    //void setMqttPassword(QString value); not for secuity reasons
    void setMqttPort(uint32_t value);
    void setRs485Dev(QString value);
    void setStartNo(uint32_t value);
    void setEndNo(uint32_t value);
    void setAutoStartDelayMs(uint32_t value);
    void setWaitTimeMs(uint32_t value);
    uint32_t getStartNo();
    uint32_t getEndNo();
    uint32_t getAutoStartDelayMs();
    uint32_t getWaitTimeMs();
    uint32_t getMqttPort();
private:
    uint32_t m_StartNo;
    uint32_t m_EndNo;
    uint32_t m_AutoStartDelayMs;
    uint32_t m_WaitTimeMs;
    QString  m_MqttPrefix;
    QString  m_MqttHost;
    uint32_t m_MqttPort;
    QString  m_MqttUser;
    QString  m_MqttPassword;
    QString  m_Rs485Dev;

};

setting*  settingProvider(void);

#endif // SETTING_H
