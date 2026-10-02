// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QBuffer>
#include <QGuiApplication>
#include <QHash>
#include <QLocale>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>

#include "MainWindow.h"

#define GSETTINGS_THEME_NAME "org.ukui.style"
#define GSETTINGS_THEME_KEY "styleName"

MainWindow::MainWindow(QWindow *parent)
    : QQuickView(parent), m_gsettings(new GSettingsManager(this)),
      m_dynamicModel(new DynamicWidgetModel(this))
{
    rootContext()->setContextProperty("dynamicModel", m_dynamicModel);
    rootContext()->setContextProperty("gsettingsManager", m_gsettings);
    rootContext()->setContextProperty("controller", this);
}

MainWindow::~MainWindow()
{
    if (engine()) {
        engine()->trimComponentCache();
        engine()->collectGarbage();
    }
}

void MainWindow::setFinalSize(qreal width, qreal height)
{
    updateWindowSize(width, height);
    setMinimumSize(QSize(width, height).boundedTo(m_maxSize));
    show();
    m_client = new WaylandClient(clientType, this);
    setupParentWindow();
    setDialogModal();
}

void MainWindow::updateWindowSize(qreal width, qreal height)
{
    QScreen *screen = this->screen();
    if (screen) {
        m_maxSize.setHeight(screen->availableGeometry().height() - 38);
        m_maxSize.setWidth(screen->availableGeometry().width());
    }

    int finalWidth = qMax(width, 400.0);
    int finalHeight = qMax(height, 200.0);
    QSize newSize(qMin(finalWidth, m_maxSize.width()), qMin(finalHeight, m_maxSize.height()));
    if (size() != newSize) {
        resize(newSize);
        qDebug() << QString("Resize to final correct size: %1 x %2")
                        .arg(newSize.width())
                        .arg(newSize.height());
    }
}

void MainWindow::deny()
{
    qDebug() << "deny is clicked";
    emit rejectedByDeny();
}

void MainWindow::allow()
{
    qDebug() << "allow is clicked";
    emit rejectedByAllow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "Window is closing (closeEvent)...";
    emit rejectedByClose();
    event->ignore();
}

bool MainWindow::event(QEvent *ev)
{
    if (ev->type() == QEvent::Close) {
        closeEvent(static_cast<QCloseEvent *>(ev));
    }

    return QQuickView::event(ev);
}

QColor MainWindow::windowColor() const
{
    return m_windowColor;
}

void MainWindow::setWindowColor(const QColor &windowColor)
{
    if (m_windowColor != windowColor) {
        m_windowColor = windowColor;
        emit windowColorChanged();
    }
}

QString MainWindow::iconSource() const
{
    return m_iconSource;
}

void MainWindow::setIconSource(const QString &icon)
{
    if (m_iconSource != icon) {
        m_iconSource = icon;
        emit iconSourceChanged();
    }
}

QString MainWindow::mainTitle() const
{
    return m_title;
}

void MainWindow::setMainTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit mainTitleChanged();
    }
}

bool MainWindow::titleFontBold() const
{
    return m_titleFontBold;
}

void MainWindow::setTitleFontBold(bool bold)
{
    if (m_titleFontBold != bold) {
        m_titleFontBold = bold;
        emit titleFontBoldChanged();
    }
}

QString MainWindow::subTitle() const
{
    return m_subTitle;
}

void MainWindow::setSubTitle(const QString &subTitle)
{
    if (m_subTitle != subTitle) {
        m_subTitle = subTitle;
        emit subTitleChanged();
    }
}

bool MainWindow::subTitleFontLight() const
{
    return m_subTitleFontLight;
}

void MainWindow::setSubTitleFontLight(bool light)
{
    if (m_subTitleFontLight != light) {
        m_subTitleFontLight = light;
        emit subTitleFontLightChanged();
    }
}

QString MainWindow::denyText() const
{
    return m_denyText;
}

void MainWindow::setDenyText(const QString &denyText)
{
    if (m_denyText != denyText) {
        m_denyText = denyText;
        emit denyTextChanged();
    }
}

QString MainWindow::allowText() const
{
    return m_allowText;
}

void MainWindow::setAllowText(const QString &allowText)
{
    if (m_allowText != allowText) {
        m_allowText = allowText;
        emit allowTextChanged();
    }
}

bool MainWindow::buttonsVisible() const
{
    return m_buttonsVisible;
}

void MainWindow::setButtonsVisible(bool visible)
{
    if (m_buttonsVisible != visible) {
        m_buttonsVisible = visible;
        emit buttonsVisibleChanged();
    }
}

bool MainWindow::denyButtonVisible() const
{
    return m_denyButtonVisible;
}

void MainWindow::setDenyButtonVisible(bool visible)
{
    if (m_denyButtonVisible != visible) {
        m_denyButtonVisible = visible;
        emit denyButtonVisibleChanged();
    }
}

bool MainWindow::allowButtonVisible() const
{
    return m_allowButtonVisible;
}

void MainWindow::setAllowButtonVisible(bool visible)
{
    if (m_allowButtonVisible != visible) {
        m_allowButtonVisible = visible;
        emit allowButtonVisibleChanged();
    }
}

QString MainWindow::messageText() const
{
    return m_messageText;
}

void MainWindow::setMessageText(const QString &messageText)
{
    if (m_messageText != messageText) {
        m_messageText = messageText;
        emit messageTextChanged();
    }
}

