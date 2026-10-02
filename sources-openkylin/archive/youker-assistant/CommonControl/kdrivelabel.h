#ifndef KDRIVELABEL_H
#define KDRIVELABEL_H

#include <QLabel>
#include <QContextMenuEvent>

class KDriveLabel : public QLabel
{
    Q_OBJECT
public:
    KDriveLabel(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // KDRIVELABEL_H