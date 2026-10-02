#ifndef MESSAGEBOXUTILS_H
#define MESSAGEBOXUTILS_H

#include <QWidget>
#include <QString>
#include <QMessageBox>

class MyMessageBox : public QMessageBox
{
    Q_OBJECT
public:
    MyMessageBox(QWidget *parent = nullptr);
    virtual ~MyMessageBox();

    void setThemeIcon(const QString& themeIcon);
    void setMsgIcon(QMessageBox::Icon icon);

public slots:
    void on_themeIconChanged();
    void on_iconChanged();

private:
    QString m_themeIcon;
    QMessageBox::Icon m_icon;
};

class MessageBoxUtils
{
public:
    static void QMESSAGE_BOX_INFORMATION(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static void QMESSAGE_BOX_WARNING(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static void QMESSAGE_BOX_CRITICAL(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static bool QMESSAGE_BOX_WARNING_CANCEL(QWidget* q_parent, const QString& typelabel, const QString& message,
                                            const QString& label_yes, const QString& label_no);
};

#endif // MESSAGEBOXUTILS_H
