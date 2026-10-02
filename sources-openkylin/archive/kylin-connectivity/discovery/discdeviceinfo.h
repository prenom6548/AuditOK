#ifndef KYLIN_CONNECTIVITY_DISCDEVICEINFO_H
#define KYLIN_CONNECTIVITY_DISCDEVICEINFO_H

#include <vector>
#include <string>
#include <map>

namespace tlv
{
enum InfoType { DEVICE_ALL = 1, DEVICE_UUID = 2, DEVICE_TYPE = 3, DEVICE_NAME = 4, DEVICE_IP = 5, DEVICE_PORT = 6 };

class Tlv;
class DiscoveryDeviceInfo
{

public:
    DiscoveryDeviceInfo();
    virtual ~DiscoveryDeviceInfo();

private:
    DiscoveryDeviceInfo(const DiscoveryDeviceInfo &c);
    DiscoveryDeviceInfo &operator=(const DiscoveryDeviceInfo &c);

public:
    // put one TLV object
    bool PutRequestValue(int type, int value);

    bool PutObjectValue(int type, const DiscoveryDeviceInfo &value);

    // do encode
    bool Serialize();

    // access encoded buffer and length
    unsigned char *GetSerializedBuffer() const;
    int GetSerializedBytes() const;

    // returns number of TLVs in TlvBox, along with a vector of the types
    int GetTLVList(std::vector<int> &list) const;

public:
    // do decode
    bool Parse(const unsigned char *buffer, int buffersize);

    // get one TLV object
    bool GetBytesValue(int type, unsigned char *value, int &length) const;

    bool GetResponseValue(int type, int &value) const;
    bool GetUuidValue(int type, char *value, int &length) const;
    bool GetDevTypeValue(int type, int &value) const;
    bool GetDevNameValue(int type, char *value, int &length) const;
    bool GetDevIpValue(int type, char *value, int &length) const;
    bool GetDevPortValue(int type, int &value) const;

    bool GetObjectValue(int type, DiscoveryDeviceInfo &value) const;

private:
    bool PutValue(Tlv *value);

private:
    std::map<int, Tlv *> mTlvMap;
    unsigned char *mSerializedBuffer = nullptr;
    int mSerializedBytes = 0;
};

} // namespace tlv
// #endif

#endif // KYLIN_CONNECTIVITY_DISCDEVICEINFO_H
