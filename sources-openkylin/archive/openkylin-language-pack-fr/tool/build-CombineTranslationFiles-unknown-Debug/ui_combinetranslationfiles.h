/********************************************************************************
** Form generated from reading UI file 'combinetranslationfiles.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMBINETRANSLATIONFILES_H
#define UI_COMBINETRANSLATIONFILES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CombineTranslationFiles
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_6;
    QWidget *titleWidget;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_5;
    QWidget *titleWidgetInner;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *helpPushButton;
    QSpacerItem *horizontalSpacer_8;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_3;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLineEdit *sourceFile;
    QPushButton *sourceFilePushButton;
    QPushButton *sourceFilePushButton_2;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *targetFile;
    QPushButton *targetFilePushButton;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label;
    QLabel *language;
    QTextBrowser *textBrowser;
    QDialogButtonBox *buttonBox;
    QSpacerItem *horizontalSpacer_4;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QTextBrowser *textBrowser_2;

    void setupUi(QMainWindow *CombineTranslationFiles)
    {
        if (CombineTranslationFiles->objectName().isEmpty())
            CombineTranslationFiles->setObjectName(QString::fromUtf8("CombineTranslationFiles"));
        CombineTranslationFiles->resize(1111, 718);
        QFont font;
        font.setPointSize(10);
        CombineTranslationFiles->setFont(font);
        CombineTranslationFiles->setStyleSheet(QString::fromUtf8("#titleWidget{background-color:white;}"));
        centralWidget = new QWidget(CombineTranslationFiles);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_6 = new QVBoxLayout(centralWidget);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        titleWidget = new QWidget(centralWidget);
        titleWidget->setObjectName(QString::fromUtf8("titleWidget"));
        titleWidget->setStyleSheet(QString::fromUtf8(""));
        horizontalLayout_6 = new QHBoxLayout(titleWidget);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);

        titleWidgetInner = new QWidget(titleWidget);
        titleWidgetInner->setObjectName(QString::fromUtf8("titleWidgetInner"));
        titleWidgetInner->setMinimumSize(QSize(700, 0));
        titleWidgetInner->setMaximumSize(QSize(700, 16777215));
        titleWidgetInner->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_4 = new QVBoxLayout(titleWidgetInner);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(titleWidgetInner);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        horizontalLayout_4->addWidget(label_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        helpPushButton = new QPushButton(titleWidgetInner);
        helpPushButton->setObjectName(QString::fromUtf8("helpPushButton"));
        helpPushButton->setFont(font);
        helpPushButton->setStyleSheet(QString::fromUtf8("padding:0.5em;"));
        helpPushButton->setCheckable(true);

        horizontalLayout_4->addWidget(helpPushButton);


        verticalLayout_4->addLayout(horizontalLayout_4);


        horizontalLayout_6->addWidget(titleWidgetInner);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_8);


        verticalLayout_6->addWidget(titleWidget);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);

        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setMaximumSize(QSize(700, 16777215));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        widget_2 = new QWidget(widget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        widget_2->setMinimumSize(QSize(700, 0));
        widget_2->setMaximumSize(QSize(700, 16777215));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        sourceFile = new QLineEdit(widget_2);
        sourceFile->setObjectName(QString::fromUtf8("sourceFile"));
        sourceFile->setFont(font);
        sourceFile->setStyleSheet(QString::fromUtf8("padding:0.5em;"));

        horizontalLayout->addWidget(sourceFile);

        sourceFilePushButton = new QPushButton(widget_2);
        sourceFilePushButton->setObjectName(QString::fromUtf8("sourceFilePushButton"));
        sourceFilePushButton->setFont(font);
        sourceFilePushButton->setStyleSheet(QString::fromUtf8("padding:0.5em;"));

        horizontalLayout->addWidget(sourceFilePushButton);

        sourceFilePushButton_2 = new QPushButton(widget_2);
        sourceFilePushButton_2->setObjectName(QString::fromUtf8("sourceFilePushButton_2"));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        sourceFilePushButton_2->setFont(font1);
        sourceFilePushButton_2->setStyleSheet(QString::fromUtf8("padding:0.5em;"));

        horizontalLayout->addWidget(sourceFilePushButton_2);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        targetFile = new QLineEdit(widget_2);
        targetFile->setObjectName(QString::fromUtf8("targetFile"));
        targetFile->setFont(font);
        targetFile->setStyleSheet(QString::fromUtf8("padding:0.5em;"));

        horizontalLayout_2->addWidget(targetFile);

        targetFilePushButton = new QPushButton(widget_2);
        targetFilePushButton->setObjectName(QString::fromUtf8("targetFilePushButton"));
        targetFilePushButton->setFont(font);
        targetFilePushButton->setStyleSheet(QString::fromUtf8("padding:0.5em;"));

        horizontalLayout_2->addWidget(targetFilePushButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        label = new QLabel(widget_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        language = new QLabel(widget_2);
        language->setObjectName(QString::fromUtf8("language"));
        language->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(language);


        verticalLayout_3->addLayout(horizontalLayout_3);

        textBrowser = new QTextBrowser(widget_2);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setFont(font);

        verticalLayout_3->addWidget(textBrowser);

        buttonBox = new QDialogButtonBox(widget_2);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStyleSheet(QString::fromUtf8("padding:0.5em; font-size:10pt;"));
        buttonBox->setStandardButtons(QDialogButtonBox::Apply);

        verticalLayout_3->addWidget(buttonBox);


        verticalLayout->addWidget(widget_2);


        horizontalLayout_5->addWidget(widget);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);


        verticalLayout_6->addLayout(horizontalLayout_5);

        CombineTranslationFiles->setCentralWidget(centralWidget);
        dockWidget = new QDockWidget(CombineTranslationFiles);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        dockWidget->setMinimumSize(QSize(400, 78));
        dockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable);
        dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        textBrowser_2 = new QTextBrowser(dockWidgetContents);
        textBrowser_2->setObjectName(QString::fromUtf8("textBrowser_2"));
        QFont font2;
        font2.setPointSize(8);
        textBrowser_2->setFont(font2);
        textBrowser_2->setFrameShape(QFrame::NoFrame);
        textBrowser_2->setFrameShadow(QFrame::Plain);

        verticalLayout_2->addWidget(textBrowser_2);

        dockWidget->setWidget(dockWidgetContents);
        CombineTranslationFiles->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);

        retranslateUi(CombineTranslationFiles);

        QMetaObject::connectSlotsByName(CombineTranslationFiles);
    } // setupUi

    void retranslateUi(QMainWindow *CombineTranslationFiles)
    {
        CombineTranslationFiles->setWindowTitle(QApplication::translate("CombineTranslationFiles", "Merge Qt .ts files", nullptr));
        label_2->setText(QApplication::translate("CombineTranslationFiles", "<html><head/><body><p><span style=\" font-weight:600;\">Combine multiple Qt .ts files into one<br/></span><a href=\"https://github.com/savolai/Merge-Qt-TS-files/\"><span style=\" text-decoration: underline; color:#0000ff;\">https://github.com/savolai/Merge-Qt-TS-files/</span></a></p></body></html>", nullptr));
        helpPushButton->setText(QApplication::translate("CombineTranslationFiles", "Show Help", nullptr));
        sourceFile->setPlaceholderText(QApplication::translate("CombineTranslationFiles", "Source file for translations", nullptr));
        sourceFilePushButton->setText(QApplication::translate("CombineTranslationFiles", "Browse source...", nullptr));
        sourceFilePushButton_2->setText(QApplication::translate("CombineTranslationFiles", "Read in selected file", nullptr));
        targetFile->setPlaceholderText(QApplication::translate("CombineTranslationFiles", "File to save translations in", nullptr));
        targetFilePushButton->setText(QApplication::translate("CombineTranslationFiles", "Browse target...", nullptr));
        label->setText(QApplication::translate("CombineTranslationFiles", "Language of last read file:", nullptr));
        language->setText(QApplication::translate("CombineTranslationFiles", "[none]", nullptr));
        dockWidget->setWindowTitle(QApplication::translate("CombineTranslationFiles", "Help", nullptr));
        textBrowser_2->setHtml(QApplication::translate("CombineTranslationFiles", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8pt; font-weight:400; font-style:normal;\" bgcolor=\"#ffffff\">\n"
"<p style=\" margin-top:10px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\">Usage: </span></p>\n"
"<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:400;\">Open each source file by pressing &quot;Browse"
                        " source...&quot;. After doing that, click &quot;Read in selected file&quot;. You will get a count of translations loaded into memory.</span></li>\n"
"<li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\">Repeat step 1 for each file you want to combine</li>\n"
"<li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\">After reading into memory all your translation data, select target file by pressing &quot;Browse target...&quot;. After selecting filename to save to, press &quot;Write translations to file&quot;. </li></ol>\n"
"<p style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'"
                        "; font-size:10pt; font-weight:600;\">This app is designed to preserve as much as possible of the information in the translation files. </span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">When combining multiple translation files, this makes sure translations with empty content never override translations with existing content.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">If multiple of your source files contain conflicting, nonemp"
                        "ty, translations for the same translation units (i.e classname+source text content), make sure to load the file that has the translations you want, last.</span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">Caveat: This tool does NOT currently include </span><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\">multiple</span><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"> &lt;location&gt; elements if many exist in source translation units, only one of them is included for each translation. Mostly this never makes any difference, since a single run of lupdate project1.pro project2.pro -ts projects.ts (where projects.ts is the output file of this program) will put all the additional locations back again. </span></p>\n"
"<p style=\" margin-top:12px; m"
                        "argin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\">Background</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">This is useful when you have multiple applications</span></p>\n"
"<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\">that have language files of their own</li>\n"
"<li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right"
                        ":10px; -qt-block-indent:0; text-indent:0px;\">that statically link to common libraries that also have translations</li></ol>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">    </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\">In this situation, overlaps in translation files can cause strings not getting translated in the UI.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">Also, Qt docs don't exactly warn about this situation, and also it tends to creep up on you; I found myself in"
                        " a situation with hundreds of translation spread across multiple translation files that should have been one. So I wrote this to fix it.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt; font-weight:600;\">The solution is to produce a common translation file for all the apps. </span><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">First run your ts files through this app, combining translations. Then run lupdate like below - this updates the location tags of the translations to point to the right place(s).</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px"
                        "; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">After that, you can normally run lrelease on your ts file to produce .qm and load it from all your apps.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">Thanks @David Faure for pointing me in the right direction.</span>"
                        "</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">Another option might be to load first the app specific language file and then on top of that, the translations for the common components. This occured to me too late. \\o/ however, having all project translations in a single file tends to be a cleaner solution overall, as you will always know what has already been translated.</span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\"><a name=\"js-repo-pjax-container\"></a><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dl"
                        "g 2'; font-size:10pt;\">O</span><span style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\">ther options that might help:</span></p>\n"
"<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\"><li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:12px; margin-bottom:0px; margin-left:10px; margin-right:10px; -qt-block-indent:0; text-indent:0px;\">When loading translation files into your app, first load the app specific language file and then on top of that, the translations for the common components. This occured to me too late. \\o/ however, having all project translations in a single file tends to be a cleaner solution overall, as you will always know what has already been translated.</li>\n"
"<li style=\" font-family:'Segoe UI,Arial,Helvetica,MS Shell Dlg 2'; font-size:10pt;\" style=\" margin-top:0px; margin-bottom:12px; margin-left:10px; margin-right:10px; -qt-block-indent:0; t"
                        "ext-indent:0px;\">Using an XML merging tool such as those mentioned <a href=\"http://stackoverflow.com/questions/80609/merge-xml-documents\"><span style=\" text-decoration: underline; color:#0000ff;\">in this StackOverflow post</span></a> might also work, but I enjoyed making this and emh, now we have a GUI :)</li></ol></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CombineTranslationFiles: public Ui_CombineTranslationFiles {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMBINETRANSLATIONFILES_H
