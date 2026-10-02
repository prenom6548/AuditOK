#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    void set_label_text(QString msg);
    void set_label_pin_text(QString msg);
    void set_warningLabel_text(QString msg);
    void set_button(int a);
    void set_button_2();
    void set_countdown(int a);
    void set_label_pin(int a);
    void set_visible(int a);
    void setLabelText(QLabel *label,QString text);
    ~Dialog();
public slots:
    void on_pushButton_allow_clicked();
    void on_pushButton_refuse_clicked();

protected:
    void paintEvent(QPaintEvent * event);

private:
    Ui::Dialog *ui;
    int flag=0;

signals:
    void allow_btn_signal();
    void refuse_btn_signal();


};

#endif // DIALOG_H
