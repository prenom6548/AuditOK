#ifndef TRAYITEM_H
#define TRAYITEM_H

#include <QQuickItem>
#include <QObject>
#include <QIcon>
#include "status-notifier-host.h"

class TrayItemPrivate;
class TrayItem : public QObject
{
    Q_OBJECT
public:
    explicit TrayItem(const QString& itemSource, QObject* parent = nullptr);
    ~TrayItem();

    QString source() const;
    QString service() const;
    QIcon attentionIcon() const;
    QString attentionIconName() const;
    QString attentionMovieName() const;
    QString category() const;
    QIcon icon() const;
    QString iconName() const;
    QString iconThemePath() const;
    QString id() const;
    bool itemIsMenu() const;
    QIcon overlayIcon() const;
    QString overlayIconName() const;
    QString status() const;
    QString title() const;
    QString toolTipSubTitle() const;
    QString toolTipTitle() const;
    QString windowId() const;
    QString desktop() const;
    uint pid() const;

    /**
     * fixed in system tray
     */
    bool fixed() const;
    void setFixed(bool fixed);

    /**
     *  need record relative order
     */
    bool recordOrder() const;
    void setRecordOrder(bool recordOrder);

    void setParentItem(QQuickItem *item);
    void setDesktop(const QString & desktop);
    void contextMenu(int x, int y);
    void activate(int x, int y);
    void secondaryActivate(int x, int y);
    void scroll(int delta, const QString &direction);
    void provideXdgActivationToken(const QString &token);

public Q_SLOTS:
    void dataUpdated();
    void contextMenuReady(QMenu *menu);

Q_SIGNALS:
    void itemDataChanged(const QString &source);
    void activateResult(bool success);
    void itemReady(TrayItem* item);
    void menuStateChanged(bool state);

private:
    TrayItemPrivate *d = nullptr;

    void getDesktop();
};
#endif // TRAYITEM_H
