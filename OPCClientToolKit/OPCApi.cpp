#include "OPCApi.h"
#include "OPCItem.h"
#include <map>

bool init(OPCOLEInitMode mode)
{
    try
    {
        return COPCClient::init(mode);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return false;
    }
}

COPCHost *connect_host(const char *hostName)
{
    try
    {
        return COPCClient::makeHost(COPCHost::S2WS(hostName));
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

StringList get_servers(COPCHost *host)
{
    StringList list = {-1};
    if (!host)
    {
        return list;
    }
    std::vector<CLSID> _serverIds;
    std::vector<std::wstring> _serverNames;
    try
    {
        host->getListOfDAServers(IID_CATID_OPCDAServer20, _serverNames, _serverIds);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return list;
    }
    list.count = _serverNames.size();
    list.data = new char *[list.count];
    for (int i = 0; i < list.count; ++i)
    {
        // printf("%d: %ws\n", i, _serverNames[i].c_str());
        list.data[i] = strdup(COPCHost::WS2S(_serverNames[i]).c_str());
    }
    return list;
}

COPCServer *connect_server(COPCHost *host, const char *serverName)
{
    if (!host)
    {
        return nullptr;
    }
    try
    {
        return host->connectDAServer(COPCHost::S2WS(serverName));
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

ServerStatus get_server_status(COPCServer *server)
{
    if (!server)
    {
        return ServerStatus{0};
    }
    ServerStatus status = {0};
    try
    {
        if (!server->getStatus(status))
        {
            return ServerStatus{0};
        }
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return ServerStatus{0};
    }
    return status;
}

StringList get_server_items(COPCServer *server)
{
    StringList list = {-1};
    if (!server)
    {
        return list;
    }
    std::vector<std::wstring> _opcItemNames;
    try
    {
        if (!server->getItemNames(_opcItemNames))
        {
            return list;
        }
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return list;
    }
    list.count = _opcItemNames.size();
    list.data = new char *[list.count];
    for (int i = 0; i < list.count; ++i)
    {
        // printf("%3d: '%ws'\n", i + 1, _opcItemNames[i].c_str());
        list.data[i] = strdup(COPCHost::WS2S(_opcItemNames[i]).c_str());
    }
    return list;
}

COPCGroup *add_group(COPCServer *server, const char *groupName, bool active, unsigned long reqUpdateRate_ms,
                     unsigned long &revisedUpdateRate_ms, float deadBand)
{
    if (!server)
    {
        return nullptr;
    }
    try
    {
        return server->makeGroup(COPCHost::S2WS(groupName), active, reqUpdateRate_ms, revisedUpdateRate_ms, deadBand);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

bool remove_group(COPCServer *server, COPCGroup *group)
{
    if (!server)
    {
        return true;
    }
    if (!group)
    {
        return true;
    }
    try
    {
        delete (group);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return false;
    }
    return true;
}

COPCItem *add_item(COPCGroup *group, const char *name, bool active)
{
    if (!group)
    {
        return nullptr;
    }
    try
    {
        return group->addItem(COPCHost::S2WS(name), active);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

bool remove_item(COPCGroup *group, COPCItem *item)
{
    if (!group)
    {
        return true;
    }
    if (!item)
    {
        return true;
    }
    try
    {
        return group->removeItem(item);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return false;
    }
}

COPCItemNameList add_items(COPCGroup *group, StringList names, bool active)
{
    COPCItemNameList list = {-1};
    if (!group)
    {
        return list;
    }
    std::vector<std::wstring> itemNames;
    std::vector<COPCItem *> itemsCreated;
    std::vector<HRESULT> errors;
    for (int i = 0; i < names.count; ++i)
    {
        // printf("%d: %s\n", i, names[i]);
        itemNames.push_back(COPCHost::S2WS(strdup(names.data[i])));
    }
    try
    {
        list.errorCount = group->addItems(itemNames, itemsCreated, errors, active);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return list;
    }
    list.count = itemsCreated.size();
    list.names = new char *[list.count];
    list.items = new COPCItem *[list.count];
    for (int i = 0; i < list.count; ++i)
    {
        // printf("%d: %ws\n", i, itemsCreated[i]->getName().c_str());
        list.names[i] = strdup(COPCHost::WS2S(itemsCreated[i]->getName()).c_str());
        list.items[i] = itemsCreated[i];
    }
    return list;
}

int remove_items(COPCGroup *group, COPCItemList items)
{
    if (!group)
    {
        return 0;
    }
    std::vector<COPCItem *> itemsCreated;
    std::vector<HRESULT> errors;
    for (int i = 0; i < items.count; ++i)
    {
        itemsCreated.push_back(items.items[i]);
    }
    try
    {
        return group->removeItems(itemsCreated, errors);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return -1;
    }
}

COPCItemPropertyValueList get_item_properties(COPCItem *item)
{
    COPCItemPropertyValueList list = {-1};
    if (!item)
    {
        return list;
    }
    std::vector<CPropertyDescription> propDesc;
    CAutoPtrArray<SPropertyValue> propVals;
    try
    {
        if (!item->getSupportedProperties(propDesc))
        {
            return list;
        }
        list.count = propDesc.size();
        if (!item->getProperties(propDesc, propVals))
        {
            return list;
        }
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return list;
    }
    list.data = new COPCItemPropertyValue[list.count];
    for (int i = 0; i < list.count; ++i)
    {
        // printf("%d: %ws %d\n", propVals[i]->propDesc.id, propVals[i]->propDesc.desc.c_str(),
        // propVals[i]->propDesc.type);
        list.data[i].id = propVals[i]->propDesc.id;
        list.data[i].desc = strdup(COPCHost::WS2S(propVals[i]->propDesc.desc).c_str());
        list.data[i].type = propVals[i]->propDesc.type;
        list.data[i].value = propVals[i]->value;
    }
    return list;
}

OPCItemData read_sync(COPCItem *item, OPCDATASOURCE source)
{
    if (!item)
    {
        return nullptr;
    }
    OPCItemData data;
    try
    {
        if (!item->readSync(data, source))
        {
            return nullptr;
        }
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
    return data;
}

OPCItemDataList multi_read_sync(COPCGroup *group, COPCItemList items, OPCDATASOURCE source)
{
    OPCItemDataList list = {-1};
    if (!group)
    {
        return list;
    }
    std::vector<COPCItem *> itemsCreated;
    for (int i = 0; i < items.count; i++)
    {
        itemsCreated.push_back(items.items[i]);
    }
    COPCItemDataMap itemDataMap;
    std::vector<OPCItemData> itemsData;
    try
    {
        group->readSync(itemsCreated, itemDataMap, OPC_DS_DEVICE);
        for (size_t i = 0; i < itemsCreated.size(); i++)
        {
            OPCHANDLE handle = itemsCreated.at(i)->getHandle();
            OPCItemData *data;
            itemDataMap.Lookup(handle, data);
            itemsData.push_back(*data);
        }
        /*POSITION pos = itemDataMap.GetStartPosition();
        while (pos)
        {
            OPCHANDLE handle = itemDataMap.GetKeyAt(pos);
            OPCItemData *data = itemDataMap.GetNextValue(pos);
            if (data)
            {
                itemsData.push_back(*data);
                printf("-----> handle: %u, group sync read quality %d value %d\n", handle, data->wQuality,
                       data->vDataValue.iVal);
            }
        }*/
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return list;
    }
    list.count = itemsData.size();
    list.data = new OPCItemData[list.count];
    for (int i = 0; i < list.count; ++i)
    {
        list.data[i] = itemsData[i];
    }
    return list;
}

bool write_sync(COPCItem *item, VARIANT data)
{
    if (!item)
    {
        return false;
    }
    try
    {
        return item->writeSync(data);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return false;
    }
}

class AsyncDataCallback : public IAsyncDataCallback
{
  private:
    AsyncDataCallbackFunction callback;
    const void *closure;

  public:
    AsyncDataCallback(COPCGroup *group, AsyncDataCallbackFunction cb, const void *cb_closure) noexcept
    {
        try
        {
            group->disableAsync();
        }
        catch (OPCException variable)
        {
            // printf("OPCException: %ws\n", variable.reasonString().c_str());
        }
        callback = cb;
        closure = cb_closure;
    }
    ~AsyncDataCallback()
    {
        /*if (callback)
        {
            delete &callback;
        }*/
    }

    void OnDataChange(COPCGroup &group, COPCItemDataMap &changes)
    {
        // printf("-----> group '%ws', item(s) changed:\n", group.getName().c_str());
        try
        {
            POSITION pos = changes.GetStartPosition();
            while (pos)
            {
                OPCItemData *data = changes.GetNextValue(pos);
                if (data)
                {
                    if (callback)
                    {
                        printf("-----> group '%ws','%ws', changed async read quality %d value %d\n",
                               group.getName().c_str(), data->Item->getName().c_str(), data->wQuality,
                               data->vDataValue.iVal);
                        callback(
                            AsyncCallbackData{
                                strdup(COPCHost::WS2S((group.getName())).c_str()),
                                strdup(COPCHost::WS2S((data->Item->getName())).c_str()),
                                data->ftTimeStamp,
                                data->wQuality,
                                data->vDataValue,
                                data->Error,
                            },
                            closure);
                    }
                }
            }
        }
        catch (OPCException variable)
        {
            printf("OPCException: %ws\n", variable.reasonString().c_str());
        }
    }
};

AsyncDataCallback *enable_async(COPCGroup *group, AsyncDataCallbackFunction callback, const void *cb_closure)
{
    if (!group)
    {
        return nullptr;
    }
    try
    {
        AsyncDataCallback *usrCallBack = new AsyncDataCallback(group, callback, cb_closure);
        if (group->enableAsync(usrCallBack))
        {
            return usrCallBack;
        }
        delete usrCallBack;
        return nullptr;
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

bool disable_async(COPCGroup *group, AsyncDataCallback *usrCallBack)
{
    if (!group)
    {
        return true;
    }
    try
    {
        bool ret = group->disableAsync();
        delete usrCallBack;
        return ret;
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return true;
    }
}

class TransactionCompleteCallback : public ITransactionComplete
{
  private:
    TransactionCompleteCallbackFunction callback;
    const void *closure;

  public:
    TransactionCompleteCallback(TransactionCompleteCallbackFunction cb, const void *cb_closure) noexcept
    {
        callback = cb;
        closure = cb_closure;
    }
    ~TransactionCompleteCallback()
    {
    }

    void complete(CTransaction &transaction) noexcept
    {
        try
        {
            // printf("transaction isCompleted: %d\n", transaction.isCompleted());
            if (callback)
            {
                callback(&transaction, closure);
            }
        }
        catch (OPCException variable)
        {
            printf("OPCException: %ws\n", variable.reasonString().c_str());
        }
    }
};
Transaction read_async(COPCItem *item, TransactionCompleteCallbackFunction transactionCB, const void *cb_closure)
{
    if (!item)
    {
        return Transaction{};
    }
    TransactionCompleteCallback *callback = new TransactionCompleteCallback{transactionCB, cb_closure};
    try
    {
        CTransaction *trans = item->readAsync(callback);
        return Transaction{
            trans,
            callback,
        };
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return Transaction{};
    }
}

Transaction multi_read_async(COPCGroup *group, COPCItemList items, TransactionCompleteCallbackFunction transactionCB,
                             const void *cb_closure)
{
    if (!group)
    {
        return Transaction{};
    }
    std::vector<COPCItem *> itemsCreated;
    for (int i = 0; i < items.count; i++)
    {
        itemsCreated.push_back(items.items[i]);
    }
    TransactionCompleteCallback *callback = new TransactionCompleteCallback{transactionCB, cb_closure};
    try
    {
        CTransaction *trans = group->readAsync(itemsCreated, callback);
        return Transaction{
            trans,
            callback,
        };
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return Transaction{};
    }
}

Transaction refresh_async(COPCGroup *group, OPCDATASOURCE source, TransactionCompleteCallbackFunction transactionCB,
                          const void *cb_closure)
{
    if (!group)
    {
        return Transaction{};
    }
    TransactionCompleteCallback *callback = new TransactionCompleteCallback{transactionCB, cb_closure};
    try
    {
        CTransaction *trans = group->refresh(source, callback);
        return Transaction{
            trans,
            callback,
        };
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return Transaction{};
    }
}

Transaction write_async(COPCItem *item, VARIANT data, TransactionCompleteCallbackFunction transactionCB,
                        const void *cb_closure)
{
    if (!item)
    {
        return Transaction{};
    }
    TransactionCompleteCallback *callback = new TransactionCompleteCallback{transactionCB, cb_closure};
    try
    {
        CTransaction *trans = item->writeAsync(data, callback);
        return Transaction{
            trans,
            callback,
        };
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return Transaction{};
    }
}

int transaction_completed(CTransaction *transaction)
{
    if (!transaction)
    {
        return -1;
    }
    try
    {
        return static_cast<int>(transaction->isCompleted());
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return -1;
    }
}

OPCItemData get_item_value(CTransaction *transaction, COPCItem *item)
{
    if (!transaction)
    {
        return nullptr;
    }
    try
    {
        return *transaction->getItemValue(item);
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return nullptr;
    }
}

bool delete_transaction(COPCGroup *group, Transaction transaction)
{
    if (!group)
    {
        return true;
    }
    try
    {
        bool ret = group->deleteTransaction(transaction.transaction);
        delete (transaction.callback);
        delete (transaction.transaction);
        return ret;
    }
    catch (OPCException variable)
    {
        printf("OPCException: %ws\n", variable.reasonString().c_str());
        return false;
    }
}

void close()
{
    COPCClient::stop();
}
