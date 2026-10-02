#ifndef CLINEEDITLABEL_H
#define CLINEEDITLABEL_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QAction>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include "../PluginService/ui_unit/arrowtooltip.h"

namespace KInstaller {

class CLineEditLabel : public QWidget
{
    Q_OBJECT
public:
    explicit CLineEditLabel(QString strName, QIcon icon, QWidget *parent = nullptr);
    void initUI();
    void initAllConnect();

//    void setLeadingPng(QIcon icon){ leadingPng = icon;}
    void setTrailingPng(QIcon icon);

    void setLineEditText(QString str) {m_lineEdit->setText(str);}
    void setPlaceholderText(QString str);
    QString getLineEditText() {return m_lineEdit->text();}
    void setBLToolTip(bool flag){bltoopTip = flag;}
signals:
    void signalToolTip();
public slots:
    void changeCodePng();

protected:
//    bool eventFilter(QObject *watched, QEvent *event);
//    virtual void mousePressEvent(QMouseEvent *event);
public:
    QVBoxLayout *layout;

    QLineEdit *m_lineEdit;
    QAction *action;
    bool m_flag;
    QIcon leadingPng;
    bool bltoopTip = false;
    ArrowWidget *w = nullptr;
    QString m_strName = "";

};
class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(QString strName, QIcon icon, QWidget *parent = nullptr);
    ~CustomLineEdit();
//    void initUI();
//    void initAllConnect();

//    void setLeadingPng(QIcon icon){ leadingPng = icon;}
    void setTrailingPng(QIcon icon);

    void setLineEditText(QString str) {this->setText(str);}
    void setPlaceholderText2(QString str);
    QString getLineEditText() {return this->text();}
    void setBLToolTip(bool flag){bltoopTip = flag;}
    void resizeEvent(QResizeEvent *event);
signals:
    void signalToolTip();
public slots:
    void changeCodePng();

protected:
//    bool eventFilter(QObject *watched, QEvent *event);
//    virtual void mousePressEvent(QMouseEvent *event);
public:
    QVBoxLayout *layout;

   // QLineEdit *m_lineEdit;
   // QAction *action;
    bool m_flag;
    QIcon leadingPng;
    bool bltoopTip = false;
    ArrowWidget *w = nullptr;
    QString m_strName = "";
    QToolButton *m_button = nullptr;
};
}
#endif // CLINEEDITLABEL_H
