#ifndef __KOM_LABEL_H__
#define __KOM_LABEL_H__

#include <QLabel>
#include <QScopedPointer>

namespace kom {

class KomLabelPrivate;

class KomLabel : public QLabel {
    Q_OBJECT
public:
    KomLabel(QWidget *parent = nullptr);
    ~KomLabel();

    void setAlignment(Qt::Alignment a);
    void setText(const QString &text);
    void setBold(bool enable);
    void setFontSize(int size);
    void setTransparency(float t);

protected:
    void paintEvent(QPaintEvent *e) override;
    QScopedPointer<KomLabelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(KomLabel);
    Q_DECLARE_PRIVATE(KomLabel);
};

}

#endif
