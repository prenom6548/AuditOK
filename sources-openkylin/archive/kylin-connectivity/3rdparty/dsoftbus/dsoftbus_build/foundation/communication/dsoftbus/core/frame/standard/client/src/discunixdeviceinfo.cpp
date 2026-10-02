/*
 *  COPYRIGHT NOTICE
 *  Copyright (C) 2015, Jhuster, All Rights Reserved
 *  Author: Jhuster(lujun.hust@gmail.com)
 *
 *  https://github.com/Jhuster/TLV
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License,
 *  or (at your option) any later version.
 */
#include <string.h>
#include <arpa/inet.h>
#include "tlv.h"
#include "discunixdeviceinfo.h"

// static float swapFloat(float f) // assumes sender & receiver use same float format, such as IEEE-754
// {
// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//     union
//     {
//         float f;
//         int i;
//     } u32;
//     u32.f = f;
//     u32.i = htonl(u32.i);
//     return u32.f;
// #else
//     return f;
// #endif
// }

// static double swapDouble(double d) // assumes sender & receiver use same double format, such as IEEE-754
// {
// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//     union
//     {
//         double d;
//         int i[2];
//     } u64; // used i[2] instead of long long since some compilers don't support long long
//     u64.d = d;
//     int temp = htonl(u64.i[1]);
//     u64.i[1] = htonl(u64.i[0]);
//     u64.i[0] = temp;
//     return u64.d;
// #else
//     return d;
// #endif
// }

namespace tlv
{

    DiscoveryDeviceInfo::DiscoveryDeviceInfo() : mSerializedBuffer(NULL), mSerializedBytes(0) {}

    DiscoveryDeviceInfo::~DiscoveryDeviceInfo()
    {
        if (mSerializedBuffer != NULL)
        {
            delete[] mSerializedBuffer;
            mSerializedBuffer = NULL;
        }

        std::map<int, Tlv *>::iterator itor;
        for (itor = mTlvMap.begin(); itor != mTlvMap.end(); itor++)
        {
            delete itor->second;
        }

        mTlvMap.clear();
    }

    bool DiscoveryDeviceInfo::Serialize()
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }

        int offset = 0;
        mSerializedBuffer = new unsigned char[mSerializedBytes];

        std::map<int, Tlv *>::iterator itor;
        for (itor = mTlvMap.begin(); itor != mTlvMap.end(); itor++)
        {
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
        if (mSerializedBuffer != NULL || buffer == NULL)
        {
            return false;
        }

        unsigned char *cached = new unsigned char[buffersize];
        memcpy(cached, buffer, buffersize);

        int offset = 0;
        while (offset < buffersize)
        {
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
        if (itor != mTlvMap.end())
        {
            Tlv *prevTlv = itor->second;
            mSerializedBytes = mSerializedBytes - (sizeof(int) * 2 + prevTlv->GetLength());
            delete itor->second;
            itor->second = value;
        }
        else
        {
            mTlvMap.insert(std::pair<int, Tlv *>(value->GetType(), value));
        }

        mSerializedBytes += (sizeof(int) * 2 + value->GetLength());
        return true;
    }
    bool DiscoveryDeviceInfo::PutResponseValue(int type, int value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        int nwvalue = htonl(value);
        return PutValue(new Tlv(type, nwvalue));
    }

    bool DiscoveryDeviceInfo::PutUuidValue(int type, char *value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        return PutValue(new Tlv(type, value));
    }

    bool DiscoveryDeviceInfo::PutDevTypeValue(int type, int value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        int nwvalue = htonl(value);
        return PutValue(new Tlv(type, nwvalue));
    }

    bool DiscoveryDeviceInfo::PutDevNameValue(int type, char *value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        return PutValue(new Tlv(type, value));
    }

    bool DiscoveryDeviceInfo::PutDevIpValue(int type, char *value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        return PutValue(new Tlv(type, value));
    }

    bool DiscoveryDeviceInfo::PutDevPortValue(int type, int value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        int nwvalue = htonl(value);
        return PutValue(new Tlv(type, nwvalue));
    }

    bool DiscoveryDeviceInfo::PutObjectValue(int type, const DiscoveryDeviceInfo &value)
    {
        if (mSerializedBuffer != NULL)
        {
            return false;
        }
        unsigned char *buffer = value.GetSerializedBuffer();
        if (buffer == NULL)
        {
            return false;
        }
        return PutValue(new Tlv(type, buffer, value.GetSerializedBytes()));
    }

    bool DiscoveryDeviceInfo::GetRequestValue(int type, int &value) const
    {
        std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
        if (itor != mTlvMap.end())
        {
            value = ntohl((*(int *)(itor->second->GetValue())));
            return true;
        }
        return false;
    }

    bool DiscoveryDeviceInfo::GetDevTypeValue(int type, int &value) const
    {
        std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
        if (itor != mTlvMap.end())
        {
            value = ntohl((*(int *)(itor->second->GetValue())));
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
        if (itor != mTlvMap.end())
        {
            value = ntohl((*(int *)(itor->second->GetValue())));
            return true;
        }
        return false;
    }

    bool DiscoveryDeviceInfo::GetBytesValue(int type, unsigned char *value, int &length) const
    {
        std::map<int, Tlv *>::const_iterator itor = mTlvMap.find(type);
        if (itor == mTlvMap.end())
        {
            return false;
        }

        if (length < itor->second->GetLength())
        {
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
        if (itor == mTlvMap.end())
        {
            return false;
        }
        return value.Parse(itor->second->GetValue(), itor->second->GetLength());
    }

    int DiscoveryDeviceInfo::GetTLVList(std::vector<int> &list) const
    {
        std::map<int, Tlv *>::const_iterator iter;
        for (iter = mTlvMap.begin(); iter != mTlvMap.end(); iter++)
        {
            list.push_back(iter->first);
        }
        return list.size();
    }

} // namespace tlv
