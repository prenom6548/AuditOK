#ifndef TRANSMISSIONDIALOG_H
#define TRANSMISSIONDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

class TransmissionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TransmissionDialog(QWidget *parent = nullptr);

    void setProgressBarValue(qint64 value);
    void setProgressBarRange(qint64 min, qint64 max);

private:
    QLabel *m_titleLab = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPushButton *m_closeBtn = nullptr;

Q_SIGNALS:
    void sigCancelAbort();
};

#endif // TRANSMISSIONDIALOG_H
