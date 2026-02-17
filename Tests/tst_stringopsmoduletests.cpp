#include <QTest>

class StringOpsModuleTests : public QObject
{
    Q_OBJECT

public:
    StringOpsModuleTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testTest();
};

StringOpsModuleTests::StringOpsModuleTests()
{
}

void StringOpsModuleTests::initTestCase()
{
}

void StringOpsModuleTests::cleanupTestCase()
{
}

void StringOpsModuleTests::init()
{
}

void StringOpsModuleTests::cleanup()
{
}

void StringOpsModuleTests::testTest()
{
}

QTEST_APPLESS_MAIN(StringOpsModuleTests)

#include "tst_stringopsmoduletests.moc"
