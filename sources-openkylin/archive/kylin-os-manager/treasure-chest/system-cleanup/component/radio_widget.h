#ifndef RADIOWIDGET_H
#define RADIOWIDGET_H

#include <QWidget>
#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include "text_label.h"
using namespace K;

class RadioWidget : public QWidget
{
    Q_OBJECT
public:
    RadioWidget(QWidget *parent = nullptr);
    ~RadioWidget();
    void setLabelIcon(QIcon icon);
    void setLabelText(QString text);
    bool getCilckStyle();
    void setClickStyle();

protected:
    void mouseReleaseEvent(QMouseEvent * ev); 
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();
    void clickWidget();

    QRadioButton *m_btn = nullptr;
    QLabel *m_labelIcon = nullptr;
    TextLabel *m_labelInfo = nullptr;

    bool m_isClicked = false;

    QSize m_iconSize = QSize(144, 144);
    QSize m_widgetSize = QSize(254, 230);

Q_SIGNALS:
    void sigClicked();
    void sigCannel();

public Q_SLOTS:
};
#endif
