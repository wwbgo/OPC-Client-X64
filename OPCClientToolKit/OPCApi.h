#pragma once

#include "OPCClient.h"
#include "OPCHost.h"
#include "OPCProperties.h"
#include "OPCServer.h"

struct OPCDACLIENT_API StringList
{
    int count;
    char **data;
};
struct OPCDACLIENT_API COPCItemNameList
{
    int count;
    char **names;
    COPCItem **items;
    int errorCount;
};
struct OPCDACLIENT_API COPCItemList
{
    int count;
    COPCItem **items;
};
struct OPCDACLIENT_API OPCItemDataList
{
    int count;
    OPCItemData *data;
};
struct OPCDACLIENT_API COPCItemPropertyValue
{
    /// properties identifier
    DWORD id;
    /// server supplied textual description
    char *desc;
    /// data type of the property
    VARTYPE type;
    /// properties value.
    VARIANT value;
};
struct OPCDACLIENT_API COPCItemPropertyValueList
{
    int count;
    COPCItemPropertyValue *data;
};
struct OPCDACLIENT_API AsyncCallbackData
{
    char *groupName;
    char *itemName;
    FILETIME ftTimeStamp;
    WORD wQuality;
    VARIANT vDataValue;
    HRESULT Error;
};
typedef class OPCDACLIENT_API AsyncDataCallback;
typedef class TransactionCompleteCallback;
struct OPCDACLIENT_API Transaction
{
    CTransaction *transaction;
    TransactionCompleteCallback *callback;
};
typedef void (*AsyncDataCallbackFunction)(AsyncCallbackData changed, const void *cb_closure);
typedef void (*TransactionCompleteCallbackFunction)(CTransaction *transaction, const void *cb_closure);
extern "C"
{
    OPCDACLIENT_API bool init(OPCOLEInitMode mode = APARTMENTTHREADED);

    OPCDACLIENT_API COPCHost *connect_host(const char *hostName);

    OPCDACLIENT_API StringList get_servers(COPCHost *host);

    OPCDACLIENT_API COPCServer *connect_server(COPCHost *host, const char *serverName);

    OPCDACLIENT_API ServerStatus get_server_status(COPCServer *server);

    OPCDACLIENT_API StringList get_server_items(COPCServer *server);

    OPCDACLIENT_API COPCGroup *add_group(COPCServer *server, const char *groupName, bool active,
                                         unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms,
                                         float deadBand);

    OPCDACLIENT_API bool remove_group(COPCServer *server, COPCGroup *groupName);

    OPCDACLIENT_API COPCItem *add_item(COPCGroup *group, const char *name, bool active);

    OPCDACLIENT_API bool remove_item(COPCGroup *group, COPCItem *item);

    OPCDACLIENT_API COPCItemNameList add_items(COPCGroup *group, StringList names, bool active);

    OPCDACLIENT_API int remove_items(COPCGroup *group, COPCItemList items);

    OPCDACLIENT_API COPCItemPropertyValueList get_item_properties(COPCItem *item);

    OPCDACLIENT_API OPCItemData read_sync(COPCItem *item, OPCDATASOURCE source);

    OPCDACLIENT_API OPCItemDataList multi_read_sync(COPCGroup *group, COPCItemList items, OPCDATASOURCE source);

    OPCDACLIENT_API bool write_sync(COPCItem *item, VARIANT data);

    OPCDACLIENT_API AsyncDataCallback *enable_async(COPCGroup *group, AsyncDataCallbackFunction callback,
                                                    const void *cb_closure);

    OPCDACLIENT_API bool disable_async(COPCGroup *group, AsyncDataCallback *usrCallBack);

    OPCDACLIENT_API Transaction read_async(COPCItem *item, TransactionCompleteCallbackFunction transactionCB,
                                           const void *cb_closure);

    OPCDACLIENT_API Transaction multi_read_async(COPCGroup *group, COPCItemList items,
                                                 TransactionCompleteCallbackFunction transactionCB,
                                                 const void *cb_closure);

    OPCDACLIENT_API Transaction refresh_async(COPCGroup *group, OPCDATASOURCE source,
                                              TransactionCompleteCallbackFunction transactionCB,
                                              const void *cb_closure);

    OPCDACLIENT_API Transaction write_async(COPCItem *item, VARIANT data,
                                            TransactionCompleteCallbackFunction transactionCB, const void *cb_closure);

    OPCDACLIENT_API int transaction_completed(CTransaction *transaction);

    OPCDACLIENT_API OPCItemData get_item_value(CTransaction *transaction, COPCItem *item);

    OPCDACLIENT_API bool delete_transaction(COPCGroup *group, Transaction transaction);

    OPCDACLIENT_API void close();
}
