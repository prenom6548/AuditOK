#ifndef MULTIMEDIAWIN_H
#define MULTIMEDIAWIN_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractItemModel>
#include "androiditem.h"
#include "publicattributes.hpp"

class MultiMediaWin : public QListWidget
{
    Q_OBJECT
public:
    MultiMediaWin(QWidget *parent = nullptr);
    ~MultiMediaWin();

    void setAppItem();
    void setItemCount(AndroidItem::Type type, int num);
    void setTheme(PublicAttributes::Theme theme);
    void changeFontSize(double fontSize);

protected:
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:
    void sigBtnClicked(AndroidItem::Type);

private Q_SLOTS:
private:
    void addMultiMediaItem(AndroidItem::Type type);

private:
    QListWidget *m_fileListWidget = nullptr;
    QListWidget *m_storageWidget = nullptr;
    QPushButton *m_searchButton = nullptr;
    QMap<AndroidItem::Type, AndroidItem *> m_itemMap;

    int m_maxItemIndex = 0;
};

#endif // MULTIMEDIAWIN_H
