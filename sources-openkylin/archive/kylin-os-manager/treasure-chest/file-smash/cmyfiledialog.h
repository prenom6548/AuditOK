#ifndef CMYFILEDIALOG_H
#define CMYFILEDIALOG_H

#include <QWidget>
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QDialogButtonBox>

class CMyFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    CMyFileDialog( QString titleName, QWidget *parent = nullptr);

protected:

    void showEvent(QShowEvent *event) override;
Q_SIGNALS:
public Q_SLOTS:
    void slot_myAccetp();
};

#endif // CMYFILEDIALOG_H
