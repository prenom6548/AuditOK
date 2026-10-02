#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QSize>
#include "clicklabel.h"

class PixmapLabel : public ClickLabel
{
    Q_OBJECT

public:
    explicit PixmapLabel(QWidget *parent = nullptr);
    ~PixmapLabel();

    // 应用场景一：设置深浅主题分别展示的图片
    void setLightAndDarkPixmap(const QString &light, const QString &dark);

    // 应用场景二：设置跟随主题风格图标，不随深浅背景色变化
    void setUkuiIconSchema(const QString &schema, QSize size = QSize(24, 24));
    // 应用场景三：设置跟随主题风格图标（主题中也可能不存在该图标），随深浅色背景色变化
    void setUkuiIconSchema(const QString &schema, const QString &schemaDefault, QSize size = QSize(16, 16));
    // 应用场景四：设置跟随主题风格图标（主题中也可能不存在该图标），随主体风格：寻光、启典、合印等图标风格变化而变换，不随深浅背景色变化
    void setThemeIconSchema(const QString &schema, const QString &schemaDefault, QSize size = QSize(24, 24));
    // 已绑定信号，仅更新图片/图标即可
    void setOnlyFlush(bool reflush) { m_onlyFlush = reflush; }

private slots:
    void on_styleNameChanged(bool isDark);
    void on_themeIconChanged();

private:
    // 浅色背景时显示图
    QString m_light;
    // 深色背景时显示图
    QString m_dark;

    // 主题图标
    QString m_iconTheme;
    QString m_iconThemeDefault;
    // 是否自行更改颜色
    bool m_autoChangeColor = false;
    // 主题图标大小
    QSize m_iconSize;
    // 已绑定信号，仅更新图片/图标即可
    bool m_onlyFlush = false;
};

#endif // PIXMAPLABEL_H
