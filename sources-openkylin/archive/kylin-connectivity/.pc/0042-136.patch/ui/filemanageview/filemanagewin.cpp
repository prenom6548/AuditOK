#include "filemanagewin.h"

#include <QHBoxLayout>
#include <QFont>
#include <QScrollBar>
#include <QDebug>

const int ICON_SIZE = 16;
const int BTN_SIZE = 40;
const int SEARCHLINE_WIDTH = 200;
const int CRUMB_SPACE = 40;

FileManageWin::FileManageWin(QWidget *parent) : QWidget(parent)
{
    setProperty("setWidth", 0);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    installEventFilter(this);

    m_retreatBtn = new kdk::KToolButton(this);
    connect(m_retreatBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_retreatBtn->setIcon(QIcon::fromTheme("go-next-rtl-symbolic"));
    m_retreatBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_retreatBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_retreatBtn->setToolTip(tr("Go Back"));
    m_retreatBtn->setType(kdk::KToolButtonType::Flat);
    m_retreatBtn->setCursor(Qt::PointingHandCursor);
    m_retreatBtn->setFocusPolicy(Qt::NoFocus);
    m_retreatBtn->setEnabled(false);
    m_retreatBtn->hide();

    m_forwardBtn = new kdk::KToolButton(this);
    connect(m_forwardBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_forwardBtn->setIcon(QIcon::fromTheme("go-next-symbolic"));
    m_forwardBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_forwardBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_forwardBtn->setToolTip(tr("Go Forward"));
    m_forwardBtn->setType(kdk::KToolButtonType::Flat);
    m_forwardBtn->setCursor(Qt::PointingHandCursor);
    m_forwardBtn->setFocusPolicy(Qt::NoFocus);
    m_forwardBtn->setEnabled(false);
    m_forwardBtn->hide();

    QFont font;
    font.setBold(true);
    // 面包屑
    m_crumb = new kdk::KBreadCrumb(this);
    connect(m_crumb, &kdk::KBreadCrumb::currentChanged, this, &FileManageWin::slotCrumbIndexChange);
    m_crumb->setFont(font);
    m_crumb->setFlat(true);

    m_searchBtn = new kdk::KToolButton(this);
    connect(m_searchBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_searchBtn->setIcon(QIcon::fromTheme("system-search-symbolic"));
    m_searchBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_searchBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_searchBtn->setToolTip(tr("Search File"));
    m_searchBtn->setType(kdk::KToolButtonType::Flat);
    m_searchBtn->setFocusPolicy(Qt::NoFocus);
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    m_searchBtn->hide();

    m_searchLine = new kdk::KSearchLineEdit(this);
    connect(m_searchLine, &kdk::KSearchLineEdit::textChanged, this, &FileManageWin::slotSearchTextChanged);
    m_searchLine->setFixedHeight(BTN_SIZE);
    m_searchLine->setFixedWidth(0);
    m_searchLine->setFocusPolicy(Qt::NoFocus);
    m_searchLine->setClearButtonEnabled(true);
    m_searchLine->setContextMenuPolicy(Qt::DefaultContextMenu);
    m_searchLine->installEventFilter(this);
    m_searchLine->hide();

    m_searchLineAnimation = new QPropertyAnimation(this, "setWidth");
    connect(m_searchLineAnimation, &QPropertyAnimation::valueChanged, this, &FileManageWin::slotAnimationValueChanged);
    // 设置动画时长;
    m_searchLineAnimation->setDuration(200);
    // 设置动画变化类型;
    m_searchLineAnimation->setEasingCurve(QEasingCurve::OutQuad);

    m_editBtn = new kdk::KToolButton(this);
    connect(m_editBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_editBtn->setIcon(QIcon::fromTheme("document-edit-symbolic"));
    m_editBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_editBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_editBtn->setToolTip(tr("Select"));
    m_editBtn->setType(kdk::KToolButtonType::Flat);
    m_editBtn->setFocusPolicy(Qt::NoFocus);
    m_editBtn->setCursor(Qt::PointingHandCursor);
    m_editBtn->hide();

    m_verticalLine = new QFrame(this);
    m_verticalLine->setLineWidth(1);
    m_verticalLine->setFixedHeight(16);
    m_verticalLine->setFrameShape(QFrame::VLine);
    m_verticalLine->setFrameShadow(QFrame::Raised);
    m_verticalLine->setFocusPolicy(Qt::NoFocus);
    m_verticalLine->hide();

    // 模式
    m_modeBtn = new kdk::KToolButton(this);
    connect(m_modeBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_modeBtn->setIcon(QIcon::fromTheme("view-grid-symbolic"));
    m_modeBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_modeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_modeBtn->setType(kdk::KToolButtonType::Flat);
    m_modeBtn->setToolTip(tr("List Mode"));
    m_modeBtn->setFocusPolicy(Qt::NoFocus);
    m_modeBtn->setCursor(Qt::PointingHandCursor);
    m_modeBtn->hide();

    // 刷新
    m_refreshBtn = new kdk::KToolButton(this);
    connect(m_refreshBtn, &kdk::KToolButton::clicked, this, &FileManageWin::slotKdkBtnClicked);
    m_refreshBtn->setIcon(QIcon::fromTheme("view-refresh-symbolic"));
    m_refreshBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_refreshBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_refreshBtn->setType(kdk::KToolButtonType::Flat);
    m_refreshBtn->setToolTip(tr("Refresh"));
    m_refreshBtn->setFocusPolicy(Qt::NoFocus);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->hide();

    m_statusLab = new QLabel(this);
    m_statusLab->setText(tr("Select File"));
    m_statusLab->setFont(font);
    m_statusLab->setContentsMargins(8, 0, 0, 0);
    m_statusLab->setFixedHeight(BTN_SIZE);
    m_statusLab->setFocusPolicy(Qt::NoFocus);
    m_statusLab->hide();

    m_selectBtn = new ClickLabel(this);
    connect(m_selectBtn, &ClickLabel::clicked, this, &FileManageWin::slotBtnClicked);
    m_selectBtn->setText(tr("Select All"));
    m_selectBtn->setStyleSheet("color:#3790FA;");
    m_selectBtn->setCursor(Qt::PointingHandCursor);
    m_selectBtn->setFixedHeight(BTN_SIZE);
    m_selectBtn->setFocusPolicy(Qt::NoFocus);
    m_selectBtn->hide();

    m_finishBtn = new ClickLabel(this);
    connect(m_finishBtn, &ClickLabel::clicked, this, &FileManageWin::slotBtnClicked);
    m_finishBtn->setText(tr("Finish"));
    m_finishBtn->setStyleSheet("color:#3790FA;");
    m_finishBtn->setCursor(Qt::PointingHandCursor);
    m_finishBtn->setFixedHeight(BTN_SIZE);
    m_finishBtn->setFocusPolicy(Qt::NoFocus);
    m_finishBtn->hide();

    m_lab1 = new QLabel(this);
    m_lab1->setFixedSize(16, BTN_SIZE);
    m_lab1->setStyleSheet("background:transparent");
    m_lab2 = new QLabel(this);
    m_lab2->setFixedSize(28, BTN_SIZE);
    m_lab2->setStyleSheet("background:transparent");
    m_lab3 = new QLabel(this);
    m_lab3->setFixedSize(12, BTN_SIZE);
    m_lab3->setStyleSheet("background:transparent");
    m_lab4 = new QLabel(this);
    m_lab4->setFixedSize(12, BTN_SIZE);
    m_lab4->setStyleSheet("background:transparent");
    m_lab5 = new QLabel(this);
    m_lab5->setFixedSize(12, BTN_SIZE);
    m_lab5->setStyleSheet("background:transparent");
    m_lab6 = new QLabel(this);
    m_lab6->setFixedSize(12, BTN_SIZE);
    m_lab6->setStyleSheet("background:transparent");

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->setAlignment(Qt::AlignBottom);
    hLayout->addWidget(m_retreatBtn);
    hLayout->addWidget(m_lab1);
    hLayout->addWidget(m_forwardBtn);
    hLayout->addWidget(m_statusLab);
    hLayout->addWidget(m_lab2);
    hLayout->addWidget(m_crumb);
    hLayout->addStretch();
    hLayout->addWidget(m_searchBtn);
    hLayout->addWidget(m_searchLine);
    hLayout->addWidget(m_lab3);
    hLayout->addWidget(m_editBtn);
    hLayout->addWidget(m_selectBtn);
    hLayout->addWidget(m_lab4);
    hLayout->addWidget(m_verticalLine);
    hLayout->addWidget(m_lab5);
    hLayout->addWidget(m_finishBtn);
    hLayout->addWidget(m_modeBtn);
    hLayout->addWidget(m_lab6);
    hLayout->addWidget(m_refreshBtn);

    m_loadAnimation = new LoadAnimation(this);

    m_stackedWin = new QStackedWidget(this);
    m_stackedWin->setContentsMargins(0, 0, 0, 0);
    m_stackedWin->addWidget(m_loadAnimation);
    m_stackedWin->setCurrentIndex(FileManageWin::PageIndex::LoadPage);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_stackedWin);
    setLayout(vLayout);
}

void FileManageWin::setEditType(bool isSelectAll)
{
    if (isSelectAll) {
        m_selectBtn->setText(tr("Deselect All"));
    } else {
        m_selectBtn->setText(tr("Select All"));
    }
    changeTabMode(FileManageWin::TabType::Edit);
}

void FileManageWin::setWidget(QWidget *win, FileManageWin::TabType type)
{
    m_type = type;
    changeTabMode(m_type);
    if (m_stackedWin->count() > 1) {
        m_stackedWin->removeWidget(m_stackedWin->widget(FileManageWin::PageIndex::InfoPage));
    }
    win->setAcceptDrops(true);
    m_stackedWin->addWidget(win);
    m_stackedWin->setCurrentIndex(FileManageWin::PageIndex::InfoPage);
}

void FileManageWin::pushCrumb(QString str)
{
    m_crumbList.append(str);
    generateDisplayTextList();
    autoResetCrumb();
}
void FileManageWin::clearCrumb()
{
    m_crumbList.clear();
    for (int i = m_crumb->count() - 1; i >= 0; i--) {
        m_crumb->removeTab(i);
    }
}

void FileManageWin::load()
{
    if (m_searchLine->isHidden()) {
        changeTabMode(FileManageWin::TabType::Load);
    }
    m_stackedWin->setCurrentIndex(FileManageWin::PageIndex::LoadPage);
    if (m_stackedWin->count() > 1) {
        m_stackedWin->removeWidget(m_stackedWin->widget(FileManageWin::PageIndex::InfoPage));
    }
}

void FileManageWin::setGoBackEnabled(bool isEnabled)
{
    m_retreatBtn->setEnabled(isEnabled);
    m_retreatBtn->update();
}
void FileManageWin::setGoForwardEnabled(bool isEnabled)
{
    m_forwardBtn->setEnabled(isEnabled);
    m_forwardBtn->update();
}

QStringList FileManageWin::getPathList() const
{
    return m_crumbList;
}

QString FileManageWin::crumbText(CrumbTag type)
{
    switch (type) {
        case CrumbTag::FileList:
            return m_fileListTag;
        case CrumbTag::Picture:
            return m_pictureTag;
        case CrumbTag::Video:
            return m_videoTag;
        case CrumbTag::Music:
            return m_musicTag;
        case CrumbTag::Doc:
            return m_docTag;
        case CrumbTag::QQ:
            return m_qqTag;
        case CrumbTag::WeChat:
            return m_wechatTag;
        case CrumbTag::Storage:
            return m_storageTag;
    }
    return "";
}

void FileManageWin::changeFontSize(double fontSize)
{
    m_fontSize = fontSize;
    QFont font;
    font.setPointSizeF(fontSize);
    m_selectBtn->setFont(font);
    m_finishBtn->setFont(font);
    font.setBold(true);
    m_statusLab->setFont(font);
    m_crumb->setFont(font);
    generateDisplayTextList();
    autoResetCrumb();
    update();
}

bool FileManageWin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::Resize && !m_crumbList.isEmpty()) {
            autoResetCrumb();
        }
    }
    if (obj == m_searchLine) {
        if (m_searchLine->text().isEmpty() && event->type() == QEvent::FocusOut) {
            m_isSearchLineHiddenIn = true;
            m_searchLine->clear();
            m_searchLineAnimation->setStartValue(SEARCHLINE_WIDTH);
            m_searchLineAnimation->setEndValue(0);
            m_searchLineAnimation->start();
        }
    }

    return QWidget::eventFilter(obj, event);
}

void FileManageWin::mousePressEvent(QMouseEvent *event)
{
    hideSearchLine();
}

void FileManageWin::changeTabMode(FileManageWin::TabType type)
{
    qInfo() << "ChangeTabMode ------------ " << type;
    switch (type) {
        case FileManageWin::TabType::Default: {
            m_retreatBtn->show();
            m_lab1->show();
            m_forwardBtn->show();
            m_statusLab->hide();
            m_lab2->show();
            m_crumb->show();
            if (m_searchLine->isHidden()) {
                m_searchBtn->show();
            }
            m_lab3->show();
            m_editBtn->show();
            m_selectBtn->hide();
            m_lab4->show();
            m_verticalLine->show();
            m_lab5->show();
            m_finishBtn->hide();
            m_modeBtn->show();
            m_lab6->show();
            m_refreshBtn->show();
        } break;
        case FileManageWin::TabType::AndroidHomePage: {
            m_statusLab->setText(crumbText(CrumbTag::FileList));
            m_retreatBtn->hide();
            m_lab1->show();
            m_forwardBtn->hide();
            m_statusLab->show();
            m_lab2->hide();
            m_crumb->hide();
            if (m_searchLine->isHidden()) {
                m_searchBtn->show();
            }
            m_lab3->hide();
            m_editBtn->hide();
            m_selectBtn->hide();
            m_lab4->show();
            m_verticalLine->hide();
            m_lab5->hide();
            m_finishBtn->hide();
            m_modeBtn->hide();
            m_lab6->hide();
            m_refreshBtn->hide();
        } break;
        case FileManageWin::TabType::AppHomePage: {
            m_retreatBtn->show();
            m_lab1->show();
            m_forwardBtn->show();
            m_statusLab->hide();
            m_lab2->show();
            m_crumb->show();
            if (m_searchLine->isHidden()) {
                m_searchBtn->show();
            }
            m_lab3->hide();
            m_editBtn->hide();
            m_selectBtn->hide();
            m_lab4->show();
            m_verticalLine->hide();
            m_lab5->hide();
            m_finishBtn->hide();
            m_modeBtn->hide();
            m_lab6->hide();
            m_refreshBtn->hide();
        } break;
        case FileManageWin::TabType::SearchResultsPage: {
            m_retreatBtn->setEnabled(true);
            m_retreatBtn->show();
            m_lab1->hide();
            m_forwardBtn->hide();
            m_statusLab->setText(tr("Search File"));
            m_statusLab->show();
            m_lab2->hide();
            m_crumb->hide();
            if (m_searchLine->isHidden()) {
                m_searchBtn->show();
            }
            m_lab3->show();
            m_editBtn->show();
            m_selectBtn->hide();
            m_lab4->show();
            m_verticalLine->show();
            m_lab5->show();
            m_finishBtn->hide();
            m_modeBtn->show();
            m_lab6->show();
            m_refreshBtn->show();

            m_isSearch = true;
        } break;
        case FileManageWin::TabType::Load: {
        } break;
        case FileManageWin::TabType::Edit: {
            m_retreatBtn->hide();
            m_lab1->hide();
            m_forwardBtn->hide();
            m_statusLab->setText(tr("Select File"));
            m_statusLab->show();
            m_lab2->hide();
            m_crumb->hide();
            m_searchBtn->hide();
            m_lab3->show();
            m_editBtn->hide();
            m_selectBtn->show();
            m_lab4->hide();
            m_verticalLine->hide();
            m_lab5->show();
            m_finishBtn->show();
            m_modeBtn->hide();
            m_lab6->hide();
            m_refreshBtn->hide();
        } break;
    }
    update();
}

void FileManageWin::hideSearchLine()
{
    if (m_searchLine->text().isEmpty()) {
        setFocus();
        m_searchLine->setFocusPolicy(Qt::ClickFocus);
    }
}

void FileManageWin::generateDisplayTextList()
{
    m_textList.clear();
    m_textMap.clear();
    QFont font;
    font.setPointSizeF(m_fontSize);
    font.setBold(true);
    QFontMetrics fm(font);
    for (int i = 0; i < m_crumbList.count(); i++) {
        QString str = m_crumbList.at(i);
        QString text = fm.elidedText(str, Qt::ElideRight, 120);
        m_textList.append(text);
        m_textMap.insert(text, str);
    }
}

void FileManageWin::autoResetCrumb()
{
    if (m_textList.isEmpty()) {
        return;
    }
    resetCrumbMaxWidth();
    m_offset = 0;
    QFont font;
    font.setPointSizeF(m_fontSize);
    font.setBold(true);
    QFontMetrics fm(font);
    int currentWidth = 30;
    for (QString str : m_textList) {
        currentWidth += (fm.width(str) * 1.2 + CRUMB_SPACE);
    }
    QStringList list = m_textList;
    if (currentWidth < m_crumbMaxWidth) {
        m_isAutoReset = false;
    } else {
        m_isAutoReset = true;
        QString str = list.at(1);
        currentWidth = currentWidth - fm.width(str) * 1.2 + fm.width("...") * 1.2;
        list[1] = "...";
        for (int i = 2; i < m_textList.count(); i++) {
            if (currentWidth < m_crumbMaxWidth) {
                break;
            }
            str = m_textList.at(i);
            list.removeAt(2);
            m_offset++;
            currentWidth = currentWidth - fm.width(str) * 1.2 - CRUMB_SPACE;
        }
    }
    m_crumb->setFixedWidth(currentWidth);
    for (int i = m_crumb->count() - 1; i >= 0; i--) {
        m_crumb->removeTab(i);
    }
    int index = 0;
    for (QString text : list) {
        m_crumb->addTab(text);
        QString str = m_textMap.value(text);
        if (text != str) {
            m_crumb->setTabToolTip(index, str);
        }
        index++;
    }
    m_crumb->setCurrentIndex(m_crumb->count() - 1);
}

void FileManageWin::resetCrumbMaxWidth()
{
    m_crumbMaxWidth = width() - m_forwardBtn->width() - m_lab2->width();
    if (!m_retreatBtn->isHidden()) {
        m_crumbMaxWidth -= m_retreatBtn->width();
    }
    if (!m_lab1->isHidden()) {
        m_crumbMaxWidth -= m_lab1->width();
    }
    if (!m_lab3->isHidden()) {
        m_crumbMaxWidth -= m_lab3->width();
    }
    if (!m_editBtn->isHidden()) {
        m_crumbMaxWidth -= m_editBtn->width();
    }
    if (!m_searchBtn->isHidden()) {
        m_crumbMaxWidth -= m_searchBtn->width();
    }
    if (!m_searchLine->isHidden()) {
        m_crumbMaxWidth -= m_searchLine->width();
    }
    if (!m_lab4->isHidden()) {
        m_crumbMaxWidth -= m_lab4->width();
    }
    if (!m_verticalLine->isHidden()) {
        m_crumbMaxWidth -= m_verticalLine->width();
    }
    if (!m_lab5->isHidden()) {
        m_crumbMaxWidth -= m_lab5->width();
    }
    if (!m_modeBtn->isHidden()) {
        m_crumbMaxWidth -= m_modeBtn->width();
    }
    if (!m_lab6->isHidden()) {
        m_crumbMaxWidth -= m_lab6->width();
    }
    if (!m_refreshBtn->isHidden()) {
        m_crumbMaxWidth -= m_refreshBtn->width();
    }
    if (m_crumbMaxWidth < 640) {
        m_crumbMaxWidth = 640;
    }
}

void FileManageWin::slotCrumbIndexChange(int index)
{
    if (index == m_crumb->count() - 1 || (m_isAutoReset && index == 1)) {
        return;
    }
    hideSearchLine();
    load();
    QStringList list = m_crumbList;
    for (int i = m_crumb->count() - 1; i > index; i--) {
        m_crumb->removeTab(i);
    }
    if (index == 0) {
        for (int i = m_crumbList.count() - 1; i > index; i--) {
            m_crumbList.removeAt(i);
        }
    } else {
        for (int i = m_crumbList.count() - 1; i > index; i--) {
            m_crumbList.removeAt(i + m_offset);
        }
    }
    generateDisplayTextList();
    autoResetCrumb();
    Q_EMIT sigCrumbIndexChange(m_textMap.value(m_crumb->tabText(index)), list);
}

void FileManageWin::slotKdkBtnClicked()
{
    hideSearchLine();
    kdk::KToolButton *btn = qobject_cast<kdk::KToolButton *>(sender());
    if (btn == m_retreatBtn) {
        load();
        if (m_isSearch && m_type == FileManageWin::TabType::SearchResultsPage) {
            m_isSearch = false;
            m_searchLine->clear();
            m_isSearchLineHiddenIn = true;
            m_searchLineAnimation->setStartValue(SEARCHLINE_WIDTH);
            m_searchLineAnimation->setEndValue(0);
            m_searchLineAnimation->start();
        } else {
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::GoBacktBtn);
        }
    } else if (btn == m_forwardBtn) {
        load();
        Q_EMIT sigBtnCliked(FileManageWin::BtnType::GoForwardBtn);
    } else if (btn == m_searchBtn) {
        m_searchBtn->hide();
        m_searchLine->show();
        m_searchLineAnimation->setStartValue(0);
        m_searchLineAnimation->setEndValue(SEARCHLINE_WIDTH);
        m_searchLineAnimation->start();
    } else if (btn == m_editBtn) {
        Q_EMIT sigBtnCliked(FileManageWin::BtnType::EditBtn);
        changeTabMode(FileManageWin::TabType::Edit);
    } else if (btn == m_modeBtn) {
        if (m_isListMode) {
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::IconModeBtn);
            m_modeBtn->setIcon(QIcon::fromTheme("view-grid-symbolic"));
            m_modeBtn->setToolTip(tr("List Mode"));
            m_isListMode = false;
        } else {
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::ListModeBtn);
            m_modeBtn->setIcon(QIcon::fromTheme("view-list-symbolic"));
            m_modeBtn->setToolTip(tr("Icon Mode"));
            m_isListMode = true;
        }
    } else if (btn == m_refreshBtn) {
        if (m_type != FileManageWin::TabType::SearchResultsPage) {
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::RefreshBtn);
        }
    }
}

