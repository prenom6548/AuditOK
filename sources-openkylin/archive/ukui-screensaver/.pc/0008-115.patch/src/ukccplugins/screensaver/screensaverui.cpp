#include "screensaverui.h"
#include <QPluginLoader>


ScreensaverUi::ScreensaverUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    uiLayout = new QVBoxLayout(this);
    uiLayout->setSpacing(8);
    uiLayout->setMargin(0);
    initUi();
    sendSignals();
}

ScreensaverUi::~ScreensaverUi()
{

}

void ScreensaverUi::initUi()
{
    initPreview();
    initSetting();
    uiLayout->addStretch();
}

void ScreensaverUi::initPreview()
{
    screensaverTitle = new KLabel(this);
    screensaverTitle->setContentsMargins(16, 0, 0, 0);
    previewFrame     = new UkccFrame(this);
    previewLayout    = new QHBoxLayout(previewFrame);
    previewWidget    = new PreviewWidget(this);

    uiLayout->addWidget(screensaverTitle);
    uiLayout->addWidget(previewFrame);

    previewLayout->addWidget(previewWidget);

    //~ contents_path /Screensaver/Screensaver
    screensaverTitle->setText(tr("Screensaver"));
    previewFrame->setFixedHeight(212);
    previewWidget->setFixedSize(300, 180);
    previewLayout->addWidget(previewWidget);
    previewLayout->addStretch();
    QHBoxLayout *layout = new QHBoxLayout(previewWidget);
    layout->setMargin(0);
}

void ScreensaverUi::initSetting()
{
    screensaverGroup = new SettingGroup(this);
    //~ contents_path /Screensaver/Idle time
    idleTimeWidget   = new ComboxWidget(tr("Idle time"), screensaverGroup);
    //~ contents_path /Screensaver/Screensaver program
    programWidget    = new ComboxWidget(tr("Screensaver program"), screensaverGroup);
    customizeGroup   = new SettingGroup(screensaverGroup);

    uiLayout->addWidget(screensaverGroup);

    screensaverGroup->addWidget(idleTimeWidget);
    screensaverGroup->addWidget(programWidget);
    screensaverGroup->addWidget(customizeGroup);

    initSourceFrame();
    initSwitchModeFrame();
    initSwitchTimeFrame();
    initCustomTextFrame();
    initTextPositionFrame();
    initBreakTimeFrame();
    initScreenlockFrame();
}

void ScreensaverUi::initSourceFrame()
{
    UkccFrame   *screensaverSourceFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *sourceLayout           = new QHBoxLayout(screensaverSourceFrame);
    KLabel    *sourceLabel            = new KLabel();

    sourcePathLine = new QLineEdit();
    sourceButton   = new QPushButton();

    sourceLayout->setContentsMargins(16, 0, 16, 0);
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(sourcePathLine);
    sourceLayout->addWidget(sourceButton);
    //~ contents_path /Screensaver/Screensaver source
    sourceLabel->setText(tr("Screensaver source"));
    sourceLabel->setFixedWidth(220);
    sourcePathLine->setMinimumWidth(252);
    sourcePathLine->setReadOnly(true);
    sourceButton->adjustSize();
    sourceButton->setText(tr("Select"));
    sourceButton->raise();

    customizeGroup->addWidget(screensaverSourceFrame);
}

void ScreensaverUi::initSwitchModeFrame()
{
    UkccFrame    *pictureSwitchFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout  *randomLayout       = new QHBoxLayout(pictureSwitchFrame);
    KLabel     *randomLabel        = new KLabel();
    QRadioButton *radioOrder         = new QRadioButton();
    QRadioButton *radioRand          = new QRadioButton();

    switchModeGroup  = new QButtonGroup();

    switchModeGroup->addButton(radioOrder, 0);
    switchModeGroup->addButton(radioRand, 1);

    radioOrder->setFixedWidth(220);
    radioRand->adjustSize();
    radioOrder->setText(tr("Ordinal"));
    radioRand->setText(tr("Random"));

    randomLayout->setContentsMargins(16, 0, 16, 0);
    randomLayout->addWidget(randomLabel);
    randomLayout->addWidget(radioOrder);
    randomLayout->addWidget(radioRand);
    //~ contents_path /Screensaver/Random switching
    randomLabel->setText(tr("Random switching"));
    randomLabel->setFixedWidth(220);
    randomLayout->addStretch();

    customizeGroup->addWidget(pictureSwitchFrame);
}

