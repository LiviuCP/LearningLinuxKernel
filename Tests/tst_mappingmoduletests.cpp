#include <QTest>

#include <cassert>
#include <filesystem>
#include <list>
#include <map>
#include <string_view>

#include "utils.h"

static constexpr std::string_view keyFilePath{"/sys/kernel/mapping/key"};
static constexpr std::string_view valueFilePath{"/sys/kernel/mapping/value"};
static constexpr std::string_view commandFilePath{"/sys/kernel/mapping/command"};
static constexpr std::string_view statusFilePath{"/sys/kernel/mapping/status"};
static constexpr std::string_view countFilePath{"/sys/kernel/mapping/count"};
static constexpr std::string_view mapDirPath{"/sys/kernel/mapping/Map"};

static constexpr std::string_view updateCommandStr{"update"};
static constexpr std::string_view removeCommandStr{"remove"};
static constexpr std::string_view resetCommandStr{"reset"};
static constexpr std::string_view getCommandStr{"get"};

static constexpr std::string_view syncedStatusStr{"synced"};
static constexpr std::string_view dirtyStatusStr{"dirty"};

static constexpr std::string_view mappingModuleName{"mapping"};

using ElementsMap = std::map<std::string, int>;
using ElementsList = std::list<std::pair<std::string, int>>;

/* These tests should be run from a terminal using sudo */

class MappingModuleTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testAddElement();
    void testAddMultipleElements();
    void testModifyElementValue();
    void testRemoveElement();
    void testGetElementValue();
    void testAllCommands();

private:
    bool isKernelModuleReset();
    bool areKernelModuleFilesAndDirsValid();

    void writeKey(const std::string& key);
    std::optional<std::string> readKey();
    void writeValue(int value);
    std::optional<int> readValue();
    void writeCommand(const std::string& command);
    std::optional<std::string> readStatus();
    std::optional<size_t> readCount();

    void addOrModifyElements(const ElementsList& elements);
    std::optional<ElementsMap> retrieveElements();

    std::optional<std::filesystem::path> getMappingModulePath();
};

void MappingModuleTests::initTestCase()
{
    try
    {
        const auto mappingModulePath{getMappingModulePath()};
        QVERIFY(mappingModulePath.has_value());

        // ensure a fresh start by reloading the module in case already loaded
        if (Utilities::isKernelModuleLoaded(mappingModuleName))
        {
            Utilities::unloadKernelModule(mappingModuleName);
        }

        Utilities::loadKernelModule(*mappingModulePath);

        QVERIFY(Utilities::isKernelModuleLoaded(mappingModuleName));
        QVERIFY(areKernelModuleFilesAndDirsValid());

        QVERIFY(readKey().has_value() && readValue().has_value() && readStatus().has_value() &&
                readCount().has_value());
    }
    catch (const std::runtime_error& err)
    {
        QFAIL(err.what());
    }
}

void MappingModuleTests::cleanupTestCase()
{
    try
    {
        if (Utilities::isKernelModuleLoaded(mappingModuleName))
        {
            Utilities::unloadKernelModule(mappingModuleName);
        }
    }
    catch (const std::runtime_error& err)
    {
        QFAIL(err.what());
    }
}

void MappingModuleTests::init()
{
    QVERIFY(isKernelModuleReset());
}

void MappingModuleTests::cleanup()
{
    writeCommand(std::string{resetCommandStr});
}

