#ifndef NODRIVERUNINSTALLPAGE_H
#define NODRIVERUNINSTALLPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class NoDriverUninstallPage : public QWidget{
Q_OBJECT
public:
    NoDriverUninstallPage(QWidget *parent = nullptr);

private:
    QLabel *m_nodriverIcon = nullptr;
    QLabel *m_nodriverText = nullptr;
    QVBoxLayout *m_mainVboxLayout = nullptr;

    void initGui();
    void initConnect();

public slots:
    void initTheme();

};


#endif //NODRIVERUNINSTALLPAGE_H