QString MainWindow::detailsText() const
{
    return m_detailsText;
}

void MainWindow::setDetailsText(const QString &detailsText)
{
    if (m_detailsText != detailsText) {
        m_detailsText = detailsText;
        emit detailsTextChanged();
    }
}

void MainWindow::initFromJson(const QJsonObject &req)
{
    parseJson(req);
    setSource(QUrl("qrc:/MainWindow.qml"));
}

void MainWindow::getCheckBoxes(QJsonObject &req, const QString &parent,
                               const QMap<QString, bool> checkboxes)
{
    QJsonArray checkBoxStatesArray;
    for (auto it = checkboxes.constBegin(); it != checkboxes.constEnd(); ++it) {
        if (it.key().startsWith(parent)) {
            QJsonObject item;
            item["name"] = it.key().mid(parent.length());
            item["checked"] = it.value();
            checkBoxStatesArray.append(item);
            qDebug() << QString("CheckBox: %1 status: %2").arg(it.key()).arg(it.value());
        }
    }

    if (checkBoxStatesArray.isEmpty()) {
        qDebug() << "[result] checkBoxStatesArray is empty";
        return;
    }

    req["check_boxes"] = checkBoxStatesArray;
}

void MainWindow::results(QJsonObject &results)
{
    QMap<QString, bool> checkboxes = m_dynamicModel->getCheckBoxStates();
    if (!checkboxes.isEmpty()) {
        getCheckBoxes(results, "", checkboxes);
    }
}

void MainWindow::parseJson(const QJsonObject &req)
{
    if (req.contains("parent_window")) {
        QString parentIdStr = req["parent_window"].toString();
        if (!parentIdStr.startsWith(QLatin1String("wayland:"))) {
            qWarning() << "JSON: The format of the parent_window field is incorrect";
        } else {
            m_parentWindowHandle = parentIdStr.mid(strlen("wayland:"));
            clientType |= WaylandClient::Protocol::XDG_FOREIGN;
        }
    }

    if (req.contains("modal")) {
        m_isModal = req["modal"].toBool();
        clientType |= WaylandClient::Protocol::XDG_DIALOG;
    }

    if (req.contains("title_icon")) {
        QString name = req["title_icon"].toString();
        QIcon icon = QIcon::fromTheme(name);
        if (!icon.isNull()) {
            QPixmap pixmap = icon.pixmap(48, 48);
            if (!pixmap.isNull()) {
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                pixmap.save(&buffer, "PNG");

                QString dataUrl =
                    QStringLiteral("data:image/png;base64,%1").arg(QString(byteArray.toBase64()));
                setIconSource(dataUrl);
            }
        }
    }

    if (req.contains("title")) {
        QJsonValue val = req["title"];
        if (val.isObject()) {
            setMainTitle(val.toObject()["text"].toString());
            setTitleFontBold(val.toObject()["font_bold"].toBool());
        }
    }

    if (req.contains("sub_title")) {
        QJsonValue val = req["sub_title"];
        if (val.isObject()) {
            setSubTitle(val.toObject()["text"].toString());
            setSubTitleFontLight(val.toObject()["font_light"].toBool(true));
        }
    }

    if (req.contains("message")) {
        setMessageText(req["message"].toString());
    }

    if (req.contains("details")) {
        setDetailsText(req["details"].toString());
    }

    if (req.contains("button_mode")) {
        setButtonsVisible(true);
        if (req["button_mode"].toString() == "deny_only") {
            setAllowButtonVisible(false);
        } else if (req["button_mode"].toString() == "allow_only") {
            setDenyButtonVisible(false);
        }

        const QHash<QString, QString> denyHash = { { "zh_CN", "拒绝" }, { "zh_TW", "拒絕" } };
        const QHash<QString, QString> allowHash = { { "zh_CN", "允许" }, { "zh_TW", "允許" } };
        QString localeName = QLocale::system().name();
        QString denyText = denyHash.value(localeName);
        QString allowText = allowHash.value(localeName);

        QString denyTextFromJson = req["deny_text"].toString();
        QString allowTextFromJson = req["allow_text"].toString();
        QString finalDenyText = denyTextFromJson.isEmpty()
                                    ? (denyText.isEmpty() ? "Deny" : denyText)
                                    : denyTextFromJson;
        QString finalAllowText = allowTextFromJson.isEmpty()
                                     ? (allowText.isEmpty() ? "Allow" : allowText)
                                     : allowTextFromJson;
        setDenyText(finalDenyText);
        setAllowText(finalAllowText);
    }

    m_dynamicModel->loadFromJson(req);
}

void MainWindow::setupParentWindow()
{
    if (m_parentWindowHandle.isEmpty()) {
        return;
    }

    m_xdgForeign = new XdgForeign(m_client, this);
    connect(m_xdgForeign, &XdgForeign::parentWindowLost, m_xdgForeign, &QObject::deleteLater);

    if (!m_xdgForeign->setParentWindow(this, m_parentWindowHandle)) {
        qWarning() << "setParentWindow failed! Running without parent linkage";
        m_xdgForeign->deleteLater();
        m_xdgForeign = nullptr;
    }
}

void MainWindow::setDialogModal()
{
    if (!m_isModal) {
        return;
    }

    m_xdgDialog = new XdgDialog(m_client, this);
    m_xdgDialog->setDialogModal(this, m_isModal);
}
