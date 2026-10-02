#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QObject>
#include <QWidget>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QSlider>
#include <QScrollArea>
#include <QTabWidget>
#include <QTableWidget>
#include <QIcon>
#include <QPixmap>
#include <QDir>
#include <QStandardPaths>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QSizeGrip>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include "ui/generalsettingswidget.h"
#include "ui/modelsettingswidget.h"
#include "ui/skillssettingswidget.h"
#include "ui/personasettingswidget.h"
#include "ui/memorysettingswidget.h"
#include "ui/logsettingswidget.h"
#include "ui/usagesettingswidget.h"

class SettingsTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit SettingsTitleBar(QWidget *parent = nullptr);

signals:
    void dragPressed(const QPoint &globalPos);
    void dragMoved(const QPoint &globalPos);
    void dragReleased();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

class SettingsDialog : public QDialog {
    Q_OBJECT


public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    void showFromOwner(const QRect &ownerGeometry);

signals:
    void closed();

private:
    void setupUi();
    void setupUI();
    void setupConnections();
    void updateTitleBarStyle();
    void positionSizeGrip();
    void beginWindowDrag(const QPoint &globalPos);
    void dragWindowTo(const QPoint &globalPos);
    void setDragBypassEnabled(bool enabled);
    void saveBackend();
    void openModelSettings();
    
    QWidget *m_titleBar;
    QListWidget *m_categoryList;
    QStackedWidget *m_contentStack;
    QSplitter *m_splitter;
    QSizeGrip *m_sizeGrip;
    QLineEdit *m_backendEdit;
    QPushButton *m_saveButton;
    QPoint m_dragPosition;
    bool m_dragging;
    bool m_dragBypassActive = false;

};

#endif // SETTINGSDIALOG_H
