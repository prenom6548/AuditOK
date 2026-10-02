#include "ui/modelsettingswidget.h"
#include "services/apiservice.h"
#include "services/cuamodelconfigsync.h"
#include "services/modeldownloadservice.h"

#include <QComboBox>
#include <QColor>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <kmessagebox.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QPainter>
#include <QPaintEvent>
#include <QPair>
#include <QProgressBar>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSet>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <kprogressbar.h>
#include <kdialog.h>

#include <algorithm>

namespace {

struct ModelProbeResult
{
    bool success = false;
    QString message;
};

QString probeErrorMessage(const QByteArray &body)
{
    const QJsonDocument document = QJsonDocument::fromJson(body);
    if (!document.isObject()) {
        return QString::fromUtf8(body).trimmed().left(500);
    }

    const QJsonObject object = document.object();
    const QJsonValue error = object.value(QStringLiteral("error"));
    if (error.isObject()) {
        const QJsonObject errorObject = error.toObject();
        const QString message = errorObject.value(QStringLiteral("message")).toString().trimmed();
        if (!message.isEmpty()) {
            return message;
        }
    } else if (error.isString()) {
        return error.toString().trimmed();
    }
    return object.value(QStringLiteral("message")).toString().trimmed();
}

ModelProbeResult probeModel(const QString &baseUrl, const QString &apiKey, const QString &modelId)
{
    QString normalizedBaseUrl = baseUrl.trimmed();
    if (!normalizedBaseUrl.isEmpty() && !normalizedBaseUrl.contains(QStringLiteral("://"))) {
        normalizedBaseUrl.prepend(QStringLiteral("http://"));
    }
    while (normalizedBaseUrl.endsWith(QLatin1Char('/'))) {
        normalizedBaseUrl.chop(1);
    }
    const QUrl parsedBaseUrl(normalizedBaseUrl);
    if (normalizedBaseUrl.isEmpty() || !parsedBaseUrl.isValid() ||
        (parsedBaseUrl.scheme() != QStringLiteral("http") &&
         parsedBaseUrl.scheme() != QStringLiteral("https")) ||
        parsedBaseUrl.host().isEmpty()) {
        return {false, QObject::tr("基础 URL 无效。请输入包含 http:// 或 https:// 的完整地址。")};
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(normalizedBaseUrl + QStringLiteral("/models")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!apiKey.trimmed().isEmpty()) {
        request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(apiKey.trimmed()).toUtf8());
    }

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeout.start(15000);
    loop.exec();

    const bool timedOut = !timeout.isActive();
    timeout.stop();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    reply->deleteLater();

    if (timedOut) {
        return {false, QObject::tr("连接超时。请检查网络连接和基础 URL。")};
    }
    if (status == 401 || status == 403) {
        const QString detail = probeErrorMessage(body);
        return {false, detail.isEmpty()
                           ? QObject::tr("API Key 无效、已过期或没有访问权限。")
                           : QObject::tr("API Key 验证失败：%1").arg(detail)};
    }
    if (status == 404) {
        return {false, QObject::tr("基础 URL 不正确：未找到 /models 接口。请确认地址包含正确的 API 版本路径（例如 /v1）。")};
    }
    if (networkError != QNetworkReply::NoError) {
        return {false, QObject::tr("网络连接失败：%1。请检查网络、基础 URL 和服务状态。").arg(networkErrorText)};
    }
    if (status < 200 || status >= 300) {
        const QString detail = probeErrorMessage(body);
        return {false, detail.isEmpty()
                           ? QObject::tr("服务返回 HTTP %1，请检查模型配置。").arg(status)
                           : QObject::tr("服务返回 HTTP %1：%2").arg(status).arg(detail)};
    }

    const QJsonDocument document = QJsonDocument::fromJson(body);
    if (!document.isObject() || !document.object().value(QStringLiteral("data")).isArray()) {
        return {false, QObject::tr("服务已连接，但 /models 返回格式不兼容，无法确认模型是否可用。")};
    }

    const QString requestedModel = modelId.trimmed();
    bool found = false;
    for (const QJsonValue &value : document.object().value(QStringLiteral("data")).toArray()) {
        const QString availableModel = value.toObject().value(QStringLiteral("id")).toString().trimmed();
        if (availableModel == requestedModel ||
            availableModel.section(QLatin1Char('/'), -1) == requestedModel.section(QLatin1Char('/'), -1)) {
            found = true;
            break;
        }
    }
    if (!found) {
        return {false, QObject::tr("服务连接和 API Key 验证成功，但模型“%1”不在服务返回的可用模型列表中。请检查模型 ID。").arg(requestedModel)};
    }

    return {true, QObject::tr("模型“%1”检测成功，可以正常访问。\n基础 URL：%2")
                              .arg(requestedModel, normalizedBaseUrl)};
}

class PlaceholderLineEdit : public QLineEdit
{
public:
    explicit PlaceholderLineEdit(QWidget *parent = nullptr)
        : QLineEdit(parent)
    {
        QLineEdit::setPlaceholderText(QString());
    }

