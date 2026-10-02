#ifndef THEMESTYLE_H
#define THEMESTYLE_H

#include <QObject>
#include <QFont>
#include <QPalette>
class ThemeStylePrivate;
class ThemeStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int radius READ getRadius WRITE setRadius NOTIFY radiusChanged)
public:
    ThemeStyle(QObject *parent = nullptr);

    static ThemeStyle *instance();

    Q_INVOKABLE QColor highlight();
    Q_INVOKABLE QColor midlight();

    int getRadius() const {
        return m_radius;
    };
    void setRadius(int newRadius) {
        if (m_radius != newRadius) {
            m_radius = newRadius;
            Q_EMIT radiusChanged();
        }
    };

Q_SIGNALS:
    void radiusChanged();

private:
    int m_radius = 6;
};


#endif // THEMESTYLE_H
