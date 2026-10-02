#ifndef BOOTUI_H
#define BOOTUI_H

#include <QWidget>
#include <QHBoxLayout>

#include "titlelabel.h"
#include "lightlabel.h"

#include <kysdk/applications/kswitchbutton.h>
using namespace kdk;

class BootUi : public QWidget
{
    Q_OBJECT
public:
    explicit BootUi(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~BootUi();

public:
    inline KSwitchButton *grubSwitchButton()
    {
        return mGrubBtn;
    }

    inline QPushButton *resetButton()
    {
        return mResetPasswdBtn;
    }

private:
    QVBoxLayout *mVlayout = nullptr;

    TitleLabel *mBootTitleLabel;
    QFrame *mBootFrame;
    QLabel *mGrubLabel;
    KSwitchButton *mGrubBtn;
    QPushButton *mResetPasswdBtn;
    LightLabel *mNeedPasswdLabel;

private:
    void initUI();

};

#endif // BOOTUI_H
