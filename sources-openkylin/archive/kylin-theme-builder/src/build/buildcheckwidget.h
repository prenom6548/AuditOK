#ifndef BUILDCHECKWIDGET_H
#define BUILDCHECKWIDGET_H

#include "../fileProcess/fileprocess.h"
#include "../fileProcess/filecheck.h"
#include "buildWidget.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCryptographicHash>

struct gsettingOption{
    QString accentRole = "";
};

class buildCheckWidget : public QWidget
{
    Q_OBJECT
public:
    explicit buildCheckWidget(QWidget *parent = nullptr);

    void buildCheck(QDir m_buildPath);
    void buildCheckResult(QDir m_buildPath);

signals:
    void gohomesignals();

private:
    QVBoxLayout *m_buildCheckWidget;
    QLabel *m_buildWarningMessage;
    QLabel *m_warningMessageList;
    QHBoxLayout *m_buildCheckBtnWidget;
    QPushButton *m_buildBtn;
    QPushButton *m_exitBuildBtn;

    QString m_debSourceFormat;
    QString m_debRules;
    QString m_debCopyRight;
    QString m_debCopyPostinst;
    QString m_debCopyPostrm;


    QString m_debPath = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src";

    gsettingOption *m_option;
    int buildCheckState = 0;
    void buildCopy();
    void buildForGlobal();
    void buildForIcon();
    void buildForCursor();
    void buildForPlymouth();
    void buildForGrub();

    void initSetupShell();
    void initSetupSchemas();

    QMap <QString ,int> appIconInfo;
    int appIconInfoID = 1;
    QMap <QString ,int> systemIconInfo;
    int systemIconInfoID = 1;
    QMap <QString ,int> cacheIconInfo;
    int cacheIconInfoID = 1;
};

#endif // BUILDCHECKWIDGET_H
