#ifndef ACCEPTREBOOTWIDGET_H
#define ACCEPTREBOOTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "../component/text_label.h"
#include "kborderlessbutton.h"
class AcceptRebootWidget : public QWidget
{
    Q_OBJECT
public:
    AcceptRebootWidget(QWidget *parent = nullptr);
    ~AcceptRebootWidget();
    void setLabelsText();

protected:
    void paintEvent(QPaintEvent *event);
private:
    void initUI();
    
    QLabel *m_labeIconSize = nullptr;
    K::TextLabel *m_lableSize = nullptr;
    QLabel *m_labeIconTime = nullptr;
    K::TextLabel *m_lableTime = nullptr;
    
    QHBoxLayout *m_layoutHLabel = nullptr;

};
#endif