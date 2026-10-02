#ifndef FILEPATHEDIT_H
#define FILEPATHEDIT_H

#include <QLineEdit>
#include <QDropEvent>
#include <QMimeData>

class FilePathEdit : public QLineEdit
{
    Q_OBJECT
public:
    FilePathEdit(QWidget *parent = nullptr);
    virtual ~FilePathEdit();

protected:
    void dropEvent(QDropEvent *e);
};

#endif // FILEPATHEDIT_H
