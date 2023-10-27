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
typedef void (*AsyncDataCallbackFunction)(COPCGroup &group, OPCItemData changed);
typedef void (*TransactionCompleteCallbackFunction)(CTransaction &transaction);
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

    OPCDACLIENT_API COPCItem *add_item(COPCGroup *group, const char *name, bool active);

    OPCDACLIENT_API COPCItemNameList add_items(COPCGroup *group, StringList names, bool active);

    OPCDACLIENT_API COPCItemPropertyValueList get_item_properties(COPCItem *item);

    OPCDACLIENT_API OPCItemData read_sync(COPCItem *item, OPCDATASOURCE source);

    OPCDACLIENT_API OPCItemDataList multi_read_sync(COPCGroup *group, COPCItemList items, OPCDATASOURCE source);

    OPCDACLIENT_API bool write_sync(COPCItem *item, VARIANT data);

    OPCDACLIENT_API bool enable_async(COPCGroup *group, AsyncDataCallbackFunction callback);

    OPCDACLIENT_API CTransaction *read_async(COPCItem *item, TransactionCompleteCallbackFunction transactionCB);

    OPCDACLIENT_API CTransaction *multi_read_async(COPCGroup *group, COPCItemList items,
                                                   TransactionCompleteCallbackFunction transactionCB);

    OPCDACLIENT_API CTransaction *refresh_async(COPCGroup *group, OPCDATASOURCE source,
                                                TransactionCompleteCallbackFunction transactionCB);

    OPCDACLIENT_API CTransaction *write_async(COPCItem *item, VARIANT data,
                                              TransactionCompleteCallbackFunction transactionCB);

    OPCDACLIENT_API int transaction_completed(const CTransaction *transaction);

    OPCDACLIENT_API OPCItemData get_item_value(const CTransaction *transaction, COPCItem *item);

    OPCDACLIENT_API bool delete_transaction(COPCGroup *group, CTransaction *transaction);

    OPCDACLIENT_API void close();
}
