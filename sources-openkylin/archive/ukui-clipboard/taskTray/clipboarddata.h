#ifndef CLIPBOARDDATA_H
#define CLIPBOARDDATA_H
#include <QObject>
#include <QImage>
#include <QDateTime>
#include <QUrl>
#include <QClipboard>

class clipboardData : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        Text = 0, /**> 文本 */
        Image, /**> 图片 */
        File, /**> 文件 */
        Link /**> 链接 */
    };
    Q_ENUM(Mode)
    clipboardData(QObject *parent = nullptr);

    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(bool hasImage READ hasImage WRITE setHasImage NOTIFY HasImageChanged);
    Q_PROPERTY(bool isLinkMode READ isLinkMode WRITE setIsLinkMode NOTIFY isLinkModeChanged);
    Q_PROPERTY(QUrl imageUrl READ imageUrl NOTIFY imageUrlChanged)
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged);
    Q_PROPERTY(QDateTime sourceTime READ sourceTime WRITE setSourceTime NOTIFY sourceTimeChanged);
    Q_PROPERTY(bool isCollected READ isCollected WRITE setIsCollected NOTIFY isCollectedChanged);

    Mode mode = Mode::Text;
    Q_INVOKABLE Mode getMode();
    Q_INVOKABLE QString getFilePath();
    Q_INVOKABLE QString getToolTipString();
    QMimeData *m_mineData;
    QClipboard *m_dataClipboard;

    QString id();
    bool hasImage();
    bool isLinkMode();
    QString sourceText();
    QUrl imageUrl();
    QDateTime sourceTime();
    bool isCollected();

    void setId(QString str);
    void setHasImage(bool value);
    void setIsLinkMode(bool value);
    //void setImageUrl(QImage ima);
    void setSourceText(QString str);
    void setSourceTime(QDateTime dataTime);
    QMimeData* getMimedata(QMimeData* mData);

    void setImage(QImage ima);
    bool isSame(clipboardData *data);
    void updateImage();
    void setIconName(QString str);

public slots:
    Q_INVOKABLE void setIsCollected(bool flag);
    Q_INVOKABLE void copyToClipboard();
    Q_INVOKABLE void sendNeedMoveToFirst();
    Q_INVOKABLE QString getIconName();
    Q_INVOKABLE void sendNeedToDelete();
    Q_INVOKABLE void sendNeedToDeleteAll();

signals:
    void imageUrlChanged();
    void sourceTextChanged();
    void sourceTimeChanged();
    void HasImageChanged();
    void isLinkModeChanged();
    void isCollectedChanged();
    void dataCopyed();
    void needMoveToFirst();
    void needToDelete();
    void needToDeleteAll();

private:
    QString m_id;
    bool m_hasImage;
    bool m_isLinkMode;
    QDateTime m_sourceTime;
    QString m_sourceText;
    QUrl m_imageUrl;
    bool m_isCollected = false;
    QImage m_image;
    QString m_iconName;
};

#endif // CLIPBOARDDATA_H
