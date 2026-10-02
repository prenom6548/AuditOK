#ifndef SHORTCUTWIN_H
#define SHORTCUTWIN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "publicattributes.hpp"

class ShortCutWin : public QWidget
{
    Q_OBJECT
public:
    ShortCutWin(QWidget *parent = nullptr);

    void changeTheme(PublicAttributes::Theme theme);
    void changeFont(double fontSize);

private:
    // void initInfoTable();
    QLabel *m_titleIcon = nullptr;
    QLabel *m_titleName = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
    QLabel *m_tableName;
    QLabel *m_gaugeOutfit1 = nullptr;
    QLabel *m_gaugeOutfit2 = nullptr;
    QLabel *m_itemLab1 = nullptr;
    QLabel *m_itemLab2 = nullptr;
};

#endif // SHORTCUTWIN_H
