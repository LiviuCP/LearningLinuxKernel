#include <QTest>

#include "utils.h"

static constexpr std::string_view stringOpsModuleName{"string_ops"};
static constexpr std::string_view utilitiesModuleName{"kernelutilities"};

/* These tests should be run from a terminal using sudo */

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

private:
    // TODO: this could be moved into a "testutils" directory as it is also used by the mapping module tests
    std::optional<std::filesystem::path> getModulePath(const std::string_view moduleName);

    const bool m_IsUtilitiesModuleInitiallyLoaded;
};

StringOpsModuleTests::StringOpsModuleTests()
    : m_IsUtilitiesModuleInitiallyLoaded{Utilities::isKernelModuleLoaded(utilitiesModuleName)}
{
}

void StringOpsModuleTests::initTestCase()
{
    try
    {
        const auto utilitiesModulePath{getModulePath(utilitiesModuleName)};
        QVERIFY(utilitiesModulePath.has_value());

        const auto stringOpsModulePath{getModulePath(stringOpsModuleName)};
        QVERIFY(stringOpsModulePath.has_value());

        if (!m_IsUtilitiesModuleInitiallyLoaded)
        {
            Utilities::loadKernelModule(*utilitiesModulePath);
        }

        QVERIFY(Utilities::isKernelModuleLoaded(utilitiesModuleName));

        // ensure a fresh start by reloading the module in case already loaded
        if (Utilities::isKernelModuleLoaded(stringOpsModuleName))
        {
            Utilities::unloadKernelModule(stringOpsModuleName);
        }

        Utilities::loadKernelModule(*stringOpsModulePath);

        QVERIFY(Utilities::isKernelModuleLoaded(stringOpsModuleName));
    }
    catch (const std::runtime_error& err)
    {
        QFAIL(err.what());
    }
}

void StringOpsModuleTests::cleanupTestCase()
{
    try
    {
        if (Utilities::isKernelModuleLoaded(stringOpsModuleName))
        {
            Utilities::unloadKernelModule(stringOpsModuleName);
        }

        if (!m_IsUtilitiesModuleInitiallyLoaded && Utilities::isKernelModuleLoaded(utilitiesModuleName))
        {
            Utilities::unloadKernelModule(utilitiesModuleName);
        }
    }
    catch (const std::runtime_error& err)
    {
        QFAIL(err.what());
    }
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

std::optional<std::filesystem::path> StringOpsModuleTests::getModulePath(const std::string_view moduleName)
{
    std::filesystem::path modulePath{Utilities::getApplicationPath()};
    modulePath = modulePath.parent_path().parent_path();
    modulePath /= Utilities::getModulesDirRelativePath();
    modulePath /= moduleName;
    modulePath += Utilities::getModuleFileExtension();

    return std::filesystem::is_regular_file(modulePath) ? std::optional{modulePath} : std::nullopt;
}

QTEST_APPLESS_MAIN(StringOpsModuleTests)

#include "tst_stringopsmoduletests.moc"
