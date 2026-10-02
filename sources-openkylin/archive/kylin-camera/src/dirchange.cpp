#include "dirchange.h"
#include <QFile>

DirChange::DirChange(){
    watcher = new QFileSystemWatcher;
    dirRead = new DirRead;
    gsettingItem = new DefaultGetSet;
    gsettingItem->setAccessibleName("kylin-camera_dirchange_DefaultGetSet_gsettingItem");
    savePath = gsettingItem->getSavePath();
    videoSavePath = gsettingItem->getVideoSavePath();
    watcher->addPath(savePath);
    watcher->addPath(videoSavePath);

    connect(watcher,&QFileSystemWatcher::directoryChanged,dirRead,[=](){dirRead->start();});
    //启动计时器，监听设备，
    connect(dirRead,&DirRead::finish,this,&DirChange::handle);
}
DirChange::~DirChange(){
    delete watcher;
    delete dirRead;
    delete gsettingItem;
}
void DirChange::handle(){
    QString filename = gsettingItem->getFileName();
    if(!QFile(filename).exists()){
        Q_EMIT setPixLabNull();
    }
}
void DirChange::dirChangeHandle(){
    watcher->removePath(savePath);
    savePath = gsettingItem->getSavePath();
    watcher->addPath(savePath);
    connect(watcher,&QFileSystemWatcher::directoryChanged,dirRead,[=](){dirRead->start();});
    QString filename = gsettingItem->getFileName();
    if(filename.contains(".jpg") || filename.contains(".png") || filename.contains(".bmp")){
        Q_EMIT setPixLabNull();
    }
}
void DirChange::videoDirChangeHandle(){
    watcher->removePath(videoSavePath);
    videoSavePath = gsettingItem->getVideoSavePath();
    watcher->addPath(videoSavePath);
    connect(watcher,&QFileSystemWatcher::directoryChanged,dirRead,[=](){dirRead->start();});
     QString filename = gsettingItem->getFileName();
    if(filename.contains(".mp4")){
        Q_EMIT setPixLabNull();
    }
}
