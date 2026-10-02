#ifndef PIXMAPBUTTON_H
#define PIXMAPBUTTON_H

#include <QPushButton>

class PixmapButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PixmapButton(QWidget *parent = nullptr);
    ~PixmapButton();

    // 设置跟随主题图标
    void setThemeIcon(const QString &themeIconName, const QString &defaultIconName = "", int size = 16);

    QString pluginBtnHoverColor(bool hoverFlag);

private:
    // 主题图标
    QString m_iconTheme;
    // 默认本地图标文件
    QString m_defaultIconName;
    // 主题图标大小
    int m_iconSize;

    QString m_hoverColor;
    QString m_clickColor;
};

#endif // PIXMAPBUTTON_H
