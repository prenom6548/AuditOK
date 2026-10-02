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
#include <QGSettings/QGSettings>
#include <QTimer>
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>

#include "clipboardmodel.h"
#include "clipboarddata.h"
#include "waylandbackendThread.h"

class ClipboardTray : public QWidget
{
    Q_OBJECT
    Q_PROPERTY (Clipboardmodel* currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged);
    Q_PROPERTY (QVector<Clipboardmodel*> models READ models CONSTANT);
    Q_PROPERTY (bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged);
    Q_PROPERTY (double transparency READ transparency);
    Q_PROPERTY (bool keepAbove READ keepAbove WRITE setkeepAbove NOTIFY keepAboveChanged);
    Q_PROPERTY (bool isMirror READ isMirror WRITE setIsMirror NOTIFY isMirrorChanged);

public:
    ClipboardTray(QWidget *parent = 0);
    ~ClipboardTray();

    QSystemTrayIcon* mSystemTray;	//系统托盘
    QList<clipboardData> datas;
    QClipboard *m_clipboard;
    void handelData(QString m_fromText,QString m_timeText,QImage m_image,QString m_sourceText,clipboardData::Mode mode,const QMimeData* mData);

    Clipboardmodel* currentModel();
    //void setCurrentModel(Clipboardmodel* model);
    QVector<Clipboardmodel*> models();
    bool windowVisible();
    double transparency();
    bool keepAbove();
    bool isMirror();

    Q_INVOKABLE QColor dtColor(QString str);
    Q_INVOKABLE void setWindowBlurEffect(bool isActive);
    Q_INVOKABLE void xdgOpen(QString str);
    Q_INVOKABLE void openImageOrFile(QString str);
    Q_INVOKABLE void openDir(QString str);

    void getQmlWindow(QWindow *window);

    Clipboardmodel *allModel = new Clipboardmodel();
    Clipboardmodel *textModel = new Clipboardmodel();
    Clipboardmodel *imageModel = new Clipboardmodel();
    Clipboardmodel *fileModel = new Clipboardmodel();
    Clipboardmodel *linkModel = new Clipboardmodel();
    Clipboardmodel *collectionModel = new Clipboardmodel();

    int num = 0;
    QGSettings *settings;

signals:
    void myTextDatasChanged();
    void currentModelChanged();
    void windowVisibleChanged();
    void styleThemeChanged();
    void firstCopy();
    void keepAboveChanged();
    void isMirrorChanged();
    void firstShowWindow();

public slots:
    void dataAdd();
    void setCurrentModel(Clipboardmodel* model);
    void dataNeedRemove();
    void removeAllData();
    void setWindowVisible(bool flag = true);
    void transparencyChanged();
    void setkeepAbove(bool arg);
    void themeChanged(QString str);
    void updateModels();
    void setIsMirror(bool arg);
    void handelSelecPro();
    void selectionNilSolt();
    void selectionNewSolt();

private:
    void InitClipboardTray();
    void InitMenu();
    void InitData();
    bool uniqueCheck(clipboardData *data, clipboardData::Mode mode);

private slots:
    void ShowMenu();    	//菜单栏
    void ActiveTray(QSystemTrayIcon::ActivationReason reason);
    void HoverTray(QSystemTrayIcon::ActivationReason reason);
    void dataChanged();

private:
    QTimer mTimer;
    QMenu* mMenu;					//菜单栏按钮
    QAction* mQuitAction;  			//退出
    QAction* mShowMainWindowAction; //显示主面板

    Clipboardmodel* m_currentModel;
    QVector<Clipboardmodel*> m_models;

    WaylandBackendThread* wayBackendThread;

    bool m_isDisplay=true;
    bool m_windowVisible = false;
    double m_transparency = 0.5;
    QWindow *m_Window;
    UkuiWindowHelper *m_helper;
    bool m_keepAbove = false;
    bool m_isFirstCopy = true;
    bool m_isMirror = false;
    bool m_isFirstShow = true;
    bool selectionProIsAlive = false;
};

#endif // CLIPBOARDTRAY_H
