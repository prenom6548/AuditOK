#ifndef ACCEPTDATASTATUS_H
#define ACCEPTDATASTATUS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "../component/text_label.h"
#include "../component/status_widget.h"
class AcceptDataStatus : public QWidget
{
    Q_OBJECT
public:
    AcceptDataStatus(QWidget *parent = nullptr);
    ~AcceptDataStatus();
    enum FlowPart {
        FirstPart = 0,
        SecondPart,
        ThirdPart,
        LastPart,
    };
    void setStatusStyle(FlowPart part);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    StatusWidget *m_statusDevice = nullptr;
    StatusWidget *m_statusData = nullptr;
    StatusWidget *m_statusMigration = nullptr;
    StatusWidget *m_statusReboot = nullptr;

    QHBoxLayout *m_hlayout = nullptr;

};
#endif