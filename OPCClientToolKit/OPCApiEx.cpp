#pragma once

#include "OPCApiEx.h"

#include "OPCServer.h"
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
using Json = nlohmann::json;

struct OPCJsonItem
{
    int id;
    string name;

    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(OPCJsonItem, id, name)
};
struct OPCJsonGroup
{
    string name;
    unsigned long updateRate;
    float deadBand;
    bool subscribe;
    vector<OPCJsonItem> items;

    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(OPCJsonGroup, name, subscribe, items)
};
struct OPCJson
{
    string host;
    string server;
    vector<OPCJsonGroup> groups;

    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(OPCJson, host, server)
};
OPCJson readOPCJson(const string &jsonFile)
{
    ifstream ifs(jsonFile);
    if (!ifs.is_open())
    {
        throw OPCException(L"Json file open failed!");
    }
    // OPCJson data = Json::parse(ifs);
    Json json = Json::parse(ifs);
    if (!json.is_object())
    {
        throw OPCException(L"json is not object");
    }
    Json hostKey = json.at("Host");
    if (!hostKey.is_string())
    {
        throw OPCException(L"Host field is not string type");
    }
    OPCJson data;
    hostKey.get_to(data.host);
    if (data.host.empty())
    {
        data.host = "localhost";
    }
    Json serverKey = json.at("Server");
    if (!serverKey.is_string())
    {
        throw OPCException(L"Server field is not string type");
    }
    serverKey.get_to(data.server);
    if (data.server.empty())
    {
        throw OPCException(L"Server field is empty");
    }
    Json groups = json.at("Groups");
    if (!groups.is_array())
    {
        throw OPCException(L"Groups field is not array");
    }
    for (int i = 0; i < groups.size(); ++i)
    {
        OPCJsonGroup _group;

        Json group = groups[i];
        Json groupNameKey = group.at("Group");
        if (!groupNameKey.is_string())
        {
            throw OPCException(L"Group field is not string type");
        }
        groupNameKey.get_to(_group.name);
        if (_group.name.empty())
        {
            throw OPCException(L"Group field is empty");
        }
        Json items = group.at("Variables");
        if (!items.is_array())
        {
            throw OPCException(L"Variables field is not array");
        }
        for (int j = 0; j < items.size(); ++j)
        {
            OPCJsonItem _item;

            Json item = items[j];
            if (!item.is_object())
            {
                throw OPCException(L"Variable field is not object");
            }
            Json itemIdKey = item.at("Id");
            if (!itemIdKey.is_number_integer())
            {
                throw OPCException(L"Variable Id field is not int type");
            }
            itemIdKey.get_to(_item.id);
            Json itemNameKey = item.at("Name");
            if (!itemNameKey.is_string())
            {
                throw OPCException(L"Variable Name field is not string type");
            }
            itemNameKey.get_to(_item.name);
            if (_item.name.empty())
            {
                throw OPCException(L"Variable Name field is empty");
            }

            _group.items.push_back(_item);
        }
        Json updateRateKey = group.at("UpdateRate");
        if (updateRateKey.is_null())
        {
            _group.updateRate = 1000;
        }
        else
        {
            if (!updateRateKey.is_number_unsigned())
            {
                throw OPCException(L"Group UpdateRate field is not ulong type");
            }
            updateRateKey.get_to(_group.updateRate);
            if (_group.updateRate < 100)
            {
                _group.updateRate = 100;
            }
        }
        Json deadBandKey = group.at("DeadZone");
        if (deadBandKey.is_null())
        {
            _group.deadBand = 0.0;
        }
        else
        {
            if (!deadBandKey.is_number())
            {
                throw OPCException(L"Group DeadZone field is not float type");
            }
            deadBandKey.get_to(_group.deadBand);
        }
        Json subscribeKey = group.at("IsSubscribe");
        if (subscribeKey.is_null())
        {
            _group.subscribe = false;
        }
        else
        {
            if (!subscribeKey.is_boolean())
            {
                throw OPCException(L"Group IsSubscribe field is not bool type");
            }
            subscribeKey.get_to(_group.subscribe);
        }

        data.groups.push_back(_group);
    }
    return data;
}
// 116444736000000000: 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
const uint64_t DATETIMEDIFF = 116444736000000000;
/// <summary>
/// 精确到纳秒
/// </summary>
/// <param name="fileTime"></param>
/// <returns></returns>
static uint64_t ConvertFiletimeToLong(FILETIME fileTime) noexcept
{
    return (((static_cast<uint64_t>(fileTime.dwHighDateTime)) << 32 | static_cast<uint64_t>(fileTime.dwLowDateTime)) -
            DATETIMEDIFF) *
           100;
}
/// <summary>
/// 精确到纳秒
/// </summary>
/// <param name="timestamp"></param>
/// <returns></returns>
static FILETIME ConvertLongToFiletime(uint64_t timestamp) noexcept
{
    const auto fileTime = timestamp / 100 + DATETIMEDIFF;
    return FILETIME{static_cast<DWORD>(fileTime), static_cast<DWORD>(fileTime >> 32)};
}
static double ConvertDecimalToDouble(const DECIMAL &dec) noexcept
{
    const ULONGLONG integerPart = static_cast<ULONGLONG>(dec.Hi32) << 32 | (dec.Mid32 << 16) | dec.Lo32;

    double result = static_cast<double>(integerPart);

    // Adjust for the scale (decimal places)
    result /= std::pow(10, dec.scale);

    // Handle the sign
    if (dec.sign)
        result = -result;

    return result;
}
static DECIMAL ConvertDoubleToDecimal(double value) noexcept
{
    DECIMAL decValue;

    // Handle sign
    decValue.sign = (value < 0) ? 0x80 : 0; // 0x80 represents the sign bit
    value = std::abs(value);

    // Calculate integer and fractional parts
    decValue.Lo64 = static_cast<ULONGLONG>(value * 10000 + 0.5); // Assuming 4 decimal places

    // Calculate the scale (number of decimal places)
    int scale = 0;
    while (value < 1.0)
    {
        value *= 10.0;
        scale++;
    }
    decValue.scale = scale;

    return decValue;
}
static int8_t ConvertOPCDataToByteArray(OPCItemData data, vector<uint8_t> &byteArray) noexcept
{
    const auto dataType = data.vDataValue.vt;
    switch (dataType)
    {
    case VT_EMPTY:
        byteArray.resize(0);
        return 1;
    case VT_NULL:
        byteArray.resize(0);
        return 1;
    case VT_I2:
        byteArray.resize(sizeof(SHORT));
        memcpy(byteArray.data(), &data.vDataValue.iVal, sizeof(SHORT));
        return 1;
    case VT_I4:
        byteArray.resize(sizeof(INT));
        memcpy(byteArray.data(), &data.vDataValue.intVal, sizeof(INT));
        return 1;
    case VT_R4:
        byteArray.resize(sizeof(FLOAT));
        memcpy(byteArray.data(), &data.vDataValue.fltVal, sizeof(FLOAT));
        return 1;
    case VT_R8:
        byteArray.resize(sizeof(DOUBLE));
        memcpy(byteArray.data(), &data.vDataValue.dblVal, sizeof(DOUBLE));
        return 1;
    case VT_CY:
        byteArray.resize(sizeof(LONGLONG));
        memcpy(byteArray.data(), &data.vDataValue.cyVal.int64, sizeof(LONGLONG));
        return 1;
    case VT_DATE: {
        byteArray.resize(sizeof(uint64_t));
        if (data.vDataValue.date <= 0)
        {
            return 1;
        }
        SYSTEMTIME d;
        FILETIME ft;
        VariantTimeToSystemTime(data.vDataValue.date, &d);
        SystemTimeToFileTime(&d, &ft);
        const auto timestamp = ConvertFiletimeToLong(ft);
        memcpy(byteArray.data(), &timestamp, sizeof(uint64_t));
    }
        return 1;
    case VT_BSTR: {
        const string str = COPCHost::WS2S(data.vDataValue.bstrVal);
        byteArray.resize(str.size());
        memcpy(byteArray.data(), str.c_str(), str.size());
    }
        return 1;
    /*case VT_DISPATCH:
        return 1;*/
    case VT_ERROR:
        byteArray.resize(sizeof(SCODE));
        memcpy(byteArray.data(), &data.vDataValue.scode, sizeof(SCODE));
        return 1;
    case VT_BOOL:
        byteArray.resize(sizeof(uint8_t));
        if (data.vDataValue.boolVal != VARIANT_FALSE)
        {
            byteArray[0] = 1;
        }
        else
        {
            byteArray[0] = 0;
        }
        // memcpy(byteArray.data(), &data.vDataValue.boolVal, sizeof(uint8_t));
        return 1;
    /*case VT_VARIANT:
        return 1;*/
    /*case VT_UNKNOWN:
        return 1;*/
    case VT_DECIMAL: {
        double decVal = ConvertDecimalToDouble(data.vDataValue.decVal);
        byteArray.resize(sizeof(double));
        memcpy(byteArray.data(), &decVal, sizeof(double));
    }
        return 1;
    /*case VT_RECORD:
        return 1;*/
    case VT_I1:
        byteArray.resize(sizeof(BYTE));
        memcpy(byteArray.data(), &data.vDataValue.bVal, sizeof(BYTE));
        return 1;
    case VT_UI1:
        byteArray.resize(sizeof(CHAR));
        memcpy(byteArray.data(), &data.vDataValue.cVal, sizeof(CHAR));
        return 1;
    case VT_UI2:
        byteArray.resize(sizeof(USHORT));
        memcpy(byteArray.data(), &data.vDataValue.uiVal, sizeof(SHORT));
        return 1;
    case VT_UI4:
        byteArray.resize(sizeof(ULONG));
        memcpy(byteArray.data(), &data.vDataValue.ulVal, sizeof(ULONG));
        return 1;
    case VT_I8:
        byteArray.resize(sizeof(LONG64));
        memcpy(byteArray.data(), &data.vDataValue.ulVal, sizeof(LONG64));
        return 1;
    case VT_UI8:
        byteArray.resize(sizeof(ULONG64));
        memcpy(byteArray.data(), &data.vDataValue.ulVal, sizeof(ULONG64));
        return 1;
    case VT_INT:
        byteArray.resize(sizeof(INT));
        memcpy(byteArray.data(), &data.vDataValue.intVal, sizeof(INT));
        return 1;
    case VT_UINT:
        byteArray.resize(sizeof(UINT));
        memcpy(byteArray.data(), &data.vDataValue.uintVal, sizeof(UINT));
        return 1;
    /*case VT_ARRAY:
        return 1;
    case VT_BYREF:
        return 1;*/
    default:
        byteArray.resize(0);
        printf("data type unsupported: %d\n", dataType);
        break;
    }
    return 0;
}
static bool ConvertByteArrayToOPCData(uint8_t *byteArray, VARIANT *value) noexcept
{
    if (!value)
    {
        return false;
    }
    const auto dataType = (*value).vt;
    switch (dataType)
    {
    case VT_EMPTY:
        return true;
    case VT_NULL:
        return true;
    case VT_I2:
        memcpy(&(*value).iVal, byteArray, sizeof(SHORT));
        return true;
    case VT_I4:
        memcpy(&(*value).intVal, byteArray, sizeof(INT));
        return true;
    case VT_R4:
        memcpy(&(*value).fltVal, byteArray, sizeof(FLOAT));
        return true;
    case VT_R8:
        memcpy(&(*value).dblVal, byteArray, sizeof(DOUBLE));
        return true;
    case VT_CY:
        memcpy(&(*value).cyVal.int64, byteArray, sizeof(LONGLONG));
        return true;
    case VT_DATE: {
        uint64_t timestamp;
        memcpy(&timestamp, byteArray, sizeof(uint64_t));
        const auto ft = ConvertLongToFiletime(timestamp);
        SYSTEMTIME d;
        FileTimeToSystemTime(&ft, &d);
        SystemTimeToVariantTime(&d, &(*value).date);
        // memcpy(&(*value).date, byteArray, sizeof(DATE));
    }
        return true;
    case VT_BSTR: {
        const string str = (char *)byteArray;
        const CComBSTR wstr = CComBSTR(COPCHost::S2WS(str).c_str());
        (*value).bstrVal = wstr;
    }
        return true;
    /*case VT_DISPATCH:
        return true;*/
    case VT_ERROR:
        memcpy(&(*value).scode, byteArray, sizeof(SCODE));
        return true;
    case VT_BOOL: {
        if (*byteArray != 0)
        {
            (*value).boolVal = VARIANT_TRUE;
        }
        else
        {
            (*value).boolVal = VARIANT_FALSE;
        }
        // memcpy(&(*value).boolVal, byteArray, sizeof(VARIANT_BOOL));
    }
        return true;
    /*case VT_VARIANT:
        return true;*/
    /*case VT_UNKNOWN:
        return true;*/
    case VT_DECIMAL: {
        double val;
        memcpy(&val, byteArray, sizeof(double));
        (*value).decVal = ConvertDoubleToDecimal(val);
    }
        return true;
    /*case VT_RECORD:
        return true;*/
    case VT_I1:
        memcpy(&(*value).bVal, byteArray, sizeof(BYTE));
        return true;
    case VT_UI1:
        memcpy(&(*value).cVal, byteArray, sizeof(CHAR));
        return true;
    case VT_UI2:
        memcpy(&(*value).uiVal, byteArray, sizeof(USHORT));
        return true;
    case VT_UI4:
        memcpy(&(*value).ulVal, byteArray, sizeof(ULONG));
        return true;
    case VT_I8:
        memcpy(&(*value).ulVal, byteArray, sizeof(LONG64));
        return true;
    case VT_UI8:
        memcpy(&(*value).ulVal, byteArray, sizeof(ULONG64));
        return true;
    case VT_INT:
        memcpy(&(*value).intVal, byteArray, sizeof(INT));
        return true;
    case VT_UINT:
        memcpy(&(*value).uintVal, byteArray, sizeof(UINT));
        return true;
    /*case VT_ARRAY:
        return true;
    case VT_BYREF:
        return true;*/
    default:
        printf("data type unsupported: %d\n", dataType);
        break;
    }
    return false;
}

