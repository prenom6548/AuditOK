#include <QIcon>
#include <QHBoxLayout>
#include <QTimer>
#include <QPushButton>
#include <QButtonGroup>
#include <QApplication>
#include <QScreen>

#include <kbadge.h>
#include <kpixmapcontainer.h>
#include <kballontip.h>
#include <kbreadcrumb.h>
#include <kcolorbutton.h>
#include <kdialog.h>
#include <kuninstalldialog.h>
#include <kaboutdialog.h>
#include <kinputdialog.h>
#include <kbubblewidget.h>
#include <kbuttonbox.h>
#include <kpushbutton.h>
#include <kcolorcombobox.h>
#include <kmessagebox.h>
#include <kpressbutton.h>
#include <kpasswordedit.h>
#include <kprogressbar.h>
#include <kprogresscircle.h>
#include <kprogressdialog.h>
#include <ksecuritylevelbar.h>
#include <ksearchlineedit.h>
#include <kslider.h>
#include <kswitchbutton.h>
#include <kborderlessbutton.h>
#include <ktoolbutton.h>

#include "main_widget.h"

namespace application_example {

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    /* -------------------------------------------- 进度条 begin ----------------------------------------------------- */

    // 进度条
    auto *progressBar = new kdk::KProgressBar(this);
    progressBar->setTextVisible(true);
    progressBar->setRange(0, 100);
    progressBar->setValue(10);

    // 圆形进度条
    auto *progressCircle1 = new kdk::KProgressCircle(this);
    progressCircle1->setValue(40);
    auto *progressCircle2 = new kdk::KProgressCircle(this);
    progressCircle2->setValue(10);
    progressCircle2->setState(kdk::ProgressBarState::FailedProgress);
    auto *progressCircle3 = new kdk::KProgressCircle(this);
    progressCircle3->setValue(75);
    progressCircle3->setState(kdk::ProgressBarState::SuccessProgress);
    auto *processCircleHLayout1 = new QHBoxLayout;
    processCircleHLayout1->addWidget(progressCircle1);
    processCircleHLayout1->addWidget(progressCircle2);
    processCircleHLayout1->addWidget(progressCircle3);

    auto *progressCircle4 = new kdk::KProgressCircle(this);
    progressCircle4->setValue(40);
    progressCircle4->setTextVisible(false);
    auto *progressCircle5 = new kdk::KProgressCircle(this);
    progressCircle5->setValue(90);
    progressCircle5->setTextVisible(false);
    progressCircle5->setState(kdk::ProgressBarState::FailedProgress);
    auto *progressCircle6 = new kdk::KProgressCircle(this);
    progressCircle6->setValue(75);
    progressCircle6->setTextVisible(false);
    progressCircle6->setState(kdk::ProgressBarState::SuccessProgress);
    auto *processCircleHLayout2 = new QHBoxLayout;
    processCircleHLayout2->addWidget(progressCircle4);
    processCircleHLayout2->addWidget(progressCircle5);
    processCircleHLayout2->addWidget(progressCircle6);

    // 进度条对话框
    auto *progressDialog = new kdk::KProgressDialog("运行中", "取消", 0, 100);
    progressDialog->setSubContent("正在下载");
    progressDialog->setSuffix("MB");
    progressDialog->setWindowTitle("麒麟管家");
    progressDialog->setWindowIcon("kylin-os-manager");
    progressDialog->setValue(0);
    progressDialog->setShowDetail(true);
    progressDialog->setAutoReset(false);
    auto *progressDialogBtn = new QPushButton(this);
    progressDialogBtn->setText("显示进度条弹窗");
    connect(progressDialogBtn, &QPushButton::clicked, this, [=]() {
        progressDialog->show();
    });

    auto *progressCircleVLayout = new QVBoxLayout;
    progressCircleVLayout->addLayout(processCircleHLayout1);
    progressCircleVLayout->addLayout(processCircleHLayout2);
    auto *progressCircleHLayout3 = new QHBoxLayout;
    progressCircleHLayout3->addLayout(progressCircleVLayout);
    progressCircleHLayout3->addWidget(progressDialogBtn);

