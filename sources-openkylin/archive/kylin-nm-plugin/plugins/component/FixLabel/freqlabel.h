#ifndef FREQLABEL_H
#define FREQLABEL_H

#include <QWidget>
#include <QLabel>

class FreqLabel : public QLabel
{
    Q_OBJECT
public:
    FreqLabel(QWidget *parent = nullptr);
    ~FreqLabel() = default;
protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void changedFontSlot();
};

#endif // FREQLABEL_H
