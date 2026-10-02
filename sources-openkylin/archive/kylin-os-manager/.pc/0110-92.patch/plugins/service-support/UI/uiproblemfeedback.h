#ifndef UIPROBLEMFEEDBACK_H
#define UIPROBLEMFEEDBACK_H

#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QGSettings/QGSettings>
#include <kballontip.h>
#include "kom-label.h"
#include "service-support-backend/src/settings.h"
#include "../constant.h"

class UploadFileItem : public QWidget
{
    Q_OBJECT
signals:
    void removeUploadFile(const QString &);

public:
    UploadFileItem(const QString &filename);
    QListWidgetItem *getWidgetItem();

protected:
    bool event(QEvent *event);

private:
    void getType();
    void textUpdate();
    enum FileType { Unknow = 0, Image, Video, Package } m_fileType = Unknow;
    QString m_filename;
    QLabel *m_item = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QLabel *m_icon = nullptr;
    QLabel *m_name = nullptr;
    QListWidgetItem *m_widgetItem = nullptr;
};

class UiProblemFeedbackDialog;
class QWebEngineView;
class UiProblemFeedback : public QWidget
{
    Q_OBJECT
public:
    UiProblemFeedback(QWidget *parent = nullptr);
    ~UiProblemFeedback();

public slots:
    void indexChanged(QString name);

private slots:
    void onSubmitClicked();
    void onScreenCutClicked();
    void onAddFileClicked();
    void onExpertchange(bool b);
    void getShotImage();
    void removeUploadFile(const QString &fileName);
    void errorMessage(const QString &msg);
    void retryUpload();

protected:
    void paintEvent(QPaintEvent *e);

private:
    struct FormLayoutRow
    {
        FormLayoutRow() {}
        FormLayoutRow(QLabel *lab, QWidget *wid) : label(lab), widget(wid) {}
        QLabel *label = nullptr;
        QWidget *widget = nullptr;
    };
    void initUI();
    void initDialogs();
    void initClassUI();
    void initDetailsUI();
    void initContactUI();
    void initSubmitUI();
    void initExpertUI();
    void initConnect();
    void initFinish();
    void resetState();
    const QString creatFormTitle(const QString &text, bool required = true) const;
    void addUploadFile(const QString &filename);
    void addUploadFileItem(const QString &filename);
    void setDateTime();
    void hideRow(int i);
    void showRow(int i);
    void creatDetails(QString &orig, QString level, QString color, QString title, QString value);
    QString creatTitle();
    void saveFormLayoutPtr(int row, const QString &labelText, QWidget *wid);
    void creatFormLayout();
    void showMessageBox(const QString &msg, kdk::TipType type);
    void onFontSizeChange();
    void onthemeChange();
    void onHighLightChange();
    void changeSubmitState();
    void setVerticalSpacingLayout(QWidget *parent, QLayout *bl, int vSpacing = 0);
    enum CheckStringType { PhoneNumber = 0, Mail };
    bool checkString(const QString &str, CheckStringType type);
    void getUserCode();
    QMap<int, FormLayoutRow> m_formLayoutPtrMap;
    QList<QCheckBox *> m_systemInfoGroup;
    QList<QRadioButton *> m_classTypeGroup;
    QList<int> m_modelExpert;
    QFormLayout *m_formLayout = nullptr;
    int m_formLayoutHorizontalSpacing;
    int m_formLayoutVerticalSpacing;
    int m_formLayoutContentsMarginsLeft;
    int m_classType;
    QString m_classText;
    QGSettings *m_screenShotSetting = nullptr;
    bool m_startShot = false;
    QComboBox *m_secondClass = nullptr;
    QDateEdit *m_dateEdit = nullptr;
    QString m_tmpDetailsEditText;
    QTimeEdit *m_dateEditStart = nullptr;
    QTimeEdit *m_dateEditEnd = nullptr;
    QTextEdit *m_detailsEdit = nullptr;
    QLabel *m_detailsEditNumber = nullptr;
    QString m_dateEditText;
    QString m_detailsEditNumberText;
    QTextEdit *m_tmpDetailsEdit = nullptr;
    QWidget *m_uploadText = nullptr;
    kom::KomLabel *m_uploadText1 = nullptr;
    kom::KomLabel *m_uploadText2 = nullptr;
    QLineEdit *m_phoneNumberEdit = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_mailEdit = nullptr;
    QPushButton *m_connectGiteeBtn = nullptr;
    QString m_giteeUserCode;
    QLabel *m_mailErrorText = nullptr;
    QWebEngineView *m_webView = nullptr;
    QLineEdit *m_exportEdit = nullptr;
    QCheckBox *m_agreeAll = nullptr;
    QCheckBox *m_checkAdvanced = nullptr;
    QPushButton *m_screenCut = nullptr;
    QRadioButton *m_logYes = nullptr;
    QComboBox *m_logDays = nullptr;
    QListWidget *m_fileListWidget = nullptr;
    QLabel *m_systemInfoLabel = nullptr;
    QPushButton *m_submit = nullptr;
    QPushButton *m_addFile = nullptr;
    UiProblemFeedbackDialog *m_dialog = nullptr;
    QStringList m_uploadFileNameList;
    QString m_dateTime;
    QImage m_lastMimeImage;
    bool m_expertchange;
    bool m_loaded = false;
    QString m_objectName = NAME_PROBLEM_FEEDBACK;
    bool m_getCodeFinish;
};

#endif // UIPROBLEMFEEDBACK_H
