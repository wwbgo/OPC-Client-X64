#pragma once

#include "OPCClient.h"
#include "OPCClientToolKitDLL.h"
#include "OPCHost.h"
#include "OPCItem.h"
using namespace std;

enum OPCDACLIENT_API EnumDrvRet
{
    ENUMDRVRET_OK = 0,
    ENUMDRVRET_PartialOK = -1,
    ENUMDRVRET_ERROR = -2,
    ENUMDRVRET_UNKNOWCMD = -3,
    ENUMDRVRET_DisConnected = -4,
};
extern "C"
{
    /*********************************************************************/
    /*[��������]�����ṩ��ʹ���ߵ�ͳһ�ӿ�
    /*[����˵��]cmd(�ӿ�����)-��"InitDriver"-��ʾ�����������󲢳�ʼ��,
    /*							"Read"-����ģʽ��,
    /*							"Write"-����ģʽд,
    /*							"CloseDriver"-������������,
    /*							"Subscribe"-����,
    /*							"UnSubscribe"-ȡ������,
    /*							"SubscribeCallBack"-���Ļص�,
    /*							"GetStatus"-��ȡ����״̬,
    /*			driverHandle(��������)-"InitDriver"��int*(������½�����)
    /*								     ����������int*(Ҫ���ʵ���������)
    /*			request(�������)-��"InitDriver",((void*)request)��InitDriverParameter*(ͨѶ����ָ��)
    /*								"Read",((void*)request)��VariablesParameter*(�Ĵ�����Ŀ�������ָ��)
    /*								"Write",((void*)request)��VariableParameter*(�Ĵ�����Ŀָ��)
    /*								"Subscribe",((void*)request)��VariablesParameter*(�Ĵ�����Ŀ�������ָ��)
    /*								"UnSubscribe",((void*)request)��VariablesParameter*(�Ĵ�����Ŀ�������ָ��)
    /* "SubscribeCallBack",((void*)request)��void*(�ص�����ָ��)���������ݸ�ʽ-VariableParameter
    /*								"GetStatus",param��Ч,��ΪNULL
    /*								"CloseDriver",param��Ч,��ΪNULL
    /*[����ֵ]�ɹ��������
    /**********************************************************************/
    OPCDACLIENT_API EnumDrvRet DriverCmd(const char *cmd, int *driverHandle, void *param);
}

struct OPCDACLIENT_API CommonProperty
{
    const char *name;
    const char *value;
};

struct OPCDACLIENT_API InitDriverParameter
{
    int propertyLen;
    CommonProperty *property;
};

struct OPCDACLIENT_API VariableParameter
{
    int dataLength;
    int attributesLen;
    const char *id;
    const char *name;
    const char *dataType;
    int8_t *status; // 0-fai1,1-successĬ��ֵ0
    uint8_t *data;
    uint64_t *timestamp;
    CommonProperty *attributes;
};
class OPCManager;

struct OPCDACLIENT_API VariablesParameter
{
    int length;
    VariableParameter *variables;
};

typedef void (*SubscribeCallbackFunction)(const VariableParameter *variableParameter);
class SubscribeCallback : public IAsyncDataCallback
{
  private:
    SubscribeCallbackFunction Callback;
    OPCManager *Manager;

  protected:
    friend class OPCManager;

  public:
    SubscribeCallback(SubscribeCallbackFunction callback, OPCManager *manager) noexcept
    {
        Callback = callback;
        Manager = manager;
    }
    ~SubscribeCallback()
    {
        Callback = nullptr;
        Manager = nullptr;
    }

    void OnDataChange(COPCGroup &group, COPCItemDataMap &changes);
};
class OPCManager
{
  private:
    string JsonFile;
    COPCHost *Host;
    COPCServer *Server;
    vector<COPCGroup *> SubscribeGroups;
    CAtlMap<int, COPCItem *> ItemMap;
    CAtlMap<OPCHANDLE, int> ItemRevoteMap;
    SubscribeCallback *Callback;

  public:
    OPCManager(const string &jsonFile) noexcept
    {
        JsonFile = jsonFile;
        Host = nullptr;
        Server = nullptr;
        Callback = nullptr;
    }
    ~OPCManager()
    {
        close();

        SubscribeGroups.clear();
        try
        {
            ItemMap.RemoveAll();
            ItemRevoteMap.RemoveAll();
        }
        catch (...)
        {
        }
        Server = nullptr;
        Host = nullptr;
    }

    COPCItem *getItem(int id)
    {
        COPCItem *data;
        if (ItemMap.Lookup(id, data))
        {
            return data;
        }
        return nullptr;
    }
    bool getItemId(OPCHANDLE item, int &id)
    {
        return ItemRevoteMap.Lookup(item, id);
    }

    void connect();
    void reconnect();
    bool checkServerStatus(bool retryConnect);

    void read(const vector<int> &itemIds, vector<OPCItemData> &data);

    int read(int itemId, OPCItemData &value);

    int OPCManager::write(int itemId, VARIANT data);

    void setCallback(SubscribeCallback *callback) noexcept
    {
        if (Callback)
        {
            delete Callback;
        }
        Callback = callback;
    }

    void subscribe();

    void unsubscribe();

    void close();
};
static CAtlMap<int, OPCManager *> OPCMap;

/* OPC DA Quality Codes
0

0x00000000

Bad [Non-Specific]

4

0x00000004

Bad [Configuration Error]

8

0x00000008

Bad [Not Connected]

12

0x0000000c

Bad [Device Failure]

16

0x00000010

Bad [Sensor Failure]

20

0x00000014

Bad [Last Known Value]

24

0x00000018

Bad [Communication Failure]

28

0x0000001C

Bad [Out of Service]

64

0x00000040

Uncertain [Non-Specific]

65

0x00000041

Uncertain [Non-Specific] (Low Limited)

66

0x00000042

Uncertain [Non-Specific] (High Limited)

67

0x00000043

Uncertain [Non-Specific] (Constant)

68

0x00000044

Uncertain [Last Usable]

69

0x00000045

Uncertain [Last Usable] (Low Limited)

70

0x00000046

Uncertain [Last Usable] (High Limited)

71

0x00000047

Uncertain [Last Usable] (Constant)

80

0x00000050

Uncertain [Sensor Not Accurate]

81

0x00000051

Uncertain [Sensor Not Accurate] (Low Limited)

82

0x00000052

Uncertain [Sensor Not Accurate] (High Limited)

83

0x00000053

Uncertain [Sensor Not Accurate] (Constant)

84

0x00000054

Uncertain [EU Exceeded]

85

0x00000055

Uncertain [EU Exceeded] (Low Limited)

86

0x00000056

Uncertain [EU Exceeded] (High Limited)

87

0x00000057

Uncertain [EU Exceeded] (Constant)

88

0x00000058

Uncertain [Sub-Normal]

89

0x00000059

Uncertain [Sub-Normal] (Low Limited)

90

0x0000005a

Uncertain [Sub-Normal] (High Limited)

91

0x0000005b

Uncertain [Sub-Normal] (Constant)

192

0x000000c0

Good [Non-Specific]

193

0x000000c1

Good [Non-Specific] (Low Limited)

194

0x000000c2

Good [Non-Specific] (High Limited)

195

0x000000c3

Good [Non-Specific] (Constant)

216

0x000000d8

Good [Local Override]

217

0x000000d9

Good [Local Override] (Low Limited)

218

0x000000da

Good [Local Override] (High Limited)

219

0x000000db

Good [Local Override] (Constant)
*/