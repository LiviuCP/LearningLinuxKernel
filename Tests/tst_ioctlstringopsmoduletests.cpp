#include <QTest>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "testutils.h"
#include "utils.h"

#define GET_BUFFER_SIZE _IOR(9999, 'a', size_t*)
#define TRIM_USER_INPUT _IOW(9999, 'b', uint8_t*)
#define DO_MODULE_RESET _IOW(9999, 'c', void*)
#define IS_MODULE_RESET _IOR(9999, 'd', uint8_t*)
#define SET_OUTPUT_PREFIX _IOW(9999, 'e', void*)

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
    void testEnableUserInputTrimming();
    void testSetOutputPrefix();

private:
    void initializeDeviceFile();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::optional<std::string> readFromDeviceFile(const std::filesystem::path& deviceFile, bool shouldTrimInput = true);

    size_t ioctlReadBufferSize();
    void ioctlEnableUserInputTrimming(bool enabled);
    void ioctlSetOutputPrefix(const std::string& outputPrefix);

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

void IoctlStringOpsModuleTests::testEnableUserInputTrimming()
{
    writeToDeviceFile(m_DeviceFile, "  This is the test of my life! ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "This is the test of my life!");

    ioctlEnableUserInputTrimming(false);

    writeToDeviceFile(m_DeviceFile, "  This is another good test! ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "  This is another good test! ");

    writeToDeviceFile(m_DeviceFile, "     This is the final test!  ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "     This is the final test!  ");

    ioctlEnableUserInputTrimming(true);

    writeToDeviceFile(m_DeviceFile, "   End of story!    ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "End of story!");
}

void IoctlStringOpsModuleTests::testSetOutputPrefix()
{
    writeToDeviceFile(m_DeviceFile, "This Is just a TEST!");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "This Is just a TEST!");

    ioctlSetOutputPrefix("My take: ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "My take: This Is just a TEST!");

    writeToDeviceFile(m_DeviceFile, "This Is just another TEST!");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "My take: This Is just another TEST!");

    ioctlSetOutputPrefix("My hot take: ");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "My hot take: This Is just another TEST!");

    ioctlSetOutputPrefix("");
    QVERIFY(readFromDeviceFile(m_DeviceFile, false) == "This Is just another TEST!");
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

std::optional<std::string> IoctlStringOpsModuleTests::readFromDeviceFile(const std::filesystem::path& deviceFile,
                                                                         bool shouldTrimInput)
{
    return Utilities::readStringFromFile(deviceFile, maxCharsCountToRead, shouldTrimInput);
}

size_t IoctlStringOpsModuleTests::ioctlReadBufferSize()
{
    size_t result{0};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

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

void IoctlStringOpsModuleTests::ioctlEnableUserInputTrimming(bool enabled)
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd < 0)
    {
        qDebug("Cannot open device file %s for writing!\n", m_DeviceFile.filename().c_str());
    }
    else
    {
        ioctl(fd, TRIM_USER_INPUT, &enabled);
        close(fd);
    }
}

void IoctlStringOpsModuleTests::ioctlSetOutputPrefix(const std::string& outputPrefix)
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd < 0)
    {
        qDebug("Cannot open device file %s for writing!\n", m_DeviceFile.filename().c_str());
    }
    else
    {
        assert(outputPrefix.size() < 251 /* output buffer minus last char ('\0') minus size_t size */);

        char data[256];
        memset(data, '\0', 256);
        size_t* prefixSize{reinterpret_cast<size_t*>(data)};
        *prefixSize = outputPrefix.size();
        ++prefixSize;
        char* prefix{reinterpret_cast<char*>(prefixSize)};
        strncpy(prefix, outputPrefix.c_str(), outputPrefix.size());
        ioctl(fd, SET_OUTPUT_PREFIX, data);
        close(fd);
    }
}

void IoctlStringOpsModuleTests::resetKernelModule()
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd < 0)
    {
        qDebug("Cannot open device file %s for writing!\n", m_DeviceFile.filename().c_str());
    }
    else
    {
        ioctl(fd, DO_MODULE_RESET, nullptr);
        close(fd);
    }
}

bool IoctlStringOpsModuleTests::isKernelModuleReset()
{
    bool isReset{false};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd < 0)
    {
        qDebug("Cannot open device file %s for reading!\n", m_DeviceFile.filename().c_str());
    }
    else
    {
        ioctl(fd, IS_MODULE_RESET, &isReset);
        close(fd);
    }

    return isReset;
}

QTEST_APPLESS_MAIN(IoctlStringOpsModuleTests)

#include "tst_ioctlstringopsmoduletests.moc"
