#include "messageboxutils.h"

#include <QScopedPointer>
#include <QPushButton>
#include <QMessageBox>

void MessageBoxUtils::QMESSAGE_BOX_INFORMATION(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<QMessageBox> box(new QMessageBox(q_parent));
    box->setIcon(QMessageBox::Information);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}

void MessageBoxUtils::QMESSAGE_BOX_WARNING(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<QMessageBox> box(new QMessageBox(q_parent));
    box->setIcon(QMessageBox::Warning);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}

bool MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(QWidget *q_parent, const QString &typelabel, const QString &message, const QString &label_yes, const QString &label_no)
{
    QScopedPointer<QMessageBox> box(new QMessageBox(q_parent));
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label_yes, QMessageBox::AcceptRole);
    box->addButton(label_no, QMessageBox::RejectRole);
    box->setDefaultButton(okbtn);
    box->exec();
    if (box->clickedButton() != okbtn) {
        return false;
    }

    return true;
}

MessageBoxResult MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL2(
    QWidget *q_parent,
    const QString &typelabel,
    const QString &message,
    const QString &label_yes,
    const QString &label_no,
    const QString &label_cancel)
{
    QScopedPointer<QMessageBox> box(new QMessageBox(q_parent));
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(typelabel);
    box->setText(message);

    QPushButton *yesButton = box->addButton(label_yes, QMessageBox::AcceptRole);
    QPushButton *noButton = box->addButton(label_no, QMessageBox::RejectRole);
    QPushButton *cancelButton = box->addButton(label_cancel, QMessageBox::DestructiveRole);
    box->setDefaultButton(yesButton);

    box->exec();

    if (box->clickedButton() == yesButton) {
        return MessageBoxResult::Yes;
    } else if (box->clickedButton() == noButton) {
        return MessageBoxResult::No;
    } else if (box->clickedButton() == cancelButton) {
        return MessageBoxResult::Cancel;
    } else {
        return MessageBoxResult::Cancel;
    }
}

void MessageBoxUtils::QMESSAGE_BOX_CRITICAL(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<QMessageBox> box(new QMessageBox(q_parent));
    box->setIcon(QMessageBox::Critical);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}


