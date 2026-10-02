#ifndef ANDROIDITEM_H
#define ANDROIDITEM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "publicattributes.hpp"

class AndroidItem : public QPushButton
{
    Q_OBJECT
public:
    enum Type {
        Picture = 0, // 图片
        Video,       // 视频
        Music,       // 音乐
        Doc,         // 文档
        WeChat,      // 微信
        QQ,          // QQ
        File,        // 文件存储
    };
    AndroidItem(AndroidItem::Type type, QWidget *parent = nullptr);
    ~AndroidItem() {};

    void setItemCount(int count = -1);
    // 设置主题
    void setTheme(PublicAttributes::Theme theme);

    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigBtnClicked(AndroidItem::Type);

private:
    void initType();

private:
    QLabel *m_itemIconLabel = nullptr;  // 项按钮
    QLabel *m_itemNameLabel = nullptr;  // 项名称
    QLabel *m_itemCountLabel = nullptr; // 每一项总数
    AndroidItem::Type m_type;           // 类型
};

#endif // ANDROIDITEM_H
