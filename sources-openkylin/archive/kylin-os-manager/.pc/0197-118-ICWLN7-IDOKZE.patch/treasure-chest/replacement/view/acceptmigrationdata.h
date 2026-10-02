#ifndef ACCEPTMIGRATIONDATA_H
#define ACCEPTMIGRATIONDATA_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDBusInterface>
#include "../component/text_label.h"
#include "../component/icon_widget.h"
#include "../component/progressbar.h"
#include "kprogressbar.h"
class AcceptMigrationData : public QWidget
{
    Q_OBJECT
public:
    AcceptMigrationData(QWidget *parent = nullptr);
    ~AcceptMigrationData();

protected:
    void paintEvent(QPaintEvent *event);
private:
    void initUI();
    void initStyle();
    void initConn(); 
    void getMigrationFinishData();
    
    IconWidget *m_iconWidgetOld = nullptr;
    IconWidget *m_iconWidgetNew = nullptr;
    IconWidget *m_iconWidgetSend = nullptr;

    K::TextLabel *m_label = nullptr;
    K::TextLabel *m_labelCurrent = nullptr;
    ProgressBar *m_progressBar = nullptr;
    
    QPushButton *m_btnWaring = nullptr;
    K::TextLabel *m_labelWarning = nullptr;
    QWidget *m_widget = nullptr;
    
    QHBoxLayout *m_layoutHLabel = nullptr;
    QVBoxLayout *m_layouV = nullptr;
    QDBusInterface *m_dbus = nullptr;

public slots:
    void slotConfigData();
    void slotMigrationDataInfo(QStringList list);
    void slotSetProgressRange(qint64 num);
    void slotConfigFinish();
signals:
    void sigMigratDataFinish();
};
#endif