void ScreensaverUi::initSwitchTimeFrame()
{
    UkccFrame   *timeSetFrame  = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *timeSetLayout = new QHBoxLayout();
    KLabel    *timeSetLabel  = new KLabel();

    switchTimeComboBox = new QComboBox();

    timeSetFrame->setLayout(timeSetLayout);
    timeSetLayout->setContentsMargins(16, 0, 16, 0);
    timeSetLayout->addWidget(timeSetLabel);
    timeSetLayout->addWidget(switchTimeComboBox);
    //~ contents_path /Screensaver/Switching time
    timeSetLabel->setText(tr("Switching time"));
    timeSetLabel->setFixedWidth(220);
    switchTimeComboBox->setMinimumWidth(340);

    customizeGroup->addWidget(timeSetFrame);
}

void ScreensaverUi::initCustomTextFrame()
{
    UkccFrame   *showTextFrame  = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *showTextLayout = new QHBoxLayout();
    KLabel    *showLabel      = new KLabel();

    textLineEdit = new QLineEdit(); //用户输入文字
    showTextFrame->setLayout(showTextLayout);
    showTextLayout->setContentsMargins(16, 6, 15, 8);
    showTextLayout->addWidget(showLabel);
    showTextLayout->addWidget(textLineEdit);
    //~ contents_path /Screensaver/Text(up to 30 characters):
    showLabel->setText(tr("Text(up to 30 characters):"));
    showLabel->setFixedWidth(220);
    textLineEdit->setMaxLength(30);

    customizeGroup->addWidget(showTextFrame);
}

