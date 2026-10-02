
from ast import Return
import os
import re
import binascii
from systemboardthread import *
import platform

KILOBYTE_FACTOR = 1000.0
MEGABYTE_FACTOR = (1000.0 * 1000.0)
GIGABYTE_FACTOR = (1000.0 * 1000.0 * 1000.0)
TERABYTE_FACTOR = (1000.0 * 1000.0 * 1000.0 * 1000.0)


def Judgment_LENOVO():
    sysBoardInfo = SystemBoard()
    sysInfo = sysBoardInfo.getSystemInfo()        
    baseBoardInfo = sysBoardInfo.getBaseBoardInfo()
    customedType = 0 ## 0 通用，1 联想 主板名称:
    if Board_Manufacturer in baseBoardInfo and "lenovo" in baseBoardInfo[Board_Manufacturer].lower() \
        and Board_Product in baseBoardInfo and ("FD2000ZX200MB1" in baseBoardInfo[Board_Product]) :
        customedType = 1
    return customedType

def Judgment_m70zg1ts():
    sysBoardInfo = SystemBoard()
    sysInfo = sysBoardInfo.getSystemInfo()        
    baseBoardInfo = sysBoardInfo.getBaseBoardInfo()
    customedType = 0 ## 0 通用，1 联想 主板名称:
    if Board_Manufacturer in baseBoardInfo and "lenovo" in baseBoardInfo[Board_Manufacturer].lower() \
        and Board_Product in baseBoardInfo and ("3157" in baseBoardInfo[Board_Product]) :
        customedType = 1
    return customedType

def Judgment_TN133A2():
    # getSystemInfo
    sysBoardInfo = SystemBoard()
    sysInfo = sysBoardInfo.getSystemInfo()        
    if System_Product in sysInfo and "长城TN133A2" in sysInfo[System_Product]:
        return True
    return False

def Judgment_323F():
    # getSystemInfo
    args = ["dmidecode","--type","11"]
    pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = pipe.stdout.readlines()
    for l in output:
        line = bytes.decode(l.strip(),"utf-8","ignore")
        if line.find("323F") >= 0:
            return True
    return False

def Judgment_PANGUW():
    args = ["lscpu"]
    pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = pipe.stdout.readlines()
    for l in output:
        line = bytes.decode(l.strip(),"utf-8","ignore")
        if line.find("PANGU") >= 0:
            return True
    return False

def Judgment_L410():
    args = ["dmidecode", "|","grep","'String 4'"]
    pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = pipe.stdout.readlines()
    for l in output:
        line = bytes.decode(l.strip(),"utf-8","ignore")
        if line.find("KVK90") >= 0 or line.find("KVK90A"):
            return True
    return False

def Judgment_HW990():
    if os.path.exists("/proc/hardware"):
        with open("/proc/hardware",'r') as fd:
            info = fd.readline()
            if re.findall("kirin.*990", info.lower()) or info.find("HUAWEI Kirin 990") >= 0 or info.find("kirin990") >= 0 or info.find("HUAWEI Kirin 9006C") >= 0:
                return True
            else:
                return False
    else:
        return False

def Judgment_HW9A0():
    if os.path.exists("/proc/hardware"):
        with open("/proc/hardware",'r') as fd:
            info = fd.readline()
            if re.findall("kirin.*9006c", info.lower()) or info.find("HUAWEI Kirin 9006C") >= 0:
                return True
            else:
                return False
    else:
        return False

def Judgment_Virt():
    args = ["systemd-detect-virt"]
    pipe = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = pipe.stdout.readlines()
    for l in output:
        line = bytes.decode(l.strip(),"utf-8","ignore")
        if "none" == line:
            return False
    return True

def Judgment_3A5000():
    arch = platform.machine()
    if "loongarch64" == arch:
        return True
    else:
        return False

def Judgment_SW():
    arch = platform.machine()
    if "sw_64" == arch:
        return True
    else:
        return False

