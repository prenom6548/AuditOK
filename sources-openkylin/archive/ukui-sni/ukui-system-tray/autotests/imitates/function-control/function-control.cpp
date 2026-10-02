#include "function-control.h"

namespace UkuiQuick {

FunctionControl *FunctionControl::instance()
{
    static FunctionControl control;
    return &control;
}

FunctionControl::FunctionControl(QObject *parent) : QObject(parent)
{
}

void FunctionControl::addWatch(const QString &name)
{
    if (!m_watches.contains(name)) {
        m_watches.append(name);
    }
}

QJsonObject FunctionControl::functionControlObject(const QString &name) const
{
    return m_objects.value(name);
}

void FunctionControl::setFunctionControlObject(const QString &name, const QJsonObject &object)
{
    m_objects.insert(name, object);
}

QStringList FunctionControl::watches() const
{
    return m_watches;
}

void FunctionControl::emitFunctionControlChanged(const QString &name)
{
    Q_EMIT functionControlChanged(name);
}

void FunctionControl::emitFunctionControlExit()
{
    Q_EMIT functionControlExit();
}

} // namespace UkuiQuick
