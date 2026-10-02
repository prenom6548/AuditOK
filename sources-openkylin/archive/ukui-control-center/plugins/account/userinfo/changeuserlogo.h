#ifndef CHANGEUSERLOGO_H
#define CHANGEUSERLOGO_H

#include <QDialog>
#include <QPainterPath>
#include <QCheckBox>
#include <QStackedWidget>
#include <knavigationbar.h>
#include <kborderlessbutton.h>
#include <kwidget.h>
#include <klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#include "circlelabel.h"

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QFrame;
class QButtonGroup;
class FlowLayout;
class QPixmap;
class QDBusInterface;
class QGridLayout;

#define FACEPATH "/usr/share/ukui/faces/"
#define CUSTOMFACEPATH "/var/lib/AccountsService/icons/"

#define CHECKBOXWIDTH 20
#define CHECKBOXHEIGHT 20

class ChangeUserLogo : public KWidget
{
    Q_OBJECT

public:
    explicit ChangeUserLogo(QString realname, QString username, QString currentIcon, QWidget *parent = nullptr);
    ~ChangeUserLogo();

public:
    void initUI();
    void loadSystemLogo();
    void loadSystemPath(QString systemPath);
    void loadCustomLogo();
    void refreshUserLogo(QString logo);
    void setupConnect();
    void setButtonVisible(bool visible);
    bool isDateValid(const QString &dateString, const QString &format);

public:
    CircleLabel *m_pCurrentLogoLabel = nullptr;
    KLabel *m_pNickNameLabel = nullptr;
    KLabel *m_pTitleLabel = nullptr;
    KBorderlessButton *m_pEditButton = nullptr;
    KBorderlessButton *m_pDeleteButton = nullptr;
    KBorderlessButton *m_pCancelButton = nullptr;
    QCheckBox *m_pSelectAllCheckBox = nullptr;

    KNavigationBar *m_pLeftBarScrollArea = nullptr;
    QStandardItem *m_pSystemLogoItem = nullptr;
    QStandardItem *m_pCustomLogoItem = nullptr;
    KPushButton *m_pCancelBtn = nullptr;
    KPushButton *m_pConfirmBtn = nullptr;
    QHBoxLayout * m_pBottomBtnsHorLayout = nullptr;

    QPushButton *m_pAddCustomLogoBtn = nullptr;

    QStackedWidget *m_pLogosWidget = nullptr;
    FlowLayout *m_pSystemLogosFlowLayout = nullptr;
    FlowLayout *m_pCustomLogosFlowLayout = nullptr;

private:
    QString m_realname;
    QString m_username;
    QString m_currentIcon;
    QString m_selected = QString();
    QStringList m_selectedLogos;
    QList<QCheckBox *> m_pCustomLogoCheckboxList;
    QList<QPushButton *> m_pCustomLogoButtonList;
    bool m_systemFaces = true;
    QButtonGroup *m_pLogosBtnGroup;

Q_SIGNALS:
    void confirmChanged(QString filename, bool systemFaces);
    void deleteLogos(QStringList logoList);

private slots:
    void showLocalFaceDialog();
    void refreshUI(const QModelIndex &index);
    void editCustomLogo();
    void deleteCustomLogo();
    void cancelEditCustomLogo();
    void selectAllCustomLogo(int checked);
    void refreshDeleteStatus();
};

#endif // CHANGEUSERLOGO_H