    void setGrayPlaceholderText(const QString &text)
    {
        m_placeholderText = text;
        QLineEdit::setPlaceholderText(QString());
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QLineEdit::paintEvent(event);
        if (!text().isEmpty() || m_placeholderText.isEmpty()) {
            return;
        }

        QStyleOptionFrame option;
        initStyleOption(&option);
        QRect contents = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        contents.adjust(2, 0, -2, 0);

        QPainter painter(this);
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        const QString elided = fontMetrics().elidedText(m_placeholderText, Qt::ElideRight, contents.width());
        painter.drawText(contents, alignment() | Qt::AlignVCenter, elided);
    }

private:
    QString m_placeholderText;
};

class SearchLineEdit : public QLineEdit
{
public:
    explicit SearchLineEdit(QWidget *parent = nullptr)
        : QLineEdit(parent)
    {
        QLineEdit::setPlaceholderText(QString());
    }

    void setGrayPlaceholderText(const QString &text)
    {
        m_placeholderText = text;
        QLineEdit::setPlaceholderText(QString());
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QLineEdit::paintEvent(event);
        if (!text().isEmpty() || m_placeholderText.isEmpty()) {
            return;
        }

        QStyleOptionFrame option;
        initStyleOption(&option);
        QRect contents = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        contents.adjust(2, 0, -2, 0);

        QPainter painter(this);
        painter.setPen(QColor(QStringLiteral("#8a8a8a")));
        const QString elided = fontMetrics().elidedText(m_placeholderText, Qt::ElideRight, contents.width());
        painter.drawText(contents, alignment() | Qt::AlignVCenter, elided);
    }

private:
    QString m_placeholderText;
};

void clearLayout(QLayout *layout) {
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout);
            childLayout->deleteLater();
        }
        delete item;
    }
}

bool confirmAction(QWidget *parent, const QString &title, const QString &message, const QString &acceptText)
{
    kdk::KMessageBox dialog(parent);
    dialog.setIcon(kdk::KMessageBox::Question);
    dialog.setWindowTitle(title);
    dialog.setText(message);
    QPushButton *acceptButton = dialog.addButton(acceptText, kdk::KMessageBox::AcceptRole);
    dialog.addButton(QObject::tr("取消"), kdk::KMessageBox::RejectRole);
    dialog.exec();
    return dialog.clickedButton() == acceptButton;
}

bool ensureParentDir(const QString &filePath) {
    QDir dir = QFileInfo(filePath).absoluteDir();
    return dir.exists() || dir.mkpath(QStringLiteral("."));
}

QString normalizedModelBaseUrl(QString baseUrl)
{
    baseUrl = baseUrl.trimmed().toLower();
    if (!baseUrl.isEmpty() && !baseUrl.contains(QStringLiteral("://"))) {
        baseUrl.prepend(QStringLiteral("http://"));
    }
    while (baseUrl.endsWith(QLatin1Char('/'))) {
        baseUrl.chop(1);
    }
    return baseUrl;
}

QString normalizedModelId(QString model, const QString &baseUrl)
{
    Q_UNUSED(baseUrl)
    model = model.trimmed().toLower();
    return model;
}

QString modelDedupeKey(const QString &provider, const QString &baseUrl, const QString &model,
                       const QString &name = QString())
{
    Q_UNUSED(provider)
    const QString normalizedBaseUrl = normalizedModelBaseUrl(baseUrl);
    const QString normalizedModel = normalizedModelId(model, normalizedBaseUrl);
    const QString normalizedName = name.trimmed().isEmpty()
        ? normalizedModel : name.trimmed().toLower();
    return normalizedBaseUrl + QLatin1Char('\n') + normalizedModel + QLatin1Char('\n') + normalizedName;
}

