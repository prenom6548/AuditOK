#include "build.h"

void build::run()
{
    //生成源码orig文件
    QProcess process1;
    QString dir1 = QDir::homePath() + "/.cache/theme-build/"+FileProcess::g_date;
//    qDebug() << dir1;
    QString command1 = "tar";
    QStringList arguments1;
    arguments1 << "-zcvf" << FileProcess::g_themeENName+"_1.0.orig.tar.xz"
               << "--exclude=.git" << "--exclude=debian" << FileProcess::g_date;


    process1.setWorkingDirectory(dir1);
    process1.start(command1, arguments1);

    QObject::connect(&process1, &QProcess::readyReadStandardOutput, [&]() {
        QString output = process1.readAllStandardOutput();
        qDebug() << "Output from tar: " << output;
    });

    process1.waitForFinished(-1);
    emit progressChanged(10);

    //编译
    QProcess process2;
    QString dir2 = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src/";
//    qDebug() << dir2;
    QString command2 = "debuild";
    QStringList arguments2;
    arguments2 << "-us"<<"-uc";

    process2.setWorkingDirectory(dir2);
    process2.start(command2, arguments2);
    QString allOutput;
    // 连接debuild进程的输出信号和槽
    QObject::connect(&process2, &QProcess::readyReadStandardOutput, [&]() {
        QString output = process2.readAllStandardOutput();
        allOutput += output;
        qDebug() << "Output from debuild: " << allOutput;

        int startIndex = allOutput.indexOf("错误");

        if (startIndex != -1) {
            // 输出提取的部分
            m_errorinfo = allOutput.mid(startIndex);
        }

        if (allOutput.contains("错误", Qt::CaseInsensitive)) {
              qDebug() << "Error detected. Stopping the process.";
              m_iserror = true;
              process2.kill();  // 停止进程
           } else{
            if(output=="dh clean\n"){
                emit progressChanged(20);
            }else if(output=="   dh_installdocs\n"){
                emit progressChanged(60);
            }else if(output=="   dh_builddeb\n"){
                emit progressChanged(70);
            }
        }

    });

    // 等待debuild进程完成
    process2.waitForFinished(-1);

    // 完成后更新进度条的值为90
    if(!m_iserror){
        emit progressChanged(90);

        // 编译记录清除
        QProcess process3;
        QString command3 = "debuild";
        QStringList arguments3;
        arguments3 << "--"<<"clean";

        process3.setWorkingDirectory(dir2);
        process3.start(command3, arguments3);
        // 等待debuild进程完成
        process3.waitForFinished(-1);
        // 完成后更新进度条的值为100
        emit progressChanged(100);

    }else{
        emit progressError(m_errorinfo);
    }
}
