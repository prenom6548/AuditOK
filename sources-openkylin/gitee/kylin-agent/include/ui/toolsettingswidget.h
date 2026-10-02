#ifndef TOOLSETTINGSWIDGET_H
#define TOOLSETTINGSWIDGET_H

#include <QWidget>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QSettings>
#include <QApplication>
#include <QListWidget>
#include <QStackedWidget>
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
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QObject>
#include <QScreen>
#include <QFontDialog>
#include <QColorDialog>
#include <QSignalMapper>
#include <QMenu>
#include <QAction>

class ToolSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ToolSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

private:
    QFormLayout *m_formLayout;
    
    // Tools section
    QCheckBox *m_enableClipboardToolCheckBox;
    QCheckBox *m_enableFileManagementToolCheckBox;
    QCheckBox *m_enableSystemInfoToolCheckBox;
    QCheckBox *m_enableNetworkToolCheckBox;
    QCheckBox *m_enableScriptingToolCheckBox;
    QCheckBox *m_enableDataAnalysisToolCheckBox;
    QComboBox *m_toolExecutionModeCombo;
    QCheckBox *m_enableToolAutoUpdateCheckBox;
    
    void createToolsGroup();
};

#endif // TOOLSETTINGSWIDGET_H