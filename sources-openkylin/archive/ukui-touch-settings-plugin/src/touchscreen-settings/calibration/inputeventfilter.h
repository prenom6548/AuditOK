#ifndef INPUTEVENTFILTER_H
#define INPUTEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>

class xcb_input_hierarchy_event_t;
class InputEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit InputEventFilter(QObject* parent = nullptr);
    ~InputEventFilter() override = default;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

Q_SIGNALS:
    void deviceAdded(int deviceId);
    void deviceRemoved(int deviceId);
    void deviceEnabled(int deviceId);
    void deviceDisabled(int deviceId);

private:
    void handleHierarchyEvent(const xcb_input_hierarchy_event_t *event);
};

#endif // INPUTEVENTFILTER_H