QString normalizedInputBaseUrl(QString baseUrl)
{
    baseUrl = baseUrl.trimmed();
    if (!baseUrl.isEmpty() && !baseUrl.contains(QStringLiteral("://"))) {
        baseUrl.prepend(QStringLiteral("http://"));
    }
    while (baseUrl.endsWith(QLatin1Char('/'))) {
        baseUrl.chop(1);
    }
    return baseUrl;
}

bool isGenericModelProvider(const QString &provider)
{
    const QString p = provider.trimmed().toLower();
    return p.isEmpty() ||
           p == QStringLiteral("custom") ||
           p == QStringLiteral("local") ||
           p == QStringLiteral("local / custom") ||
           p == QStringLiteral("local/custom");
}

int modelPreferenceScore(const QString &provider, const QString &name, int contextLength)
{
    const QString p = provider.trimmed().toLower();
    const QString n = name.trimmed().toLower();
    const bool genericProvider = isGenericModelProvider(p);
    int score = genericProvider ? 0 : 20;
    if (!n.isEmpty() &&
        n != QStringLiteral("custom") &&
        n != QStringLiteral("local") &&
        n != QStringLiteral("local / custom") &&
        n != QStringLiteral("local/custom") &&
        !n.startsWith(QStringLiteral("custom - ")) &&
        !n.startsWith(QStringLiteral("local / custom - ")) &&
        !n.startsWith(QStringLiteral("local/custom - ")) &&
        n != QStringLiteral("model")) {
        score += 2;
    }
    if (contextLength > 0) {
        score += 1;
    }
    return score;
}

QString comparableModelText(QString value)
{
    value = value.trimmed().toLower();
    value.remove(QRegularExpression(QStringLiteral("[^a-z0-9]+")));
    return value;
}

QString cleanModelDisplayName(const QString &name, const QString &modelId)
{
    const QString cleanName = name.trimmed();
    const QString cleanModelId = modelId.trimmed();
    if (cleanName.isEmpty()) {
        return cleanModelId;
    }

    const QString separator = QStringLiteral(" - ");
    const int separatorIndex = cleanName.indexOf(separator);
    if (separatorIndex > 0) {
        const QString left = cleanName.left(separatorIndex).trimmed();
        const QString right = cleanName.mid(separatorIndex + separator.size()).trimmed();
        if (!left.isEmpty() && !right.isEmpty() &&
            comparableModelText(left) == comparableModelText(right)) {
            return cleanModelId.isEmpty() ? right : cleanModelId;
        }
    }

    return cleanName;
}

QString fixedLocalModelBaseUrl()
{
    return QStringLiteral("http://127.0.0.1:18080/v1");
}

QString fixedLocalModelProviderName(const QString &modelId)
{
    QString slug = modelId.trimmed().toLower();
    slug.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    while (slug.startsWith(QLatin1Char('-'))) {
        slug.remove(0, 1);
    }
    while (slug.endsWith(QLatin1Char('-'))) {
        slug.chop(1);
    }
    return QStringLiteral("kylin-local-%1").arg(slug.isEmpty() ? QStringLiteral("model") : slug);
}

QString fixedLocalModelRootPath()
{
    return QDir(QDir::homePath()).filePath(QStringLiteral(".local/share/kylin-agent/models"));
}

QString fixedLocalModelPath(const QString &modelId)
{
    const QString currentPath = QDir(fixedLocalModelRootPath()).filePath(modelId);
    if (QDir(currentPath).exists()) {
        return currentPath;
    }
    const QString legacyPath = QDir(QDir::homePath()).filePath(
        QStringLiteral("models/%1").arg(modelId));
    return QDir(legacyPath).exists() ? legacyPath : currentPath;
}

