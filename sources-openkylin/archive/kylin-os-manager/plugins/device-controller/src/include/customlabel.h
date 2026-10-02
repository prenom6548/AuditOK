#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>

class CustomLabel : public QLabel
{
Q_OBJECT

public:
    explicit CustomLabel(QWidget *parent = nullptr);
    virtual QSize sizeHint() const override;
    void SetText(const QString &text);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void resizeEvent(QEvent *event);
signals:
    void clicked();

private:
    QString originalText;
    QString displayText;
    bool isHovered;
    bool isElided;

    void updateDisplayText();

};

#endif // CUSTOMLABEL_H
