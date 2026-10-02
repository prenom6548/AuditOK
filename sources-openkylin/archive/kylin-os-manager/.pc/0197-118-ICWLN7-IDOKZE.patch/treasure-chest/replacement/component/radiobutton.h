#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../globaldata.h"
#include "text_label.h"
#include "kborderbutton.h"

using namespace kdk;
using namespace K;

// class RadioButton : public KBorderButton
class RadioButton : public QWidget
{
    Q_OBJECT
public:
    RadioButton(bool isRect = false, QWidget *parent = nullptr);
    ~RadioButton();

    void setLabelIcon(QIcon icon);
    void setLabelText(SysType type);

    void paintEvent(QPaintEvent *event);
private:
    void initUI();
    void initStyle();
    void initLayout();
    void initConn();
    void adjustBackground();

    bool m_isRect = false;

    QRadioButton *m_radioBtn = nullptr;
    QLabel *m_labelIcon = nullptr;
    
    TextLabel *m_labelDevic = nullptr;
    TextLabel *m_labelDeviceNote = nullptr;

    QHBoxLayout *m_layoutHRadio = nullptr;
    QHBoxLayout *m_layoutHMain = nullptr;
    QVBoxLayout *m_layoutVRadio = nullptr;
    QVBoxLayout *m_layoutVMain = nullptr;

signals:

public slots:
};
#endif

