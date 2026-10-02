/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cleanerwidget.h"
#include "utils.h"
#include "global.h"
#include "messagebox.h"

#include <QDebug>
#include <QScrollArea>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDBus>
#include <QStackedLayout>
#include <QListWidget>

#include <unistd.h>
#include <sys/syslog.h>

using namespace Global;
CleanerWidget::CleanerWidget(QWidget *parent) :
    /*SettingScrollContent*/QWidget(parent)
    , m_dbusInterface(new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus(), this))
{
    qRegisterMetaType<ContainerMeta>();
    qRegisterMetaType<Dict>();
    qDBusRegisterMetaType<Dict>();

//    m_title = new QLabel(this);
//    m_title->setFixedHeight(36);
//    m_title->setAlignment(Qt::AlignCenter);
//    m_title->setWordWrap(true);
//    m_title->setText(tr("Image cleaning"));
    m_tipFrame = new QFrame;
    m_mainFrame = new QFrame;
    m_stackedLayout = new QStackedLayout;
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->setMargin(0);

    m_clearButton = new QPushButton;
    m_clearButton->setFixedSize(120,36);
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(tr("Delete all idle images"), Qt::ElideRight, 90);
    m_clearButton->setText(elided_text);
    if (fm.width(tr("Delete all idle images")) > 110) {
        m_clearButton->setToolTip(tr("Delete all idle images"));
    }
    m_clearButton->setVisible(false);
    connect(m_clearButton, &QPushButton::clicked, this, &CleanerWidget::onRemoveAllImages);

    m_list = new QListWidget;
    m_list->setAlternatingRowColors(true);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);

    QListWidgetItem *item = new QListWidgetItem(m_list,0);
    item->setSizeHint(QSize(490,40));
    QWidget *titlebar = new QWidget(m_list);
    QHBoxLayout *titlebarlayout = new QHBoxLayout(titlebar);
    QLabel *Name = new QLabel;
    QLabel *Size = new QLabel;
    QLabel *Date = new QLabel;
    QPushButton *separator1 = new QPushButton;
    QPushButton *separator2 = new QPushButton;
    Name->setText(tr("Name"));
    Name->setFixedWidth(210);
    Size->setText(tr("Size"));
    Size->setFixedWidth(50);
    Date->setText(tr("Date"));
    Date->setFixedWidth(100);
    separator1->setFixedSize(1,14);
    separator1->setEnabled(false);
    separator2->setFixedSize(1,14);
    separator2->setEnabled(false);
    titlebarlayout->addWidget(Name);
    titlebarlayout->addWidget(separator1);
    titlebarlayout->addWidget(Size);
    titlebarlayout->addWidget(separator2);
    titlebarlayout->addWidget(Date);
    titlebarlayout->addStretch();
    titlebar->setLayout(titlebarlayout);
    m_list->setItemWidget(item,titlebar);

    scrollarea = new QScrollArea;
    scrollarea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    scrollarea->setWidget(m_list);
    scrollarea->setWidgetResizable(true);
    scrollarea->setFocusPolicy(Qt::NoFocus);
    scrollarea->setFrameStyle(QFrame::NoFrame);
    scrollarea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollarea->setContentsMargins(0, 0, 0, 0);
    scrollarea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    scrollarea->setAttribute(Qt::WA_TranslucentBackground, true);

    m_null = new QLabel();
    m_null->setFixedSize(201,180);
    QImage *img = new QImage;
    img->load(":/res/Illustration .png");
    m_null->setPixmap(QPixmap::fromImage(*img));
    m_null->setScaledContents(true);
//    m_null->setVisible(false);

    title = new QLabel();
    title->setText(tr("Image cleaning"));
    title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    QLabel *m_title = new QLabel;
    m_title->setText(tr("Image cleaning"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    tiplabel = new QLabel;
    tiplabel->setText(tr("No idle image"));
    QHBoxLayout *titlelayout = new QHBoxLayout;
    titlelayout->addWidget(title);
    titlelayout->addStretch();
    titlelayout->addWidget(m_clearButton, 0, Qt::AlignLeft);

    QVBoxLayout *tiplayout = new QVBoxLayout(m_tipFrame);
    tiplayout->setContentsMargins(30,2,39,40);
    tiplayout->addWidget(m_title);
    tiplayout->addSpacing(76);
    tiplayout->addWidget(m_null, 0, Qt::AlignHCenter);
    tiplayout->addWidget(tiplabel, 0, Qt::AlignHCenter);
    tiplayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->setContentsMargins(30,2,39,40);
    mainLayout->addLayout(titlelayout);
    //mainLayout->setMargin(0);
    //mainLayout->setSpacing(10);
    //mainLayout->addWidget(m_title, 0, Qt::AlignHCenter);
//    mainLayout->addWidget(m_null, 0, Qt::AlignCenter);
//    mainLayout->addWidget(tiplabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(scrollarea);
//    mainLayout->addWidget(m_clearButton, 0, Qt::AlignHCenter);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
//    this->setLayout(mainLayout);

    m_stackedLayout->addWidget(m_tipFrame);
    m_stackedLayout->addWidget(m_mainFrame);
    m_stackedLayout->setCurrentWidget(m_tipFrame);
    this->setLayout(m_stackedLayout);


//    QDBusReply<Dict> reply = m_dbusInterface->call("GetAllContainersAndImages", "kylin", 1000);
//    if (reply.isValid()) {
//        Dict maps = reply.value();
//        for (auto it(maps.begin()); it != maps.end(); ++it) {
//            syslog(LOG_DEBUG, "item: key:%s, value:%s", it.key().toStdString().c_str(), it.value().toStdString().c_str());
//        }
//    }
    QDBusPendingReply<Dict> reply = m_dbusInterface->asyncCall("GetAllContainersAndImages", kmre::utils::getUserName(), (int32_t)getuid());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &CleanerWidget::onGetAllImagesInfoFinished);

    connect(m_dbusInterface, SIGNAL(ImageRemoved(QString,bool)), this, SLOT(onImageRemoved(QString,bool)));
}

