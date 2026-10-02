#ifndef FILEMANAGERHELPER_H
#define FILEMANAGERHELPER_H

#include <QDBusConnection>
#include <QVariantMap>
#include <QObject>

class FileManagerHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit FileManagerHelper(QObject *parent = nullptr);
    /**
     * @brief 析构
     *
     */
    virtual ~FileManagerHelper();
    
    bool showFolders(QStringList path);
    
private:
    void init();
    
private:
    QDBusInterface *m_fileManagerInterface = nullptr;
};

#endif // FILEMANAGERHELPER_H
