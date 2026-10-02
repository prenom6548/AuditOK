/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kdragwidget.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QStyleOptionFrame>
#include "accessinfohelper.h"

namespace kdk
{

class FileDialog : public QFileDialog
{
    Q_OBJECT
public:
    FileDialog(QWidget *parent = nullptr);
    virtual ~FileDialog();

public slots:
    void goAccept();

protected:
    void showEvent(QShowEvent *event) override;
};

class KDragWidgetPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KDragWidget)
public:
    KDragWidgetPrivate(KDragWidget *parent);
    void openFile();

protected:
    void changeTheme();

private:
    KDragWidget *q_ptr;
    QColor lineColor;
    Qt::PenStyle mainState;
    KPushButton *m_pIconButton;
    QLabel *m_pTextLabel;
    FileDialog *m_FileDialog;
    QStringList m_NameFilterList;
    QStringList m_pStringList;
    bool m_cursorFlag;
};

KDragWidgetPrivate::KDragWidgetPrivate(KDragWidget *parent)
    : q_ptr(parent)
    , mainState(Qt::DashLine)
{
    Q_Q(KDragWidget);
    setParent(parent);
    m_pIconButton = new KPushButton();
    m_pTextLabel = new QLabel();
    m_FileDialog = nullptr;
    m_cursorFlag = false;
    lineColor = ThemeController::getCustomColorFromDT("kline-normal");

    KDK_ALL_INFO_FORMAT(m_pIconButton,"");
    KDK_ALL_INFO_FORMAT(m_pTextLabel,"");

}

void KDragWidgetPrivate::openFile()
{
    Q_Q(KDragWidget);
    if(!m_FileDialog)
    {
        m_FileDialog = new FileDialog(q);
        m_FileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        m_FileDialog->installEventFilter(this);
        if(!m_NameFilterList.isEmpty())
            m_FileDialog->setNameFilters(m_NameFilterList);
        KDK_ALL_INFO_FORMAT(m_FileDialog,"");
    }
    m_FileDialog->setWindowTitle(tr("Please select file"));
    m_pStringList.clear();

    if (m_FileDialog->exec() == QDialog::Accepted) {
        QStringList selectFiles = m_FileDialog->selectedFiles();
        if (selectFiles.count() > 1) {
            for (const QString &fileName : selectFiles) {
                QFileInfo fileinfo(fileName);

                if (m_FileDialog->nameFilters().count() == 1 && m_FileDialog->nameFilters().first().contains("(*)")) {
                    m_pStringList.append(fileName);
                } else {
                    if (fileinfo.isFile() && !fileName.isEmpty())
                        m_pStringList.append(fileName);
                }
            }
            if (m_pStringList.count() > 0)
                Q_EMIT q->getPathList(m_pStringList);
        } else {
            QString fileName = selectFiles.first();
            QFileInfo fileinfo(fileName);
            if (m_FileDialog->nameFilters().first().contains("(*)"))
                Q_EMIT q->getPath(fileName);
            else if (fileinfo.isFile() && !fileName.isEmpty())
                Q_EMIT q->getPath(fileName);
        }
    }
}

void KDragWidgetPrivate::changeTheme()
{
    Q_Q(KDragWidget);
    initThemeStyle();
}

KDragWidget::KDragWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KDragWidgetPrivate(this))
{
    Q_D(KDragWidget);
    setAcceptDrops(true); // 设置接受拖拽

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();
    QHBoxLayout *hLayout1 = new QHBoxLayout();
    //    d->m_pIconButton = new KPushButton();
    d->m_pIconButton->setFixedSize(54, 54);
    d->m_pIconButton->setTranslucent(true);
    d->m_pIconButton->setButtonType(KPushButton::CircleType);
    d->m_pIconButton->setIcon(QIcon::fromTheme("list-add.symbolic"));

    //    d->m_pTextLabel = new QLabel();
    d->m_pTextLabel->setText(tr("Select or drag and drop the folder identification path"));
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();
    hLayout->addWidget(d->m_pIconButton);
    hLayout->addStretch();

    hLayout1->setContentsMargins(0, 0, 0, 0);
    hLayout1->addStretch();
    hLayout1->addWidget(d->m_pTextLabel);
    hLayout1->addStretch();

    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(hLayout1);
    mainLayout->addStretch();

    d->changeTheme();

    connect(d->m_pIconButton, &KPushButton::clicked, this, [=]() {
        d->openFile();
    });
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });

    installEventFilter(this);

}

KPushButton *KDragWidget::iconButton()
{
    Q_D(KDragWidget);
    return d->m_pIconButton;
}

QLabel *KDragWidget::textLabel()
{
    Q_D(KDragWidget);
    return d->m_pTextLabel;
}

