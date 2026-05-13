#include <QTest>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "testutils.h"
#include "utils.h"

#define IOCTL_DO_MODULE_RESET _IOW(9999, 'a', void*)
#define IOCTL_IS_MODULE_RESET _IOR(9999, 'b', bool*)
#define IOCTL_ENABLE_USER_INPUT_TRIMMING _IOW(9999, 'c', bool*)
#define IOCTL_IS_USER_INPUT_TRIMMING_ENABLED _IOR(9999, 'd', bool*)
#define IOCTL_GET_BUFFER_SIZE _IOR(9999, 'e', size_t*)
#define IOCTL_SET_OUTPUT_PREFIX _IOW(9999, 'f', void*)
#define IOCTL_GET_OUTPUT_PREFIX_SIZE _IOR(9999, 'g', size_t*)
#define IOCTL_ENABLE_INPUT_APPEND_MODE _IOW(9999, 'h', bool*)
#define IOCTL_IS_INPUT_APPEND_MODE_ENABLED _IOR(9999, 'i', bool*)
#define IOCTL_SET_MAX_OUTPUT_SIZE _IOWR(9999, 'j', size_t*)
#define IOCTL_GET_MAX_OUTPUT_SIZE _IOR(9999, 'k', size_t*)

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
    void testEnableInputAppendMode();
    void testSetMaxOutputSize_FullBufferTraverseWithFixedOutputSize();
    void testSetMaxOutputSize_FullBufferTraverseWithVariableOutputSize();
    void testSetMaxOutputSize_UseOutputPrefix();
    void testSetMaxOutputSize_UseManualOutputSizeReset();

private:
    void initializeDeviceFile();

    bool writeToDeviceFile(const std::filesystem::path& deviceFile, const std::string& str);
    std::optional<std::string> readFromDeviceFile(const std::filesystem::path& deviceFile);

    void ioctlEnableUserInputTrimming(bool enabled);
    bool ioctlIsUserInputTrimmingEnabled();
    std::optional<size_t> ioctlGetBufferSize();
    void ioctlSetOutputPrefix(const std::string& outputPrefix);
    std::optional<size_t> ioctlGetOutputPrefixSize();
    void ioctlEnableInputAppendMode(bool enabled);
    bool ioctlIsInputAppendModeEnabled();

    // value has both input and output role:
    // - input: maximum output size to be set
    // - output: remaining chars to be read from data buffer (excluding output prefix)
    bool ioctlSetMaxOutputSize(size_t& value);

    std::optional<size_t> ioctlGetMaxOutputSize();

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
    QVERIFY(ioctlGetBufferSize() == 0);
    QVERIFY(ioctlGetOutputPrefixSize() == 0);
    QVERIFY(ioctlGetMaxOutputSize() == 0);

    QVERIFY(isKernelModuleReset());
}

void IoctlStringOpsModuleTests::cleanup()
{
    resetKernelModule();
}

