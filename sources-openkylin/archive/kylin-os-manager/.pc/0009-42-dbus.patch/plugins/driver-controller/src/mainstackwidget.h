#ifndef MAINSTACKWIDGET_H
#define MAINSTACKWIDGET_H

#include <QStackedWidget>
#include "QGSettings/qgsettings.h"

#include "selectionwidget.h"
#include "loadingwidget.h"

class MainStackWidget : public QStackedWidget {
Q_OBJECT
public:
    explicit MainStackWidget(QWidget *parent = nullptr);

    QGSettings *m_themeData = nullptr;

private:
    SelectionWidget *m_selectionWidget = nullptr;
    LoadingWidget *m_loadingWidget = nullptr;

    void initGsetting();

    void initConnection();

    void initGui();
};


#endif // MAINSTACKWIDGET_H
