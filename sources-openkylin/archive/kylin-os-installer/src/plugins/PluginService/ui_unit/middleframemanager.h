#ifndef MIDDLEFRAMEMANAGER_H
#define MIDDLEFRAMEMANAGER_H

#include <QObject>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QGridLayout>
#include <QDebug>

namespace KInstaller {

class MiddleFrameManager :public QWidget
{
    Q_OBJECT
public:
    explicit MiddleFrameManager(QWidget *parent = nullptr);
    virtual ~MiddleFrameManager();

    void initUI();                  //显示ui
    void addStyleSheet();           //添加样式
    void translateStr();
    void initNextBnt();
    void initPreBnt();

signals:
    void signalStackPageChanged(int step);
    void signalStackFrameManager(QWidget* widget);
    void signalStartInstall();
    void signalInstallError(QString str);
    void signalInstalScriptlFinished();
    void signalKeyboradClose();
    void backspacepressed();
    void enterpressed();\
    void testSignal(QString objectName);

public slots:
    virtual void clickNextButton() = 0;
    virtual void clickPrevButton(QWidget *p){
        if(p==this)
            emit backspacepressed();
    }
    virtual void getmessage(QWidget *p,QString str){
        return;
    }
    virtual  void slotStartInstall(){return;}
    virtual void setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight){
         return;
    }

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public:
    QGridLayout* gLayout;
    QWidget* m_Widget ;
//    QPushButton* m_preBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
};
}
#endif // MIDDLEFRAMEMANAGER_H
