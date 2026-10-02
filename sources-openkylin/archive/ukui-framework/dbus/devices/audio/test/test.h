#ifndef __TEST_H__
#define __TEST_H__

#include "audio_define.h"
#include "audio.h"

class AudioTest : public QObject
{
    Q_OBJECT

public:
    explicit AudioTest(QObject *obj, QObject *parent = nullptr);
    void initSignals();

    /**
     * @brief paVolumeToValue 音量值转换成滑动条值
     * @param volume 音量值，范围0~65536
     * @return  音量值，范围0~100
     */
    int paVolumeToValue(int volume);

    ~AudioTest();

private:
    QObject *m_Obj;

public slots:
    /**
     * @brief updateSinkVolume 系统输出设备音量改变信号
     * @param volume 音量值，范围0~100
     */
    void updateSinkVolume(int volume);

    /**
     * @brief updateSinkMute 系统输出设备静音状态改变信号
     * @param mute 静音状态，true为静音，false为取消静音
     */
    void updateSinkMute(bool mute);

    /**
     * @brief updateSinkDevice 系统输出设备改变信号
     * @param device 输入设备的名称
     */
    void updateSinkDevice(QString device);

    /**
     * @brief updateSourceVolume 系统输入设备音量改变信号
     * @param volume 音量值，范围0~100
     */
    void updateSourceVolume(int volume);

    /**
     * @brief updateSourceMute 系统输入设备静音状态改变信号
     * @param mute 静音状态，true为静音，false为取消静音
     */
    void updateSourceMute(bool mute);

    /**
     * @brief updateSourceDevice 系统输入设备改变信号
     * @param device 输入设备的名称
     */
    void updateSourceDevice(QString device);
};

#endif // TEST_H
