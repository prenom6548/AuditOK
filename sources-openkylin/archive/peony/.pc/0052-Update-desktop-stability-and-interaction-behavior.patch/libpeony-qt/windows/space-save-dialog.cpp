#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include "space-save-dialog.h"
#include "kysdk/applications/kswitchbutton.h"
#include "kysdk/applications/ktag.h"
#include "flowlayout.h"
#include "idm-dbus-manager.h"
#include "idm-actions-manager.h"
#include "search-bar-container.h"
#include <kmessagebox.h>
#include <QApplication>
#include <QMessageBox>
SpaceSaveDialog::SpaceSaveDialog(const QStringList &paths, const QString &conditions, const QString &keyWord, QWidget *parent)
    : m_paths(paths), m_conditions(conditions), m_keyWord(keyWord), QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::Dialog);
    setWindowIcon(QIcon::fromTheme("folder-space"));
    setWindowTitle(tr("Save as space"));
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 10, 24, 20);
    mainLayout->setSpacing(16);

    // Info Banner
    QFrame *infoBanner = new QFrame(this);
    infoBanner->setFixedHeight(56);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoBanner);
    infoLayout->setContentsMargins(12, 8, 12, 8);
    infoLayout->setSpacing(8);
    infoBanner->setFrameShape(QFrame::Shape::StyledPanel);
    infoBanner->setStyleSheet("QFrame { background-color: rgba(0, 0, 0, 0.03); border-radius: 6px; border: none; }");

    QLabel *warningIcon = new QLabel(infoBanner);
    warningIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
    warningIcon->setStyleSheet("QLabel { background-color: transparent; }");
    warningIcon->setAlignment(Qt::AlignVCenter);

    QLabel *infoText = new QLabel(tr("Intelligent Space does not support aggregating folders. Folders in search results will not be displayed in this space."), infoBanner);
    infoText->setStyleSheet("QLabel { background-color: transparent; }");
    infoText->setWordWrap(true);
    infoText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    infoLayout->addWidget(warningIcon);
    infoLayout->addWidget(infoText, 1);

    mainLayout->addWidget(infoBanner);

    mainLayout->addSpacing(30);
    // Space Name Label
    QLabel *nameLabel = new QLabel(tr("Space Name And Description"), this);
    QFont font = nameLabel->font();
    font.setBold(true);
    nameLabel->setFont(font);
    mainLayout->addWidget(nameLabel);

    // Name Input
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(0);

    QLineEdit *nameEdit = new QLineEdit(this);
    nameEdit->setFixedHeight(36);
    nameEdit->setPlaceholderText(tr("Please input space name"));
    nameEdit->setFocus();
    Peony::GradientLineEditStyle *lineStyle = new Peony::GradientLineEditStyle();
    nameEdit->setStyle(lineStyle);
    connect(nameEdit, &QLineEdit::textChanged, [=](const QString &currentText) {
        if (currentText.toUtf8().size() > 255) {
            QString truncatedText = truncateToByteLimit(currentText, 255);
            nameEdit->blockSignals(true);
            nameEdit->setText(truncatedText);
            nameEdit->blockSignals(false);
        }
    });

    inputLayout->addWidget(nameEdit);
    mainLayout->addLayout(inputLayout);

    // Dynamic Update Checkbox Row
    mainLayout->addSpacing(6);
    QHBoxLayout *checkLayout = new QHBoxLayout();
    checkLayout->setSpacing(10);

//    kdk::KSwitchButton *switchBtn = new kdk::KSwitchButton;
//    switchBtn->setChecked(true);
    QCheckBox *m_dynamicCheck = new QCheckBox(tr("realtime intelligent updates"), this);
    m_dynamicCheck->setChecked(true);
//    QLabel *checkLabel = new QLabel(tr("realtime intelligent updates"), this);
    QLabel *helpIcon = new QLabel(this);
    helpIcon->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(16, 16));
    helpIcon->setToolTip(tr("When enabled, the system continuously monitors for new files matching the above criteria and automatically adds them to this space."));
    checkLayout->addWidget(m_dynamicCheck);
    checkLayout->addWidget(helpIcon);
    checkLayout->addStretch();

    mainLayout->addLayout(checkLayout);

    // Button Row
    mainLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setFixedSize(96, 36);
    QPushButton *okBtn = new QPushButton(tr("Ok"), this);
    okBtn->setFixedSize(96, 36);
    okBtn->setDefault(true);
    okBtn->setAutoDefault(true);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);
    okBtn->setDefault(true);
    okBtn->setAutoDefault(true);
    okBtn->setCheckable(true);
    okBtn->setChecked(true);
    mainLayout->addLayout(buttonLayout);
    setFixedSize(QSize(466, 272));

    bool isCreatedFromSearch = keyWord.isEmpty() ? false : true;

    setLayout(mainLayout);
    QWidget *widget = QApplication::topLevelAt(QCursor::pos());
    connect(cancelBtn, &QPushButton::clicked, this, &SpaceSaveDialog::close);
    connect(okBtn, &QPushButton::clicked, this, [=](){
        bool isDymaic = m_dynamicCheck->isChecked();
        bool success = Peony::IDMDBusManager::getInstance()->onSpaceCreateRequestWithFiles(nameEdit->text(), m_conditions, isCreatedFromSearch, m_paths);
        qDebug() << "============" << success << m_conditions;
        close();
        if (success) {
            Peony::IDMDBusManager::getInstance()->updateSpacestatus(nameEdit->text(), isDymaic);
        }else{
            QMessageBox::warning(widget,"", tr("Failed to create space %1,%1 is existed").arg(nameEdit->text()), QMessageBox::Ok);
        }
    });
}

SpaceSaveDialog::~SpaceSaveDialog()
{

}
