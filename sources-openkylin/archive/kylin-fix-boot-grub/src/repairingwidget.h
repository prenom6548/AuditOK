#ifndef REPAIRINGWIDGET_H
#define REPAIRINGWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QPainterPath>

class repairingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit repairingWidget(QWidget *parent = nullptr);
    ~repairingWidget();

public:
    void initUI();
    void finishLoadDisk();

public:
    QVBoxLayout *m_pVboxRepairLayout = nullptr;

    QWidget *m_pRepairIconWidget = nullptr;
    QHBoxLayout *m_pRepairIconLayout = nullptr;
    QLabel *m_pRepairIconLabel = nullptr;

    QWidget *m_pRepairTextWidget = nullptr;
    QHBoxLayout *m_pRepairTextLayout = nullptr;
    QLabel *m_pRepairTextLabel = nullptr;

    QTimer *m_pTime = nullptr;
    int m_iconNum = 0;
signals:

};

#endif // REPAIRINGWIDGET_H
