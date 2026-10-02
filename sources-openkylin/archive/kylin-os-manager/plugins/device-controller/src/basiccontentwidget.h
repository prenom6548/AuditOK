#ifndef BASICCONTENTWIDGET_H
#define BASICCONTENTWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QAction>

class BasicContentWidget : public QWidget {
    Q_OBJECT

public:
    BasicContentWidget(QWidget *parent = nullptr);
    QAction *m_actionExport = nullptr;

protected:
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    QMenu *m_menu = nullptr;

private:
    void rightClickedMenuRequest();
};


#endif //BASICCONTENTWIDGET_H
