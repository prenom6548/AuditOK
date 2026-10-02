#include "diroperation.h"
#include <QDebug>
#include <QDir>

DirOperation::DirOperation(QObject *parent) : QObject(parent)
{

}

void DirOperation::DeleteHistoryDir(const QString &date)
{
    QString dirPath = QDir::homePath() + "/.cache/theme-build/" + date;

    QDir builderDir(dirPath);
    if (builderDir.exists()){
        if (builderDir.removeRecursively()){
            qDebug() << "成功删除目录：" << dirPath;
        }else{
            qDebug() << "无法删除目录：" << dirPath;
        }
    }else{
        qDebug() << "目录不存在：" << dirPath;
    }
}
