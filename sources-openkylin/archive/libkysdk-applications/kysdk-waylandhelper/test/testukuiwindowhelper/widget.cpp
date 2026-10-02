#include "widget.h"

#include "ukuiwindowhelper.h"
#include "ukuistartup.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QProcess>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setSkipTaskBar(true);
    windowHelper->setSkipSwitcher(true);

    QGridLayout *layout = new QGridLayout(this);

    // 位置
    QHBoxLayout *hbox_pos = new QHBoxLayout();
    QLabel *label = new QLabel("位置", this);
    lineEdit_pos = new QLineEdit("200,200", this);
    hbox_pos->addWidget(label);
    hbox_pos->addWidget(lineEdit_pos);

    // role
    QHBoxLayout *hbox_role = new QHBoxLayout();
    QLabel *label_role = new QLabel("Role", this);
    comboBox_role = new QComboBox(this);
    comboBox_role->addItem("normal");
    comboBox_role->addItem("desktop");
    comboBox_role->addItem("panel");
    comboBox_role->addItem("onscreendisplay");
    comboBox_role->addItem("notification");
    comboBox_role->addItem("tooltip");
    comboBox_role->addItem("criticalnotification");
    comboBox_role->addItem("appletpopup");
    comboBox_role->addItem("screenlock");
    comboBox_role->addItem("watermark");
    comboBox_role->addItem("systemwindow");
    comboBox_role->addItem("inputpanel");
    comboBox_role->addItem("logout");
    comboBox_role->addItem("screenlocknotification");
    comboBox_role->addItem("switcher");
    comboBox_role->addItem("authentication");
    hbox_pos->addWidget(label_role);
    hbox_pos->addWidget(comboBox_role);
    layout->addLayout(hbox_pos, 0, 0, 1, 4);

    int row = 1;
    int col = 0;

    auto addWidget = [layout, &row, &col](QWidget *widget) {
        layout->addWidget(widget, row, col);
        if (++col > 3) {
            col = 0;
            ++row;
        }
    };

    // 跳过任务栏
    checkBox_skipTaskbar = new QCheckBox("SkipTaskBar", this);
    addWidget(checkBox_skipTaskbar);

    // 跳过Switcher
    checkBox_skipSwitcher = new QCheckBox("SkipSwitcher", this);
    addWidget(checkBox_skipSwitcher);

    // state
    checkBox_state_maximizable = new QCheckBox("Maximizable", this);
    addWidget(checkBox_state_maximizable);

    checkBox_state_minimizable = new QCheckBox("Minimizable", this);
    addWidget(checkBox_state_minimizable);

    checkBox_state_fullscreenable = new QCheckBox("Fullscreenable", this);
    addWidget(checkBox_state_fullscreenable);

    checkBox_state_closeable = new QCheckBox("Closeable", this);
    addWidget(checkBox_state_closeable);

    checkBox_state_movable = new QCheckBox("Movable", this);
    addWidget(checkBox_state_movable);

    checkBox_state_resizable = new QCheckBox("Resizable", this);
    addWidget(checkBox_state_resizable);

    checkBox_state_focusable = new QCheckBox("Focusable", this);
    addWidget(checkBox_state_focusable);

    checkBox_state_activatable = new QCheckBox("Activatable", this);
    addWidget(checkBox_state_activatable);

    checkBox_state_keep_above = new QCheckBox("KeepAbove", this);
    addWidget(checkBox_state_keep_above);

    checkBox_state_keep_below = new QCheckBox("KeepBelow", this);
    addWidget(checkBox_state_keep_below);

    // 窗口标题栏
    checkBox_titleBar = new QCheckBox("NoTitleBar", this);
    addWidget(checkBox_titleBar);

    checkBox_round_corner = new QCheckBox("RoundCorner(Popup Only)", this);
    addWidget(checkBox_round_corner);

    checkBox_border = new QCheckBox("Border(Popup Only)", this);
    addWidget(checkBox_border);

    checkBox_shadow = new QCheckBox("Shadow(Popup Only)", this);
    addWidget(checkBox_shadow);

    QHBoxLayout *hbox_icon = new QHBoxLayout();
    QLabel *label_icon = new QLabel("图标", this);
    lineEdit_icon = new QLineEdit("kylin-printer", this);
    lineEdit_icon->setPlaceholderText("请输入主题库中包含的图标名称");
    hbox_icon->addWidget(label_icon);
    hbox_icon->addWidget(lineEdit_icon);

    layout->addLayout(hbox_icon, 5, 0, 1, 4);

    QPushButton *btn = new QPushButton("打开窗口", this);
    connect(btn, &QPushButton::clicked, [this] {
        if (childWindow == nullptr) {
            childWindow = new QDialog(this);
            initChildWindow(childWindow);
            childWindow->resize(420, 360);
            childWindow->show();
        }
    });

    QPushButton *btnClose = new QPushButton("关闭窗口", this);
    connect(btnClose, &QPushButton::clicked, [this] {
        if (childWindow != nullptr) {
            childWindow->close();
            childWindow = nullptr;
        }
    });

    QPushButton *btn_openPopup = new QPushButton("打开Popup窗口", this);
    connect(btn_openPopup, &QPushButton::clicked, [this] {
        QScopedPointer<QMenu> menu(new QMenu(this));
        menu->addAction("菜单1");
        menu->addAction("菜单2");
        menu->addAction("菜单3");
        initChildWindow(menu.data());
        menu->exec(QCursor::pos());
    });

    QHBoxLayout *hbox_btn = new QHBoxLayout();
    hbox_btn->addWidget(btn);
    hbox_btn->addWidget(btnClose);
    hbox_btn->addWidget(btn_openPopup);
    layout->addLayout(hbox_btn, 6, 0, 1, 4);

    QPushButton *btnStartupGeo = new QPushButton("启动终端(StartUpGeometry接口测试)", this);
    connect(btnStartupGeo, &QPushButton::clicked, [this] {
        QProcess p;
        p.setProgram("mate-terminal");
        qint64 pid;
        p.startDetached(&pid);
        ukuiStartupManager()->setStartupGeometryByPid(pid, 200, 200, 100, 100);
    });
    layout->addWidget(btnStartupGeo, 7, 0, 1, 4);

    button_show_tile_flyout = new QPushButton("显示分屏窗口", this);
    connect(button_show_tile_flyout, &QPushButton::clicked, [this, windowHelper] {
        windowHelper->showTileFlyout(button_show_tile_flyout->geometry());
    });
    layout->addWidget(button_show_tile_flyout, 8, 0, 1, 4);

    setLayout(layout);
}