    auto *progressTimer = new QTimer(this);
    progressTimer->setInterval(50);
    connect(progressTimer, &QTimer::timeout, this, [=]() {
        if (progressBar->value() < progressBar->maximum()) {
            progressBar->setValue(progressBar->value() + 1);
        } else {
            progressBar->setValue(progressBar->minimum());
            progressBar->setState(kdk::ProgressBarState::NormalProgress);
        }

        if (progressDialog->value() < progressDialog->maximum()) {
            progressDialog->setValue(progressDialog->value() + 1);
        } else {
            progressDialog->setValue(progressDialog->minimum());
        }

        if (progressCircle1->value() < progressCircle1->maximum()) {
            progressCircle1->setValue(progressCircle1->value() + 1);
        } else {
            progressCircle1->setValue(progressCircle1->minimum());
            progressCircle1->setState(kdk::ProgressBarState::NormalProgress);
        }

        if (progressCircle4->value() < progressCircle1->maximum()) {
            progressCircle4->setValue(progressCircle1->value() + 1);
        } else {
            progressCircle4->setValue(progressCircle1->minimum());
            progressCircle4->setState(kdk::ProgressBarState::NormalProgress);
        }
    });
    progressTimer->start();

    // 进度条布局
    auto *progressVLayout = new QVBoxLayout;
    progressVLayout->addWidget(progressBar);
    progressVLayout->addLayout(progressCircleHLayout3);

    /* -------------------------------------------- 进度条 end ------------------------------------------------------- */

    /* -------------------------------------------- 弹窗按钮 begin --------------------------------------------------- */

    // 气泡弹窗
    auto *ballontip = new kdk::KBallonTip(this);
    ballontip->setWindowFlag(Qt::FramelessWindowHint);
    ballontip->setAttribute(Qt::WA_TranslucentBackground);
    ballontip->setTipType(kdk::TipType::Normal);
    ballontip->setText("欢迎使用麒麟管家");
    ballontip->setContentsMargins(10, 10, 10, 10);  // 设置内容边距
    auto *ballontipButton = new QPushButton(this);
    ballontipButton->setText("显示气泡弹窗");
    connect(ballontipButton, &QPushButton::clicked, this, [=]() {
        ballontip->setTipTime(1000);
        ballontip->showInfo();
        QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
        ballontip->move((availableGeometry.width() - ballontip->width()) / 2, (availableGeometry.height() - ballontip->height()) / 2);
    });

    // 普通对话框
    auto *normalDialog = new kdk::KDialog;  // ToDo: 内存问题
    normalDialog->setWindowTitle("普通对话框");
    normalDialog->setWindowIcon("kylin-os-manager");
    normalDialog->minimumButton()->show();
    normalDialog->maximumButton()->show();
    normalDialog->menuButton()->show();
    auto *normalDialogLabel = new QLabel;
    normalDialogLabel->setAlignment(Qt::AlignCenter);
    normalDialogLabel->setText("欢迎使用麒麟管家。");
    auto *normalDialogHLayout = new QHBoxLayout;
    normalDialogHLayout->addWidget(normalDialogLabel);
    normalDialog->mainWidget()->setLayout(normalDialogHLayout); // 往 Dialog 中添加自定义控件
    auto *normalDialogBtn = new QPushButton(this);
    normalDialogBtn->setText("显示普通对话框");
    connect(normalDialogBtn, &QPushButton::clicked, this, [=]() {
        normalDialog->show();
    });

    // 卸载对话框
    auto * uninstallDialog = new kdk::KUninstallDialog("卸载 xxx", "1.0.0");    // ToDo: 内存问题
    uninstallDialog->setWindowTitle("卸载对话框");
    uninstallDialog->setWindowIcon("kylin-os-manager");
    auto *uninstallDialogBtn = new QPushButton(this);
    uninstallDialogBtn->setText("显示卸载对话框");
    connect(uninstallDialogBtn, &QPushButton::clicked, this, [=]() {
        uninstallDialog->show();
    });

