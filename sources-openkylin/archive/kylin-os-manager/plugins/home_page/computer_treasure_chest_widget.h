#ifndef KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_COMPUTER_TREASURE_CHEST_H
#define KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_COMPUTER_TREASURE_CHEST_H

#include <QWidget>

namespace home_page {

class ComputerTreasureChestWidget: public QWidget {
    Q_OBJECT
public:
    ComputerTreasureChestWidget(QWidget *parent = nullptr);
    ~ComputerTreasureChestWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
};

}

#endif
