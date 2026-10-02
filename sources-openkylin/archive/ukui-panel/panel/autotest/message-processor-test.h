#ifndef MESSAGE_PROCESSOR_TEST_H
#define MESSAGE_PROCESSOR_TEST_H

#include <QObject>

class MessageProcessorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNodeUrlUsesDisplay_data();
    void testNodeUrlUsesDisplay();
    void testMessageProcessorMetaObject();
    void testCliHelperHelpAndVersion_data();
    void testCliHelperHelpAndVersion();
    void testRemoteConfigReplicaGeneratedCode();
    void testRemoteConfigSourceGeneratedCode();
    void testPreprocessMessageOutputsExpectedValue_data();
    void testPreprocessMessageOutputsExpectedValue();
};

#endif // MESSAGE_PROCESSOR_TEST_H
