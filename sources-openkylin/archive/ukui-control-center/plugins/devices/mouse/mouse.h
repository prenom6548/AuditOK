#ifndef MOUSE_H
#define MOUSE_H

#include <QObject>
#include <QWidget>

#include "shell/interface.h"

#include "mouseui.h"

class Mouse : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Mouse();
    ~Mouse();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    bool            mFirstLoad   = true;
    QString         pluginName   = "";
    int             pluginType   = 0;
    MouseUI         *mouseWidget = nullptr;
    QDBusInterface  *mouseDbus   = nullptr;

    void initMouseStatus();
    void initDominantHand();
    void initScrollDirection();
    void initWheelSpeed();
    void initDoubleClickInterval();
    void initPointerSpeed();
    void initMouseAcceleration();
    void initTailLightEffect();
    void initPointerPosition();
    void initPointerSize();
    void initBlinkingCursor();
    void initCursorSpeed();

    void initConnection();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

public Q_SLOTS:
    void dominantHandSlot(int id);
    void scrollDirectionSlot(int id);
    void wheelSpeedSlot();
    void doubleClickIntervalSlot();
    void pointerSpeedSlot(int value);
    void tailLightEffectSlot(bool checked);
    void mouseAccelerationSlot(bool checked);
    void pointerPositionSlot(int index);
    void pointerSizeSlot(QAbstractButton *button);
    void blinkCursorOnTextSlot(bool checked);
    void cursorSpeedSlot();

    void dataChanged(QString key);

signals:

};

#endif // MOUSE_H
