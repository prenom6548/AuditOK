#ifndef GLOBALSETTINGSADAPTER_H
#define GLOBALSETTINGSADAPTER_H

#include <QObject>
#include <QVariant>
#include <QMargins>
class QGSettings;

class GlobalSettingsAdapter : public QObject
{
    Q_OBJECT
    // 拆分的四个边距属性
    Q_PROPERTY(int panelMarginLeft READ panelMarginLeft NOTIFY panelMarginsChanged)
    Q_PROPERTY(int panelMarginTop READ panelMarginTop NOTIFY panelMarginsChanged)
    Q_PROPERTY(int panelMarginRight READ panelMarginRight NOTIFY panelMarginsChanged)
    Q_PROPERTY(int panelMarginBottom READ panelMarginBottom NOTIFY panelMarginsChanged)
public:
    static GlobalSettingsAdapter *getInstance();

    explicit GlobalSettingsAdapter(QObject *parent = nullptr);

    Q_INVOKABLE QVariant getValue(const QString &key) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE bool isKeyAvailable(const QString &key) const;
    Q_INVOKABLE bool useAutoLayout();

public:
    int panelMarginLeft() const { return m_margins.left(); }
    int panelMarginTop() const { return m_margins.top(); }
    int panelMarginRight() const { return m_margins.right(); }
    int panelMarginBottom() const { return m_margins.bottom(); }

    QMargins panelMargins() const { return m_margins; }

    void setPanelMargins(const QMargins &newMargins) {
        if (m_margins != newMargins) {
            m_margins = newMargins;
            Q_EMIT panelMarginsChanged();
        }
    }

public Q_SLOTS:
    void panelSettingsChanged();

Q_SIGNALS:
    void panelMarginsChanged();
    void valueChanged(const QString &key, const QVariant &value);

private:
    // 连接到单例的信号
    void onGlobalSettingsChanged(const QString &key);

    QGSettings *m_panelSetting = nullptr;
    QMargins m_margins;
};

#endif // GLOBALSETTINGSADAPTER_H