QString providerDefaultBaseUrl(const QString &provider)
{
    if (provider == QStringLiteral("custom")) {
        return QStringLiteral("http://127.0.0.1:18080/v1");
    }
    if (provider == QStringLiteral("aliyun-bailian")) {
        return QStringLiteral("https://dashscope.aliyuncs.com/compatible-mode/v1");
    }
    if (provider == QStringLiteral("baidu-qianfan")) {
        return QStringLiteral("https://qianfan.baidubce.com/v2");
    }
    if (provider == QStringLiteral("volcengine-ark")) {
        return QStringLiteral("https://ark.cn-beijing.volces.com/api/v3");
    }
    if (provider == QStringLiteral("iflytek-spark")) {
        return QStringLiteral("https://spark-api-open.xf-yun.com/v1");
    }
    if (provider == QStringLiteral("moonshot")) {
        return QStringLiteral("https://api.moonshot.cn/v1");
    }
    if (provider == QStringLiteral("minimax")) {
        return QStringLiteral("https://api.minimax.chat/v1");
    }
    if (provider == QStringLiteral("deepseek")) {
        return QStringLiteral("https://api.deepseek.com/v1");
    }
    if (provider == QStringLiteral("zhipu")) {
        return QStringLiteral("https://open.bigmodel.cn/api/paas/v4");
    }
    return QString();
}

QString providerForBaseUrl(const QString &baseUrl)
{
    const QString normalized = normalizedModelBaseUrl(baseUrl);
    const QStringList providers = {
        QStringLiteral("aliyun-bailian"),
        QStringLiteral("baidu-qianfan"),
        QStringLiteral("volcengine-ark"),
        QStringLiteral("iflytek-spark"),
        QStringLiteral("moonshot"),
        QStringLiteral("minimax"),
        QStringLiteral("deepseek"),
        QStringLiteral("zhipu")
    };
    for (const QString &provider : providers) {
        if (normalized == normalizedModelBaseUrl(providerDefaultBaseUrl(provider))) {
            return provider;
        }
    }
    if (normalized == normalizedModelBaseUrl(QStringLiteral("https://api.deepseek.com"))) {
        return QStringLiteral("deepseek");
    }
    return QString();
}

bool fixedLocalModelAvailable(const QString &modelId)
{
    const QString modelPath = fixedLocalModelPath(modelId);
    const QFileInfo modelFile(QDir(modelPath).filePath(QStringLiteral("model.gguf")));
    return QDir(modelPath).exists() && modelFile.isFile() && modelFile.size() > 0;
}

QString humanFileSize(qint64 bytes)
{
    static const QStringList units = {
        QStringLiteral("B"),
        QStringLiteral("KB"),
        QStringLiteral("MB"),
        QStringLiteral("GB"),
        QStringLiteral("TB")
    };
    double value = static_cast<double>(qMax<qint64>(0, bytes));
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < units.size() - 1) {
        value /= 1024.0;
        ++unitIndex;
    }
    const int precision = unitIndex <= 1 ? 0 : 1;
    return QStringLiteral("%1 %2").arg(QString::number(value, 'f', precision), units.at(unitIndex));
}

} // namespace

ModelSettingsWidget::ModelSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_searchEdit(nullptr),
      m_cardsWidget(nullptr),
      m_cardsLayout(nullptr),
      m_emptyLabel(nullptr),
      m_downloadRefreshTimer(new QTimer(this)) {
    createModelGroup();
    connect(&ModelDownloadService::instance(), &ModelDownloadService::downloadChanged,
            this, [this](const QString &) {
                if (ModelDownloadService::instance().hasActiveDownloads()) {
                    m_downloadRefreshTimer->start();
                } else {
                    m_downloadRefreshTimer->stop();
                }
                refreshModels();
            });
    m_downloadRefreshTimer->setInterval(1000);
    connect(m_downloadRefreshTimer, &QTimer::timeout, this, [this]() {
        if (ModelDownloadService::instance().hasActiveDownloads()) {
            updateCards();
        } else {
            m_downloadRefreshTimer->stop();
        }
    });
    refreshModels();
    if (ModelDownloadService::instance().hasActiveDownloads()) {
        m_downloadRefreshTimer->start();
    }
}

