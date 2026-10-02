#ifndef PERSONASETTINGSWIDGET_H
#define PERSONASETTINGSWIDGET_H

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class PersonaSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit PersonaSettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    QVBoxLayout *m_mainLayout;
    QPlainTextEdit *m_editor;
    QPushButton *m_saveButton;
    QPushButton *m_resetButton;
    QString m_savedSoul;
    bool m_loading = false;

    void createPersonaGroup();
    bool savePersona(bool showConfirmation);
    void resetPersona();
    QString soulFilePath() const;
    QString runtimeHome() const;
    QString readSoul() const;
    bool writeSoul(const QString &content) const;
    QString defaultSoul() const;
};

#endif // PERSONASETTINGSWIDGET_H