CleanerWidget::~CleanerWidget()
{

}

void CleanerWidget::onImageRemoved(const QString &imageName, bool success)
{
    if (success) {
        for (auto it(m_itemList.begin()); it != m_itemList.end(); ++it) {
            if (imageName == it.value()["name"].toString()) {
//                m_itemsLayout->removeWidget(it.key());
//                int row = m_list->row(remove_item);
                m_list->takeItem(m_list->row(remove_item));
                m_itemList.remove(it.key());
                it.key()->freeItem();
                break;
            }
        }
    }
}

void CleanerWidget::onResponseAddImage(const QJsonObject &value, const QString &currentImage, const QString &container)
{
    m_clearButton->setVisible(true);
    m_item = new CleanerItem(value, currentImage, container, this);
    m_list->addItem(m_item->getItem());
    m_item->getItem()->setSizeHint(QSize(490,40));
    m_list->setItemWidget(m_item->getItem(),m_item);
    m_item->setContentsMargins(0, 0, 0, 0);
    m_itemList.insert(m_item, value);
    connect(m_item, &CleanerItem::requestRemove, this, &CleanerWidget::onResponseRemoveImage);
    connect(m_item,&CleanerItem::itemRemove,[=](QListWidgetItem *item) {
        remove_item = item;
    });
    update();
}

void CleanerWidget::onResponseRemoveImage(const QString &id, const QString &containerName, const QString &imageName)
{
    //TODO: 根据id删镜像
    Q_UNUSED(id);
    m_dbusInterface->asyncCall("RemoveOneImage", containerName, imageName);

    /*
    for (auto it(m_itemList.begin()); it != m_itemList.end(); ++it) {
        if (imageName == it.value()["name"].toString()) {
            m_itemsLayout->removeWidget(it.key());
            m_itemList.remove(it.key());
//            QDBusReply<bool> reply = m_dbusInterface->call("RemoveOneImage", containerName, imageName);
//            if (reply.isValid()) {
//            }
            m_dbusInterface->asyncCall("RemoveOneImage", containerName, imageName);
            break;
        }
    }
    update();
    */
}

