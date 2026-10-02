#ifndef TITLELABELITEM_H
#define TITLELABELITEM_H

#include <QLabel>

/**
* @brief 包含TitleLabel子项的类型
*/
enum class TitleLabelType
{
    TITLE_LABEL_OUTPUT = 0,         // 输出标题
    TITLE_LABEL_INPUT,              // 输入标题
    TITLE_LABEL_SOUNDEFFECT,        // 系统音效标题
    TITLE_LABEL_ADVANCESETTINGS     // 高级设置标题
};

class TitleLabelItem : public QWidget
{
public:
    TitleLabelItem(QLabel*, QWidget* = nullptr);
    virtual ~TitleLabelItem() = default;

public:
    void initUi();

    QWidget* getWidget();

private:
    QLabel* m_pLabel;
};

#endif // TITLELABELITEM_H