def myctoascii(buf):
    ch = bytes(buf.encode('utf-8'))
    asci = binascii.b2a_hex(ch)
    asci = int(asci,16)
    return asci

def mystrip(s):
    if(len(s) == 0):
        return s
    while len(s) > 0 and myctoascii(s[0]) <= 32:
        s = s[1: ]
    k = len(s)
    while len(s) > 0 and myctoascii(s[k-1]) <= 32:
        s = s[ :k-1]
        k = len(s)
    i = 0
    while i < len(s):
        if myctoascii(s[i]) < 32:
            s = s[ :i] + s[i+1: ]
            i -= 1
        i += 1
    return s

def get_url(v,p):
    vendors = {
#CPU产商
        "INTEL":["Intel"],
        "AMD":["AMD"],
        "VIMICRO":["Vimicro"],
#显卡产商
        "ATI":["ATI"],
        "1002":["ATI"],
        "SIS":["SIS"],
        "1039":["SIS"],
        "NVIDIA":["Nvidia"],
        "VIA":["VIA"],
        "XFX":["XFX"],
        "SUPERGRAPHIC":["Supergraphic"],
        "JINGJIA":["JJM"],
        "Wuhan Digital Engineering":["WDE"],
#显示器产商
        "AUO":["AUO"],
        "AOC":["AOC"],
        "PHILIPS":["Philips"],
        "PHL":["Philips"],
        "LEN":["Lenovo"],
        "SEC":["SAMSUNG"],
#电脑品牌
        "HASEE":["Hasee"],
        "FOUNDER":["Founder"],
        "TONGFANG":["Tongfang"],
        "TSINGHUA":["Tongfang"],
        "ACER":["Acer"],
        "LENOVO":["Lenovo"],
        "ASUSTEK":["ASUS"],
        "NEC":["NEC"],
        "HP":["HP"],
        "HEWLETT-PACKARD":["HP"],
        "SAMSUNG":["SAMSUNG"],
        "TOSHIBA":["TOSHIBA"],
        "APPLE":["Apple"],
        "DELL":["DELL"],
        "FUJITSU":["FUJITSU"],
        "PANASONIC":["Panasonic"],
        "SONY":["SONY"],
        "IBM":["IBM"],
#虚拟机
        "INNOTEK":["VirtualBox"],
        "VBOX":["VirtualBox"],
        "VIRTUALBOX":["VirtualBox"],
#网卡产商
        "3COM":["3COM"],
        "D-LINK":["D-LINK"],
        "RALINK":["Ralink"],
        "ATHEROS":["Atheros"],
        "MARVELL":["Marvell"],
        "BROADCOM":["Broadcom"],
#硬盘产商
        "EXCELSTOR":["Excelstor"],
        "HITACHI":["Hitachi"],
        "MAXTOR":["Maxtor"],
        "WESTERN":["Western Digital"],
        "LITEON":["Liteon"],
        "SEAGATE":["Seagate"],
        "QUANTUM":["Quantum"],
#光驱产商
        "PLDS":["PLDS"],
        "PBDS":["PLDS"],
        "HL-DT-ST":["LG"],
        "OPTIARC":["SONY"],
        "TSSTCORP":["TSSTcorp"],
        "PIONEER":["Pioneer"],
        "MATSHITA":["Panasonic"],
#声卡产商
        "REALTEK":["Realtek"],
        "CREATIVE":["Creative"],
        "LOONGSON":["Loongson"],
        "HISILICON":["HiSilicon"],
#摄像头
        "SONIX":["Sonix"],
        "ETRON":["Etron"],
        "AVEO":["Aveo"],
        "SYNTEK":["Syntek"],
        "EMPIA":["Empia"],
        "CHICONY":["Chicony"],
        "OMNIVISION":["OmniVision"],
#鼠标产商
        "LOGITECH":["Logitech"],
        "SUNPLUS":["Sunplus"],
        "PRIMAX":["Primax"],
        "PIXART":["Pixart"],
        "TRUST":["Trust"],
        "1BCF":["Rapoo"],
        "AVAGO":["Avago"],
        "MICROSOFT":["Microsoft"],
#键盘产商
        "RAPOO":["Rapoo"],
#主板产商
        "GIGABYTE":["Gigabyte"],
        "BIOSTAR":["Biostar"],
        "COLORFUL":["Colorful"],
        "YESTON":["Yeston"],
#指纹识别
        "UPEK":["Authentec"],
        "AUTHENTEC":["Authentec"],
#闪存产商
        "KINGSTON":["Kingston"],
        "KINGMAX":["Kingmax"],
        "KINGBOX":["Kingbox"],
        "HYNIX":["Hynix"],
        "HYUNDAI":["Hynix"],
        "MICRON":["Micron"],
        "06C1":["Asint"],
        "ADATA":["ADATA"],
        "ZTE":["ZTE"],
        "EAGET":["Eaget"],
        "TEXAS":["Texas Instruments"],
        "MOTOROLA":["Motorola"],
#电源产商
        "SMP":["SMP"],
        "SIMPLO":["SMP"],
#BIOS产商
        "AMERICAN":["AMI"],
        "AWARD":["Phoenix"],
        "PHOENIX":["Phoenix"]
        }
    tmp = v.split(" ")[0]
    tmp = re.findall("([a-zA-Z0-9-]+)", tmp)
    if tmp :
        url = vendors.get(tmp[0].upper()) 
        if url:
            return url[0]
        else :
            k = p.split(" ")[0]
            url = vendors.get(k.upper())
            if url:
                return url[0]
    else:
        tmp = p.split(" ")[0]
        url = vendors.get(tmp.upper())
        if url:
            return url[0]

    tmp = re.findall("JingJia", p)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("Loongson", p, flags=re.IGNORECASE)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]

    tmp = re.findall("Wuhan Digital Engineering", p)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]

    tmp = re.findall("ATI", v)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("SIS", v)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("Intel", v)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("ATI", p)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("SIS", p)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    tmp = re.findall("Intel", p)
    if tmp :
        url = vendors.get(tmp[0].upper())
        if url:
            return url[0]
    return v

