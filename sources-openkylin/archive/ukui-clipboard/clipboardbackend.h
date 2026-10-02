#ifndef CLIPBOARDBACKEND_H
#define CLIPBOARDBACKEND_H

#include <QObject>
#include <QScopedPointer>
#include <QVariant>
#include <QIcon>
#include <QTimer>
#include <kwidget.h>
#include <kballontip.h>
#include <QGSettings/QGSettings>
#include "data.h"

/*
 * Q_INVOKABL是个宏定义
 * 这个宏定义 针对的是函数，不是变量
 * 经过Q_INVOKABL声明过得函数可以被元对象系统调用
 * QtQuick 也在元对象系统内，所以再QML中也可以访问这个被声明了的函数
*/

class ClipboardBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant contentMenus READ contentMenus NOTIFY contentMenusChanged)
    Q_PROPERTY(int displayingType READ displayingType WRITE setDisplayingType NOTIFY displayingTypeChanged)
    Q_PROPERTY(QString displayedIconName READ displayedIconName NOTIFY displayedIconNameChanged)
    Q_PROPERTY(int originalType READ originalType WRITE setOriginalType NOTIFY originalTypeChanged)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)
    Q_PROPERTY(QString richText READ richText WRITE setRichText NOTIFY richTextChanged)
    Q_PROPERTY(QString plainText READ plainText WRITE setPlainText NOTIFY plainTextChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(double transparency READ transparency NOTIFY transparencyChanged)
public:
    explicit ClipboardBackend(QObject *parent = nullptr);
    QVariant contentMenus() const { return QVariant::fromValue(m_contentMenus); }

    int displayingType() { return m_displayingType; }
    Q_INVOKABLE void setDisplayingType(int type);

    int originalType() { return m_originalType; }
    Q_INVOKABLE void setOriginalType(int type);

    QSize imageSize() { return m_imageSize; }
    Q_INVOKABLE void setImageSize(QSize size);

    QString richText() const { return m_richText; }
    Q_INVOKABLE void setRichText(const QString &richText);

    QString plainText() const { return m_plainText; }
    Q_INVOKABLE void setPlainText(const QString &plainText);

    Q_INVOKABLE QString displayedIconName() { return m_displayedIconName; }
    Q_INVOKABLE QIcon getIcon(QString iconName) {
        return QIcon::fromTheme(iconName);
    }

    Q_INVOKABLE int imageNumber() const;
    Q_INVOKABLE void copyToClipboard(int contentType);
    Q_INVOKABLE void performPasteOperation();
    Q_INVOKABLE QPoint getGlobalMousePosition() const;    
    Q_INVOKABLE void removeContentMenu(int index);
    Q_INVOKABLE void showClipboardWindow();
    Q_INVOKABLE bool processClipboardData();

    Q_INVOKABLE void setClipboardWindowVisible(bool visible);
    QColor backgroundColor() const {return m_backgroundColor;}
    double transparency() {return m_transparency;}
    Q_INVOKABLE void clearContentMenu();
    Q_INVOKABLE void clearImages();

public:
    InMemoryImageProvider *getImageProvider();
    void open();

public slots:
    void getResult(const QString &text);

private:
    void addContentMenu(ContentMenu *menu);
    void addImage(const QImage &image);
    void monitorShortCut();
    void showContentMenu();
    void pasteClicked() const;
    void monitorIconThemeChange();
    void setThemeSettings(QString str);

signals:
    void contentMenusChanged();
    void richTextChanged();
    void plainTextChanged();
    void displayingTypeChanged();
    void displayedIconNameChanged();
    void originalTypeChanged();
    void imageSizeChanged();
    void contentMenuVisibleChanged(bool visible);
    void clipboardWindowVisibleChanged(bool visible);
    void iconThemeChanged();
    void backgroundColorChanged();
    void transparencyChanged();
    void actionTriggered();

private:
    QList<QObject*> m_contentMenus;
    kdk::KBallonTip* m_pNormalTip;
    int m_displayingType;
    int m_originalType;
    QSize m_imageSize;
    QString m_richText;
    QString m_plainText;
    QString m_displayedIconName;
    QTimer m_timer;
    // 不需要自己释放，engine 会管理释放
    InMemoryImageProvider *m_imageProvider = nullptr;
    bool m_isWayland = false;
    bool m_clipboardWindowVisible = false;
    QColor m_backgroundColor;
    double m_transparency = 0.3;
    QGSettings *m_settings;
    bool m_firstTriggered = false;
};

#endif // CLIPBOARDBACKEND_H