void IoctlStringOpsModuleTests::testEnableUserInputTrimming()
{
    // by default trimming is enabled
    QVERIFY(ioctlIsUserInputTrimmingEnabled());

    writeToDeviceFile(m_DeviceFile, "  This is the test of my life! ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This is the test of my life!");
    QVERIFY(ioctlGetBufferSize() == 28);

    ioctlEnableUserInputTrimming(false);
    QVERIFY(!ioctlIsUserInputTrimmingEnabled());

    writeToDeviceFile(m_DeviceFile, "  This is another good test! ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "  This is another good test! ");
    QVERIFY(ioctlGetBufferSize() == 29);

    writeToDeviceFile(m_DeviceFile, "     This is the final test!  ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "     This is the final test!  ");
    QVERIFY(ioctlGetBufferSize() == 30);

    writeToDeviceFile(m_DeviceFile, " ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == " ");
    QVERIFY(ioctlGetBufferSize() == 1);

    writeToDeviceFile(m_DeviceFile, "");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlGetBufferSize() == 0);

    ioctlEnableUserInputTrimming(true);
    QVERIFY(ioctlIsUserInputTrimmingEnabled());

    writeToDeviceFile(m_DeviceFile, "   The end of story!\n");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "The end of story!");
    QVERIFY(ioctlGetBufferSize() == 17);

    writeToDeviceFile(m_DeviceFile, " ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlGetBufferSize() == 0);

    writeToDeviceFile(m_DeviceFile, "");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlGetBufferSize() == 0);
}

void IoctlStringOpsModuleTests::testSetOutputPrefix()
{
    writeToDeviceFile(m_DeviceFile, "This Is just a TEST!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just a TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 0);

    ioctlSetOutputPrefix("My take: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My take: This Is just a TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 9);

    writeToDeviceFile(m_DeviceFile, "This Is just another TEST!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My take: This Is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 9);

    ioctlSetOutputPrefix("My hot take: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "My hot take: This Is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 13);

    ioctlSetOutputPrefix("aprefix");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "aprefixThis Is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 7);

    ioctlSetOutputPrefix(" ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == " This Is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 1);

    ioctlSetOutputPrefix("");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 0);

    ioctlEnableUserInputTrimming(false);

    writeToDeviceFile(m_DeviceFile, "  This is just another TEST! ");
    ioctlSetOutputPrefix("A prefix: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "A prefix:   This is just another TEST! ");
    QVERIFY(ioctlGetOutputPrefixSize() == 10);
    QVERIFY(ioctlGetBufferSize() == 29);

    resetKernelModule();

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    QVERIFY(ioctlGetOutputPrefixSize() == 0);
    QVERIFY(ioctlGetBufferSize() == 0);

    writeToDeviceFile(m_DeviceFile, "  This is just another TEST! ");
    ioctlSetOutputPrefix("A prefix: ");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "A prefix: This is just another TEST!");
    QVERIFY(ioctlGetOutputPrefixSize() == 10);
    QVERIFY(ioctlGetBufferSize() == 26);
}

void IoctlStringOpsModuleTests::testEnableInputAppendMode()
{
    ioctlEnableUserInputTrimming(false);

    writeToDeviceFile(m_DeviceFile, "This Is just a TEST!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just a TEST!");
    QVERIFY(!ioctlIsInputAppendModeEnabled());

    ioctlEnableInputAppendMode(true);

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just a TEST!");
    QVERIFY(ioctlIsInputAppendModeEnabled());

    writeToDeviceFile(m_DeviceFile, " And I passed it!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "This Is just a TEST! And I passed it!");
    QVERIFY(ioctlIsInputAppendModeEnabled());

    ioctlEnableInputAppendMode(false);
    writeToDeviceFile(m_DeviceFile, "Oops, I've been overwriting my content!");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "Oops, I've been overwriting my content!");
    QVERIFY(!ioctlIsInputAppendModeEnabled());

    writeToDeviceFile(m_DeviceFile, "The magic number is: ");

    ioctlEnableUserInputTrimming(true);
    ioctlEnableInputAppendMode(true);

    writeToDeviceFile(m_DeviceFile, "1- ");
    writeToDeviceFile(m_DeviceFile, " 2- ");
    writeToDeviceFile(m_DeviceFile, "\n3-");
    writeToDeviceFile(m_DeviceFile, "4!\t");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "The magic number is: 1-2-3-4!");
    QVERIFY(ioctlIsInputAppendModeEnabled());

    resetKernelModule();

    writeToDeviceFile(m_DeviceFile, " first ");
    writeToDeviceFile(m_DeviceFile, " last \n");

    QVERIFY(readFromDeviceFile(m_DeviceFile) == "last");
    QVERIFY(!ioctlIsInputAppendModeEnabled());

    /* Test appending when maximum buffer capacity is reached */

    resetKernelModule();
    ioctlEnableInputAppendMode(true);

    std::string str;
    constexpr size_t maxCharsCountToWriteToModule{1023}; // buffer size excluding last character (should be '\0')

    str.resize(maxCharsCountToWriteToModule - 1, 'a');

    writeToDeviceFile(m_DeviceFile, str);
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str);

    writeToDeviceFile(m_DeviceFile, "bc");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str);

    writeToDeviceFile(m_DeviceFile, "d");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str + "d");

    writeToDeviceFile(m_DeviceFile, "e");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str + "d");

    QVERIFY(ioctlIsInputAppendModeEnabled());

    ioctlEnableInputAppendMode(false);

    writeToDeviceFile(m_DeviceFile, "e");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == "e");

    writeToDeviceFile(m_DeviceFile, str);
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str);

    writeToDeviceFile(m_DeviceFile, str + "bc");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str + "b");

    writeToDeviceFile(m_DeviceFile, str + "d");
    QVERIFY(readFromDeviceFile(m_DeviceFile) == str + "d");

    QVERIFY(!ioctlIsInputAppendModeEnabled());
}