QFileDialog *KDragWidget::fileDialog()
{
    Q_D(KDragWidget);
    if(!d->m_FileDialog)
    {
        d->m_FileDialog = new FileDialog(this);
        d->m_FileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        d->m_FileDialog->installEventFilter(this);
        if(!d->m_NameFilterList.isEmpty())
            d->m_FileDialog->setNameFilters(d->m_NameFilterList);
        KDK_ALL_INFO_FORMAT(d->m_FileDialog,"");
    }
    return dynamic_cast<QFileDialog *>(d->m_FileDialog);
}

void KDragWidget::setNameFilter(const QString &filter)
{
    Q_D(KDragWidget);
    QString f(filter);
    if (f.isEmpty())
        return;
    QString sep(QLatin1String(";;"));
    int i = f.indexOf(sep, 0);
    if (i == -1) {
        if (f.indexOf(QLatin1Char('\n'), 0) != -1) {
            sep = QLatin1Char('\n');
            i = f.indexOf(sep, 0);
        }
    }
    setNameFilters(f.split(sep));
}

void KDragWidget::setNameFilters(const QStringList &filters)
{
    Q_D(KDragWidget);
    if(!d->m_FileDialog)
    {
        d->m_FileDialog = new FileDialog(this);
        d->m_FileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        d->m_FileDialog->installEventFilter(this);
        KDK_ALL_INFO_FORMAT(d->m_FileDialog,"");
    }
    d->m_FileDialog->setNameFilters(filters);

    for (int i = 0; i < filters.size(); i++) {
        if (!d->m_NameFilterList.contains(filters.at(i))) {
            d->m_NameFilterList.append(filters.at(i));
        }
    }
}

void KDragWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(KDragWidget);
    return QWidget::mouseMoveEvent(event);
}

void KDragWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(KDragWidget);
    return QWidget::mousePressEvent(event);
}

void KDragWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KDragWidget);
    return QWidget::mouseReleaseEvent(event);
}

void KDragWidget::keyPressEvent(QKeyEvent *event)
{
    return QWidget::keyPressEvent(event);
}

void KDragWidget::keyReleaseEvent(QKeyEvent *event)
{
    return QWidget::keyReleaseEvent(event);
}

void KDragWidget::paintEvent(QPaintEvent *event)
{
    Q_D(KDragWidget);
    QPainter painter(this);

    QPen pen = painter.pen();
    pen.setColor(d->lineColor);
    pen.setStyle(d->mainState);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), radius, radius);
}

