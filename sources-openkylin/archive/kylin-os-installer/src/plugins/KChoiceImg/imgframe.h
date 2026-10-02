#ifndef IMGFRAME_H
#define IMGFRAME_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include <QTextBrowser>
#include <QLabel>
#include "QLineEdit"
#include "QRadioButton"
#include "QPushButton"
#include "QComboBox"

#include <QThread>
#include <QTimer>
#include "../PluginService/ui_unit/AutoResize.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "kyimgosproberprocess.h"

namespace KInstaller {


class ImgFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit ImgFrame(QWidget *parent = nullptr);

    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void translateStr();
    bool liveInstallStatus;
    bool ghostInstallStatus;
    QStringList getKyimgs();
    void initFocusPolicy();                     //初始化焦点序列

signals:
public slots:
    void clickNextButton();
   // void SetFilePath();
    void ChangeLoad1();
    void ChangeLoad2();
    void changefilePath(const QString &text);
//    void kyimgThread();
    void proberKyimgs();

private:
    void keyPressEvent(QKeyEvent *event);
     void changeEvent(QEvent *event);
public:

    QLabel* m_mainTitle;
    QRadioButton *liveChoice;
    QRadioButton *ghostChoice;
    QPushButton *openFile;
    QComboBox *filePath;
    QString imgPath;
    QStringList kyimgList;
    QStringList oldKyimgs;
   // QThread* m_thread;
   // KyimgOsproberProcess *m_kyimgOsproberProcess;
    KyimgOsproberProcess *m_thread;


    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};
}
#endif // ImgFrame_H
