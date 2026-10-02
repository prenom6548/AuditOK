#include "prompt.h"
#include <QIcon>
#include <QApplication>
#include "utils.h"

namespace fault_diagnosis {

Prompt::Prompt(QWidget *parent) : QLabel(parent) {
    setWindowFlags(Qt::FramelessWindowHint);

    auto pixmap = QIcon::fromTheme("preferences-system-details-symbolic").pixmap(16, 16);
    setPixmap(K::Utils::ChangePixmapColor(pixmap, qApp->palette().color(QPalette::Text)));
}

Prompt::~Prompt() {}

void Prompt::SetPromptContent(QString content) {
    content_ = content;
    setToolTip(content);
}

}
