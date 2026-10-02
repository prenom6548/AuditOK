#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include "../globaldata.h"
#include "text_label.h"

class IconWidget : public QWidget
{
    Q_OBJECT
public:
    IconWidget(QWidget *parent);
    ~IconWidget();

    void setLabelIcon(QSize iconSize, QIcon icon, QString str);

private:
    void initUI();

    QLabel *m_labelIcon = nullptr;
    K::TextLabel *m_labelInfo = nullptr;
    QVBoxLayout *m_layoutV = nullptr;

public slots:
};
#endif