/*
{'containers': '[{"name":"kmre-1000-lixiang","repo":"kmre2","tag":"v2.0-210901.10"}]\n',
 'current': '{"name":"kmre2:v2.0-210901.10","repo":"kmre2","tag":"v2.0-210901.10"}\n',
 'images': '[{"created":"2021-09-01 17:23:43 +0800 '
           'CST","id":"6f0d45bf52b3","name":"kmre2:v2.0-210901.10","repo":"kmre2","size":"1.78GB","tag":"v2.0-210901.10"},{"created":"2021-08-30 '
           '17:25:55 +0800 '
           'CST","id":"9d3beeecbf5e","name":"kmre2:v2.0-210830.10","repo":"kmre2","size":"1.78GB","tag":"v2.0-210830.10"},{"created":"2021-08-24 '
           '13:49:19 +0800 '
           'CST","id":"ae7fcc36ee0f","name":"kmre2:v2.0-210824.10","repo":"kmre2","size":"1.78GB","tag":"v2.0-210824.10"},{"created":"2021-08-19 '
           '11:33:39 +0800 '
           'CST","id":"811a3392f3b5","name":"kmre2:v2.0-210819.10","repo":"kmre2","size":"1.78GB","tag":"v2.0-210819.10"},{"created":"2021-08-17 '
           '18:31:17 +0800 '
           'CST","id":"f33184f26f4d","name":"kmre2:v2.0-210817.10","repo":"kmre2","size":"1.78GB","tag":"v2.0-210817.10"},{"created":"2021-07-20 '
           '17:02:56 +0800 '
           'CST","id":"731adc7f5166","name":"kmre2:v2.0-210720.10","repo":"kmre2","size":"1.76GB","tag":"v2.0-210720.10"},{"created":"2021-07-10 '
           '13:55:23 +0800 '
           'CST","id":"6e1bab7e4520","name":"kmre2:v2.0-210710.10","repo":"kmre2","size":"1.76GB","tag":"v2.0-210710.10"},{"created":"2021-07-09 '
           '15:03:57 +0800 '
           'CST","id":"d722ad28a453","name":"kmre2:v2.0-210709.10","repo":"kmre2","size":"1.76GB","tag":"v2.0-210709.10"},{"created":"2021-07-03 '
           '20:17:18 +0800 '
           'CST","id":"0251e9738e71","name":"kmre2:v2.0-210703.10","repo":"kmre2","size":"1.72GB","tag":"v2.0-210703.10"},{"created":"2021-06-08 '
           '20:58:26 +0800 '
           'CST","id":"de57beea7e7e","name":"kmre2:v2.0-210608.11","repo":"kmre2","size":"1.44GB","tag":"v2.0-210608.11"}]\n'}
*/
void CleanerWidget::onGetAllImagesInfoFinished(QDBusPendingCallWatcher *w)
{
    QString m_currentImage;//"kmre2:v2.0-210121.10"
    QString containersInfo;
    QString imagesInfo;

    QDBusPendingReply<Dict> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
    //if (reply.isError()) {
    //    qDebug() << "Error: " << reply.error().name() << ", Happened: " << reply.error().message();
    //}
//    if (w->isError()) {
//        qDebug() << "Error: " << w->error().name() << ", Happened: " << w->error().message();
//    }
    if (reply.isValid()) {
        Dict maps = reply.value();
        for (auto it(maps.begin()); it != maps.end(); ++it) {
            if (it.key() == "current") {
                const QJsonDocument &jsonDocument = QJsonDocument::fromJson(it.value().toLocal8Bit().data());
                if (!jsonDocument.isNull()) {
                    QJsonObject jsonObject = jsonDocument.object();
                    if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                        if (jsonObject.contains("name")) {
                            m_currentImage = jsonObject.value("name").toString();
                        }
                        if (jsonObject.contains("repo")) {

                        }
                        if (jsonObject.contains("tag")) {

                        }
                    }
                }
            }
            else if (it.key() == "containers") {
                containersInfo = it.value();
            }
            else if (it.key() == "images") {
                imagesInfo = it.value();
            }
        }
    }
    w->deleteLater();

    QList<ContainerMeta> containerList;
    if (!containersInfo.isEmpty()) {
        const QJsonDocument &jsonDocument = QJsonDocument::fromJson(containersInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            const QJsonArray &jsonArray = jsonDocument.array();
            for (const QJsonValue &value : jsonArray) {
                const QJsonObject &obj = value.toObject();
                if (obj.isEmpty()) {
                    continue;
                }

                if (obj.contains("name") && obj.contains("repo") && obj.contains("tag")) {
                    containerList << ContainerMeta{ obj["name"].toString(), obj["repo"].toString(), obj["tag"].toString()};
                }
            }
        }
    }

    if (!imagesInfo.isEmpty()) {
        const QJsonDocument &jsonDocument = QJsonDocument::fromJson(imagesInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            const QJsonArray &jsonArray = jsonDocument.array();
            for (const QJsonValue &value : jsonArray) {
                QString containerName = QString("");
                const QJsonObject &obj = value.toObject();
                if (obj.isEmpty()) {
                    continue;
                }
                for (const ContainerMeta &container : containerList) {
                    QString imageName = QString("%1:%2").arg(container.m_repo).arg(container.m_tag);
                    if (obj.contains("name") && imageName == obj["name"].toString()) {
                        containerName = container.m_name;
                        break;
                    }
                }
                if (obj.contains("name")) {
                    if (m_currentImage != obj["name"].toString()) {
                        this->onResponseAddImage(obj, m_currentImage, containerName);
                    }
                }
            }
        }
    }
}

void CleanerWidget::onRemoveAllImages()
{
    if (KylinUI::MessageBox::question(this, tr("KMRE"), tr("After all images are deleted, KMRE will no longer be able to switch to the image. Are you sure you want to perform this operation?"))) {
        for (CleanerItem *item : m_itemList.keys()) {
            item->onRemove();
        }
    }
}

void CleanerWidget::paintEvent(QPaintEvent *event)
{
//    QPainter painter(this);
//    painter.fillRect(rect(), QColor(255, 255, 255, 255 * 0.03));
    if (!m_itemList.isEmpty()) {
        m_stackedLayout->setCurrentWidget(m_mainFrame);
//        m_null->setVisible(true);
//        tiplabel->setVisible(true);
//        QString s(tr("No idle image"));
//        painter.drawText(rect(),Qt::AlignCenter||Qt::AlignTop,s);
//        m_clearButton->setVisible(false);
    }

    QWidget::paintEvent(event);
}
