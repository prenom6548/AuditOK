#ifndef OCRHANDLER_H
#define OCRHANDLER_H
#ifdef HAVE_COREAI_PATH
#include <coreai/vision/textrecognition.h>
#include <QObject>
#include <QImage>

class OcrHandler : public QObject
{
    Q_OBJECT
public:
    static OcrHandler& instance() {
        static OcrHandler handler;
        return handler;
    }

    void setImage(const QImage &image);

signals:
    void readToGetResult(const QString &text);

private:
    static void callback(TextRecognitionResult *result, void *user_data);

private:
    explicit OcrHandler(QObject *parent = nullptr);

    TextRecognitionSession *m_session = text_recognition_create_session();
    TextRecognitionModelConfig *m_config = text_recognition_model_config_create();
};

#endif
#endif // OCRHANDLER_H
