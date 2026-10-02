#ifndef DetailWidget_H
#define DetailWidget_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include "../component/FixLabel/fixlabel.h"

class DetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *valueWidget = nullptr, bool isOobe = false, QWidget *parent = nullptr);
    ~DetailWidget();

    void setKey(const QString &keyLabel);

private:
    QHBoxLayout * m_mainLayout = nullptr;
    FixLabel * m_keyLabel = nullptr;
    QWidget * m_valueWidget = nullptr;
    bool m_isOobe = false;

    void initUI();

};

#endif // DetailWidget_H