void OPCManager::connect()
{
    Status = OPCManagerStatus::CONNECTING;

    OPCJson json = readOPCJson(JsonFile);
    printf("host: %s, server: %s\n", json.host.c_str(), json.server.c_str());

    COPCClient::init(OPCOLEInitMode::MULTITHREADED);

    Host = COPCClient::makeHost(COPCHost::S2WS(json.host));
    Server = Host->connectDAServer(COPCHost::S2WS(json.server));
    if (!Server)
    {
        throw OPCException(L"connect opc server failed");
    }
    Sleep(1000);
    ServerStatus status;
    if (!Server->getStatus(status))
    {
        throw OPCException(L"opc server status has error");
    }
    printf("server status: %d\n", status.dwServerState);
    if (status.dwServerState == tagOPCSERVERSTATE::OPC_STATUS_FAILED)
    {
        throw OPCException(L"opc server not running");
    }
    if (status.dwServerState == tagOPCSERVERSTATE::OPC_STATUS_COMM_FAULT)
    {
        throw OPCException(L"opc server not running");
    }
    for (auto &groupJson : json.groups)
    {
        unsigned long revisedUpdateRate_ms;
        COPCGroup *group = Server->makeGroup(COPCHost::S2WS(groupJson.name), true, groupJson.updateRate,
                                             revisedUpdateRate_ms, groupJson.deadBand);
        if (!group)
        {
            throw OPCException(L"opc client make group failed");
        }
        if (groupJson.subscribe)
        {
            SubscribeGroups.push_back(group);
        }
        vector<std::wstring> itemNames;
        std::vector<COPCItem *> itemsCreated;
        std::vector<HRESULT> errors;
        for (auto &itemJson : groupJson.items)
        {
            // printf("opc client add item: %s\n", itemJson.name.c_str());
            itemNames.push_back(COPCHost::S2WS(itemJson.name));
        }
        const int errorCount = group->addItems(itemNames, itemsCreated, errors, true);
        if (errorCount > 0)
        {
            printf("opc client add items has error: %s %d\n", groupJson.name.c_str(), errorCount);
            // throw OPCException(L"opc client add items has error");
        }
        for (size_t j = 0; j < itemsCreated.size(); j++)
        {
            OPCJsonItem itemJson = groupJson.items.at(j);
            COPCItem *item = itemsCreated.at(j);
            if (item)
            {
                ItemMap.SetAt(itemJson.id, item);
                ItemRevoteMap.SetAt(item->getHandle(), itemJson.id);
            }
        }
    }

    Status = OPCManagerStatus::CONNECTED;
}
std::mutex mtx;
void OPCManager::reconnect()
{
    if (Status == OPCManagerStatus::STOP)
    {
        return;
    }
    if (Status == OPCManagerStatus::CONNECTED)
    {
        return;
    }
    if (Status == OPCManagerStatus::CONNECTING)
    {
        Sleep(1000);
        return;
    }
    std::lock_guard<std::mutex> lock(mtx);

    Status = OPCManagerStatus::CONNECTING;

    try
    {
        COPCClient::stop();
    }
    catch (...)
    {
        printf("opc close failed\n");
    }
    SubscribeGroups.clear();
    ItemMap.RemoveAll();
    ItemRevoteMap.RemoveAll();

    unsubscribe();
    connect();
    subscribe();

    printf("SubscribeGroups: %d, Items: %d, %d\n", SubscribeGroups.size(), ItemMap.GetCount(),
           ItemRevoteMap.GetCount());

    Status = OPCManagerStatus::CONNECTED;
}
bool OPCManager::checkServerStatus(bool retryConnect)
{
    if (Status == OPCManagerStatus::STOP)
    {
        return false;
    }
    if (Status == OPCManagerStatus::CONNECTING)
    {
        Sleep(1000);
    }
    ServerStatus status;
    if (Status == OPCManagerStatus::DISCONNECTED)
    {
        if (retryConnect)
        {
            Sleep(1000);
            try
            {
                reconnect();
            }
            catch (OPCException ex2)
            {
                printf("opc server reconnect failed: %ws\n", ex2.reasonString().c_str());
                Status = OPCManagerStatus::DISCONNECTED;
                return false;
            }
            catch (...)
            {
                printf("opc server reconnect failed\n");
                Status = OPCManagerStatus::DISCONNECTED;
                return false;
            }
            for (size_t i = 0; i < 5; i++)
            {
                Sleep(1000);
                try
                {
                    if (!Server->getStatus(status))
                    {
                        throw OPCException(L"opc server status has error");
                    }
                    printf("server status: %d\n", status.dwServerState);
                    if (status.dwServerState != tagOPCSERVERSTATE::OPC_STATUS_RUNNING)
                    {
                        throw OPCException(L"opc server not running");
                    }
                    Status = OPCManagerStatus::CONNECTED;
                    return true;
                }
                catch (OPCException ex1)
                {
                    printf("opc reconnect error: %ws\n", ex1.reasonString().c_str());
                }
                catch (...)
                {
                    printf("opc reconnect error\n");
                }
            }
        }
        Status = OPCManagerStatus::DISCONNECTED;
        return false;
    }
    try
    {
        if (!Server->getStatus(status))
        {
            throw OPCException(L"opc server status has error");
        }
        printf("server status: %d\n", status.dwServerState);
        if (status.dwServerState != tagOPCSERVERSTATE::OPC_STATUS_RUNNING)
        {
            throw OPCException(L"opc server not running");
        }
        Status = OPCManagerStatus::CONNECTED;
        return true;
    }
    catch (OPCException ex)
    {
        printf("opc get status error: %ws\n", ex.reasonString().c_str());
        if (retryConnect)
        {
            Sleep(1000);
            try
            {
                reconnect();
            }
            catch (OPCException ex2)
            {
                printf("opc server reconnect failed: %ws\n", ex2.reasonString().c_str());
                Status = OPCManagerStatus::DISCONNECTED;
                return false;
            }
            catch (...)
            {
                printf("opc server reconnect failed\n");
                Status = OPCManagerStatus::DISCONNECTED;
                return false;
            }
            for (size_t i = 0; i < 5; i++)
            {
                Sleep(1000);
                try
                {
                    if (!Server->getStatus(status))
                    {
                        throw OPCException(L"opc server status has error");
                    }
                    printf("server status: %d\n", status.dwServerState);
                    if (status.dwServerState != tagOPCSERVERSTATE::OPC_STATUS_RUNNING)
                    {
                        throw OPCException(L"opc server not running");
                    }
                    Status = OPCManagerStatus::CONNECTED;
                    return true;
                }
                catch (OPCException ex1)
                {
                    printf("opc reconnect error: %ws\n", ex1.reasonString().c_str());
                }
                catch (...)
                {
                    printf("opc reconnect error\n");
                }
            }
        }
        Status = OPCManagerStatus::DISCONNECTED;
        return false;
    }
}

