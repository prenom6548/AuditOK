#pragma once

#include <QPushButton>
#include <QWidget>

class QLineEdit;
class QComboBox;
class QCheckBox;

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    enum state {
        state_minimizable = 0x1,
        state_maximizable = 0x2,
        state_fullscreenable = 0x4,
        state_closeable = 0x8,
        state_movable = 0x10,
        state_resizable = 0x20,
        state_focusable = 0x40,
        state_activatable = 0x80,
        state_keep_above = 0x100,
        state_keep_below = 0x200,
    };

private:
    QWidget *childWindow = nullptr;
    QLineEdit *lineEdit_pos = nullptr;
    QComboBox *comboBox_role = nullptr;
    QCheckBox *checkBox_skipTaskbar = nullptr;
    QCheckBox *checkBox_skipSwitcher = nullptr;
    QCheckBox *checkBox_state_minimizable = nullptr;
    QCheckBox *checkBox_state_maximizable = nullptr;
    QCheckBox *checkBox_state_fullscreenable = nullptr;
    QCheckBox *checkBox_state_closeable = nullptr;
    QCheckBox *checkBox_state_movable = nullptr;
    QCheckBox *checkBox_state_resizable = nullptr;
    QCheckBox *checkBox_state_focusable = nullptr;
    QCheckBox *checkBox_state_activatable = nullptr;
    QCheckBox *checkBox_state_keep_above = nullptr;
    QCheckBox *checkBox_state_keep_below = nullptr;
    QLineEdit *lineEdit_icon = nullptr;
    QCheckBox *checkBox_titleBar = nullptr;
    QCheckBox *checkBox_round_corner = nullptr;
    QCheckBox *checkBox_border = nullptr;
    QCheckBox *checkBox_shadow = nullptr;

    QPushButton *button_show_tile_flyout = nullptr;

    void initChildWindow(QWidget *window);
};