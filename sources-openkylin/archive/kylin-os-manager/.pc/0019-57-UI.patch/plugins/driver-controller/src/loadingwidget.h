#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <kysdk/applications/gsettingmonitor.h>
#include <QVBoxLayout>
#include <QHBoxLayout>

class LoadingWidget : public QWidget {
Q_OBJECT
public:
    LoadingWidget(QWidget *parent = nullptr);

    ~LoadingWidget();

    QLabel *m_loadingIcon = nullptr;
    QLabel *m_loadingLabel = nullptr;
    QHBoxLayout *m_iconLayout= nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

private:
    void initConnect();

    void initGUi();

    void themeSetting();
};


#endif //LOADINGWIDGET_H