void IoctlStringOpsModuleTests::testSetMaxOutputSize_FullBufferTraverseWithFixedOutputSize()
{
    size_t value;

    size_t& maxOutputSize{value};
    const size_t& remainingCharsToReadCount{value};

    {
        const std::string quoteFromMargaretThatcher{
            "The biggest problem of socialism is that they eventually run out of other "
            "people's money! (Margaret Thatcher)"};

        writeToDeviceFile(m_DeviceFile, quoteFromMargaretThatcher);

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "The biggest problem ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "of socialism is that");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " they eventually run");

        // check remaining number of characters, max size remains unchanged
        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 49);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " out of other people");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "'s money! (Margaret ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Thatcher)");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);

        maxOutputSize = 1;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 1);
        QVERIFY(remainingCharsToReadCount == 109);

        std::string reconstructedString;
        reconstructedString.reserve(remainingCharsToReadCount);

        for (size_t index = 0; index < remainingCharsToReadCount; ++index)
        {
            const std::optional<std::string> currentSubstring{readFromDeviceFile(m_DeviceFile)};
            QVERIFY(currentSubstring.has_value());

            reconstructedString += *currentSubstring;
        }

        QVERIFY(reconstructedString == quoteFromMargaretThatcher);

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);
    }

    {
        writeToDeviceFile(m_DeviceFile, "1a2b3c4d5e6f7g8h");

        maxOutputSize = 4;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "3c4d");

        // check remaining number of characters, max size remains unchanged
        maxOutputSize = 4;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 8);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "5e6f");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "7g8h");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 16;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 16);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 17;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 16);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 4;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "3c4d");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "5e6f");

        // test reading the last sequence that equals the maximum size
        maxOutputSize = 4;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 4);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "7g8h");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");
    }

    {
        writeToDeviceFile(m_DeviceFile, "");

        maxOutputSize = 1;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "");

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    }
}

void IoctlStringOpsModuleTests::testSetMaxOutputSize_FullBufferTraverseWithVariableOutputSize()
{
    size_t value;

    size_t& maxOutputSize{value};
    const size_t& remainingCharsToReadCount{value};

    {
        const std::string quoteFromMargaretThatcher{
            "The biggest problem of socialism is that they eventually run out of other "
            "people's money! (Margaret Thatcher)"};

        writeToDeviceFile(m_DeviceFile, quoteFromMargaretThatcher);

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "The biggest problem ");

        maxOutputSize = 25;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 25);
        QVERIFY(remainingCharsToReadCount == 89);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "of socialism is that they");

        maxOutputSize = 15;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 15);
        QVERIFY(remainingCharsToReadCount == 64);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " eventually run");

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 49);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " out of other people");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "'s money! (Margaret ");

        // test remaining number of characters, max size remains unchanged
        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 9);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Thatcher)");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);
    }

    {
        writeToDeviceFile(m_DeviceFile, "1a2b3c4d5e6f7g8h");

        maxOutputSize = 16;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 16);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 4;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "3c4d");

        maxOutputSize = 8;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 8);
        QVERIFY(remainingCharsToReadCount == 8);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "5e6f7g8h");

        maxOutputSize = 17;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 16);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 5;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 5);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3");

        maxOutputSize = 12;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 12);
        QVERIFY(remainingCharsToReadCount == 11);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "c4d5e6f7g8h");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");
    }
}

