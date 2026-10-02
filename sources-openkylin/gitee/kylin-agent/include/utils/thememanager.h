#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QPalette>

class QApplication;
class QEvent;
class QTimer;

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum ThemeMode {
        FollowSystem = 0,
        Light = 1,
        Dark = 2
    };
    Q_ENUM(ThemeMode)

    static ThemeManager &instance();

    void applySavedTheme(QApplication *app);
    void setThemeMode(ThemeMode mode);
    ThemeMode themeMode() const { return m_mode; }
    bool isDark() const { return m_dark; }

signals:
    void themeChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit ThemeManager(QObject *parent = nullptr);

    void apply();
    bool systemPrefersDark() const;

    QApplication *m_app = nullptr;
    QPalette m_systemPalette;
    QTimer *m_systemThemeTimer = nullptr;
    ThemeMode m_mode = FollowSystem;
    bool m_dark = false;
};

#endif // THEMEMANAGER_H
