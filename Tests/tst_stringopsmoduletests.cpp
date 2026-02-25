#include <QTest>

#include <set>

#include "testutils.h"
#include "utils.h"

static constexpr std::string_view stringOpsModuleName{"string_ops"};
static constexpr std::string_view utilitiesModuleName{"kernelutilities"};
static constexpr std::string_view baseDeviceFileName{"teststringops"};

static constexpr int minorNumber0{0};
static constexpr int minorNumber1{1};
static constexpr int minorNumber2{2};
static constexpr int minorNumber3{3};
static constexpr int minorNumber4{4};

static constexpr size_t maxCharsCountToRead{255};

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

    void testCanWriteToDeviceFiles();
    void testReadFromAfterWriteToMinorNumber1();
    void testReadFromAfterWriteToMinorNumber2();
    void testReadFromAfterWriteToMinorNumber3();
    void testCombinedReadFromAfterWriteTo();

private:
    void createDeviceFiles();
    void removeDeviceFiles();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::optional<std::string> readFromDeviceFile(const std::filesystem::path& deviceFile);

    void resetKernelModule();
    bool isKernelModuleReset();

    int m_MajorNumber;

    std::filesystem::path m_DeviceFileMinor0;
    std::filesystem::path m_DeviceFileMinor1;
    std::filesystem::path m_DeviceFileMinor2;
    std::filesystem::path m_DeviceFileMinor3;
    std::filesystem::path m_DeviceFileMinor4;

    const bool m_IsUtilitiesModuleInitiallyLoaded;
};

StringOpsModuleTests::StringOpsModuleTests()
    : m_MajorNumber{0}, m_IsUtilitiesModuleInitiallyLoaded{Utilities::isKernelModuleLoaded(utilitiesModuleName)}
{
}

void StringOpsModuleTests::initTestCase()
{
    try
    {
        const auto utilitiesModulePath{Utilities::Test::getModulePath(utilitiesModuleName)};
        QVERIFY(utilitiesModulePath.has_value());

        const auto stringOpsModulePath{Utilities::Test::getModulePath(stringOpsModuleName)};
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

        m_MajorNumber = Utilities::getMajorDriverNumber(stringOpsModuleName);
        QVERIFY(m_MajorNumber > 0);

        createDeviceFiles();
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
        removeDeviceFiles();

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
    QVERIFY(isKernelModuleReset());
}

void StringOpsModuleTests::cleanup()
{
    resetKernelModule();
}

void StringOpsModuleTests::testCanWriteToDeviceFiles()
{
    const std::string emptyStr;
    const std::string str{"First user input!"};

    /* empty string */
    bool success{writeToDeviceFile(m_DeviceFileMinor0, emptyStr)};
    QVERIFY(!success);

    success = writeToDeviceFile(m_DeviceFileMinor1, emptyStr);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor2, emptyStr);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor3, emptyStr);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor4, emptyStr);
    QVERIFY(!success);

    /* non-empty string */

    success = writeToDeviceFile(m_DeviceFileMinor0, str);
    QVERIFY(!success);

    success = writeToDeviceFile(m_DeviceFileMinor1, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor2, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor3, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor4, str);
    QVERIFY(!success);
}

