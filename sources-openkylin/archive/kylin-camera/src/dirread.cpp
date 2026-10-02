#include "dirread.h"
#include <QDebug>
DirRead::DirRead(){

}
//每间隔一段时间，就重新监听当前存储路径（文件夹）的变化
void DirRead::run(){
    qDebug() << "dirread.cpp:34";
    QThread::msleep(1000);
    Q_EMIT finish();
}
