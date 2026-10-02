#ifndef PASSWORDLABEL_H
#define PASSWORDLABEL_H
#include <QLabel>
#include <QLineEdit>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QPushButton>

#include "libukcc_global.h"

class LIBUKCC_EXPORT EyeBtn: public QLabel
{
    Q_OBJECT
public:
    EyeBtn(QWidget *parent = nullptr);
    ~EyeBtn();
    void setIconStatus(bool isHidden);
protected:
    void mouseReleaseEvent(QMouseEvent *event);
private:
    bool mIsHidden = true;

Q_SIGNALS:
    void clicked(bool isHidden);
};

class LIBUKCC_EXPORT PasswordLabel : public QWidget
{
    Q_OBJECT
public:
    PasswordLabel(QWidget *parent = nullptr);
    ~PasswordLabel();

    void setStatus(bool isPassword);
    void setText(const QString &);
    QString text() const;

    void updateSize();

private:
    QLineEdit *mPassword;
    EyeBtn *mEyesBtn;
};

#endif // PASSWORDLABEL_H
