#include <QTest>

#include <fcntl.h>
#include <filesystem>
#include <set>
#include <string_view>
#include <unistd.h>

#include "utils.h"

static constexpr std::string_view stringOpsModuleName{"string_ops"};
static constexpr std::string_view utilitiesModuleName{"kernelutilities"};
static constexpr std::string_view baseDeviceFileName{"teststringops"};

static constexpr int minorNumber0{0};
static constexpr int minorNumber1{1};
static constexpr int minorNumber2{2};
static constexpr int minorNumber3{3};
static constexpr int minorNumber4{4};

static constexpr size_t maxBufferSize{256};

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

private:
    void createDeviceFiles();
    void removeDeviceFiles();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::string readFromDeviceFile(const std::filesystem::path& deviceFile);

    bool isKernelModuleReset();

    // TODO: this could be moved into a "testutils" directory as it is also used by the mapping module tests
    std::optional<std::filesystem::path> getModulePath(const std::string_view moduleName);

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

        m_MajorNumber = Utilities::getMajorDriverNumber(stringOpsModuleName);
        QVERIFY(m_MajorNumber > 0);

        createDeviceFiles();
        QVERIFY(isKernelModuleReset());
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
}

void StringOpsModuleTests::cleanup()
{
}

void StringOpsModuleTests::testCanWriteToDeviceFiles()
{
    const std::string str{"First user input!"};

    bool success = writeToDeviceFile(m_DeviceFileMinor0, str);
    QVERIFY(!success); // minor number 0 only supported for reading

    success = writeToDeviceFile(m_DeviceFileMinor1, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor2, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor3, str);
    QVERIFY(success);

    success = writeToDeviceFile(m_DeviceFileMinor4, str);
    QVERIFY(!success); // unsupported minor number
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
    bool success{false};

    const int fd{open(deviceFile.string().c_str(), O_WRONLY | O_NONBLOCK)};

    if (fd >= 0)
    {
        const ssize_t writtenCharsCount{write(fd, str.c_str(), str.size())};
        success = writtenCharsCount >= 0;
        close(fd);
    }

    return success;
}

std::string StringOpsModuleTests::readFromDeviceFile(const std::filesystem::path& deviceFile)
{
    std::string result;
    const int fd{open(deviceFile.string().c_str(), O_RDONLY | O_NONBLOCK)};

    if (fd >= 0)
    {
        char buffer[maxBufferSize];
        memset(buffer, '\0', maxBufferSize);
        ssize_t readCharsCount{read(fd, buffer, maxBufferSize - 1)};

        if (readCharsCount >= 0)
        {
            result = std::string{buffer};
        }

        close(fd);
    }

    return result;
}

bool StringOpsModuleTests::isKernelModuleReset()
{
    const std::string str0{readFromDeviceFile(m_DeviceFileMinor0)};
    const std::string str1{readFromDeviceFile(m_DeviceFileMinor1)};
    const std::string str2{readFromDeviceFile(m_DeviceFileMinor2)};
    const std::string str3{readFromDeviceFile(m_DeviceFileMinor3)};

    return str0.empty() && str1.empty() && str2.empty() && str3.empty();
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
