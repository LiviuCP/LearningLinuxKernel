#include <QTest>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "testutils.h"
#include "utils.h"

#define GET_BUFFER_SIZE _IOR(9999, 'a', size_t*)

static constexpr std::string_view stringOpsModuleName{"ioctl_string_ops"};
static constexpr std::string_view utilitiesModuleName{"kernelutilities"};
static constexpr std::string_view deviceDirPath{"/dev"};
static constexpr std::string_view baseDeviceFileName{"ioctlstringops"};

static constexpr size_t maxCharsCountToRead{255};

/* These tests should be run from a terminal using sudo */

class IoctlStringOpsModuleTests : public QObject
{
    Q_OBJECT

public:
    IoctlStringOpsModuleTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testGetBufferSize();

private:
    void initializeDeviceFile();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::optional<std::string> readFromDeviceFile(const std::filesystem::path& deviceFile);

    size_t ioctlReadBufferSize();

    void resetKernelModule();
    bool isKernelModuleReset();

    int m_MajorNumber;
    std::filesystem::path m_DeviceFile;
    const bool m_IsUtilitiesModuleInitiallyLoaded;
};

IoctlStringOpsModuleTests::IoctlStringOpsModuleTests()
    : m_MajorNumber{0}, m_IsUtilitiesModuleInitiallyLoaded{Utilities::isKernelModuleLoaded(utilitiesModuleName)}
{
}

void IoctlStringOpsModuleTests::initTestCase()
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

        // this device file should be created automatically when the ioctl_string_ops kernel module gets loaded
        initializeDeviceFile();
    }
    catch (const std::runtime_error& err)
    {
        QFAIL(err.what());
    }
}

void IoctlStringOpsModuleTests::cleanupTestCase()
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

void IoctlStringOpsModuleTests::init()
{
    QVERIFY(isKernelModuleReset());
}

void IoctlStringOpsModuleTests::cleanup()
{
    resetKernelModule();
}

void IoctlStringOpsModuleTests::testGetBufferSize()
{
    writeToDeviceFile(m_DeviceFile, "This Is just a TEST!");
    QVERIFY(ioctlReadBufferSize() == 20);
}

void IoctlStringOpsModuleTests::initializeDeviceFile()
{
    m_DeviceFile = deviceDirPath;
    m_DeviceFile /= std::string{baseDeviceFileName};

    QVERIFY(std::filesystem::is_character_file(m_DeviceFile));
}

bool IoctlStringOpsModuleTests::writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str)
{
    return Utilities::writeStringToFile(str, deviceFile, str.size());
}

std::optional<std::string> IoctlStringOpsModuleTests::readFromDeviceFile(const std::filesystem::path& deviceFile)
{
    return Utilities::readStringFromFile(deviceFile, maxCharsCountToRead);
}

size_t IoctlStringOpsModuleTests::ioctlReadBufferSize()
{
    size_t result{0};
    int fd = open(m_DeviceFile.c_str(), O_RDONLY);

    if (fd < 0)
    {
        qDebug("Cannot open device file %s for reading!\n", m_DeviceFile.filename().c_str());
    }
    else
    {
        ioctl(fd, GET_BUFFER_SIZE, (size_t*)&result);
        close(fd);
    }

    return result;
}

void IoctlStringOpsModuleTests::resetKernelModule()
{
    writeToDeviceFile(m_DeviceFile, "");
}

bool IoctlStringOpsModuleTests::isKernelModuleReset()
{
    const std::optional<std::string> str{readFromDeviceFile(m_DeviceFile)};
    return str.has_value() && str->empty();
}

QTEST_APPLESS_MAIN(IoctlStringOpsModuleTests)

#include "tst_ioctlstringopsmoduletests.moc"
