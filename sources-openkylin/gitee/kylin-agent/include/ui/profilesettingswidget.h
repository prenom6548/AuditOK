#ifndef PROFILESETTINGSWIDGET_H
#define PROFILESETTINGSWIDGET_H

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>

class ProfileSettingsWidget : public QWidget {

    Q_OBJECT

public:
    explicit ProfileSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

private:
    QFormLayout *m_formLayout;
    
    // Profile section
    QLineEdit *m_profileNameEdit;
    QCheckBox *m_autoSaveCheckBox;
    
    void createProfileGroup();
};

#endif // PROFILESETTINGSWIDGET_H
