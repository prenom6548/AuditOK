#include <QWidget>

namespace home_page {

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

}
