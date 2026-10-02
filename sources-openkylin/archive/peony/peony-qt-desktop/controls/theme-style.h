#ifndef THEMESTYLE_H
#define THEMESTYLE_H

#include <QObject>
#include <QFont>
#include <QPalette>
#include <QColor>
class ThemeStylePrivate;
class ThemeStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int radius READ getRadius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY highlightColorChanged)
    Q_PROPERTY(QColor midlightColor READ midlightColor NOTIFY midlightColorChanged)
public:
    ThemeStyle(QObject *parent = nullptr);

    static ThemeStyle *instance();

    QColor highlightColor() const {
        return m_highlightColor;
    }
    QColor midlightColor() const {
        return m_midlightColor;
    }

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
    void highlightColorChanged();
    void midlightColorChanged();

private:
    void updateHighlightColors();

    int m_radius = 6;
    QColor m_highlightColor;
    QColor m_midlightColor;
};


#endif // THEMESTYLE_H
