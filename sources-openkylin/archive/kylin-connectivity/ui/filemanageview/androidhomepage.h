#ifndef ANDROIDHOMEPAGE_H
#define ANDROIDHOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include "multimediawin.h"
#include "androiditem.h"
#include "publicattributes.hpp"

class AndroidHomePage : public QWidget
{
    Q_OBJECT
public:
    AndroidHomePage(QWidget *parent = nullptr);

    void setItemCount(AndroidItem::Type type, int num);
    void setTheme(PublicAttributes::Theme theme);
    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigBtnClicked(AndroidItem::Type);

private:
    MultiMediaWin *m_multiMediaWin = nullptr;
    QLabel *m_title = nullptr;
    AndroidItem *m_storageItem = nullptr;
};

#endif // ANDROIDHOMEPAGE_H
