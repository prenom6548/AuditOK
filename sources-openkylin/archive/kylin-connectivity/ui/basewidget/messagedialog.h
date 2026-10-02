#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QTextEdit>
//#include <kdialog.h>
#include <QDialog>

class MessageDialog : public QDialog
{
    Q_OBJECT
public:
    MessageDialog(QWidget *parent = nullptr);

    void setText(QString title, QString text = "");
    void setIconPixmap(QIcon icon);
    QPushButton *addButton(QString text);

protected:
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void buttonClicked(QPushButton *);
    void sigClose();

private:
    QPushButton *m_titleIcon = nullptr;
    QLabel *m_titleNameLab = nullptr;
    QPushButton *m_closeBtn = nullptr; // 关闭按钮
    QLabel *m_icon = nullptr;
    QLabel *m_titleLab = nullptr;
    QTextEdit *m_textLab = nullptr;
    QVBoxLayout *m_textLayout = nullptr;
    QHBoxLayout *m_btnLayout = nullptr;
};

#endif // MESSAGEDIALOG_H