    // 关于对话框
    auto *aboutDialog = new kdk::KAboutDialog;    // ToDo: 内存问题
    aboutDialog->setAppIcon(QIcon::fromTheme("kylin-os-manager"));
    aboutDialog->setAppName("麒麟管家");
    aboutDialog->setAppVersion("1.0.0");
    auto *aboutDialogBtn = new QPushButton(this);
    aboutDialogBtn->setText("显示关于对话框");
    connect(aboutDialogBtn, &QPushButton::clicked, this, [=]() {
        aboutDialog->show();
    });

    // 消息对话框
    auto *messageBoxBtn = new QPushButton(this);
    messageBoxBtn->setText("显示消息对话框");
    connect(messageBoxBtn, &QPushButton::clicked, this, [=]() {
        kdk::KMessageBox m;
        m.setCustomIcon(QIcon::fromTheme("ukui-dialog-success"));
        m.setText("欢迎使用麒麟管家");

        auto *btn1 = new QPushButton("取消");
        auto *btn2 = new QPushButton("确定");
        m.addButton(btn1, kdk::KMessageBox::NoRole);
        m.addButton(btn2, kdk::KMessageBox::YesRole);

        m.exec();
    });

    // 单行输入框
    auto *inputTextBtn = new QPushButton(this);
    inputTextBtn->setText("显示单行输入框");
    connect(inputTextBtn, &QPushButton::clicked, this, [=]() {
        kdk::KInputDialog::getText(this, "请输入：");
    });

    // 多行输入框
    auto *inputMultiTextBtn = new QPushButton(this);
    inputMultiTextBtn->setText("显示多行输入框");
    connect(inputMultiTextBtn, &QPushButton::clicked, this, [=]() {
        kdk::KInputDialog::getMultiLineText(this, "请输入：");
    });

    // 整数输入框
    auto *inputIntBtn = new QPushButton(this);
    inputIntBtn->setText("显示整数输入框");
    connect(inputIntBtn, &QPushButton::clicked, this, [=]() {
        kdk::KInputDialog::getInt(this, "请输入：");
    });

    // 浮点数输入框
    auto *inputDoubleBtn = new QPushButton(this);
    inputDoubleBtn->setText("显示浮点数输入框");
    connect(inputDoubleBtn, &QPushButton::clicked, this, [=]() {
        kdk::KInputDialog::getDouble(this, "请输入：");
    });

    // 毛玻璃窗口
    auto *bubble = new kdk::KBubbleWidget;
    bubble->setTailPosition(kdk::TailDirection::TopDirection);    // 设置气泡尾部显示位置
    bubble->setTailSize(QSize(16, 8));
    bubble->setBorderRadius(12);
    bubble->setOpacity(0.8);
    bubble->setEnableBlur(true);
    bubble->setFixedSize(320, 320);
    auto *closeBubbleBtn = new QPushButton;
    closeBubbleBtn->setText("关闭");
    connect(closeBubbleBtn, &QPushButton::clicked, this, [=]() {
        bubble->hide();
    });
    auto *bubbleHLayout = new QHBoxLayout;
    bubbleHLayout->addStretch();
    bubbleHLayout->addWidget(closeBubbleBtn);
    bubbleHLayout->addStretch();
    bubble->setLayout(bubbleHLayout);
    auto *bubbleBtn = new QPushButton(this);
    bubbleBtn->setText("显示毛玻璃窗口");
    connect(bubbleBtn, &QPushButton::clicked, this, [=]() {
        bubble->show();
    });

    auto *dialogGridLayout = new QGridLayout;
    dialogGridLayout->addWidget(ballontipButton, 0, 0);
    dialogGridLayout->addWidget(bubbleBtn, 0, 1);
    dialogGridLayout->addWidget(normalDialogBtn, 1, 0);
    dialogGridLayout->addWidget(uninstallDialogBtn, 1, 1);
    dialogGridLayout->addWidget(aboutDialogBtn, 2, 0);
    dialogGridLayout->addWidget(messageBoxBtn, 2, 1);
    dialogGridLayout->addWidget(inputTextBtn, 3, 0);
    dialogGridLayout->addWidget(inputMultiTextBtn, 3, 1);
    dialogGridLayout->addWidget(inputIntBtn, 4, 0);
    dialogGridLayout->addWidget(inputDoubleBtn, 4, 1);

