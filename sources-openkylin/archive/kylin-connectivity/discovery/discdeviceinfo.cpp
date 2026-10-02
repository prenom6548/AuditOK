#include <string.h>
#include <algorithm>
#include <arpa/inet.h>
#include "tlv.h"
#include "discdeviceinfo.h"

#define TYPE_UNKNOW_ID 0x00
#define TYPE_PHONE_ID 0x0E
#define TYPE_TV_ID 0x9C
#define TYPE_PC_ID 0x0C

namespace tlv {

static int g_typeMapLen = 4;

static uint8_t g_typeMap[] = {
    TYPE_UNKNOW_ID,
    TYPE_PC_ID,
    TYPE_PHONE_ID,
    TYPE_TV_ID,
};

DiscoveryDeviceInfo::DiscoveryDeviceInfo()
{
}

DiscoveryDeviceInfo::~DiscoveryDeviceInfo()
{
    if (mSerializedBuffer != nullptr) {
        delete[] mSerializedBuffer;
        mSerializedBuffer = nullptr;
    }

    std::map<int, Tlv *>::iterator itor;
    for (itor = mTlvMap.begin(); itor != mTlvMap.end(); itor++) {
        delete itor->second;
    }

    mTlvMap.clear();
}

bool DiscoveryDeviceInfo::Serialize()
{
    if (mSerializedBuffer != nullptr) {
        return false;
    }

    int offset = 0;
    mSerializedBuffer = new unsigned char[mSerializedBytes];

    std::map<int, Tlv *>::iterator itor;
    for (itor = mTlvMap.begin(); itor != mTlvMap.end(); itor++) {
        int type = htonl(itor->second->GetType());
        memcpy(mSerializedBuffer + offset, &type, sizeof(int));
        offset += sizeof(int);
        int length = itor->second->GetLength();
        int nwlength = htonl(length);
        memcpy(mSerializedBuffer + offset, &nwlength, sizeof(int));
        offset += sizeof(int);
        memcpy(mSerializedBuffer + offset, itor->second->GetValue(), length);
        offset += length;
    }

    return true;
}

bool DiscoveryDeviceInfo::Parse(const unsigned char *buffer, int buffersize)
{
    if (mSerializedBuffer != nullptr || buffer == nullptr) {
        return false;
    }

    unsigned char *cached = new unsigned char[buffersize];
    memcpy(cached, buffer, buffersize);

    int offset = 0;
    while (offset < buffersize) {
        int type = ntohl((*(int *)(cached + offset)));
        offset += sizeof(int);
        int length = ntohl((*(int *)(cached + offset)));
        offset += sizeof(int);
        PutValue(new Tlv(type, cached + offset, length));
        offset += length;
    }

    mSerializedBuffer = cached;
    mSerializedBytes = buffersize;

    return true;
}

unsigned char *DiscoveryDeviceInfo::GetSerializedBuffer() const
{
    return mSerializedBuffer;
}

int DiscoveryDeviceInfo::GetSerializedBytes() const
{
    return mSerializedBytes;
}

bool DiscoveryDeviceInfo::PutValue(Tlv *value)
{
    std::map<int, Tlv *>::iterator itor = mTlvMap.find(value->GetType());
    if (itor != mTlvMap.end()) {
        Tlv *prevTlv = itor->second;
        mSerializedBytes = mSerializedBytes - (sizeof(int) * 2 + prevTlv->GetLength());
        delete itor->second;
        itor->second = value;
    } else {
        mTlvMap.insert(std::pair<int, Tlv *>(value->GetType(), value));
    }

    mSerializedBytes += (sizeof(int) * 2 + value->GetLength());
    return true;
}

bool DiscoveryDeviceInfo::PutRequestValue(int type, int value)
{
    if (mSerializedBuffer != nullptr) {
        return false;
    }
    int nwvalue = htonl(value);
    return PutValue(new Tlv(type, nwvalue));
}

bool DiscoveryDeviceInfo::PutObjectValue(int type, const DiscoveryDeviceInfo &value)
{
    if (mSerializedBuffer != nullptr) {
        return false;
    }
    unsigned char *buffer = value.GetSerializedBuffer();
    if (buffer == nullptr) {
        return false;
    }
    return PutValue(new Tlv(type, buffer, value.GetSerializedBytes()));
}

bool DiscoveryDeviceInfo::GetResponseValue(int type, int &value) const
{
    std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
    if (itor != mTlvMap.end()) {
        value = ntohl((*(int *)(itor->second->GetValue())));
        return true;
    }
    return false;
}

bool DiscoveryDeviceInfo::GetDevTypeValue(int type, int &value) const
{
    std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
    if (itor != mTlvMap.end()) {
        uint8_t typeId = ntohl((*(int *)(itor->second->GetValue())));
        uint8_t *idIndex = std::find(g_typeMap, g_typeMap + g_typeMapLen, typeId);
        value = idIndex - g_typeMap;
        return true;
    }
    return false;
}

bool DiscoveryDeviceInfo::GetUuidValue(int type, char *value, int &length) const
{
    return GetBytesValue(type, (unsigned char *)value, length);
}

bool DiscoveryDeviceInfo::GetDevNameValue(int type, char *value, int &length) const
{
    return GetBytesValue(type, (unsigned char *)value, length);
}

bool DiscoveryDeviceInfo::GetDevIpValue(int type, char *value, int &length) const
{
    return GetBytesValue(type, (unsigned char *)value, length);
}

bool DiscoveryDeviceInfo::GetDevPortValue(int type, int &value) const
{
    std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
    if (itor != mTlvMap.end()) {
        value = ntohl((*(int *)(itor->second->GetValue())));
        return true;
    }
    return false;
}

bool DiscoveryDeviceInfo::GetBytesValue(int type, unsigned char *value, int &length) const
{
    std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
    if (itor == mTlvMap.end()) {
        return false;
    }

    if (length < itor->second->GetLength()) {
        return false;
    }

    memset(value, 0, length);
    length = itor->second->GetLength();
    memcpy(value, itor->second->GetValue(), length);

    return true;
}

bool DiscoveryDeviceInfo::GetObjectValue(int type, DiscoveryDeviceInfo &value) const
{
    std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
    if (itor == mTlvMap.end()) {
        return false;
    }
    return value.Parse(itor->second->GetValue(), itor->second->GetLength());
}

int DiscoveryDeviceInfo::GetTLVList(std::vector<int> &list) const
{
    std::map<int, Tlv *>::const_iterator iter;
    for (iter = mTlvMap.begin(); iter != mTlvMap.end(); iter++) {
        list.push_back(iter->first);
    }
    return list.size();
}

}  // namespace tlv