void MappingModuleTests::testAddElement()
{
    // test valid key
    const std::string validKey{"testKey"};
    const int elementValue{55};

    writeKey(validKey);

    QVERIFY(validKey == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeValue(elementValue);

    QVERIFY(elementValue == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{updateCommandStr});

    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(1 == readCount());

    std::filesystem::path mapElementKeyPath{std::string{mapDirPath}};
    mapElementKeyPath /= validKey;

    QVERIFY(std::filesystem::is_directory(mapElementKeyPath));

    std::filesystem::path mapElementValuePath{mapElementKeyPath};
    mapElementValuePath /= "value";

    QVERIFY(elementValue == Utilities::readIntValueFromFile(mapElementValuePath));

    const std::optional<ElementsMap> mapContent{retrieveElements()};
    const ElementsMap expectedMapContent{{validKey, 55}};

    QVERIFY(expectedMapContent == mapContent);

    // test invalid (empty or whitespace-only) key
    const std::string invalidKey{" "};

    writeKey(invalidKey);

    const std::string emptyKey;

    QVERIFY(emptyKey == readKey());
    QVERIFY(elementValue == readValue());

    writeCommand(std::string{updateCommandStr});

    QVERIFY(dirtyStatusStr == readStatus());
    QVERIFY(1 == readCount());
    QVERIFY(expectedMapContent == retrieveElements());
}

void MappingModuleTests::testAddMultipleElements()
{
    const ElementsList elements{{"myhome", -2}, {"home", 3},       {"barbeque", 4}, {"homealone", 5},
                                {"Home", 0},    {"barbeque1", -5}, {"HOME2", 9}};

    addOrModifyElements(elements);

    QVERIFY(elements.size() == readCount());

    const std::optional<ElementsMap> mapContent{retrieveElements()};
    const ElementsMap expectedMapContent{{"HOME2", 9}, {"Home", 0},      {"barbeque", 4}, {"barbeque1", -5},
                                         {"home", 3},  {"homealone", 5}, {"myhome", -2}};

    QVERIFY(expectedMapContent == mapContent);
}

void MappingModuleTests::testModifyElementValue()
{
    const std::string key{"myKey"};
    int value{-5};

    writeKey(key);
    writeValue(value);
    writeCommand(std::string{updateCommandStr});

    ElementsMap expectedContent{{"myKey", -5}};

    QVERIFY(expectedContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());

    value = 3;
    writeValue(value);

    QVERIFY(key == readKey());
    QVERIFY(value == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{updateCommandStr});
    expectedContent = {{"myKey", 3}};

    QVERIFY(expectedContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(1 == readCount());
}

void MappingModuleTests::testRemoveElement()
{
    const ElementsList elements{{"household", 2}, {"homebank", -5}, {"banking", 10}};
    addOrModifyElements(elements);

    std::string keyToRemove{"banking"};

    QVERIFY(keyToRemove == readKey());
    QVERIFY(10 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    ElementsMap expectedMapContent{{"homebank", -5}, {"household", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(2 == readCount());

    const std::string emptyKey;

    QVERIFY(emptyKey == readKey());
    QVERIFY(0 == readValue());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(2 == readCount());
    QVERIFY(emptyKey == readKey());
    QVERIFY(0 == readValue());

    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(dirtyStatusStr == readStatus());
    QVERIFY(2 == readCount());
    QVERIFY(keyToRemove == readKey());

    keyToRemove = "homeban";
    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(dirtyStatusStr == readStatus());
    QVERIFY(2 == readCount());
    QVERIFY(keyToRemove == readKey());

    keyToRemove = "homebank1";
    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(dirtyStatusStr == readStatus());
    QVERIFY(2 == readCount());
    QVERIFY(keyToRemove == readKey());

    keyToRemove = "homebank";
    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});
    expectedMapContent = {{"household", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(1 == readCount());
    QVERIFY(emptyKey == readKey());

    keyToRemove = "Household";
    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(dirtyStatusStr == readStatus());
    QVERIFY(1 == readCount());
    QVERIFY(keyToRemove == readKey());

    keyToRemove = "household";
    writeKey(keyToRemove);

    QVERIFY(keyToRemove == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(isKernelModuleReset());
}

void MappingModuleTests::testGetElementValue()
{
    writeKey("laundromat");
    writeValue(10);

    QVERIFY("laundromat" == readKey());
    QVERIFY(10 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("laundromat" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(0 == readCount());

    const ElementsList elements{{"laundromat", 2}, {"banking", -5}};
    addOrModifyElements(elements);

    QVERIFY("banking" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(2 == readCount());

    writeKey("laundromat");

    QVERIFY("laundromat" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("laundromat" == readKey());
    QVERIFY(2 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeValue(10);
    writeCommand(std::string{updateCommandStr});

    QVERIFY("laundromat" == readKey());
    QVERIFY(10 == readValue());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(2 == readCount());

    writeValue(0);

    QVERIFY("laundromat" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("laundromat" == readKey());
    QVERIFY(10 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});

    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(1 == readCount());

    writeKey("laundromat");
    writeValue(10);

    QVERIFY("laundromat" == readKey());
    QVERIFY(10 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("laundromat" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeKey("bankin");
    writeValue(-5);

    QVERIFY("bankin" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("bankin" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeKey("Banking");
    writeValue(-5);

    QVERIFY("Banking" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("Banking" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeKey("banking1");
    writeValue(-5);

    QVERIFY("banking1" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("banking1" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeKey("banking");

    QVERIFY("banking" == readKey());
    QVERIFY(0 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("banking" == readKey());
    QVERIFY(-5 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    const ElementsMap expectedMapContent{{"banking", -5}};
    QVERIFY(expectedMapContent == retrieveElements());
}

void MappingModuleTests::testAllCommands()
{
    ElementsList elements{{"laundromat", 2}, {"banking", -5}, {"Laundromat", 4},
                          {"banking", -2},   {"banking", 10}, {"laundro", 8}};

    addOrModifyElements(elements);

    ElementsMap expectedMapContent{{"Laundromat", 4}, {"banking", 10}, {"laundro", 8}, {"laundromat", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(4 == readCount());

    writeKey("banking");

    QVERIFY("banking" == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{removeCommandStr});
    expectedMapContent = {{"Laundromat", 4}, {"laundro", 8}, {"laundromat", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(3 == readCount());

    writeKey("Laundromat");

    QVERIFY("Laundromat" == readKey());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{getCommandStr});

    QVERIFY("Laundromat" == readKey());
    QVERIFY(4 == readValue());
    QVERIFY(syncedStatusStr == readStatus());

    writeKey("banking");

    QVERIFY("banking" == readKey());
    QVERIFY(4 == readValue());
    QVERIFY(dirtyStatusStr == readStatus());

    writeCommand(std::string{updateCommandStr});
    expectedMapContent = {{"Laundromat", 4}, {"banking", 4}, {"laundro", 8}, {"laundromat", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(4 == readCount());

    writeCommand(std::string{resetCommandStr});
    QVERIFY(isKernelModuleReset());

    elements = {{"laundro", 8},    {"banking", 10}, {"banking", -2},
                {"Laundromat", 4}, {"banking", -5}, {"laundromat", 2}};

    addOrModifyElements(elements);
    expectedMapContent = {{"Laundromat", 4}, {"banking", -5}, {"laundro", 8}, {"laundromat", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(4 == readCount());

    writeCommand(std::string{resetCommandStr});
    QVERIFY(isKernelModuleReset());

    elements = {{"laundro", 8}, {" ", 5}, {"Laundromat", 4}, {"", -5}, {"laundromat", 2}};

    addOrModifyElements(elements);
    expectedMapContent = {{"Laundromat", 4}, {"laundro", 8}, {"laundromat", 2}};

    QVERIFY(expectedMapContent == retrieveElements());
    QVERIFY(syncedStatusStr == readStatus());
    QVERIFY(3 == readCount());
}

bool MappingModuleTests::isKernelModuleReset()
{
    const auto key{readKey()};
    const auto value{readValue()};
    const auto status{readStatus()};
    const auto count{readCount()};
    const std::string emptyKey;

    assert(key.has_value() && value.has_value() && status.has_value() && count.has_value());

    return emptyKey == key && 0 == value && syncedStatusStr == status && 0 == count;
}

bool MappingModuleTests::areKernelModuleFilesAndDirsValid()
{
    return std::filesystem::exists(keyFilePath) && std::filesystem::is_regular_file(keyFilePath) &&
           std::filesystem::exists(valueFilePath) && std::filesystem::is_regular_file(valueFilePath) &&
           std::filesystem::exists(commandFilePath) && std::filesystem::is_regular_file(commandFilePath) &&
           std::filesystem::exists(statusFilePath) && std::filesystem::is_regular_file(statusFilePath) &&
           std::filesystem::exists(countFilePath) && std::filesystem::is_regular_file(countFilePath) &&
           std::filesystem::exists(mapDirPath) && std::filesystem::is_directory(mapDirPath);
}

void MappingModuleTests::writeKey(const std::string& key)
{
    if (!key.empty())
    {
        Utilities::writeStringToFile(key, keyFilePath);
    }
    else
    {
        try
        {
            Utilities::clearFileContent(keyFilePath);
        }
        catch (std::runtime_error& err)
        {
            QFAIL(err.what());
        }
    }
}

std::optional<std::string> MappingModuleTests::readKey()
{
    return Utilities::readStringFromFile(keyFilePath);
}

void MappingModuleTests::writeValue(int value)
{
    Utilities::writeStringToFile(std::to_string(value), valueFilePath);
}

std::optional<int> MappingModuleTests::readValue()
{
    return Utilities::readIntValueFromFile(valueFilePath);
}

void MappingModuleTests::writeCommand(const std::string& command)
{
    Utilities::writeStringToFile(command, commandFilePath);
}

std::optional<std::string> MappingModuleTests::readStatus()
{
    return Utilities::readStringFromFile(statusFilePath);
}

std::optional<size_t> MappingModuleTests::readCount()
{
    std::optional<size_t> count;
    const std::optional<int> countValue{Utilities::readIntValueFromFile(countFilePath)};

    if (countValue.has_value() && countValue >= 0)
    {
        count = static_cast<size_t>(*countValue);
    }

    return count;
}

void MappingModuleTests::addOrModifyElements(const ElementsList& elements)
{
    for (const auto& [elementKey, elementValue] : elements)
    {
        writeKey(elementKey);
        writeValue(elementValue);
        writeCommand(std::string{updateCommandStr});
    }
}

std::optional<ElementsMap> MappingModuleTests::retrieveElements()
{
    std::optional<ElementsMap> mapContent;

    if (std::filesystem::is_directory(mapDirPath))
    {
        ElementsMap mapElements;
        bool invalidElementDetected{false};

        for (const auto& mapElement : std::filesystem::directory_iterator(mapDirPath))
        {
            const std::filesystem::path keyDirPath{mapElement.path()};

            if (!std::filesystem::is_directory(keyDirPath))
            {
                invalidElementDetected = true;
                break;
            }

            std::filesystem::path valueFilePath{keyDirPath};
            valueFilePath /= "value";

            if (!std::filesystem::is_regular_file(valueFilePath))
            {
                invalidElementDetected = true;
                break;
            }

            const std::optional<int> value{Utilities::readIntValueFromFile(valueFilePath)};

            if (!value.has_value())
            {
                invalidElementDetected = true;
                break;
            }

            mapElements.insert({keyDirPath.filename().string(), *value});
        }

        if (!invalidElementDetected)
        {
            mapContent = std::move(mapElements);
        }
    }

    return mapContent;
}

std::optional<std::filesystem::path> MappingModuleTests::getMappingModulePath()
{
    std::filesystem::path mappingModulePath{Utilities::getApplicationPath()};
    mappingModulePath = mappingModulePath.parent_path().parent_path();
    mappingModulePath /= Utilities::getModulesDirRelativePath();
    mappingModulePath /= mappingModuleName;
    mappingModulePath += Utilities::getModuleFileExtension();

    return std::filesystem::is_regular_file(mappingModulePath) ? std::optional{mappingModulePath} : std::nullopt;
}

QTEST_APPLESS_MAIN(MappingModuleTests)

#include "tst_mappingmoduletests.moc"