void ModelSettingsWidget::createModelGroup() {
    setObjectName(QStringLiteral("modelSettingsWidget"));
    setMinimumWidth(520);
    m_mainLayout->setContentsMargins(28, 24, 28, 28);
    m_mainLayout->setSpacing(18);

    setStyleSheet(R"(
        #modelSettingsWidget { background: palette(window); color: palette(window-text); }
        QLabel#pageTitle { font-size: 22px; font-weight: 700; color: palette(window-text); }
        QLabel#pageSubtitle { font-size: 14px; color: #555555; }
        QPushButton#primaryButton {
            background: #214a85;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 15px;
            font-weight: 700;
        }
        QPushButton#primaryButton:hover { background: #183b6d; }
        QLineEdit#searchEdit {
            background: palette(base);
            border: 1px solid palette(mid);
            border-radius: 8px;
            padding: 11px 14px;
            font-size: 14px;
            color: palette(text);
        }
        QLineEdit#searchEdit:focus { border-color: #8aa9d6; }
        QFrame#modelCard {
            background: palette(base);
            border: 1px solid palette(mid);
            border-radius: 8px;
        }
        QFrame#modelCard:hover { background: palette(midlight); border-color: palette(mid); }
        QLabel#modelName {
            background: transparent;
            color: palette(window-text);
            font-size: 15px;
            font-weight: 700;
        }
        QLabel#modelProvider {
            color: #214a85;
            background: #edf2f8;
            border-radius: 10px;
            padding: 3px 9px;
            font-size: 11px;
            font-weight: 700;
        }
        QLabel#modelId {
            background: transparent;
            color: #404040;
            font-size: 13px;
        }
        QLabel#modelUrl {
            background: transparent;
            color: #909090;
            font-size: 12px;
        }
        QLabel#modelDownloadStats {
            background: transparent;
            color: palette(text);
            font-size: 12px;
            font-weight: 600;
        }
        QPushButton#modelActionButton {
            border: none;
            background: transparent;
            color: #111111;
            font-size: 12px;
            font-weight: 600;
            padding: 2px 6px;
        }
        QPushButton#modelActionButton:hover {
            color: #214a85;
            background: #e8f0fe;
            border-radius: 6px;
        }
        QPushButton#modelDeleteButton {
            border: none;
            background: transparent;
            color: #9b9b9b;
            font-size: 12px;
            font-weight: 600;
            padding: 2px 6px;
        }
        QPushButton#modelDeleteButton:hover {
            color: #b3261e;
            background: #fce8e6;
            border-radius: 6px;
        }
        QProgressBar#modelDownloadProgress {
            border: 1px solid palette(mid);
            border-radius: 4px;
            background: palette(base);
            height: 8px;
            max-height: 8px;
            min-width: 140px;
            text-align: center;
        }
        QProgressBar#modelDownloadProgress::chunk {
            background: #214a85;
            border-radius: 3px;
        }
        QLabel#emptyLabel { color: #777777; font-size: 14px; padding: 40px; }
    )");

    QHBoxLayout *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(12);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(8);

    QLabel *title = new QLabel(tr("模型"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    titleLayout->addWidget(title);

    QLabel *subtitle = new QLabel(tr("管理您的模型库。这些模型会出现在聊天的模型选择器中。"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    titleLayout->addWidget(subtitle);

    header->addLayout(titleLayout, 1);

    QPushButton *addButton = new QPushButton(tr("+ 添加模型"), this);
    addButton->setObjectName(QStringLiteral("primaryButton"));
    addButton->setCursor(Qt::PointingHandCursor);
    connect(addButton, &QPushButton::clicked, this, [this]() { showModelDialog(); });
    header->addWidget(addButton, 0, Qt::AlignTop);
    m_mainLayout->addLayout(header);

    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("searchEdit"));
    static_cast<SearchLineEdit *>(m_searchEdit)->setGrayPlaceholderText(tr("搜索模型..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this]() { updateCards(); });
    m_mainLayout->addWidget(m_searchEdit);

    m_cardsWidget = new QWidget(this);
    m_cardsLayout = new QGridLayout(m_cardsWidget);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setHorizontalSpacing(12);
    m_cardsLayout->setVerticalSpacing(12);
    m_mainLayout->addWidget(m_cardsWidget);

    m_emptyLabel = new QLabel(this);
    m_emptyLabel->setObjectName(QStringLiteral("emptyLabel"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_emptyLabel);
    m_mainLayout->addStretch(1);
}

void ModelSettingsWidget::refreshModels() {
    m_models.clear();

    const QVector<SavedModel> savedModels = readModels();
    for (const SavedModel &saved : savedModels) {
        SavedModel model;
        model = saved;
        if (!model.model.isEmpty()) {
            if (model.name.isEmpty()) {
                model.name = model.model;
            }
            m_models.append(model);
        }
    }

    updateCards();
}

void ModelSettingsWidget::updateCards() {
    clearLayout(m_cardsLayout);

    const QString query = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : QString();
    QVector<SavedModel> filtered;
    for (const SavedModel &model : qAsConst(m_models)) {
        if (query.isEmpty() ||
            model.name.toLower().contains(query) ||
            model.model.toLower().contains(query) ||
            model.provider.toLower().contains(query) ||
            model.baseUrl.toLower().contains(query)) {
            filtered.append(model);
        }
    }

    const bool hasCards = !filtered.isEmpty();
    m_cardsWidget->setVisible(hasCards);
    m_emptyLabel->setVisible(!hasCards);
    if (!hasCards) {
        m_emptyLabel->setText(m_models.isEmpty() ? tr("暂无模型") : tr("没有匹配的模型"));
        return;
    }

    const int columns = columnCountForWidth(width());
    m_lastColumnCount = columns;
    for (int i = 0; i < filtered.size(); ++i) {
        const SavedModel model = filtered.at(i);
        QFrame *card = new QFrame(m_cardsWidget);
        card->setObjectName(QStringLiteral("modelCard"));
        card->setMinimumHeight(118);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 14, 16, 12);
        cardLayout->setSpacing(6);

        QHBoxLayout *top = new QHBoxLayout;
        QLabel *name = new QLabel(model.name, card);
        name->setObjectName(QStringLiteral("modelName"));
        name->setWordWrap(true);
        top->addWidget(name, 1);
        QLabel *provider = new QLabel(providerLabel(model.provider).toUpper(), card);
        provider->setObjectName(QStringLiteral("modelProvider"));
        top->addWidget(provider, 0, Qt::AlignTop);
        cardLayout->addLayout(top);

        QLabel *modelId = new QLabel(model.model, card);
        modelId->setObjectName(QStringLiteral("modelId"));
        modelId->setWordWrap(true);
        cardLayout->addWidget(modelId);

        QLabel *url = new QLabel(model.baseUrl, card);
        url->setObjectName(QStringLiteral("modelUrl"));
        url->setWordWrap(true);
        cardLayout->addWidget(url);

        QHBoxLayout *footer = new QHBoxLayout;
        footer->addStretch(1);
        QPushButton *editButton = new QPushButton(tr("编辑"), card);
        editButton->setObjectName(QStringLiteral("modelActionButton"));
        connect(editButton, &QPushButton::clicked, this, [this, model]() { showModelDialog(&model); });
        footer->addWidget(editButton);
        if (!model.readOnly) {
            QPushButton *deleteButton = new QPushButton(tr("删除"), card);
            deleteButton->setObjectName(QStringLiteral("modelDeleteButton"));
            connect(deleteButton, &QPushButton::clicked, this, [this, model]() { deleteModel(model.id); });
            footer->addWidget(deleteButton);
        }
        cardLayout->addLayout(footer);

        m_cardsLayout->addWidget(card, i / columns, i % columns);
    }
    for (int col = 0; col < columns; ++col) {
        m_cardsLayout->setColumnStretch(col, 1);
    }
}

void ModelSettingsWidget::showModelDialog(const SavedModel *model) {
    QDialog dialog(this);
    dialog.setWindowTitle(model ? tr("编辑模型") : tr("添加模型"));
    QFormLayout *form = new QFormLayout(&dialog);
    QLineEdit *nameEdit = new QLineEdit(model ? model->name : QString(), &dialog);
    QLineEdit *providerEdit = new QLineEdit(model ? model->provider : QStringLiteral("custom"), &dialog);
    QLineEdit *modelEdit = new QLineEdit(model ? model->model : QString(), &dialog);
    QLineEdit *baseUrlEdit = new QLineEdit(model ? model->baseUrl : providerDefaultBaseUrl(QStringLiteral("custom")), &dialog);
    QLineEdit *apiKeyEdit = new QLineEdit(model ? model->apiKey : QString(), &dialog);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    form->addRow(tr("名称"), nameEdit);
    form->addRow(tr("供应商"), providerEdit);
    form->addRow(tr("模型 ID"), modelEdit);
    form->addRow(tr("基础 URL"), baseUrlEdit);
    form->addRow(tr("API Key"), apiKeyEdit);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    SavedModel next = model ? *model : SavedModel();
    next.id = next.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : next.id;
    next.name = nameEdit->text().trimmed();
    next.provider = providerEdit->text().trimmed();
    next.model = modelEdit->text().trimmed();
    next.baseUrl = normalizedInputBaseUrl(baseUrlEdit->text());
    next.apiKey = apiKeyEdit->text();
    next.createdAt = next.createdAt > 0 ? next.createdAt : QDateTime::currentMSecsSinceEpoch();
    if (next.name.isEmpty()) {
        next.name = next.model;
    }
    if (next.model.isEmpty() || next.baseUrl.isEmpty()) {
        kdk::KMessageBox::warning(this, tr("模型配置"), tr("模型 ID 和基础 URL 不能为空。"));
        return;
    }

    bool replaced = false;
    for (SavedModel &item : m_models) {
        if (item.id == next.id) {
            item = next;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        m_models.append(next);
    }
    saveSettings();
    refreshModels();
}

QVector<ModelSettingsWidget::SavedModel> ModelSettingsWidget::readModels() {
    QVector<SavedModel> models;
    const QVector<QMap<QString, QVariant>> rows = ApiService::instance().getConfigModelsSync();
    for (const QMap<QString, QVariant> &row : rows) {
        SavedModel model;
        model.id = row.value(QStringLiteral("id")).toString();
        model.name = row.value(QStringLiteral("name")).toString();
        model.provider = row.value(QStringLiteral("provider")).toString();
        model.model = row.value(QStringLiteral("model")).toString();
        model.baseUrl = row.value(QStringLiteral("baseUrl")).toString();
        model.apiKey = row.value(QStringLiteral("apiKey")).toString();
        model.contextLength = row.value(QStringLiteral("contextLength")).toInt();
        model.createdAt = row.value(QStringLiteral("createdAt")).toLongLong();
        model.readOnly = row.value(QStringLiteral("readOnly")).toBool();
        if (model.baseUrl.isEmpty()) {
            model.baseUrl = row.value(QStringLiteral("base_url")).toString();
        }
        if (model.name.isEmpty()) {
            model.name = model.model;
        }
        if (!model.model.isEmpty()) {
            models.append(model);
        }
    }
    return models;
}

bool ModelSettingsWidget::writeModels(const QVector<SavedModel> &models) {
    QJsonArray array;
    for (const SavedModel &model : models) {
        if (model.readOnly) {
            continue;
        }
        QJsonObject object;
        object.insert(QStringLiteral("id"), model.id);
        object.insert(QStringLiteral("name"), model.name);
        object.insert(QStringLiteral("provider"), model.provider);
        object.insert(QStringLiteral("model"), model.model);
        object.insert(QStringLiteral("baseUrl"), model.baseUrl);
        object.insert(QStringLiteral("apiKey"), model.apiKey);
        object.insert(QStringLiteral("contextLength"), model.contextLength);
        object.insert(QStringLiteral("createdAt"), static_cast<double>(model.createdAt));
        array.append(object);
    }
    return ApiService::instance().replaceConfigModelsSync(array);
}

void ModelSettingsWidget::deleteModel(const QString &id) {
    m_models.erase(std::remove_if(m_models.begin(), m_models.end(),
                                  [&id](const SavedModel &model) { return model.id == id; }),
                   m_models.end());
    saveSettings();
    refreshModels();
}

void ModelSettingsWidget::openAddModelDialog() {
    showModelDialog();
}

void ModelSettingsWidget::loadSettings() {
    refreshModels();
}

void ModelSettingsWidget::saveSettings() {
    if (!writeModels(m_models)) {
        kdk::KMessageBox::warning(this, tr("保存失败"), tr("模型配置保存失败。"));
    }
}

void ModelSettingsWidget::seedDefaultModels() {
    if (m_models.isEmpty()) {
        refreshModels();
    }
}

int ModelSettingsWidget::columnCountForWidth(int width) const {
    return width >= 760 ? 2 : 1;
}

QString ModelSettingsWidget::providerLabel(const QString &provider) const {
    if (provider.isEmpty() || provider == QStringLiteral("custom")) {
        return tr("本地 / 自定义");
    }
    return provider;
}

void ModelSettingsWidget::downloadFixedModel(const SavedModel &model) {
    Q_UNUSED(model)
}

void ModelSettingsWidget::removeFixedModel(const SavedModel &model) {
    deleteModel(model.id);
}

void ModelSettingsWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    const int columns = columnCountForWidth(event->size().width());
    if (m_lastColumnCount != 0 && columns != m_lastColumnCount) {
        updateCards();
    }
}
