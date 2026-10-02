#ifndef CUSTOMEXPANDBUTTON_H
#define CUSTOMEXPANDBUTTON_H

#include <QWidget>
#include <kpushbutton.h>
#include <kborderlessbutton.h>
#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CustomExpandButton : public QWidget {
    Q_OBJECT

public:
    CustomExpandButton(QList<QPushButton *> pushBtnList, QWidget *parent = nullptr);

private:
    QList<QPushButton *> m_pushBtnList;

    QVBoxLayout *m_mainVLayout = nullptr;
    QHBoxLayout *m_btnHLayout = nullptr;
    kdk::KBorderlessButton *m_directBtn = nullptr;
    bool m_directBtnStatus = false;
private:
    void initGui();
    void initConnect();
};


#endif //CUSTOMEXPANDBUTTON_H
