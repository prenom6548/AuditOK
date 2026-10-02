#ifndef FUNCTION_CONTROL_H
#define FUNCTION_CONTROL_H

#include <QObject>
#include <QJsonObject>
#include <QHash>
#include <QStringList>

namespace UkuiQuick {

class FunctionControl : public QObject
{
    Q_OBJECT
public:
    static FunctionControl *instance();

    void addWatch(const QString &name);
    QJsonObject functionControlObject(const QString &name) const;
    void setFunctionControlObject(const QString &name, const QJsonObject &object);

    QStringList watches() const;

    void emitFunctionControlChanged(const QString &name);
    void emitFunctionControlExit();

Q_SIGNALS:
    void functionControlChanged(const QString &name);
    void functionControlExit();

private:
    explicit FunctionControl(QObject *parent = nullptr);

    QHash<QString, QJsonObject> m_objects;
    QStringList m_watches;
};

} // namespace UkuiQuick

#endif // FUNCTION_CONTROL_H
