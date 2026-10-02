#include <QPushButton>
#include <QWidget>
#include <kpushbutton.h>
#include <kom-label.h>

namespace home_page {

class SolutionBtn : public kdk::KPushButton {
    Q_OBJECT
public:
    SolutionBtn(QWidget *parent = nullptr);
    void SetText(const QString &text);
    void SetIcon(const QString &path);

private:
    QPushButton *  icon_btn_;
    kom::KomLabel *text_label_;
};

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

}