Widget::~Widget()
{
}

void Widget::initChildWindow(QWidget *w)
{
    if (QStringList pos = lineEdit_pos->text().split(","); pos.size() == 2) {
        w->move(pos[0].toInt(), pos[1].toInt());
    }
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(w);

    windowHelper->setSkipTaskBar(checkBox_skipTaskbar->isChecked());
    windowHelper->setSkipSwitcher(checkBox_skipSwitcher->isChecked());
    if (checkBox_titleBar->isChecked())
        windowHelper->removeTitleBar();
    windowHelper->setWindowRole((UkuiWindowHelper::WindowRole)comboBox_role->currentIndex());

    UkuiWindowHelper::States states;
    uint32_t state = 0;
    if (checkBox_state_minimizable->isChecked())
        states |= UkuiWindowHelper::State::Minimizable;
    if (checkBox_state_maximizable->isChecked())
        states |= UkuiWindowHelper::State::Maximizable;
    if (checkBox_state_fullscreenable->isChecked())
        states |= UkuiWindowHelper::State::Fullscreenable;
    if (checkBox_state_closeable->isChecked())
        states |= UkuiWindowHelper::State::Closeable;
    if (checkBox_state_movable->isChecked())
        states |= UkuiWindowHelper::State::Movable;
    if (checkBox_state_resizable->isChecked())
        states |= UkuiWindowHelper::State::Resizable;
    if (checkBox_state_focusable->isChecked())
        states |= UkuiWindowHelper::State::Focusable;
    if (checkBox_state_activatable->isChecked())
        states |= UkuiWindowHelper::State::Activatable;
    if (checkBox_state_keep_above->isChecked())
        states |= UkuiWindowHelper::State::KeepAbove;
    if (checkBox_state_keep_below->isChecked())
        states |= UkuiWindowHelper::State::KeepBelow;
    windowHelper->setWindowState(states);

    UkuiWindowHelper::DecorationComponents decorationComponents;
    uint32_t decorationState = 0;
    if (checkBox_round_corner->isChecked())
        decorationComponents |= UkuiWindowHelper::DecorationComponent::RoundCorner;
    if (checkBox_border->isChecked())
        decorationComponents |= UkuiWindowHelper::DecorationComponent::Border;
    if (checkBox_shadow->isChecked())
        decorationComponents |= UkuiWindowHelper::DecorationComponent::Shadow;
    windowHelper->setDecorationCompoents(decorationComponents);

    windowHelper->setWindowIcon(lineEdit_icon->text().trimmed());
}