void IoctlStringOpsModuleTests::testSetMaxOutputSize_UseOutputPrefix()
{
    size_t value;

    size_t& maxOutputSize{value};
    const size_t& remainingCharsToReadCount{value};

    {
        const std::string quoteFromMargaretThatcher{
            "The biggest problem of socialism is that they eventually run out of other "
            "people's money! (Margaret Thatcher)"};

        writeToDeviceFile(m_DeviceFile, quoteFromMargaretThatcher);

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 109);

        ioctlSetOutputPrefix("Quote: ");

        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote: The biggest problem ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote: of socialism is that");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote:  they eventually run");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote:  out of other people");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote: 's money! (Margaret ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote: Thatcher)");

        // test auto-resetting (prefix stays the same)
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Quote: " + quoteFromMargaretThatcher);

        /* start over and continue with variable prefix */

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 109);

        ioctlSetOutputPrefix("Part 1: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Part 1: The biggest problem ");

        ioctlSetOutputPrefix("Part 2: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Part 2: of socialism is that");

        ioctlSetOutputPrefix("Let's continue:");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Let's continue: they eventually run");

        ioctlSetOutputPrefix("");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " out of other people");

        ioctlSetOutputPrefix("Another part comes: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Another part comes: 's money! (Margaret ");

        ioctlSetOutputPrefix("End part: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "End part: Thatcher)");

        // test auto-resetting (prefix stays the same)
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "End part: " + quoteFromMargaretThatcher);

        // test again with changed prefix
        ioctlSetOutputPrefix("Starting again: ");

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Starting again: " + quoteFromMargaretThatcher);
    }

    {
        writeToDeviceFile(m_DeviceFile, "1a2b3c4d5e6f7g8h");

        maxOutputSize = 4;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("Initial: ");

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Initial: 1a2b");

        maxOutputSize = 5;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("Subsequent: ");

        QVERIFY(ioctlGetMaxOutputSize() == 5);
        QVERIFY(remainingCharsToReadCount == 12);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Subsequent: 3c4d5");

        maxOutputSize = 3;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("To continue: ");

        QVERIFY(ioctlGetMaxOutputSize() == 3);
        QVERIFY(remainingCharsToReadCount == 7);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "To continue: e6f");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "To continue: 7g8");

        maxOutputSize = 2;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("Ending: ");

        QVERIFY(ioctlGetMaxOutputSize() == 2);
        QVERIFY(remainingCharsToReadCount == 1);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Ending: h");

        ioctlSetOutputPrefix("Wrapped up: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Wrapped up: 1a2b3c4d5e6f7g8h");

        ioctlSetOutputPrefix("");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        maxOutputSize = 2;

        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 2);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a");

        maxOutputSize = 4;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("A new prefix: ");

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 14);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "A new prefix: 2b3c");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "A new prefix: 4d5e");

        maxOutputSize = 3;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("");

        QVERIFY(ioctlGetMaxOutputSize() == 3);
        QVERIFY(remainingCharsToReadCount == 6);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "6f7");

        ioctlSetOutputPrefix("Another new prefix: ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Another new prefix: g8h");

        // test auto-resetting
        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Another new prefix: 1a2b3c4d5e6f7g8h");

        // test again with changed prefix
        ioctlSetOutputPrefix("To conclude: ");

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "To conclude: 1a2b3c4d5e6f7g8h");
    }
}

