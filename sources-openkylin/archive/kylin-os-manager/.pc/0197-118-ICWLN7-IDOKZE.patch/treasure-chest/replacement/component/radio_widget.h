#ifndef RADIOWIDGET_H
#define RADIOWIDGET_H

#include <QWidget>
#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include "text_label.h"
#include "../globaldata.h"
using namespace K;

class RadioWidget : public QWidget
{
    Q_OBJECT
public:
    RadioWidget(bool isVLayout = false, QWidget *parent = nullptr);
    ~RadioWidget();
    void setLabelIcon(QIcon icon);
    void setLabelText(SysType type);
    // void setWidgetStyle(bool);
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
    TextLabel *m_labelName = nullptr;
    TextLabel *m_labelInfo = nullptr;

    bool m_isVLayout = false;
    bool m_isClicked = false;

signals:
    void sigClicked();
    void sigCannel();

public slots:
};
#endif
