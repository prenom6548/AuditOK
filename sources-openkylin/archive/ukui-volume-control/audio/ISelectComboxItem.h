#ifndef ISELECTSUBITEM_H
#define ISELECTSUBITEM_H

#include <memory>
#include <QLabel>
#include <QVariant>

enum class ItemType {
    MAIN_WINDOW_ITEM,           // 主界面的item
    DEVMANAGER_WINDOW_ITEM,     // 设备管理item
    APPMANAGER_WINDOW_ITEM      // 应用管理item
};

/**
* @brief 包含SelectCombox子项的类型
*/
enum class SelectComboxType
{
    SELECT_COMBOX_INPUT = 0,    // 输入设备子项
    SELECT_COMBOX_OUTPUT,       // 输出设备子项
    SELECT_COMBOX_SOUNDEFFECT,  // 音效主题子项
    SELECT_COMBOX_VOLUMEADJUST, // 音量调节子项
    SELECT_COMBOX_NOTIFICTION,  // 接收通知子项
    SELECT_COMBOX_SINKINPUT,
    SELECT_COMBOX_SOURCEOUTPUT
};

class ISelectBox {
public:
    ISelectBox() = default;
    virtual ~ISelectBox() = default;

public:
    virtual void setCurrentIndex(int) = 0;

    virtual void setEnabled(bool) = 0;

    virtual bool isEnabled() const = 0;

    virtual int findData(const QVariant&, int = Qt::UserRole) = 0;

    virtual int findText(const QString&) = 0;

    virtual void insertItem(const QString&, const QVariant& = QVariant()) = 0;

    virtual void removeItem(int) = 0;

    virtual QVariant currentData(int = Qt::UserRole) const = 0;

    virtual std::list<QVariant> getDataList() const = 0;

    virtual int count() const = 0;

    virtual void clear() = 0;

    virtual QWidget* getWidget() = 0;

    virtual void setParent(QWidget*) = 0;
};

class ISelectComboxItem {
public:
    ISelectComboxItem(std::shared_ptr<ISelectBox>, QLabel*);
    virtual ~ISelectComboxItem() = default;

public:
    void setCurrentIndex(int);

    int findData(const QVariant&, int = Qt::UserRole);

    int findText(const QString&);

    void insertItem(const QString&, const QVariant& = QVariant());

    void removeItem(int);

    QVariant currentData(int = Qt::UserRole) const;

    int count() const;

    void clear();

    std::shared_ptr<ISelectBox> getSelectBox() const;

    std::list<QVariant> getDataList() const;

    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual void setEnabled(bool) = 0;

    virtual QWidget* getWidget() = 0;

protected:
    std::shared_ptr<ISelectBox> m_pSelectBox = nullptr;
    QLabel* m_pDisplayLabel = nullptr;
};

#endif // ISELECTSUBITEM_H
