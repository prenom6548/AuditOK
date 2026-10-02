#ifndef REMOVABLELISTWIDGET_H
#define REMOVABLELISTWIDGET_H

#include <QListWidget>
#include <QStringList>
#include <QDropEvent>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "mylabel.h"
#include "pixmaplabel.h"
#include "kdragwidget.h"

class MyItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyItemWidget(QWidget* parent = nullptr, QListWidgetItem *item = nullptr);
    virtual ~MyItemWidget();

    void setSize(int width, int height);

    void setText(const QString& text);

    QString text() {
        return m_text;
    }

signals:
    void selected(bool checked);
    void deleteItem();
    void setScrollWidth(int width);
    void resetItemWidth(int width);

private:
    QString m_text;
    MyLabel *m_label;
    QPushButton *m_buttonDelete;
    int m_scrollWidth = 0;  //父窗口QListWidget的滚动条宽度
    int m_extWidth = 0; //去掉父窗口QListWidget的父窗口QFrame的右侧边距时扩展的宽度
    bool m_checked = false;

    QListWidgetItem *m_item;
};

class BackupListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit BackupListWidget(QWidget *parent = nullptr, QHBoxLayout *parentLayout = nullptr);
    virtual ~BackupListWidget();

    // 添加列表项，注意使用本类的功能不能用addItem等基类添加项的方法
    bool appendItem(const QString &text);

    // 判断列表项是否包含text，注意使用时不能用findItems等基类查找方法
    bool contains(const QString &text);

    // 获取备份路径列表
    QStringList getBackupPaths() { return m_List; }

    // 设置备份路径限制为只能是pathLimit或其子路径
    void setPathLimit(const QStringList &pathLimit) { m_pathLimit.append(pathLimit); }

    // 清空
    void clearData() {
        this->clear();
        this->m_List.clear();
        emit this->deleteEmpty();
        m_plusLogo->setVisible(true);
        m_plusText->setVisible(true);

        updateGeometry();
        viewport()->update();
    }

protected:
    void dropEvent(QDropEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

signals:
    void deleteEmpty();
    void addedItem();
    void setScrollWidth(int width);
    void resetItemWidth(int width);

private:
    bool checkPathLimit(const QString &path);

private:
    PixmapLabel *m_plusLogo;
    QLabel *m_plusText;
    int m_type = QListWidgetItem::ItemType::UserType + 1;
    QStringList m_List;
    QStringList m_pathLimit;

    QHBoxLayout *m_parentLayout;
    bool m_hadSelected = false;
};

#endif // REMOVABLELISTWIDGET_H
