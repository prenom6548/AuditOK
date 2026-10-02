#ifdef HAVE_COREAI_PATH
#include "ocrhandler.h"
#include <QBuffer>
#include <QDebug>

void OcrHandler::callback(TextRecognitionResult *result, void *user_data)
{
    OcrHandler* o = static_cast<OcrHandler*>(user_data);

    const char *value = text_recognition_result_get_value(result);

    const QString resultStr(value);

    Q_EMIT o->readToGetResult(resultStr);
}

OcrHandler::OcrHandler(QObject *parent)
{
    text_recognition_model_config_set_name(m_config,"vision");
    text_recognition_model_config_set_deploy_type(m_config,ModelDeployType::OnDevice);
    text_recognition_set_model_config(m_session,m_config);
    text_recognition_init_session(m_session);
    text_recognition_result_set_callback(m_session, callback, (void *)this);
}

void OcrHandler::setImage(const QImage &image)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // 使用PNG格式，您可以选择其他格式
    QByteArray imageData = buffer.data();
    // 将QByteArray转换为uchar*并存储到std::string
    std::string apiKey_(imageData.constData(), imageData.size());

    text_recognition_recognize_text_from_image_data_async(m_session, imageData.data(), imageData.size());
}

#endif