def exchangeManufacturer(manufacturer) :
    lowerManufacturer = manufacturer.lower()
    if "samsung" in lowerManufacturer :     ## 三星
        return "SX"
    elif "hynix" in lowerManufacturer :     ## 海力士
        return "HL"
    elif "ramaxel" in lowerManufacturer :     ## 记忆
        return "JY"
    elif "uniic" in lowerManufacturer :     ## 紫光
        return "ZG"
    elif "gloway" in lowerManufacturer or "changxin" in  lowerManufacturer:     ## 长鑫
        return "CX"
    elif "crucial" in lowerManufacturer :     ## 美光
        return "MG"
    elif "western digital" in lowerManufacturer :     ## 西数
        return "XS"
    elif "toshiba" in lowerManufacturer :     ## 东芝
        return "DZ"
    elif "ymtc" in lowerManufacturer :     ## 长江存储
        return "CC"
    elif "goke" in lowerManufacturer :     ## 国科微
        return "GK"
    elif "biwin" in lowerManufacturer :     ## 佰维
        return "BW"
    elif "amd" in lowerManufacturer :     ## AMD
        return "AMD"
    elif "nvidia" in lowerManufacturer :     ## Nvidia
        return "NV"
    elif "jjm" in lowerManufacturer :     ## 景嘉微
        return "JJW"
    elif "glenfly" in lowerManufacturer :     ## 格兰菲
        return "GLF"
    elif lowerManufacturer.startswith("st") :     ## 希捷
        return "XJ"
    elif "phison" in lowerManufacturer :     ## 大唐
        return "DT"
    else :
        return manufacturer

if __name__ == "__main__":
    Judgment_TN133A2();