void FileManageWin::slotBtnClicked()
{
    hideSearchLine();
    ClickLabel *btn = qobject_cast<ClickLabel *>(sender());
    if (btn == m_selectBtn) {
        if (m_selectBtn->text() == tr("Select All")) {
            m_selectBtn->setText(tr("Deselect All"));
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::SelectBtn);
        } else if (m_selectBtn->text() == tr("Deselect All")) {
            m_selectBtn->setText(tr("Select All"));
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::CancelSelectBtn);
        }
    } else if (btn == m_finishBtn) {
        Q_EMIT sigBtnCliked(FileManageWin::BtnType::EditFinishBtn);
        m_selectBtn->hide();
        m_finishBtn->hide();
        if (m_selectBtn->text() == tr("Deselect All")) {
            m_selectBtn->setText(tr("Select All"));
            Q_EMIT sigBtnCliked(FileManageWin::BtnType::CancelSelectBtn);
        }
        changeTabMode(m_type);
    }
}

void FileManageWin::slotAnimationValueChanged(const QVariant &value)
{
    m_searchLine->setFixedWidth(value.toInt());
    if (m_isSearchLineHiddenIn && value.toInt() == 0) {
        m_searchLine->hide();
        changeTabMode(m_type);
        m_isSearchLineHiddenIn = false;
    } else if (!m_isSearchLineHiddenIn && value.toInt() == SEARCHLINE_WIDTH) {
        m_searchLine->setFocus();
    }
}

void FileManageWin::slotSearchTextChanged()
{
    if (m_isSearchLineHiddenIn) {
        return;
    }
    load();
    if (m_searchLine->text().isEmpty()) {
        m_isSearch = false;
        Q_EMIT sigBtnCliked(FileManageWin::BtnType::ReturnBtn);
        return;
    }
    Q_EMIT sigSearchTextChanged(m_searchLine->text());
}