bool KDragWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KDragWidget);
    if (watched == this) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
            d->lineColor = ThemeController::getPaletteColorFromDT("tooltiptext-active");
            d->openFile();

            update();
            break;
        }
        case QEvent::MouseButtonRelease: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
            d->lineColor = ThemeController::getPaletteColorFromDT("tooltiptext-active");
            update();
            break;
        }
        case QEvent::Leave: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, false);
            d->lineColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
            if (d->m_cursorFlag) {
                d->m_cursorFlag = false;
                setCursor(Qt::ArrowCursor);
            }
            update();
            break;
        }
        case QEvent::Enter: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
            d->lineColor = ThemeController::getPaletteColorFromDT("tooltiptext-active");
            if (d->m_cursorFlag) {
                d->m_cursorFlag = false;
                setCursor(Qt::ArrowCursor);
            }
            update();
            break;
        }
        case QEvent::DragEnter: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, true);
            d->lineColor = ThemeController::getPaletteColorFromDT("highlight-active");
            update();
            break;
        }
        case QEvent::DragLeave: {
            d->m_pIconButton->setAttribute(Qt::WA_UnderMouse, false);
            d->lineColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
            update();
            break;
        }
        default:
            break;
        }
    }

    if (watched == d->m_FileDialog && d->m_FileDialog != nullptr) {

        if (event->type() == QEvent::Show) {
            QWidget *parentWindow = qobject_cast<QWidget *>(this->parent());

            while (parentWindow && parentWindow->parent())
                parentWindow = qobject_cast<QWidget *>(parentWindow->parent());

            if (parentWindow && parentWindow->isActiveWindow()) {
                const QPoint parentWindowCenter = parentWindow->geometry().center();
                d->m_FileDialog->move(parentWindowCenter.x() - d->m_FileDialog->width() / 2, parentWindowCenter.y() - d->m_FileDialog->height() / 2 + 40);
            } else {
                QDesktopWidget desktop;
                int screenWidth = desktop.screen()->width();
                int screenHeight = desktop.screen()->height();

                int messageBox_X = (screenWidth - this->width()) / 2;
                int messageBox_Y = (screenHeight - this->height()) / 2;
                d->m_FileDialog->move(messageBox_X, messageBox_Y);
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void KDragWidget::dragEnterEvent(QDragEnterEvent *event)
{
    Q_D(KDragWidget);

    if (event->mimeData()->hasUrls() && d->m_FileDialog != nullptr) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.count(); i++) {
            QString filePath = urls.at(i).toLocalFile();
            if (!filePath.isEmpty()) {
                QFileInfo fileInfo(filePath); // filePath 路径
                QString suffix = fileInfo.suffix(); // suffix 后缀
                QStringList nameFilters = d->m_FileDialog->nameFilters();
                for (QString str : nameFilters) {
                    if (!str.contains("(*)")) {
                        if (!fileInfo.isDir()) {
                            if (!suffix.isEmpty() && str.contains(suffix, Qt::CaseInsensitive)) {
                                d->m_cursorFlag = false;
                                break;
                            } else
                                d->m_cursorFlag = true;
                        } else {
                            d->m_cursorFlag = true;
                        }
                    }
                }
            }
        }

        if (d->m_cursorFlag) {
            setCursor(Qt::ForbiddenCursor);
            event->setDropAction(Qt::IgnoreAction);
        } else {
            setCursor(Qt::ArrowCursor);
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void KDragWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_D(KDragWidget);
    return QWidget::dragLeaveEvent(event);
}

void KDragWidget::dragMoveEvent(QDragMoveEvent *event)
{
    Q_D(KDragWidget);
    return QWidget::dragMoveEvent(event);
}

void KDragWidget::dropEvent(QDropEvent *event)
{
    Q_D(KDragWidget);

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls() && d->m_FileDialog != nullptr) {
        QList<QUrl> urlList = mimeData->urls();
        if (urlList.size() > 1) {
            d->m_pStringList.clear();
            for (int i = 0; i < urlList.size(); i++) // 选中的文件
            {
                QString fileName = urlList.at(i).toLocalFile();

                QFileInfo fileinfo(fileName);
                if (d->m_FileDialog->nameFilters().count() == 1 && d->m_FileDialog->nameFilters().first().contains("(*)")) {
                    d->m_pStringList.append(fileName);
                } else {
                    if (fileinfo.isFile()) {
                        QString endStr = fileName.mid(fileName.lastIndexOf("."), fileName.size() - fileName.lastIndexOf("."));
                        if (d->m_NameFilterList.isEmpty()) // 若无过滤则正常选中
                            d->m_pStringList.append(fileName);
                        else {
                            for (int temp = 0; temp < d->m_NameFilterList.size(); temp++) // 遍历是否符合筛选条件
                            {
                                QString curString = d->m_NameFilterList.at(temp);
                                if (curString.contains(endStr)) {
                                    d->m_pStringList.append(fileName);
                                }
                            }
                        }
                    }
                }
            }
            Q_EMIT getPathList(d->m_pStringList);
        } else {
            QString fileName = urlList.first().toLocalFile();
            QFileInfo fileinfo(fileName);
            if (d->m_FileDialog->nameFilters().count() == 1 && d->m_FileDialog->nameFilters().first().contains("(*)")) {
                Q_EMIT getPath(fileName);
            } else {
                if (fileinfo.isFile()) {
                    QString endStr = fileName.mid(fileName.lastIndexOf("."), fileName.size() - fileName.lastIndexOf("."));
                    if (d->m_NameFilterList.isEmpty()) // 若无过滤则正常选中
                        Q_EMIT getPath(fileName);
                    else {
                        for (int temp = 0; temp < d->m_NameFilterList.size(); temp++) // 遍历是否符合筛选条件
                        {
                            QString curString = d->m_NameFilterList.at(temp);
                            if (curString.contains(endStr)) {
                                Q_EMIT getPath(fileName);
                            }
                        }
                    }
                }
            }
        }
    }
    if (d->m_cursorFlag) {
        d->m_cursorFlag = false;
        setCursor(Qt::ArrowCursor);
        event->ignore();
    }
}

FileDialog::FileDialog(QWidget *parent)
    : QFileDialog(parent)
{
    this->setFileMode(QFileDialog::ExistingFiles);
}

FileDialog::~FileDialog()
{
}

void FileDialog::goAccept()
{
    accept();
}

void FileDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    if (this->isVisible()) {
        for (QWidget *widget : this->findChildren<QWidget *>()) {
            if (widget->objectName() == "acceptButton") {
                QPushButton *btn = qobject_cast<QPushButton *>(widget);
                disconnect(btn, &QPushButton::clicked, 0, 0);
                connect(btn, &QPushButton::clicked, [=](bool checked) {
                    Q_UNUSED(checked)
                    this->goAccept();
                });
            }
        }
    }
}

}

#include "kdragwidget.moc"
#include "moc_kdragwidget.cpp"