void OPCManager::read(const vector<int> &itemIds, vector<OPCItemData> &data)
{
    if (Status == OPCManagerStatus::STOP)
    {
        throw OPCException(L"opc stopped");
    }
    if (Status == OPCManagerStatus::CONNECTING || Status == OPCManagerStatus::DISCONNECTED)
    {
        throw OPCException(L"opc disconnected");
    }
    for (auto &itemId : itemIds)
    {
        COPCItem *item;
        if (ItemMap.Lookup(itemId, item))
        {
            OPCItemData value;
            try
            {
                if (item->readSync(value, OPCDATASOURCE::OPC_DS_DEVICE))
                {
                    data.push_back(value);
                    continue;
                }
            }
            catch (OPCException ex)
            {
                printf("opc read failed: %ws %ws\n", item->getName().c_str(), ex.reasonString().c_str());
            }
        }
        throw OPCException(L"opc read failed");
    }
}

int OPCManager::read(int itemId, OPCItemData &value, OPCDATASOURCE source)
{
    if (Status == OPCManagerStatus::STOP)
    {
        return -1;
    }
    if (Status == OPCManagerStatus::CONNECTING || Status == OPCManagerStatus::DISCONNECTED)
    {
        return -2;
    }
    COPCItem *item;
    if (ItemMap.Lookup(itemId, item))
    {
        try
        {
            if (item->readSync(value, source))
            {
                return 0;
            }
        }
        catch (OPCException ex)
        {
            printf("opc read failed: %ws %ws\n", item->getName().c_str(), ex.reasonString().c_str());
        }
        return -2;
    }
    return -1;
}

