#ifndef PUSHBUTTONCHECK_H
#define PUSHBUTTONCHECK_H

#include <QWidget>
#include <QPushButton>
#include <QPaintEvent>
#include <QLabel>
#include <QString>
class PushButtonCheck : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButtonCheck(QString str, QWidget *parent = nullptr);

public:
    QLabel* m_labelpix;

    void setLabelShow(bool flag) {
        m_labelpix->setVisible(flag);
        if(flag) {
            this->setChecked(true);
            //this->setFocus();
        }else
            this->setChecked(false);
    }
//   void addStyleSheet();

 public slots:
    void clicked_slot();
    void other_clicked_slot(QString objname);
signals:
    void clicked_obj(PushButtonCheck* obj);


};

#endif // PUSHBUTTONCHECK_H
