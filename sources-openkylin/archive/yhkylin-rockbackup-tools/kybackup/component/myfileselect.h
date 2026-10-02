#ifndef MYFILESELECT_H
#define MYFILESELECT_H

#include <QFileDialog>

class MyFileSelect : public QFileDialog {
    Q_OBJECT
public:
    explicit MyFileSelect(QWidget* parent = nullptr);
    virtual ~MyFileSelect();

    bool checkSelectedFiles();
public slots:
    void goAccept();

protected:
    void showEvent(QShowEvent* event) override;
};

#endif // MYFILESELECT_H
