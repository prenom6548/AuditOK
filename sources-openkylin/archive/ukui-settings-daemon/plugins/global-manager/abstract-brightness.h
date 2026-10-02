#ifndef BRIGHTNESSFACTORY_H
#define BRIGHTNESSFACTORY_H

#include <QObject>

class AbstractBrightness : public QObject
{
    Q_OBJECT
public:
    AbstractBrightness(){}

    virtual int getMaxBrightness() = 0;

    virtual int getBrightness() = 0;

    virtual int setBrightness(int brightness) = 0;

    virtual bool setOutputBrightness(QString appName, QString outputName, int brightness) = 0;

    virtual int connectTheSignal() = 0;

    virtual QString backend() = 0;

    virtual bool isEnhancementBrightness() = 0;

Q_SIGNALS:

    /**
     * @brief primaryChanged
     * @param Bright
     * @return
     */
    void enhancementBrightnessSwitchChanged(bool switchState);
};

#endif // BRIGHTNESSFACTORY_H
