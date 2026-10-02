#ifndef USAGESETTINGSWIDGET_H
#define USAGESETTINGSWIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class UsageSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit UsageSettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    void createUsageGroup();
    void refreshUsage();
    void refreshUsageWithFeedback();

    QVBoxLayout *m_mainLayout;
    QPushButton *m_refreshButton;
    QLabel *m_statsLabel;
};

#endif
