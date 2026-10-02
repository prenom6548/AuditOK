#include "cameracodestate.h"
#include "currentdeviceinfo.h"

#include <QDateTime>

CameraCodeState::CameraCodeState(KylinCamera *camera)
{
    cameraCurrent = camera;
}
void CameraCodeState::setFlag(bool res){
    flag = res;
}
void CameraCodeState::run()
{
    enum CameraState state;

    while(1)
    {
        QThread::msleep(100);//为了保证录像文件遇到特殊操作不损坏，录像结束后每隔1s读一下摄像头编码状态，直到编码结束才退出线程
        state = cameraCurrent->camera_get_state(CAMERA_ENCODE);
//        qDebug() << QDateTime::currentMSecsSinceEpoch() << "camera state:" <<state;
        if(state == CAMERA_ENCODE_STOP) break;
        else continue;
    }
    if(flag){
        exit();
    }else{
        qDebug() << "视频在正常情况下结束录制！" <<state;
    }
    Q_EMIT camerafinish();
}

