#ifndef __KOM_LABEL_H__
#define __KOM_LABEL_H__

#include <QLabel>
#include <QScopedPointer>
#include <QColor>

namespace kom
{

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

protected:
    KomLabel(KomLabelPrivate &d);
    QScopedPointer<KomLabelPrivate> d_ptr;

private:
    void paintEvent(QPaintEvent *e);
    Q_DISABLE_COPY(KomLabel);
    Q_DECLARE_PRIVATE(KomLabel);
    bool m_fontSizeChanged = true;
};

} // namespace kom

#endif
