#ifndef SHOWPROGRESSBAR_H
#define SHOWPROGRESSBAR_H

#include <QObject>
#include <QTimer>
namespace KInstaller {

class ShowProgressBar : public QObject
{
    Q_OBJECT
public:
    explicit ShowProgressBar(QObject *parent = nullptr);
    void start();

//    quint64 getDirSize(const QString filepath);
//    bool copyDir(const QString &source, const QString &destination, bool override);
    void ReadProgress();
    bool TotalInodesInDir(const QString strPathOfDir, int & nTotal);//统计目录strPathOfDir中inode的数目，并将统计结果放入nTotal中。
signals:
    void signelProgressValue(int value);
public slots:
    void flushProgressBar();
public:

    QTimer* timer;
    int m_value;
    int m_nStart=0;
    int m_nReadStart=0;
    int m_nReadEnd=0;
    int m_nEnd=1;
    int m_Cnt=1;
    int m_nEndOld=0;
    int m_SkipCnt=18;
    int m_nTotal;//镜像中文件系统中总文件和子目录数目（不含.和..目录）
    int m_nCopyed;//安装新系统时已经拷贝好的文件总数目（不含.和..目录）
};

}
#endif // SHOWPROGRESSBAR_H