void IoctlStringOpsModuleTests::testSetMaxOutputSize_UseManualOutputSizeReset()
{
    size_t value;

    size_t& maxOutputSize{value};
    const size_t& remainingCharsToReadCount{value};

    {
        const std::string quoteFromMargaretThatcher{
            "The biggest problem of socialism is that they eventually run out of other "
            "people's money! (Margaret Thatcher)"};

        writeToDeviceFile(m_DeviceFile, quoteFromMargaretThatcher);

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);

        maxOutputSize = 20;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 20);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "The biggest problem ");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "of socialism is that");

        maxOutputSize = 5;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 5);
        QVERIFY(remainingCharsToReadCount == 69);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == " they");

        QVERIFY(ioctlGetMaxOutputSize() == 5);

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);

        maxOutputSize = 110;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 109);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);

        // test manual reset after auto-reset
        QVERIFY(ioctlGetMaxOutputSize() == 0);

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 109);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == quoteFromMargaretThatcher);
    }

    {
        writeToDeviceFile(m_DeviceFile, "1a2b3c4d5e6f7g8h");
        ioctlSetOutputPrefix("Just a prefix: ");

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 1a2b3c4d5e6f7g8h");

        maxOutputSize = 4;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("Just a prefix: ");

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 1a2b");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 3c4d");

        maxOutputSize = 2;

        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 2);
        QVERIFY(remainingCharsToReadCount == 8);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 5e");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 6f");

        QVERIFY(ioctlGetMaxOutputSize() == 2);

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 1a2b3c4d5e6f7g8h");

        maxOutputSize = 8;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 8);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 1a2b3c4d");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 5e6f7g8h");

        // test manual reset after auto-reset
        QVERIFY(ioctlGetMaxOutputSize() == 0);

        maxOutputSize = 0;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just a prefix: 1a2b3c4d5e6f7g8h");

        // test again with reset prefix
        ioctlSetOutputPrefix("");

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "1a2b3c4d5e6f7g8h");

        // final test with the ioctl reset command
        maxOutputSize = 4;

        ioctlSetMaxOutputSize(maxOutputSize);
        ioctlSetOutputPrefix("Just another prefix: ");

        QVERIFY(ioctlGetMaxOutputSize() == 4);
        QVERIFY(remainingCharsToReadCount == 16);
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just another prefix: 1a2b");
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "Just another prefix: 3c4d");

        QVERIFY(ioctlGetMaxOutputSize() == 4);

        resetKernelModule();

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(isKernelModuleReset());
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "");

        maxOutputSize = 1;
        ioctlSetMaxOutputSize(maxOutputSize);

        QVERIFY(ioctlGetMaxOutputSize() == 0);
        QVERIFY(remainingCharsToReadCount == 0);
        QVERIFY(isKernelModuleReset());
        QVERIFY(readFromDeviceFile(m_DeviceFile) == "");
    }
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
        const long retVal{ioctl(fd, IOCTL_ENABLE_USER_INPUT_TRIMMING, &enabled)};
        close(fd);

        if (retVal != 0)
        {
            QFAIL("Enabling/disabling trimming failed!");
        }
    }
}

bool IoctlStringOpsModuleTests::ioctlIsUserInputTrimmingEnabled()
{
    bool isEnabled{false};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        bool isTrimmingEnabled;
        const long retVal{ioctl(fd, IOCTL_IS_USER_INPUT_TRIMMING_ENABLED, &isTrimmingEnabled)};

        if (retVal == 0)
        {
            isEnabled = isTrimmingEnabled;
        }

        close(fd);
    }

    return isEnabled;
}

std::optional<size_t> IoctlStringOpsModuleTests::ioctlGetBufferSize()
{
    std::optional<size_t> result{0};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        size_t charsCountFromBuffer;

        const long retVal{ioctl(fd, IOCTL_GET_BUFFER_SIZE, (size_t*)&charsCountFromBuffer)};

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

std::optional<size_t> IoctlStringOpsModuleTests::ioctlGetOutputPrefixSize()
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

void IoctlStringOpsModuleTests::ioctlEnableInputAppendMode(bool enabled)
{
    const int fd{open(m_DeviceFile.c_str(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_ENABLE_INPUT_APPEND_MODE, &enabled)};
        close(fd);

        if (retVal != 0)
        {
            QFAIL("Enabling/disabling trimming failed!");
        }
    }
}

bool IoctlStringOpsModuleTests::ioctlIsInputAppendModeEnabled()
{
    bool isEnabled{false};
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        bool isAppendingEnabled;
        const long retVal{ioctl(fd, IOCTL_IS_INPUT_APPEND_MODE_ENABLED, &isAppendingEnabled)};

        if (retVal == 0)
        {
            isEnabled = isAppendingEnabled;
        }

        close(fd);
    }

    return isEnabled;
}

bool IoctlStringOpsModuleTests::ioctlSetMaxOutputSize(size_t& value)
{
    bool success{false};
    const int fd{open(m_DeviceFile.c_str(), O_RDWR)};

    if (fd > 0)
    {
        size_t val{value};
        const long retVal{ioctl(fd, IOCTL_SET_MAX_OUTPUT_SIZE, &val)};

        if (retVal == 0)
        {
            value = val;
            success = true;
        }

        close(fd);
    }

    return success;
}

std::optional<size_t> IoctlStringOpsModuleTests::ioctlGetMaxOutputSize()
{
    std::optional<size_t> result;
    const int fd{open(m_DeviceFile.c_str(), O_RDONLY)};

    if (fd > 0)
    {
        size_t maxOutputSize;
        const auto retVal{ioctl(fd, IOCTL_GET_MAX_OUTPUT_SIZE, (size_t*)&maxOutputSize)};

        if (retVal == 0)
        {
            result = maxOutputSize;
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
