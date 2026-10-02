#ifndef CLIPBOARDTRAY_H
#define CLIPBOARDTRAY_H

#include <QtGui>
#include <QSystemTrayIcon>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include "clipboardmodel.h"

#include "clipboarddata.h"

class ClipboardTray : public QWidget
{
    Q_OBJECT
    Q_PROPERTY (Clipboardmodel* currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged);
    Q_PROPERTY (QVector<Clipboardmodel*> models READ models CONSTANT)
    Q_PROPERTY (bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)

public:
    ClipboardTray(QWidget *parent = 0);
    ~ClipboardTray(){};

    QSystemTrayIcon* mSystemTray;	//系统托盘
    QList<clipboardData> datas;
    QClipboard *m_clipboard;
    void handelData(QString m_fromText,QString m_timeText,QImage m_image,QString m_sourceText,clipboardData::Mode mode,const QMimeData* mData);

    Clipboardmodel* currentModel();
    //void setCurrentModel(Clipboardmodel* model);
    QVector<Clipboardmodel*> models();
    bool windowVisible();

    Clipboardmodel *allModel = new Clipboardmodel();
    Clipboardmodel *textModel = new Clipboardmodel();
    Clipboardmodel *imageModel = new Clipboardmodel();
    Clipboardmodel *fileModel = new Clipboardmodel();
    Clipboardmodel *linkModel = new Clipboardmodel();
    Clipboardmodel *collectionModel = new Clipboardmodel();

    int num = 0;

signals:
    void myTextDatasChanged();
    void currentModelChanged();
    void windowVisibleChanged();

public slots:
    void dataAdd();
    void setCurrentModel(Clipboardmodel* model);
    void dataNeedRemove();
    void setWindowVisible(bool flag = true);

private:
    void InitClipboardTray();
    void InitMenu();
    void InitData();

private slots:
    void ShowMenu();    	//菜单栏
    void ActiveTray(QSystemTrayIcon::ActivationReason reason);
    void HoverTray(QSystemTrayIcon::ActivationReason reason);
    void dataChanged();

private:

    QMenu* mMenu;					//菜单栏按钮
    QAction* mQuitAction;  			//退出
    QAction* mShowMainWindowAction; //显示主面板

    Clipboardmodel* m_currentModel;
    QVector<Clipboardmodel*> m_models;

    bool m_isDisplay=true;
    bool m_windowVisible = false;
};

#endif // CLIPBOARDTRAY_H
