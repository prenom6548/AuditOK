#ifndef INFOCREATEWIDGET_H
#define INFOCREATEWIDGET_H

#include "../fileProcess/fileprocess.h"
#include "../build/buildcheckwidget.h"

#include <kaboutdialog.h>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QRegExp>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QCheckBox>

class InfoCreateWidget : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit InfoCreateWidget(QWidget *parent = nullptr);
    explicit InfoCreateWidget(QWidget *parent, const QString &m_state);

    void InitThemeName();
    void InitBuilderInfo();
    void InitInfoBtn(const QString &m_state);
    void InfoProcess();
    void InfoCreateBtn();
    void InitDefaultInstallment();

    void renameDesktop();
    QString m_builderMail;
    QString m_builderName;

    void createControlChangelog(const QString &m_date);
    void createMeson(const QString &m_date);
    void getFileInfo(const QMap<QString, QVariant> &m_packageInfo);

    void updateLabelShow();

signals:
    void readytoSave();
    void readytoBuild();
    void buildPackage();
    void gohomesignals();

    void hasGetThemeName();
public:
    QLabel *m_warningMessage;
    QLabel *m_warningMessage2;
    QLabel *m_warningMessage3;
    QLabel *m_warningMessage4;

    QHBoxLayout *m_themeNameCNLayout;
    QLabel *m_themeNameCNLabel;
    QLineEdit *m_themeNameCNLineEdit;

    QHBoxLayout *m_themeNameENLayout;
    QLabel *m_themeNameENLabel;
    QLineEdit *m_themeNameENLineEdit;

    QHBoxLayout *m_builderNameLayout;
    QLabel *m_builderNameLabel;
    QLineEdit *m_builderNameLineEdit;

    QHBoxLayout *m_builderMailLayout;
    QLabel *m_builderMailLabel;
    QLineEdit *m_builderMailLineEdit;

    QHBoxLayout *m_defaultInstallLayout;
    QLabel *m_defaultInstallLabel;
    QCheckBox *m_defaultInstallCheckBox;

    QHBoxLayout *m_infoBtnLayout;
    QPushButton *m_exitBtn;
    QPushButton *m_recogniseBtn;
    QPushButton *m_createDebBtn;

    bool m_themeENNameState;
    bool m_builderNameState;
    bool m_builderMailState;
};

#endif // INFOCREATEWIDGET_H
