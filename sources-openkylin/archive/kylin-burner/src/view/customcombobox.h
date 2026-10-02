#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>
#include <QPainter>
#include <QString>
#include <QFontMetrics>
#include <QToolTip>
#include <QEnterEvent>

class CustomComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomComboBox(QWidget *parent = nullptr);
    void updateTooltip();
    
protected:
    void showTooltip();
    void hideTooltip();
    bool event(QEvent *event);

protected:
    // 重写绘制事件，自定义显示逻辑
    void paintEvent(QPaintEvent *e) override;
    
private:
    bool m_tooltipShown = false;
    QString m_tooltipContent;

private:
    // 裁剪文本的工具函数
    QString elideText(const QString &text, int maxWidth);
    bool isTextElided(const QString &text, int maxWidth) const;
    
private slots:
    void onCurrentIndexChanged(int index);
};

#endif // CUSTOMCOMBOBOX_H
