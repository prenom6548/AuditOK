#ifndef DATACHECKBOX_H
#define DATACHECKBOX_H

#include <QWidget>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QDBusInterface>
#include "../globaldata.h"
#include "../view/acceptmigrationappdata.h"

class DataCheckBox : public QWidget
{
    Q_OBJECT
public:
    DataCheckBox(bool isAppData = false, QWidget *parent = nullptr);
    ~DataCheckBox();

    enum DataType {
        AppData = 0,
        UserData,
        SysData,
    };

    void setLabelText(DataType type);
    bool getCilckStyle();
    void setClickStyle();
    void initAppWidget();

protected:
    void mouseReleaseEvent(QMouseEvent * ev); 
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    void initConn();
    void clickWidget();
    void cilckAppBtn();
    
    QCheckBox *m_checkBox = nullptr;
    QLabel *m_label = nullptr;
    QLabel *m_labelTip = nullptr;
    QPushButton *m_btnApp = nullptr;
    QHBoxLayout *m_layoutH = nullptr;
    AcceptMigrationAppData *m_appData = nullptr;
    QDBusInterface *dbus = nullptr;
    bool m_isAppData = false;
    bool m_isClicked = false;

signals:
    void sigClicked();
    void sigCannel();
    void sigAppData(QStringList list);

public slots:
};
#endif
