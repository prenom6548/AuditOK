#ifndef IMAGEBACKEND_H
#define IMAGEBACKEND_H

#include <QObject>
#include <QString>
#include <QUrl>

class ImageBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl imageSource READ imageSource NOTIFY imageSourceChanged)
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY hasImageChanged)
    Q_PROPERTY(bool openFailed READ openFailed NOTIFY openFailedChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit ImageBackend(QObject *parent = nullptr);

    QUrl imageSource() const;
    bool hasImage() const;
    bool openFailed() const;
    QString statusText() const;

    Q_INVOKABLE void openImage();
    Q_INVOKABLE void closeImage();
    Q_INVOKABLE void reportImageLoadFailed();

Q_SIGNALS:
    void imageSourceChanged();
    void hasImageChanged();
    void openFailedChanged();
    void statusTextChanged();

private:
    void setStatusText(const QString &text);
    bool updateImageSource(const QString &localFile);

    QUrl m_imageSource;
    bool m_hasImage;
    bool m_openFailed;
    QString m_statusText;
};

#endif // IMAGEBACKEND_H

