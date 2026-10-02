#ifndef UISERVICEONLINE_H
#define UISERVICEONLINE_H

#include <QLabel>
#ifdef HAS_WEBENGINE
#include <QWebEngineView>
#include <QWebEngineSettings>
#endif
#include <QPushButton>
#include "../constant.h"
class UiServiceOnline : public QWidget
{
    Q_OBJECT
public slots:
    void indexChanged(QString name);

public:
    UiServiceOnline(QWidget *parent = nullptr);
    ~UiServiceOnline();

private:
    enum IconType { Loading = 0, Error, NoWebEngine } m_iconType = Loading;
    void creatWebPage();
    void setIconPix();
    bool m_loaded = false;
#ifdef HAS_WEBENGINE
    QWebEngineView *m_webView = nullptr;
#endif
    QWidget *m_iconWidget = nullptr;
    QLabel *m_labelText = nullptr;
    QPushButton *m_labelIcon = nullptr;
    QPushButton *m_retryBtn = nullptr;
    QString m_onlineUrl;
    QString m_objectName = NAME_SERVICE_ONLINE;
    QWidget *m_cardBase = nullptr;
    QWidget *m_webEngineViewWiget = nullptr;
};

#endif // UISERVICEONLINE_H
