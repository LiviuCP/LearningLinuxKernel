#include <QTest>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "testutils.h"
#include "utils.h"

#define IOCTL_DO_MODULE_RESET _IOW(9999, 'a', void*)
#define IOCTL_IS_MODULE_RESET _IOR(9999, 'b', uint8_t*)
#define IOCTL_TRIM_USER_INPUT _IOW(9999, 'c', uint8_t*)
#define IOCTL_GET_CHARS_COUNT_FROM_BUFFER _IOR(9999, 'd', size_t*)
#define IOCTL_SET_OUTPUT_PREFIX _IOW(9999, 'e', void*)
#define IOCTL_GET_OUTPUT_PREFIX_SIZE _IOR(9999, 'f', size_t*)
#define IOCTL_ENABLE_INPUT_APPEND_MODE _IOW(9999, 'g', uint8_t*)
#define IOCTL_IS_INPUT_APPEND_MODE_ENABLED _IOR(9999, 'h', uint8_t*)

static constexpr std::string_view stringOpsModuleName{"ioctl_string_ops"};
static constexpr std::string_view utilitiesModuleName{"kernelutilities"};
static constexpr std::string_view deviceDirPath{"/dev"};
static constexpr std::string_view baseDeviceFileName{"ioctlstringops"};

static constexpr size_t moduleBufferSize{1024};

// '\0': there should be minimum one terminating character
static constexpr size_t maxCharsCountToRead{moduleBufferSize - sizeof('\0')};

static constexpr size_t prefixBufferSize{128};

// size_t: size provided to the kernel module, '\0': there should be minimum one terminating character
static constexpr size_t maxPrefixSize{prefixBufferSize - sizeof(size_t) - sizeof('\0')};

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

    void testEnableUserInputTrimming();
    void testSetOutputPrefix();

private:
    void initializeDeviceFile();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::optional<std::string> readFromDeviceFile(const std::filesystem::path& deviceFile);

    void ioctlEnableUserInputTrimming(bool enabled);
    std::optional<size_t> ioctlReadCharsCountFromBuffer();
    void ioctlSetOutputPrefix(const std::string& outputPrefix);
    std::optional<size_t> ioctlReadOutputPrefixSize();

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

void IoctlStringOpsModuleTests::testEnableUserInputTrimming()
{
    writeToDeviceFile(m_DeviceFile, "  This is the test of my life! ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This is the test of my life!");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 28);

    ioctlEnableUserInputTrimming(false);
    writeToDeviceFile(m_DeviceFile, "  This is another good test! ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "  This is another good test! ");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 29);

    writeToDeviceFile(m_DeviceFile, "     This is the final test!  ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "     This is the final test!  ");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 30);

    writeToDeviceFile(m_DeviceFile, " ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == " ");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 1);

    writeToDeviceFile(m_DeviceFile, "");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 0);

    ioctlEnableUserInputTrimming(true);

    writeToDeviceFile(m_DeviceFile, "   The end of story!\n");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "The end of story!");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 17);

    writeToDeviceFile(m_DeviceFile, " ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 0);

    writeToDeviceFile(m_DeviceFile, "");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlReadCharsCountFromBuffer() == 0);
}

void IoctlStringOpsModuleTests::testSetOutputPrefix()
{
    writeToDeviceFile(m_DeviceFile, "This Is just a TEST!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just a TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 0);

    ioctlSetOutputPrefix("My take: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My take: This Is just a TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 9);

    writeToDeviceFile(m_DeviceFile, "This Is just another TEST!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My take: This Is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 9);

    ioctlSetOutputPrefix("My hot take: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My hot take: This Is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 13);

    ioctlSetOutputPrefix("aprefix");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "aprefixThis Is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 7);

    ioctlSetOutputPrefix(" ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == " This Is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 1);

    ioctlSetOutputPrefix("");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 0);

    ioctlEnableUserInputTrimming(false);

    writeToDeviceFile(m_DeviceFile, "  This is just another TEST! ");
    ioctlSetOutputPrefix("A prefix: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "A prefix:   This is just another TEST! ");
    QVERIFY(ioctlReadOutputPrefixSize() == 10);
    QVERIFY(ioctlReadCharsCountFromBuffer() == 29);

    resetKernelModule();

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlReadOutputPrefixSize() == 0);
    QVERIFY(ioctlReadCharsCountFromBuffer() == 0);

    writeToDeviceFile(m_DeviceFile, "  This is just another TEST! ");
    ioctlSetOutputPrefix("A prefix: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "A prefix: This is just another TEST!");
    QVERIFY(ioctlReadOutputPrefixSize() == 10);
    QVERIFY(ioctlReadCharsCountFromBuffer() == 26);
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
    return Utilities::readStringFromFile(deviceFile, maxCharsCountToRead, NON_TRIM_MODE);
}

void IoctlStringOpsModuleTests::ioctlEnableUserInputTrimming(bool enabled)
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_TRIM_USER_INPUT, &enabled)};
        close(fd);

        if (retVal != 0)
        {
            QFAIL("Enabling/disabling trimming failed!");
        }
    }
}

std::optional<size_t> IoctlStringOpsModuleTests::ioctlReadCharsCountFromBuffer()
{
    std::optional<size_t> result{0};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        size_t charsCountFromBuffer;

        const long retVal{ioctl(fd, IOCTL_GET_CHARS_COUNT_FROM_BUFFER, (size_t*)&charsCountFromBuffer)};

        if (retVal == 0)
        {
            result = charsCountFromBuffer;
        }

        close(fd);
    }

    return result;
}

void IoctlStringOpsModuleTests::ioctlSetOutputPrefix(const std::string& outputPrefix)
{
    if (outputPrefix.size() <= maxPrefixSize)
    {
        const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

        if (fd > 0)
        {
            char data[prefixBufferSize];
            memset(data, '\0', prefixBufferSize);
            size_t* prefixSize{reinterpret_cast<size_t*>(data)};
            *prefixSize = outputPrefix.size();
            ++prefixSize;
            char* prefix{reinterpret_cast<char*>(prefixSize)};
            strncpy(prefix, outputPrefix.c_str(), outputPrefix.size());
            const long retVal{ioctl(fd, IOCTL_SET_OUTPUT_PREFIX, data)};
            close(fd);

            if (retVal != 0)
            {
                QFAIL("Setting output prefix failed!");
            }
        }
    }
    else
    {
        assert(false);
    }
}

std::optional<size_t> IoctlStringOpsModuleTests::ioctlReadOutputPrefixSize()
{
    std::optional<size_t> result;
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        size_t outputPrefixSize;
        const auto retVal{ioctl(fd, IOCTL_GET_OUTPUT_PREFIX_SIZE, (size_t*)&outputPrefixSize)};

        if (retVal == 0)
        {
            result = outputPrefixSize;
        }

        close(fd);
    }

    return result;
}

void IoctlStringOpsModuleTests::resetKernelModule()
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_DO_MODULE_RESET, nullptr)};
        close(fd);

        if (retVal != 0)
        {
            QFAIL("Module reset failed!");
        }
    }
}

bool IoctlStringOpsModuleTests::isKernelModuleReset()
{
    bool isReset{false};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        bool isModuleReset;
        const long retVal{ioctl(fd, IOCTL_IS_MODULE_RESET, &isModuleReset)};

        if (retVal == 0)
        {
            isReset = isModuleReset;
        }

        close(fd);
    }

    return isReset;
}

QTEST_APPLESS_MAIN(IoctlStringOpsModuleTests)

#include "tst_ioctlstringopsmoduletests.moc"
