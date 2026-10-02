#include <QtTest/QTest>
#include <QApplication>
#include "test_mainwindow.h"
#include "test_clipbutton.h"
#include "test_lineedit.h"
#include "test_audiofileprocess.h"
#include "test_mp3encoder.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    
    TestMainWindow testMainwindow;
    QTest::qExec(&testMainwindow);
    
    TestClipButton testClipButton;
    QTest::qExec(&testClipButton);
    
    TestLineEdit testLineEdit;
    QTest::qExec(&testLineEdit);
    
    TestAudioFileProcess testAudioFileProcess;
    QTest::qExec(&testAudioFileProcess);
    
    TestMp3Encoder testMp3Encoder;
    QTest::qExec(&testMp3Encoder);
    
//    TestMenuModule testMenuModule;
//    QTest::qExec(&testMenuModule);
    
    return 0;
}
