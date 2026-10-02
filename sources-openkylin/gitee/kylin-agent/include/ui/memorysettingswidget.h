#ifndef MEMORYSETTINGSWIDGET_H
#define MEMORYSETTINGSWIDGET_H

#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class MemorySettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit MemorySettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    QVBoxLayout *m_mainLayout;
    QLabel *m_sessionsValue;
    QLabel *m_messagesValue;
    QLabel *m_profileEntriesValue;
    QPlainTextEdit *m_editor;
    QLabel *m_hintLabel;
    QLabel *m_charLabel;
    QPushButton *m_refreshButton;
    QPushButton *m_saveButton;
    QJsonObject m_profileEntries;
    QString m_profileUpdatedAt;
    QString m_savedContent;
    bool m_loading = false;
    bool m_profileAvailable = false;

    void createMemoryGroup();
    void refreshMemory();
    void refreshMemoryWithFeedback();
    void updateEditor();
    void updateStats();
    void saveCurrentEditor();
};

#endif // MEMORYSETTINGSWIDGET_H
