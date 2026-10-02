#ifndef RENAMEWIDGET_H
#define RENAMEWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QFileInfo>
#include <QMessageBox>
class RenameWidget : public QWidget
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit RenameWidget(QWidget *parent = nullptr);
    // 重命名时设置编辑框内的初始内容
    void setLineEditContent(QString m_filePath);
    QString getNewName();

signals:
    void changeLabel(QString newfilepath);

private:
    int fixedNameLength = 20;
    // 重命名页面 标题栏布局
    QWidget *m_titleWidget= nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QPushButton *titleIcon = nullptr;
    QLabel *titleText = nullptr;
    QToolButton *m_titleButtonExit = nullptr;

    // 重命名页面 界面布局
    QVBoxLayout* m_bodyLayout = nullptr;
    QHBoxLayout* m_checkButtonLayout = nullptr;
    QHBoxLayout* m_LineEditLayout = nullptr;
    QHBoxLayout* m_renameLayout = nullptr;
    QHBoxLayout *m_wordExceedLayout = nullptr;
    QLabel* m_renameLabel = nullptr;
    QLabel *m_seatLabel = nullptr;
    QFont m_font;
    QLineEdit *m_renameLineEdit = nullptr;
    QLabel *m_nameLengthCount = nullptr;
    QLabel *m_wordExceedLabel = nullptr;


    QPushButton *m_confirmButton = nullptr;
    QPushButton *m_cancelButton = nullptr;

    // 总布局
    QVBoxLayout *m_mainLayout = nullptr;

    QString new_name;
    QFileInfo *fileInfo = nullptr;
    QString m_appShowingName = tr("Recorder");
private:
    void init();

    void initAction();
    void setTitleBar();
    void initTitleIcon();
    void initButtonExit();
    void setTitleLayout();
    void setBodyLayout();
    void setmainLayout();

    bool checkRenameable(QFileInfo *fileinfo, QString new_name);
    bool checkLegal(QString new_name);

private slots:
    void renameSlots();
    void countLengthSlots();


};

#endif // RENAMEWIDGET_H
