#ifndef GLOBALSIGNALS_H
#define GLOBALSIGNALS_H

#include <QObject>
#include <QString>

class GlobalSignals : public QObject
{
    Q_OBJECT
public:
    GlobalSignals() = default;
    ~GlobalSignals() = default;

signals:
    // 主题图标变更
    void themeIconChanged();

    // 主题背景变化信号
    void styleNameChanged(bool isDark);

    // 窗口或控件的背景色发生了变化。主要用于通知主题监控模块，用于修正控件颜色，以简化用户自定义背景色跟随主题变化的代码
    void backgroundColorChanged();

    // 字体（家族或大小）变化
    void fontChanged(int fontSize);

    // 强调色变化
    void themeColorChanged();
};

#endif // GLOBALSIGNALS_H