void ScreensaverUi::initTextPositionFrame()
{
    UkccFrame    *showTextSetFrame = new UkccFrame(customizeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout  *textSetLayout    = new QHBoxLayout(showTextSetFrame);
    KLabel     *textSetLabel     = new KLabel();
    QRadioButton *radioRandom      = new QRadioButton(this);
    QRadioButton *radioCenter      = new QRadioButton(this);

    textPositionGroup = new QButtonGroup();
    textPositionGroup->addButton(radioRandom, 0);
    textPositionGroup->addButton(radioCenter, 1);

    showTextSetFrame->setFixedHeight(60);
    textSetLayout->addWidget(textSetLabel);
    textSetLayout->setContentsMargins(16,0,16,0);
    //~ contents_path /Screensaver/Text position
    textSetLabel->setText(tr("Text position"));
    textSetLabel->setFixedWidth(220);
    radioRandom->setFixedWidth(220);
    radioCenter->adjustSize();

    textSetLayout->addWidget(radioRandom);
    textSetLayout->addWidget(radioCenter);
    textSetLayout->addStretch();
    radioCenter->setText(tr("Centered"));
    radioRandom->setText(tr("Randow(Bubble text)"));

    customizeGroup->addWidget(showTextSetFrame);
}

void ScreensaverUi::initBreakTimeFrame()
{
    UkccFrame   *showTimeFrame  = new UkccFrame(screensaverGroup, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *showTimeLayout = new QHBoxLayout(showTimeFrame);
    KLabel    *showTimeLabel  = new KLabel();

    showTimeLayout->setContentsMargins(16,0,16,0);
    showTimeLayout->addWidget(showTimeLabel);
    //添加休息时间显示按钮
    showCustomTimeBtn = new KSwitchButton(showTimeFrame);
    showUkuiTimeBtn   = new KSwitchButton(showTimeFrame);
    showTimeLayout->addStretch();
    showTimeLayout->addWidget(showCustomTimeBtn);
    showTimeLayout->addWidget(showUkuiTimeBtn);
    showTimeLabel->setFixedWidth(220);

    //~ contents_path /Screensaver/Show rest time
    showTimeLabel->setText(tr("Show rest time"));
    screensaverGroup->addWidget(showTimeFrame);
}

void ScreensaverUi::initScreenlockFrame()
{
    UkccFrame   *lockFrame  = new UkccFrame(screensaverGroup, UkccFrame::BorderRadiusStyle::Around, true);;
    QHBoxLayout *lockLayout = new QHBoxLayout(lockFrame);

    //~ contents_path /Screensaver/Lock screen when screensaver boot
    KLabel    *lockLabel  = new KLabel();
    lockLabel->setText(tr("Lock screen when screensaver boot"));
    lockLabel->setFixedWidth(400);

    lockFrame->setFixedHeight(60);
    lockLayout->setContentsMargins(16,0,16,0);
    lockLayout->addWidget(lockLabel);

    lockButton = new KSwitchButton(lockFrame);
    lockLayout->addStretch();
    lockLayout->addWidget(lockButton);

    screensaverGroup->addWidget(lockFrame);
}

void ScreensaverUi::setIdleTimeOptions(const QStringList &textList, const QList<int> &dataList)
{
    if (textList.size() == dataList.size()) {
        idleTimeWidget->comboBox()->clear();
        for (int i = 0; i < textList.size(); ++i) {
            idleTimeWidget->comboBox()->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setScreensaverOptions(const QStringList &textList, const QStringList &dataList)
{
    if (textList.size() == dataList.size()) {
        programWidget->comboBox()->clear();
        for (int i = 0; i < textList.size(); ++i) {
            programWidget->comboBox()->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList)
{
    if (textList.size() == dataList.size()) {
        switchTimeComboBox->clear();
        for (int i = 0; i < textList.size(); ++i) {
            switchTimeComboBox->addItem(textList.at(i), dataList.at(i));
        }
    }
}

void ScreensaverUi::setIdleTime(int time)
{
    for (int i = 0; i < idleTimeWidget->comboBox()->count(); ++i) {
        if (time == idleTimeWidget->comboBox()->itemData(i)) {
            idleTimeWidget->comboBox()->blockSignals(true);
            idleTimeWidget->comboBox()->setCurrentIndex(i);
            idleTimeWidget->comboBox()->blockSignals(false);
        }
    }
}

void ScreensaverUi::setScreensaver(QString type)
{
    for (int i = 0; i < programWidget->comboBox()->count(); ++i) {
        if (type == programWidget->comboBox()->itemData(i)) {
            programWidget->comboBox()->blockSignals(true);
            programWidget->comboBox()->setCurrentIndex(i);
            programWidget->comboBox()->blockSignals(false);
        }
    }

    if (type == "ukui") {
        customizeGroup->setVisible(false);
        showUkuiTimeBtn->setVisible(true);
        showCustomTimeBtn->setVisible(false);
    } else if (type == "custom") {
        customizeGroup->setVisible(true);
        showUkuiTimeBtn->setVisible(false);
        showCustomTimeBtn->setVisible(true);
    }
}

void ScreensaverUi::updatePreview(QWidget *w)
{
    for (QObject *o : previewWidget->children()) {
        if (o->objectName() == "screensaverWidget") {
            delete o;
            o = nullptr;
        }
    }
    w->setObjectName("screensaverWidget");
    previewWidget->layout()->addWidget(w);
    previewWidget->update();
}

void ScreensaverUi::setSourcePath(const QString &text)
{
    if (sourcePathLine == nullptr) {
        return;
    }

    QFontMetrics fontMetrics(sourcePathLine->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > sourcePathLine->width()) {
       sourcePathLine->setText(fontMetrics.elidedText(text, Qt::ElideRight, sourcePathLine->width()));
       sourcePathLine->setToolTip(text);
    } else {
       sourcePathLine->setText(text);
       sourcePathLine->setToolTip("");
    }
}

void ScreensaverUi::setSwitchRandom(const bool &b)
{
    switchModeGroup->blockSignals(true);
    if (b) {
        switchModeGroup->button(1)->setChecked(true);
    } else {
        switchModeGroup->button(0)->setChecked(true);
    }
    switchModeGroup->blockSignals(false);
}

void ScreensaverUi::setSwitchTime(const int &time)
{
    for (int i = 0; i < switchTimeComboBox->count(); ++i) {
        if (time == switchTimeComboBox->itemData(i)) {
            switchTimeComboBox->blockSignals(true);
            switchTimeComboBox->setCurrentIndex(i);
            switchTimeComboBox->blockSignals(false);
        }
    }
}

void ScreensaverUi::setCustomText(const QString &text)
{
    textLineEdit->blockSignals(true);
    textLineEdit->setText(text);
    textLineEdit->blockSignals(false);
}

void ScreensaverUi::setTextCentered(const bool &b)
{
    textPositionGroup->blockSignals(true);
    if (b) {
        textPositionGroup->button(1)->setChecked(true);
    } else {
        textPositionGroup->button(0)->setChecked(true);
    }
    textPositionGroup->blockSignals(false);
}

void ScreensaverUi::setShowUkuiTime(const bool &b)
{
    showUkuiTimeBtn->blockSignals(true);
    showUkuiTimeBtn->setChecked(b);
    showUkuiTimeBtn->blockSignals(false);
}

void ScreensaverUi::setShowCustomTime(const bool &b)
{
    showCustomTimeBtn->blockSignals(true);
    showCustomTimeBtn->setChecked(b);
    showCustomTimeBtn->blockSignals(false);
}

void ScreensaverUi::setLockscreen(const bool &b)
{
    lockButton->blockSignals(true);
    lockButton->setChecked(b);
    lockButton->blockSignals(false);
}

void ScreensaverUi::sendSignals()
{
    connect(idleTimeWidget->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT idleTimeChanged(idleTimeWidget->comboBox()->currentData().toInt());
    });
    connect(programWidget->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        QString type = programWidget->comboBox()->currentData().toString();
        Q_EMIT screensaverChanged(type);
        if (type == "ukui") {
            customizeGroup->setVisible(false);
            showUkuiTimeBtn->setVisible(true);
            showCustomTimeBtn->setVisible(false);
        } else if (type == "custom") {
            customizeGroup->setVisible(true);
            showUkuiTimeBtn->setVisible(false);
            showCustomTimeBtn->setVisible(true);
        }
    });
    connect(sourceButton, &QPushButton::clicked, this, [=]() {
        QString filter;
        filter = tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp *.svg)");
        QFileDialog fd(this);

        QList<QUrl> usb_list = fd.sidebarUrls();
        int sidebarNum = 8;// 最大添加U盘数，可以自己定义
        QString home_path = QDir::homePath().section("/", -1, -1);
        QString mnt = "/media/" + home_path + "/";
        QDir mntDir(mnt);
        mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList file_list = mntDir.entryInfoList();
        QList<QUrl> mntUrlList;
        for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
            QFileInfo fi = file_list.at(i);
            mntUrlList << QUrl("file://" + fi.filePath());
        }

        QFileSystemWatcher m_fileSystemWatcher(&fd);
        m_fileSystemWatcher.addPath("/media/" + home_path + "/");
        connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
                [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
            QDir m_wmntDir(path);
            m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
            mntUrlList.clear();
            for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
                QFileInfo m_fi = m_wfilist.at(i);
                mntUrlList << QUrl("file://" + m_fi.filePath());
            }
            fd.setSidebarUrls(usb_list + mntUrlList);
            fd.update();
        });

        connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
            fd.setSidebarUrls(usb_list);
        });

        fd.setDirectory(QString(sourcePathLine->text()));
        fd.setAcceptMode(QFileDialog::AcceptOpen);
        fd.setViewMode(QFileDialog::List);
        fd.setNameFilter(filter);
        fd.setFileMode(QFileDialog::Directory);
        fd.setWindowTitle(tr("select custom screensaver dir"));
        fd.setLabelText(QFileDialog::Accept, tr("Select"));
        fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
        fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
        fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
        fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

        fd.setSidebarUrls(usb_list + mntUrlList);

        if (fd.exec() != QDialog::Accepted) {
            return;
        }
        QString selectedfile;
        selectedfile = fd.selectedFiles().first();
        if(selectedfile != "") {  //非点击【取消】时
            setSourcePath(selectedfile);
            Q_EMIT customPathChanged(selectedfile);
        }
    });
    connect(switchModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id) {
        if (id == 0) {
            Q_EMIT switchRandomChanged(false);
        } else {
            Q_EMIT switchRandomChanged(true);
        }
    });
    connect(switchTimeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT switchTimeChanged(switchTimeComboBox->currentData().toInt());
    });
    connect(textLineEdit, &QLineEdit::textChanged, this, [=](){
        Q_EMIT customTextChanged(textLineEdit->text());
    });
    connect(textPositionGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id) {
        if (id == 0) {
            Q_EMIT textCenteredChanged(false);
        } else {
            Q_EMIT textCenteredChanged(true);
        }
    });
    connect(showUkuiTimeBtn, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT showUkuiTimeChanged(showUkuiTimeBtn->isChecked());
    });
    connect(showCustomTimeBtn, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT showCustomChanged(showCustomTimeBtn->isChecked());
    });
    connect(lockButton, &KSwitchButton::stateChanged, this, [=]() {
        Q_EMIT lockscreenChanged(lockButton->isChecked());
    });
}
