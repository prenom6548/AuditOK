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
#ifndef _TLV_BOX_H_
#define _TLV_BOX_H_
// #ifdef __cplusplus

#include <vector>
#include <string>
#include <map>

namespace tlv
{
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

        bool PutResponseValue(int type, int value);
        bool PutUuidValue(int type, char *value);
        bool PutDevTypeValue(int type, int value);
        bool PutDevNameValue(int type, char *value);
        bool PutDevIpValue(int type, char *value);
        bool PutDevPortValue(int type, int value);

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

        bool GetRequestValue(int type, int &value) const;
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
        unsigned char *mSerializedBuffer;
        int mSerializedBytes;
    };

} // namespace
// #endif

#endif //_TLVOBJECT_H_
