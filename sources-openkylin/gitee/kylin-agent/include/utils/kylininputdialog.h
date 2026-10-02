#ifndef KYLININPUTDIALOG_H
#define KYLININPUTDIALOG_H

#include <kinputdialog.h>

class KylinInputDialog
{
public:
    static QString getText(QWidget *parent,
                           const QString &title,
                           const QString &label,
                           QLineEdit::EchoMode echo = QLineEdit::Normal,
                           const QString &text = QString(),
                           bool *ok = nullptr)
    {
        kdk::KInputDialog dialog(parent);
        dialog.setWindowTitle(title);
        dialog.setInputMode(kdk::KInputDialog::TextInput);
        dialog.setLabelText(label);
        dialog.setTextEchoMode(echo);
        dialog.setTextValue(text);
        const bool accepted = dialog.exec() == kdk::KDialog::Accepted;
        if (ok) {
            *ok = accepted;
        }
        return accepted ? dialog.textValue() : QString();
    }
};

#endif // KYLININPUTDIALOG_H