int OPCManager::write(int itemId, VARIANT data)
{
    if (Status == OPCManagerStatus::STOP)
    {
        return -1;
    }
    if (Status == OPCManagerStatus::CONNECTING || Status == OPCManagerStatus::DISCONNECTED)
    {
        return -2;
    }
    COPCItem *item;
    if (ItemMap.Lookup(itemId, item))
    {
        try
        {
            if (item->writeSync(data))
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        catch (OPCException ex)
        {
            printf("opc write failed: %ws %ws\n", item->getName().c_str(), ex.reasonString().c_str());
        }
        return -2;
    }
    return -1;
}

void SubscribeCallback::OnDataChange(COPCGroup &group, COPCItemDataMap &changes)
{
    if (!Callback)
    {
        return;
    }
    POSITION pos = changes.GetStartPosition();
    while (pos)
    {
        // const OPCHANDLE handle = changes.GetKeyAt(pos);
        OPCItemData *data = changes.GetNextValue(pos);
        if (data)
        {
            const COPCItem *item = data->item();
            if (!item)
            {
                printf("opc OnDataChange failed: %d %ws\n", changes.GetKeyAt(pos), item->getName().c_str());
            }
            int id;
            const OPCHANDLE handle = item->getHandle();
            if (!Manager->getItemId(handle, id))
            {
                continue;
            }
            try
            {
                VariableParameter param{};
                auto idStr = to_string(id);
                param.id = idStr.c_str();
                uint64_t timestamp = ConvertFiletimeToLong(data->ftTimeStamp);
                param.timestamp = &timestamp;
                int8_t status = 0;
                vector<uint8_t> buf;
                if (data->Error < 0)
                {
                    param.dataLength = 0;
                }
                else
                {
                    status = ConvertOPCDataToByteArray(*data, buf);
                    param.dataLength = buf.size();
                    param.data = buf.data();
                }
                param.status = &status;
                // printf("'%s'%d %d\n", idStr.c_str(), param.dataLength, data->Error);
                Callback(&param);
                /*printf("'%ws' %ws quality %d error %d\n", item->getName().c_str(), idStr.c_str(), data->wQuality,
                       data->Error);*/
            }
            catch (OPCException ex)
            {
                printf("opc SubscribeCallback failed: %ws %ws\n", item->getName().c_str(), ex.reasonString().c_str());
            }
        }
    }
}
void OPCManager::subscribe()
{
    for (auto &group : SubscribeGroups)
    {
        try
        {
            group->enableAsync(Callback);
        }
        catch (OPCException ex)
        {
            printf("opc subscribe failed: %ws %ws\n", group->getName().c_str(), ex.reasonString().c_str());
        }
    }
}

void OPCManager::unsubscribe()
{
    for (auto &group : SubscribeGroups)
    {
        try
        {
            group->disableAsync();
        }
        catch (OPCException ex)
        {
            printf("opc unsubscribe failed: %ws %ws\n", group->getName().c_str(), ex.reasonString().c_str());
        }
    }
}

void OPCManager::close()
{
    Status = OPCManagerStatus::STOP;
    if (Callback)
    {
        unsubscribe();
        delete Callback;
        Callback = nullptr;
    }
    try
    {
        COPCClient::stop();
    }
    catch (...)
    {
        printf("opc close failed\n");
    }
}

EnumDrvRet DriverCmd(const char *cmd, int *driverHandle, void *param)
{
    try
    {
        string cmdStr = {cmd};
        if (cmdStr == "InitDriver")
        {
            const InitDriverParameter *initParam = static_cast<InitDriverParameter *>(param);
            if (initParam->propertyLen < 1 || !initParam->property)
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            string jsonPath = initParam->property[0].value;
            printf("json file path: %s\n", jsonPath.c_str());
            OPCManager *opc = new OPCManager(jsonPath);
            try
            {
                opc->connect();
            }
            catch (OPCException ex)
            {
                delete opc;
                throw ex;
            }
            *driverHandle = (int)opc;
            OPCMap.SetAt(*driverHandle, opc);
        }
        else if (cmdStr == "Read")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            const VariablesParameter *varParam = static_cast<VariablesParameter *>(param);
            if (varParam->length < 1 || !varParam->variables)
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            vector<uint8_t> buf;
            for (size_t i = 0; i < varParam->length; i++)
            {
                auto retryN = 0;
                const auto var = varParam->variables[i];
                const auto id = stoi(var.id);
                auto source = OPCDATASOURCE::OPC_DS_CACHE;
                if (var.attributesLen > 0)
                {
                    for (size_t j = 0; j < var.attributesLen; j++)
                    {
                        const auto attr = var.attributes[j];
                        if (strcmp(attr.name, "source") == 0)
                        {
                            if (strcmp(attr.value, "device") == 0)
                            {
                                source = OPCDATASOURCE::OPC_DS_DEVICE;
                            }
                            else if (strcmp(attr.value, "cache") == 0)
                            {
                                source = OPCDATASOURCE::OPC_DS_CACHE;
                            }
                            break;
                        }
                    }
                }
            retryRead:
                OPCItemData data;
                const auto ret = opc->read(id, data, source);
                if (ret != 0)
                {
                    if (ret == -2)
                    {
                        if (retryN < 1 && opc->checkServerStatus(true))
                        {
                            retryN++;
                            goto retryRead;
                        }
                        return EnumDrvRet::ENUMDRVRET_DisConnected;
                    }
                    return EnumDrvRet::ENUMDRVRET_ERROR;
                }
                *var.timestamp = ConvertFiletimeToLong(data.ftTimeStamp);
                if (data.Error < 0)
                {
                    *var.status = 0;
                }
                else
                {
                    *var.status = ConvertOPCDataToByteArray(data, buf);
                }
                if (var.dataLength < buf.size())
                {
                    return EnumDrvRet::ENUMDRVRET_ERROR;
                }
                memcpy(var.data, buf.data(), buf.size());
            }
        }
        else if (cmdStr == "Write")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            auto retryN = 0;
            const VariableParameter *varParam = static_cast<VariableParameter *>(param);
            const auto id = stoi(varParam->id);
        retryWrite:
            const auto item = opc->getItem(id);
            if (!item)
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            VARIANT value;
            value.vt = item->getDataType();
            const auto cvRet = ConvertByteArrayToOPCData(varParam->data, &value);
            if (!cvRet)
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            const auto ret = opc->write(id, value);
            if (ret != 0)
            {
                if (ret == -2)
                {
                    if (retryN < 1 && opc->checkServerStatus(true))
                    {
                        retryN++;
                        goto retryWrite;
                    }
                    return EnumDrvRet::ENUMDRVRET_DisConnected;
                }
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
        }
        else if (cmdStr == "SubscribeCallBack")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            SubscribeCallbackFunction callbackFunc = static_cast<SubscribeCallbackFunction>(param);
            SubscribeCallback *callback = new SubscribeCallback(callbackFunc, opc);
            opc->setCallback(callback);
        }
        else if (cmdStr == "Subscribe" || cmdStr == "EnableSubscribe")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            opc->subscribe();
        }
        else if (cmdStr == "UnSubscribe")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            opc->unsubscribe();
        }
        else if (cmdStr == "GetStatus")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            if (opc->checkServerStatus(true))
            {
                return EnumDrvRet::ENUMDRVRET_OK;
            }
            else
            {
                return EnumDrvRet::ENUMDRVRET_DisConnected;
            }
        }
        else if (cmdStr == "CloseDriver")
        {
            OPCManager *opc;
            if (!OPCMap.Lookup(*driverHandle, opc))
            {
                return EnumDrvRet::ENUMDRVRET_ERROR;
            }
            opc->close();
            OPCMap.RemoveKey(*driverHandle);
            delete opc;
        }
        else
        {
            return EnumDrvRet::ENUMDRVRET_UNKNOWCMD;
        }
        return EnumDrvRet::ENUMDRVRET_OK;
    }
    catch (OPCException ex)
    {
        printf("DriverCmd failed: %s %ws\n", cmd, ex.reasonString().c_str());
        return EnumDrvRet::ENUMDRVRET_ERROR;
    }
    catch (...)
    {
        printf("DriverCmd failed: %s\n", cmd);
        return EnumDrvRet::ENUMDRVRET_ERROR;
    }
}
