#ifndef UKMEDIATRAYICON_H
#define UKMEDIATRAYICON_H

#include <QSystemTrayIcon>

class UkmediaTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    UkmediaTrayIcon(QWidget* = nullptr);
    ~UkmediaTrayIcon();

Q_SIGNALS:
    void wheelRollEventSignal(bool);

protected:
    bool event(QEvent *e) ;
};

#endif // UKMEDIATRAYICON_H