void StringOpsModuleTests::testReadFromAfterWriteToMinorNumber1()
{
    writeToDeviceFile(m_DeviceFileMinor1, "This Is just a TEST!");

    QVERIFY("this is just a test!" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("This Is just a TEST!" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, " ");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, "");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, "MyTest\n");

    QVERIFY("mytest" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("MyTest" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, "  12A-bCd_EF+ ");

    QVERIFY("12a-bcd_ef+" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("12A-bCd_EF+" == readFromDeviceFile(m_DeviceFileMinor0));
}

void StringOpsModuleTests::testReadFromAfterWriteToMinorNumber2()
{
    writeToDeviceFile(m_DeviceFileMinor2, "!TSET a tsuj sI sihT");

    QVERIFY("This Is just a TEST!" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("!TSET a tsuj sI sihT" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, " ");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, "");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, "MyTest\n");

    QVERIFY("tseTyM" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("MyTest" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, "  +FE_dCb-A21 ");

    QVERIFY("12A-bCd_EF+" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("+FE_dCb-A21" == readFromDeviceFile(m_DeviceFileMinor0));
}

void StringOpsModuleTests::testReadFromAfterWriteToMinorNumber3()
{
    writeToDeviceFile(m_DeviceFileMinor3, "This Is just a TEST!");

    QVERIFY("This Is just a TEST!; 20" == readFromDeviceFile(m_DeviceFileMinor3));
    QVERIFY("This Is just a TEST!" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, " ");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor3));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, "");

    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor3));
    QVERIFY("" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, "MyTest\n");

    QVERIFY("MyTest; 6" == readFromDeviceFile(m_DeviceFileMinor3));
    QVERIFY("MyTest" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, "  12A-bCd_EF+ ");

    QVERIFY("12A-bCd_EF+; 11" == readFromDeviceFile(m_DeviceFileMinor3));
    QVERIFY("12A-bCd_EF+" == readFromDeviceFile(m_DeviceFileMinor0));
}

