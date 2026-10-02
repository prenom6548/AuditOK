#ifndef BACKUPPOSITIONSELECTDIALOG_H
#define BACKUPPOSITIONSELECTDIALOG_H


#include <QFileDialog>

class BackupPositionSelectDialog : public QFileDialog {
    Q_OBJECT
public:
    explicit BackupPositionSelectDialog(QWidget* parent = nullptr);
    virtual ~BackupPositionSelectDialog();

    bool checkSelectedFiles();
public slots:
    void goAccept();

protected:
    void showEvent(QShowEvent* event) override;
};


#endif // BACKUPPOSITIONSELECTDIALOG_H
