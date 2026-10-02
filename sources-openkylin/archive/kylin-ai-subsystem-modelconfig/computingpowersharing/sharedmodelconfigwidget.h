#ifndef SHAREDMODELCONFIGWIDGET_H
#define SHAREDMODELCONFIGWIDGET_H

#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>

class SharedModelConfigWidget : public kdk::KDialog {
    Q_OBJECT
public:
    explicit SharedModelConfigWidget(QWidget *parent = nullptr);

private:
    void initLayout();
    void initModelTitleLayout();
    void initModellayout();
    void initConnect();

private:
    QBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;

    QVBoxLayout *m_baseVLayout = nullptr;

    kdk::KBorderlessButton *m_openComputePowerShareBtn = nullptr;
    QPushButton *m_confirmLocalButton = nullptr;

signals:
    void openComputePowerShare();
};

#endif  // SHAREDMODELCONFIGWIDGET_H