void StringOpsModuleTests::testCombinedReadFromAfterWriteTo()
{
    writeToDeviceFile(m_DeviceFileMinor1, " Writing to minor number 1\n");
    QVERIFY("Writing to minor number 1" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, "Writing to Minor Number 2\n ");
    QVERIFY("Writing to Minor Number 2" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, "Writing to minor number 3 \n");
    QVERIFY("Writing to minor number 3" == readFromDeviceFile(m_DeviceFileMinor0));

    QVERIFY("writing to minor number 1" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("2 rebmuN roniM ot gnitirW" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("Writing to minor number 3; 25" == readFromDeviceFile(m_DeviceFileMinor3));

    writeToDeviceFile(m_DeviceFileMinor2, "\n\nAnother shOt on minor number 2!  ");
    QVERIFY("Another shOt on minor number 2!" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor3, " Another shot on minor nr 3... ");
    QVERIFY("Another shot on minor nr 3..." == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, " _ Another shot on minoR Number 1\n \n");
    QVERIFY("_ Another shot on minoR Number 1" == readFromDeviceFile(m_DeviceFileMinor0));

    QVERIFY("_ another shot on minor number 1" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("!2 rebmun ronim no tOhs rehtonA" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("Another shot on minor nr 3...; 29" == readFromDeviceFile(m_DeviceFileMinor3));

    writeToDeviceFile(m_DeviceFileMinor3, "\nMinor 3 is \nthe winner!!!\n\n");
    QVERIFY("Minor 3 is \nthe winner!!!" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor1, "Well, I would say it's MINOR NUMBER 1, isn't it? \t ");
    QVERIFY("Well, I would say it's MINOR NUMBER 1, isn't it?" == readFromDeviceFile(m_DeviceFileMinor0));

    writeToDeviceFile(m_DeviceFileMinor2, "\t\t You're wrong! \tIt's number 2. \nFinally we did it! \t");
    QVERIFY("You're wrong! \tIt's number 2. \nFinally we did it!" == readFromDeviceFile(m_DeviceFileMinor0));

    QVERIFY("well, i would say it's minor number 1, isn't it?" == readFromDeviceFile(m_DeviceFileMinor1));
    QVERIFY("!ti did ew yllaniF\n .2 rebmun s'tI\t !gnorw er'uoY" == readFromDeviceFile(m_DeviceFileMinor2));
    QVERIFY("Minor 3 is \nthe winner!!!; 25" == readFromDeviceFile(m_DeviceFileMinor3));
}

void StringOpsModuleTests::createDeviceFiles()
{
    const std::filesystem::path deviceDirPath{"/dev"};
    QVERIFY(std::filesystem::exists(deviceDirPath));

    m_DeviceFileMinor0 = deviceDirPath;
    m_DeviceFileMinor1 = deviceDirPath;
    m_DeviceFileMinor2 = deviceDirPath;
    m_DeviceFileMinor3 = deviceDirPath;
    m_DeviceFileMinor4 = deviceDirPath;

    m_DeviceFileMinor0 /= (std::string{baseDeviceFileName} + std::to_string(minorNumber0));
    m_DeviceFileMinor1 /= (std::string{baseDeviceFileName} + std::to_string(minorNumber1));
    m_DeviceFileMinor2 /= (std::string{baseDeviceFileName} + std::to_string(minorNumber2));
    m_DeviceFileMinor3 /= (std::string{baseDeviceFileName} + std::to_string(minorNumber3));
    m_DeviceFileMinor4 /= (std::string{baseDeviceFileName} + std::to_string(minorNumber4));

    QVERIFY(!std::filesystem::exists(m_DeviceFileMinor0));
    QVERIFY(!std::filesystem::exists(m_DeviceFileMinor1));
    QVERIFY(!std::filesystem::exists(m_DeviceFileMinor2));
    QVERIFY(!std::filesystem::exists(m_DeviceFileMinor3));
    QVERIFY(!std::filesystem::exists(m_DeviceFileMinor4));

    bool success{true};

    success = success && Utilities::createCharacterDeviceFile(m_DeviceFileMinor0, m_MajorNumber, minorNumber0);
    success = success && Utilities::createCharacterDeviceFile(m_DeviceFileMinor1, m_MajorNumber, minorNumber1);
    success = success && Utilities::createCharacterDeviceFile(m_DeviceFileMinor2, m_MajorNumber, minorNumber2);
    success = success && Utilities::createCharacterDeviceFile(m_DeviceFileMinor3, m_MajorNumber, minorNumber3);
    success = success && Utilities::createCharacterDeviceFile(m_DeviceFileMinor4, m_MajorNumber, minorNumber4);

    QVERIFY(success);

    QVERIFY(std::filesystem::is_character_file(m_DeviceFileMinor0));
    QVERIFY(std::filesystem::is_character_file(m_DeviceFileMinor1));
    QVERIFY(std::filesystem::is_character_file(m_DeviceFileMinor2));
    QVERIFY(std::filesystem::is_character_file(m_DeviceFileMinor3));
    QVERIFY(std::filesystem::is_character_file(m_DeviceFileMinor4));
}

void StringOpsModuleTests::removeDeviceFiles()
{
    for (const auto& deviceFile :
         std::set{m_DeviceFileMinor0, m_DeviceFileMinor1, m_DeviceFileMinor2, m_DeviceFileMinor3, m_DeviceFileMinor4})
    {
        if (std::filesystem::exists(deviceFile))
        {
            std::filesystem::remove(deviceFile);
        }
    }
}

bool StringOpsModuleTests::writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str)
{
    return Utilities::writeStringToFile(str, deviceFile, str.size());
}

std::optional<std::string> StringOpsModuleTests::readFromDeviceFile(const std::filesystem::path& deviceFile)
{
    return Utilities::readStringFromFile(deviceFile, maxCharsCountToRead);
}

void StringOpsModuleTests::resetKernelModule()
{
    writeToDeviceFile(m_DeviceFileMinor1, "");
    writeToDeviceFile(m_DeviceFileMinor2, "");
    writeToDeviceFile(m_DeviceFileMinor3, "");
}

bool StringOpsModuleTests::isKernelModuleReset()
{
    const std::optional<std::string> str0{readFromDeviceFile(m_DeviceFileMinor0)};
    const std::optional<std::string> str1{readFromDeviceFile(m_DeviceFileMinor1)};
    const std::optional<std::string> str2{readFromDeviceFile(m_DeviceFileMinor2)};
    const std::optional<std::string> str3{readFromDeviceFile(m_DeviceFileMinor3)};

    const bool areResultsValid{str0.has_value() && str1.has_value() && str2.has_value() && str3.has_value()};

    return areResultsValid && str0->empty() && str1->empty() && str2->empty() && str3->empty();
}

QTEST_APPLESS_MAIN(StringOpsModuleTests)

#include "tst_stringopsmoduletests.moc"
