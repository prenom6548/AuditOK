#ifndef __KOM_LABEL_H__
#define __KOM_LABEL_H__

#include <QLabel>
#include <QScopedPointer>
#include <QColor>

/*
 * KomLabel 支持如下功能：
 * 1. 自动响应字号变化
 * 2. 文本超长省略处理
 */

namespace kom {

class KomLabelPrivate;

class KomLabel : public QLabel
{
    Q_OBJECT
public:
    KomLabel(QWidget *parent = nullptr);
    ~KomLabel();
    void setAlignment(Qt::Alignment a);
    void setText(const QString &text);
    void setBold(bool enable);
    void setFontSize(int size);
    void setFirstNumColor(QColor color);
    void setSecondNumColor(QColor color);
    void setThirdNumColor(QColor color);
    void setTransparency(float t);

protected:
    QScopedPointer<KomLabelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(KomLabel);
    Q_DECLARE_PRIVATE(KomLabel);
    void paintEvent(QPaintEvent *e);
};

} // namespace kom

#endif
