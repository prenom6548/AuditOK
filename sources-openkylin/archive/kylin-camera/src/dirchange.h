#ifndef DIRCHANGE_H
#define DIRCHANGE_H


#include <QFileSystemWatcher>
#include <QWidget>
#include "dirread.h"
#include "defaultgetset.h"

class DirChange: public QWidget
{
    Q_OBJECT
public:
    DirChange();
    ~DirChange();

private:
    QFileSystemWatcher *watcher = nullptr;
    DirRead *dirRead = nullptr;
    DefaultGetSet *gsettingItem = nullptr;//gsetting读取设置的接口类
    QString savePath;
    QString videoSavePath;

    void handle();
Q_SIGNALS:
    void setPixLabNull();
public Q_SLOTS:
    void dirChangeHandle();
    void videoDirChangeHandle();
};

#endif // DIRCHANGE_H
