// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonObject>
#include <QQuickView>

#include "DynamicModel.h"
#include "Gsettings.h"
#include "XdgDialog.h"
#include "XdgForeign.h"

class MainWindow : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QColor windowColor READ windowColor WRITE setWindowColor NOTIFY windowColorChanged)
    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(QString mainTitle READ mainTitle WRITE setMainTitle NOTIFY mainTitleChanged)
    Q_PROPERTY(
        bool titleFontBold READ titleFontBold WRITE setTitleFontBold NOTIFY titleFontBoldChanged)
    Q_PROPERTY(QString subTitle READ subTitle WRITE setSubTitle NOTIFY subTitleChanged)
    Q_PROPERTY(bool subTitleFontLight READ subTitleFontLight WRITE setSubTitleFontLight NOTIFY
                   subTitleFontLightChanged)
    Q_PROPERTY(bool buttonsVisible READ buttonsVisible WRITE setButtonsVisible NOTIFY
                   buttonsVisibleChanged)
    Q_PROPERTY(bool denyButtonVisible READ denyButtonVisible WRITE setDenyButtonVisible NOTIFY
                   denyButtonVisibleChanged)
    Q_PROPERTY(bool allowButtonVisible READ allowButtonVisible WRITE setAllowButtonVisible NOTIFY
                   allowButtonVisibleChanged)
    Q_PROPERTY(QString denyText READ denyText WRITE setDenyText NOTIFY denyTextChanged)
    Q_PROPERTY(QString allowText READ allowText WRITE setAllowText NOTIFY allowTextChanged)
    Q_PROPERTY(QString messageText READ messageText WRITE setMessageText NOTIFY messageTextChanged)
    Q_PROPERTY(QString detailsText READ detailsText WRITE setDetailsText NOTIFY detailsTextChanged)
  public:
    explicit MainWindow(QWindow *parent = nullptr);
    virtual ~MainWindow();

    void closeEvent(QCloseEvent *event);
    bool event(QEvent *ev) override;

    QColor windowColor() const;
    void setWindowColor(const QColor &windowColor);

    QString iconSource() const;
    void setIconSource(const QString &icon);

    QString mainTitle() const;
    void setMainTitle(const QString &title);

    bool titleFontBold() const;
    void setTitleFontBold(bool bold);

    QString subTitle() const;
    void setSubTitle(const QString &subTitle);

    bool subTitleFontLight() const;
    void setSubTitleFontLight(bool light);

    QString denyText() const;
    void setDenyText(const QString &denyText);

    QString allowText() const;
    void setAllowText(const QString &allowText);

    QString messageText() const;
    void setMessageText(const QString &messageText);

    QString detailsText() const;
    void setDetailsText(const QString &detailsText);

    bool buttonsVisible() const;
    void setButtonsVisible(bool visible);

    bool denyButtonVisible() const;
    void setDenyButtonVisible(bool visible);

    bool allowButtonVisible() const;
    void setAllowButtonVisible(bool visible);

    void initFromJson(const QJsonObject &req);
    void results(QJsonObject &results);

  public slots:
    void updateWindowSize(qreal width, qreal height);
    void setFinalSize(qreal width, qreal height);
    void deny();
    void allow();

  signals:
    void windowColorChanged();
    void iconSourceChanged();
    void mainTitleChanged();
    void titleFontBoldChanged();
    void subTitleChanged();
    void subTitleFontLightChanged();
    void denyTextChanged();
    void allowTextChanged();
    void buttonsVisibleChanged();
    void denyButtonVisibleChanged();
    void allowButtonVisibleChanged();
    void messageTextChanged();
    void detailsTextChanged();

    void rejectedByDeny();
    void rejectedByAllow();
    void rejectedByClose();

  private:
    void parseJson(const QJsonObject &req);
    void getCheckBoxes(QJsonObject &req, const QString &parent, QMap<QString, bool> checkboxes);

    void setupParentWindow();
    void setDialogModal();

    XdgForeign *m_xdgForeign = nullptr;
    XdgDialog *m_xdgDialog = nullptr;
    GSettingsManager *m_gsettings = nullptr;
    WaylandClient *m_client = nullptr;
    DynamicWidgetModel *m_dynamicModel = nullptr;

    QSize m_maxSize;

    QColor m_windowColor;
    QString m_iconSource;
    QString m_title;
    QString m_subTitle;
    QString m_denyText;
    QString m_allowText;
    bool m_titleFontBold = false;
    bool m_subTitleFontLight = true;
    QString m_messageText;
    QString m_detailsText;
    bool m_buttonsVisible = false;
    bool m_denyButtonVisible = true;
    bool m_allowButtonVisible = true;

    QString m_parentWindowHandle;
    bool m_isModal = false;
    WaylandClient::Protocols clientType;
};

#endif // MAINWINDOW_H
