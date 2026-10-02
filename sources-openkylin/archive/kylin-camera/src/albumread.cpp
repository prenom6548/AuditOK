#include "albumread.h"
#include <QDebug>
AlbumRead::AlbumRead(QString str, QObject *parent) :
    QThread(parent)
{
    filename = str;
}


void AlbumRead::run()
{
    QFileInfo dir(filename);
    QFile file(filename);
    for(int i = 0;i < 50; i++)
    {
        if (dir.exists())
        {
            if(file.exists())
            {
                Q_EMIT finish(filename);
                return;
            }
        }
        QThread::msleep(100);
    }
}
