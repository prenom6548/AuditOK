#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
#include <ukui_previewhub_api.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QMainWindow win;
    win.setWindowTitle(QStringLiteral("ukui-previewhub-test"));

    auto *central = new QWidget(&win);
    auto *mainLayout = new QVBoxLayout(central);

    auto *bar = new QHBoxLayout();
    auto *pickBtn = new QPushButton(QStringLiteral("选择文件…"), central);
    auto *pathLabel = new QLabel(QStringLiteral("未选择文件，请点击上方按钮。"), central);
    pathLabel->setWordWrap(true);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bar->addWidget(pickBtn);
    bar->addWidget(pathLabel, 1);
    mainLayout->addLayout(bar);

    QWidget *preview = UkuiPreviewHub::createPreviewWidget(central);
    mainLayout->addWidget(preview, 1);

    const auto loadUri = [&](const QString &uri, const QString &displayPath) {
        pathLabel->setText(displayPath);
        UkuiPreviewHub::cancelPreview(preview);
        UkuiPreviewHub::preparePreview(preview, uri);
        UkuiPreviewHub::startPreview(preview);
    };

    QObject::connect(pickBtn, &QPushButton::clicked, &win, [&]() {
        const QString path = QFileDialog::getOpenFileName(
            &win,
            QStringLiteral("选择要预览的文件"));
        if (path.isEmpty())
            return;
        loadUri(QUrl::fromLocalFile(path).toString(), path);
    });

    win.setCentralWidget(central);
    win.resize(800, 600);
    win.show();

    if (argc > 1) {
        const QString path = QString::fromUtf8(argv[1]);
        loadUri(QUrl::fromLocalFile(path).toString(), path);
    }

    return app.exec();
}
