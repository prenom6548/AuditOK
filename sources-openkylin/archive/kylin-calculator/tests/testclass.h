#ifndef TESTCLASS_H
#define TESTCLASS_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtWidgets/QWidget>
#include <QSignalSpy>
#include <QApplication>
#include "../src/programmer/programkeyboary.h"
#include "../src/menumodule/menumodule.h"



class TestClass : public QObject
{
    Q_OBJECT
public:
    explicit TestClass(QObject *parent = nullptr);
    ~TestClass();

private Q_SLOTS:
    void testmainwindow();
    void testMainWindowEnhanced();
    void testBasicButton();
    void testOutputBase();
    void testScientificOutput();
    void testScientificModelEnhanced();
    void testStandardOutput();
    void testStandardModelEnhanced();
    void testTitleBar();
    void testProgramModel();
    void testProgramModelEnhanced();
    void testToolModelEnhanced();
void testProgramKeyboaryEnhanced();
    void testProgramKeyboary();
//     void testToolModelAdditional();
    void testScientificModelAdditional();
    void testProgramModelAdditional();
    void testStandardModelAdditional();
    void testProgramKeyboaryAdditional();
    void testMainWindowLowCoverageFunctions();
    void testToolModelLowCoverageFunctions();
    void testProgramKeyboaryLowCoverageFunctions();
    void testStandardModelLowCoverageFunctions();
    void testScientificModelLowCoverageFunctions();
    void testMainWindowAdditional();
    void testMainWindowAdditionalCoverage();
        void testToolModelAdditionalCoverage();
        void testScientificModelAdditionalCoverage();
        void testProgramKeyboaryAdditionalCoverage();
        void testProgramModelAdditionalCoverage();
        void testStandardModelAdditionalCoverage();
        void testMenuModuleAdditionalCoverage();
        void testOutputBaseAdditional();

    // Additional coverage improvement functions
    void testMainWindowCoverageImprovement();
    void testToolModelCoverageImprovement();
    void testScientificModelCoverageImprovement();
    void testProgramKeyboaryCoverageImprovement();
    void testProgramModelCoverageImprovement();
    void testStandardModelCoverageImprovement();

    };

    #endif // TESTCLASS_H