    /* ------------------------------------------- 对话框弹窗 end ---------------------------------------------------- */

    /* ------------------------------------------- 按钮 begin ---------------------------------------------------------*/

    // 标记
    auto *badge = new kdk::KBadge(this);
    badge->setValue(99);

    // 带标记的图片容器
    auto *container = new kdk::KPixmapContainer(this);
    container->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(36, 36));
    container->setFontSize(10);
    container->setValue(99);

    // 颜色按钮 - 方形
    auto *btn1 = new kdk::KColorButton; // 默认使用系统强调色
    auto *btn2 = new kdk::KColorButton;
    btn2->setBackgroundColor(Qt::cyan);
    auto *btn3 = new kdk::KColorButton;
    btn3->setBackgroundColor(Qt::darkYellow);
    auto *btn4 = new kdk::KColorButton;
    btn4->setBackgroundColor(Qt::red);
    auto *btnHLayout1 = new QHBoxLayout;
    btnHLayout1->addWidget(btn1);
    btnHLayout1->addWidget(btn2);
    btnHLayout1->addWidget(btn3);
    btnHLayout1->addWidget(btn4);

    // 颜色按钮 - 方形对勾
    auto *btn5 = new kdk::KColorButton;
    btn5->setBackgroundColor(Qt::gray);
    btn5->setButtonType(kdk::KColorButton::CheckedRect);
    auto *btn6 = new kdk::KColorButton;
    btn6->setBackgroundColor(Qt::green);
    btn6->setButtonType(kdk::KColorButton::CheckedRect);
    auto *btn7 = new kdk::KColorButton;
    btn7->setBackgroundColor(Qt::darkCyan);
    btn7->setButtonType(kdk::KColorButton::CheckedRect);
    auto *btn8 = new kdk::KColorButton;
    btn8->setBackgroundColor(Qt::darkRed);
    btn8->setButtonType(kdk::KColorButton::CheckedRect);
    auto *btnHLayout2 = new QHBoxLayout;
    btnHLayout2->addWidget(btn5);
    btnHLayout2->addWidget(btn6);
    btnHLayout2->addWidget(btn7);
    btnHLayout2->addWidget(btn8);

    // 颜色按钮 - 圆形
    auto *btn9 = new kdk::KColorButton;
    btn9->setBackgroundColor(Qt::green);
    btn9->setButtonType(kdk::KColorButton::Circle);
    auto *btn10 = new kdk::KColorButton;
    btn10->setBackgroundColor(Qt::gray);
    btn10->setButtonType(kdk::KColorButton::Circle);
    auto *btn11 = new kdk::KColorButton;
    btn11->setBackgroundColor(Qt::darkRed);
    btn11->setButtonType(kdk::KColorButton::Circle);
    auto *btn12 = new kdk::KColorButton;
    btn12->setBackgroundColor(Qt::darkGreen);
    btn12->setButtonType(kdk::KColorButton::Circle);
    auto *btnHLayout3 = new QHBoxLayout;
    btnHLayout3->addWidget(btn9);
    btnHLayout3->addWidget(btn10);
    btnHLayout3->addWidget(btn11);
    btnHLayout3->addWidget(btn12);

    auto *group = new QButtonGroup(this);
    group->addButton(btn1);
    group->addButton(btn2);
    group->addButton(btn3);
    group->addButton(btn4);
    group->addButton(btn5);
    group->addButton(btn6);
    group->addButton(btn7);
    group->addButton(btn8);
    group->addButton(btn9);
    group->addButton(btn10);
    group->addButton(btn11);
    group->addButton(btn12);
    group->setExclusive(true);  // 设置按钮互斥

    auto *btnWidget = new QWidget(this);
    btnWidget->setFixedSize(128, 128);
    auto *btnVLayout = new QVBoxLayout;
    btnVLayout->addLayout(btnHLayout1);
    btnVLayout->addLayout(btnHLayout2);
    btnVLayout->addLayout(btnHLayout3);
    btnWidget->setLayout(btnVLayout);

    // 颜色复选框
    auto *colorComboBoxCricle = new kdk::KColorComboBox(this);
    colorComboBoxCricle->setComboType(kdk::KColorComboBox::Circle);
    colorComboBoxCricle->setFixedSize(36, 36);
    colorComboBoxCricle->addColor(QColor(255, 0, 0));
    colorComboBoxCricle->addColor(QColor(0, 255, 0));
    colorComboBoxCricle->addColor(QColor(255, 255, 0));

    auto *colorComboBoxRect = new kdk::KColorComboBox(this);
    colorComboBoxRect->setComboType(kdk::KColorComboBox::RoundedRect);
    colorComboBoxRect->setFixedSize(36, 36);
    colorComboBoxRect->setColorList(QList<QColor>() << QColor(255, 0, 0) << QColor(0, 255, 0) << QColor(255, 255, 0));

    // 颜色复选框布局
    auto *colorComboBoxHLayout = new QHBoxLayout;
    colorComboBoxHLayout->addWidget(colorComboBoxCricle);
    colorComboBoxHLayout->addWidget(colorComboBoxRect);

    // 工具按钮
    auto *toolBtn1 = new kdk::KToolButton(this);
    auto *toolBtn2 = new kdk::KToolButton(this);
    toolBtn2->setArrow(true);
    auto *toolBtn3 = new kdk::KToolButton(this);
    toolBtn3->setType(kdk::KToolButtonType::Background);
    auto *toolBtn4 = new kdk::KToolButton(this);
    toolBtn4->setLoading(true);

    // 工具按钮布局
    auto *toolBtnHayout = new QHBoxLayout;
    toolBtnHayout->addWidget(toolBtn1);
    toolBtnHayout->addWidget(toolBtn2);
    toolBtnHayout->addWidget(toolBtn3);
    toolBtnHayout->addWidget(toolBtn4);

    // 可按压按钮
    auto *pressBtn1 = new kdk::KPressButton(this);
    pressBtn1->setIcon(QIcon::fromTheme("stock-people-symbolic"));
    pressBtn1->setBorderRadius(20);
    pressBtn1->setChecked(true);
    pressBtn1->setTranslucent(true);

    auto *pressBtn2 = new kdk::KPressButton(this);
    pressBtn2->setButtonType(kdk::KPressButton::CircleType);
    pressBtn2->setLoaingStatus(true);
    pressBtn2->setFixedSize(36, 36);

    auto *pressBtn3 = new kdk::KPressButton(this);
    pressBtn3->setButtonType(kdk::KPressButton::CircleType);
    pressBtn3->setIcon(QIcon::fromTheme("system-computer-symbolic"));
    pressBtn3->setCheckable(false);
    pressBtn3->setFixedSize(36, 36);

    // 可按压按钮布局
    auto *pressBtnHLayout = new QHBoxLayout;
    pressBtnHLayout->addWidget(pressBtn1);
    pressBtnHLayout->addWidget(pressBtn2);
    pressBtnHLayout->addWidget(pressBtn3);

    auto *btnGridLayout = new QGridLayout;
    btnGridLayout->addWidget(badge, 0, 0, 1, 1);
    btnGridLayout->addWidget(container, 0, 1, 1, 1);
    btnGridLayout->addLayout(colorComboBoxHLayout, 0, 2, 1, 2);
    btnGridLayout->addLayout(pressBtnHLayout, 1, 0, 1, 3);
    btnGridLayout->addLayout(toolBtnHayout, 2, 0, 1, 4);
    btnGridLayout->addWidget(btnWidget, 3, 0, 1, 2);

    /* ------------------------------------------- 按钮 end ----------------------------------------------------------*/

    // 面包屑
    auto *breadCrumb = new kdk::KBreadCrumb(this);
    breadCrumb->setFlat(true);
    breadCrumb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    breadCrumb->addTab("一级目录");
    breadCrumb->addTab("二级子目录");
    breadCrumb->addTab("三级子目录");
    breadCrumb->setIcon(QIcon::fromTheme("kylin-os-manager"));

    // 密码框
    auto *passwordEdit = new kdk::KPasswordEdit(this);
    QString text("请输入密码");
    passwordEdit->setPlaceholderText(text);
    passwordEdit->setClearButtonEnabled(true);
    passwordEdit->setEchoModeBtnVisible(true);
    auto *passwordEditBtn1 = new QPushButton(this);
    passwordEditBtn1->setText("正确");
    auto *passwordEditBtn2 = new QPushButton(this);
    passwordEditBtn2->setText("错误");
    auto *passwordEditBtn3 = new QPushButton(this);
    passwordEditBtn3->setText("加载");
    connect(passwordEditBtn1, &QPushButton::clicked, this, [=]() {
        passwordEdit->setState(kdk::LoginState::LoginSuccess);
        passwordEdit->setFocus();
    });
    connect(passwordEditBtn2, &QPushButton::clicked, this, [=]() {
        passwordEdit->setState(kdk::LoginState::LoginFailed);
        passwordEdit->setFocus();
    });
    connect(passwordEditBtn3, &QPushButton::clicked, this, [=]() {
        QString text = passwordEditBtn3->text();
        if (text == "加载") {
            passwordEdit->setLoading(true);
            passwordEditBtn3->setText("取消加载");
        }
        if (text == "取消加载") {
            passwordEdit->setLoading(false);
            passwordEditBtn3->setText("加载");
        }
    });

    // 密码框布局
    auto *passwordEditBtnHLayout = new QHBoxLayout;
    passwordEditBtnHLayout->addWidget(passwordEditBtn1);
    passwordEditBtnHLayout->addWidget(passwordEditBtn2);
    passwordEditBtnHLayout->addWidget(passwordEditBtn3);
    auto *passwordEditVLayout = new QVBoxLayout;
    passwordEditVLayout->addWidget(passwordEdit);
    passwordEditVLayout->addLayout(passwordEditBtnHLayout);

    // 密码等级
    auto *passwordLabel = new QLabel("请设置密码等级", this);
    auto *passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    auto *passwordSecHLayout = new QHBoxLayout;
    passwordSecHLayout->addWidget(passwordLabel);
    passwordSecHLayout->addWidget(passwordLineEdit);
    auto *securityLevelBar = new kdk::KSecurityLevelBar(this);
    securityLevelBar->hide();
    auto *passwordSecVLayout = new QVBoxLayout;
    passwordSecVLayout->addLayout(passwordSecHLayout);
    passwordSecVLayout->addWidget(securityLevelBar);
    connect(passwordLineEdit, &QLineEdit::textChanged, this, [=]() {
        int passwordLength = passwordLineEdit->text().length();
        if (passwordLength < 3)
            securityLevelBar->setSecurityLevel(kdk::SecurityLevel::Low);
        else if (passwordLength >= 3 && passwordLength < 6)
            securityLevelBar->setSecurityLevel(kdk::SecurityLevel::Medium);
        else
            securityLevelBar->setSecurityLevel(kdk::SecurityLevel::High);

        securityLevelBar->show();
    });

    // 搜索框
    auto *searchLineEdit = new kdk::KSearchLineEdit(this);
    searchLineEdit->setPlaceholderText("搜索");
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setAlignment(Qt::AlignLeft);

    // 滑动条
    auto *slider = new kdk::KSlider(this);
    slider->setSliderType(kdk::KSliderType::StepSlider);
    slider->setFocusPolicy(Qt::ClickFocus);
    slider->setTickInterval(10);
    slider->setSingleStep(10);
    slider->setRange(-50, 50);
    slider->setValue(20);

    auto *searchVLayout = new QVBoxLayout;
    searchVLayout->addWidget(breadCrumb);
    searchVLayout->addLayout(passwordEditVLayout);
    searchVLayout->addLayout(passwordSecVLayout);
    searchVLayout->addWidget(searchLineEdit);
    searchVLayout->addWidget(slider);

    /* -------------------------------------------------------------------------------------------------------------- */

    // 按钮盒
    auto *boxBtn1 = new kdk::KPushButton;
    boxBtn1->setIcon(QIcon::fromTheme("list-add-symbolic"));
    boxBtn1->setIconSize(QSize(32, 32));
    auto *boxBtn2 = new kdk::KPushButton;
    boxBtn2->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    boxBtn2->setIconSize(QSize(32, 32));
    auto *box1 = new kdk::KButtonBox(this);
    box1->addButton(boxBtn1);
    box1->addButton(boxBtn2);

    auto *boxBtn3 = new kdk::KPushButton;
    boxBtn3->setIcon(QIcon::fromTheme("system-conputer-symbolic"));
    auto *boxBtn4 = new kdk::KPushButton;
    boxBtn4->setIcon(QIcon::fromTheme("format-text-italic-symbolic"));
    auto *boxBtn5 = new kdk::KPushButton;
    boxBtn5->setIcon(QIcon::fromTheme("format-text-underline-symbolic"));
    auto *box2 = new kdk::KButtonBox(this);
    box2->setButtonList(QList<kdk::KPushButton *>() << boxBtn3 << boxBtn4 << boxBtn4);
    box2->setCheckable(true);    // 设置 KButtonBox 的按钮是否可以选中
    box2->setExclusive(true);    // 设置按钮间是否互斥

    auto *boxAddBtn = new kdk::KPushButton;
    boxAddBtn->setIcon(QIcon::fromTheme("format-text-strikethrough-symbolic"));
    connect(box1, &kdk::KButtonBox::buttonClicked, this, [=](QAbstractButton *clickedBtn) {
        if (clickedBtn == boxBtn1)
            box2->addButton(boxAddBtn);
        if (clickedBtn == boxBtn2)
            box2->removeButton(boxAddBtn);
    });

    // 按钮盒布局
    auto *boxVLayout = new QVBoxLayout;
    boxVLayout->addStretch();
    boxVLayout->addWidget(box1);
    boxVLayout->addWidget(box2);
    boxVLayout->addStretch();

    // 切换按钮
    auto *switchBtn = new kdk::KSwitchButton(this);
    switchBtn->setCheckable(true);
    switchBtn->setChecked(true);
    switchBtn->setFixedSize(64, 36);
    auto *borderlessBtn = new kdk::KBorderlessButton(this);
    borderlessBtn->setChecked(true);
    borderlessBtn->setText(!switchBtn->isEnabled()? "启用": "禁用");
    connect(borderlessBtn, &kdk::KBorderlessButton::clicked, this, [=]() {
        switchBtn->setEnabled(!switchBtn->isEnabled());
        borderlessBtn->setText(!switchBtn->isEnabled()? "启用": "禁用");
    });

    // 切换按钮布局
    auto *switchHLayout = new QHBoxLayout;
    switchHLayout->addWidget(switchBtn);
    switchHLayout->addWidget(borderlessBtn);

    /* -------------------------------------------------------------------------------------------------------------- */

    auto *mainHLayout1 = new QHBoxLayout;
    mainHLayout1->addLayout(progressVLayout);
    mainHLayout1->addLayout(btnGridLayout);

    auto *mainHLayout2 = new QHBoxLayout;
    mainHLayout2->addLayout(boxVLayout);
    mainHLayout2->addLayout(switchHLayout);

    auto *mainHLayout3 = new QHBoxLayout;
    mainHLayout3->addLayout(searchVLayout);
    mainHLayout3->addLayout(dialogGridLayout);

    auto *mainVLayout = new QVBoxLayout;
    mainVLayout->addLayout(mainHLayout1);
    mainVLayout->addLayout(mainHLayout2);
    mainVLayout->addLayout(mainHLayout3);

    setLayout(mainVLayout);
}

MainWindow::~MainWindow() {

}

}
