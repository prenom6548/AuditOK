#ifndef MOBILETYPE_H
#define MOBILETYPE_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>

namespace Ui {
class MobileType;
}

class MobileType : public QDialog
{
    Q_OBJECT

public:
    explicit MobileType(QWidget *parent = nullptr);
    ~MobileType();

private:
    Ui::MobileType *ui;

Q_SIGNALS:
    void sendMobileType(QString name);
};

#endif // MOBILETYPE_H
