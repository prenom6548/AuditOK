/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMCONFIG_DATA_H__
#define __KMCONFIG_DATA_H__

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <sys/mount.h>
#include <nlohmann/json.hpp>

#define KAIMING_BOX (access("/opt/kaiming-tools/bin/crun", F_OK) == 0 ? "/opt/kaiming-tools/bin/crun" : "/usr/bin/crun")

namespace nlohmann
{
    template<class J, class T>
    inline void from_json(const J &j, std::optional<T> &o)
    {
        if (j.is_null())
        {
            o = std::nullopt;
        }
        else
        {
            o = j.template get<T>();
        }
    }

    template<class J, class T>
    inline void to_json(J &j, const std::optional<T> &o)
    {
        if (o.has_value())
        {
            j = o.value();
        }
    }
} // namespace nlohmann

struct Root
{
    std::string path;
    std::optional<bool> readonly;
};

inline void to_json(nlohmann::json &j, const Root &r)
{
    j["path"] = r.path;
    if (r.readonly)
    {
        j["readonly"] = r.readonly;
    }
}

inline void from_json(const nlohmann::json &j, std::vector<std::string> &params)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            params.push_back(item.get<std::string>());
        }
    }
}

inline void from_json(const nlohmann::json &j, Root &r)
{
    r.path = j.at("path").get<std::string>();
    if (j.contains("readonly"))
    {
        r.readonly = j.at("readonly").get<bool>();
    }
}

struct User
{
    std::optional<std::vector<int64_t>> additionalGids;
    std::optional<int64_t> gid;
    std::optional<int64_t> uid;
    std::optional<int64_t> umask;
    std::optional<std::string> username;
};

inline void to_json(nlohmann::json &j, const User &u)
{
    if (u.additionalGids)
    {
        j["additionalGids"] = u.additionalGids;
    }
    if (u.gid)
    {
        j["gid"] = u.gid;
    }
    if (u.uid)
    {
        j["uid"] = u.uid;
    }
    if (u.umask)
    {
        j["umask"] = u.umask;
    }
    if (u.username)
    {
        j["username"] = u.username;
    }
}

inline void from_json(const nlohmann::json &j, User &u)
{
    if (j.contains("additionalGids"))
    {
        if (j.at("additionalGids").is_array())
        {
            u.additionalGids = j.at("additionalGids").get<std::vector<int64_t>>();
        }
    }
    if (j.contains("gid"))
    {
        u.gid = j.at("gid").get<int64_t>();
    }
    if (j.contains("uid"))
    {
        u.uid = j.at("uid").get<int64_t>();
    }
    if (j.contains("umask"))
    {
        u.umask = j.at("umask").get<int64_t>();
    }
    if (j.contains("username"))
    {
        u.username = j.at("username").get<std::string>();
    }
}

struct Capabilities
{
    std::vector<std::string> bounding;
    std::vector<std::string> permitted;
    std::vector<std::string> inheritable;
    std::vector<std::string> effective;
    std::vector<std::string> ambient;
};

inline void to_json(nlohmann::json &j, const Capabilities &c)
{
    j["bounding"] = c.bounding;
    j["permitted"] = c.permitted;
    j["inheritable"] = c.inheritable;
    j["effective"] = c.effective;
    j["ambient"] = c.ambient;
}

inline void from_json(const nlohmann::json &j, Capabilities &c)
{
    c.bounding = j.at("bounding").get<std::vector<std::string>>();
    c.permitted = j.at("permitted").get<std::vector<std::string>>();
    c.inheritable = j.at("inheritable").get<std::vector<std::string>>();
    c.effective = j.at("effective").get<std::vector<std::string>>();
    c.ambient = j.at("ambient").get<std::vector<std::string>>();
}

struct Process
{
    std::string cwd;
    std::vector<std::string> args;
    std::vector<std::string> env;
    std::optional<bool> terminal;
    std::optional<User> user;
    std::optional<Capabilities> capabilities;
};

inline void to_json(nlohmann::json &j, const Process &p)
{
    j["cwd"] = p.cwd;
    if (p.terminal)
    {
        j["terminal"] = p.terminal;
    }
    if (p.user)
    {
        j["user"] = p.user.value();
    }
    if (p.capabilities)
    {
        j["capabilities"] = p.capabilities.value();
    }
    j["args"] = p.args;
    j["env"] = p.env;
}

inline void from_json(const nlohmann::json &j, Process &p)
{
    p.cwd = j.at("cwd").get<std::string>();
    if (j.contains("terminal"))
    {
        p.terminal = j.at("terminal").get<bool>();
    }
    if (j.contains("user"))
    {
        p.user = j.at("user").get<User>();
    }
    if (j.contains("capabilities"))
    {
        p.capabilities = j.at("capabilities").get<Capabilities>();
    }
    if (j.contains("args"))
    {
        p.args = j.at("args").get<std::vector<std::string>>();
    }
    if (j.contains("env"))
    {
        p.env = j.at("env").get<std::vector<std::string>>();
    }
}

struct Hook
{
    std::string path;
    std::optional<std::vector<std::string>> args;
    std::optional<std::vector<std::string>> env;
};

inline void to_json(nlohmann::json &j, const Hook &h)
{
    j["path"] = h.path;
    if (h.args)
    {
        j["args"] = h.args;
    }
    if (h.env)
    {
        j["env"] = h.env;
    }
}

inline void from_json(const nlohmann::json &j, Hook &h)
{
    h.path = j.at("path").get<std::string>();
    if (j.contains("args"))
    {
        h.args = j.at("args").get<std::vector<std::string>>();
    }
    if (j.contains("env"))
    {
        h.env = j.at("env").get<std::vector<std::string>>();
    }
}

struct Hooks
{
    std::optional<std::vector<Hook>> prestart;
    std::optional<std::vector<Hook>> createRuntime;
    std::optional<std::vector<Hook>> createContainer;
    std::optional<std::vector<Hook>> startContainer;
    std::optional<std::vector<Hook>> postStart;
    std::optional<std::vector<Hook>> postStop;
};

inline void to_json(nlohmann::json &j, const Hooks &h)
{
    if (h.prestart)
    {
        j["prestart"] = h.prestart;
    }
    if (h.createRuntime)
    {
        j["createRuntime"] = h.createRuntime;
    }
    if (h.createContainer)
    {
        j["createContainer"] = h.createContainer;
    }
    if (h.startContainer)
    {
        j["startContainer"] = h.startContainer;
    }
    if (h.postStart)
    {
        j["poststart"] = h.postStart;
    }
    if (h.postStop)
    {
        j["poststop"] = h.postStop;
    }
}

inline void from_json(const nlohmann::json &j, std::vector<Hook> &h)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            Hook h_tmp;
            from_json(item, h_tmp);
            h.push_back(h_tmp);
        }
    }
}

inline void from_json(const nlohmann::json &j, Hooks &h)
{
    if (j.contains("prestart"))
    {
        h.prestart = j.at("prestart").get<std::vector<Hook>>();
    }
    if (j.contains("createRuntime"))
    {
        h.createRuntime = j.at("createRuntime").get<std::vector<Hook>>();
    }
    if (j.contains("createContainer"))
    {
        h.createContainer = j.at("createContainer").get<std::vector<Hook>>();
    }
    if (j.contains("startContainer"))
    {
        h.startContainer = j.at("startContainer").get<std::vector<Hook>>();
    }
    if (j.contains("poststart"))
    {
        h.postStart = j.at("poststart").get<std::vector<Hook>>();
    }
    if (j.contains("poststop"))
    {
        h.postStop = j.at("poststop").get<std::vector<Hook>>();
    }
}

struct IDMap
{
    uint64_t containerID = 0;
    uint64_t hostID = 0;
    uint64_t size = 0;
};

inline void to_json(nlohmann::json &j, const IDMap &i)
{
    j["hostID"] = i.hostID;
    j["containerID"] = i.containerID;
    j["size"] = i.size;
}

inline void from_json(const nlohmann::json &j, IDMap &i)
{
    i.hostID = j.at("hostID").get<u_int64_t>();
    i.containerID = j.at("containerID").get<u_int64_t>();
    i.size = j.at("size").get<u_int64_t>();
}

inline void from_json(nlohmann::json &j, std::vector<IDMap> &i)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            IDMap i_tmp;
            from_json(item, i_tmp);
            i.push_back(i_tmp);
        }
    }
}

struct SyscallArg
{
    uint32_t index;
    std::string op;
    uint64_t value;
};

inline void to_json(nlohmann::json &j, const SyscallArg &arg)
{
    j["index"] = arg.index;
    j["op"] = arg.op;
    j["value"] = arg.value;
}

inline void from_json(const nlohmann::json &j, SyscallArg &s)
{
    s.index = j.at("index").get<uint32_t>();
    s.op = j.at("op").get<std::string>();
    s.value = j.at("value").get<uint64_t>();
}

struct Syscall
{
    std::vector<std::string> names;
    std::string action;
    uint32_t errnoRet = 1;
    std::vector<SyscallArg> args;
};

inline void to_json(nlohmann::json &j, const Syscall &s)
{
    j["names"] = s.names;
    j["action"] = s.action;
    j["errnoRet"] = s.errnoRet;
    j["args"] = s.args;
}

inline void from_json(const nlohmann::json &j, Syscall &s)
{
    s.names = j.at("names").get<std::vector<std::string>>();
    s.action = j.at("action").get<std::string>();
    s.errnoRet = j.at("errnoRet").get<uint32_t>();
    s.args = j.at("args").get<std::vector<SyscallArg>>();
}

inline void from_json(const nlohmann::json &j, std::vector<Syscall> &s)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            Syscall s_tmp;
            from_json(item, s_tmp);
            s.push_back(s_tmp);
        }
    }
}

struct Seccomp
{
    std::string defaultAction = "SCMP_ACT_ALLOW";
    std::vector<std::string> architectures;
    std::vector<Syscall> syscalls;
};

inline void to_json(nlohmann::json &j, const Seccomp &s)
{
    j["defaultAction"] = s.defaultAction;
    j["architectures"] = s.architectures;
    j["syscalls"] = s.syscalls;
}

inline void from_json(const nlohmann::json &j, Seccomp &s)
{
    s.defaultAction = j.at("defaultAction").get<std::string>();
    s.architectures = j.at("architectures").get<std::vector<std::string>>();
    s.syscalls = j.at("syscalls").get<std::vector<Syscall>>();
}

struct ResourceMemory
{
    int64_t limit = -1;
    int64_t reservation = -1;
    int64_t swap = -1;
};

inline void to_json(nlohmann::json &j, const ResourceMemory &r)
{
    j["limit"] = r.limit;
    j["reservation"] = r.reservation;
    j["swap"] = r.swap;
}

inline void from_json(const nlohmann::json &j, ResourceMemory &r)
{
    r.limit = j.at("limit").get<int64_t>();
    r.reservation = j.at("reservation").get<int64_t>();
    r.swap = j.at("swap").get<int64_t>();
}

struct ResourceCPU
{
    uint64_t shares = 1024;
    int64_t quota = 100000;
    uint64_t period = 100000;
    int64_t realtimeRuntime;
    int64_t realtimePeriod;
    std::string cpus;
    std::string mems;
};

inline void to_json(nlohmann::json &j, const ResourceCPU &c)
{
    j["shares"] = c.shares;
    j["quota"] = c.quota;
    j["period"] = c.period;
    j["realtimeRuntime"] = c.realtimeRuntime;
    j["realtimePeriod"] = c.realtimePeriod;
    j["cpus"] = c.cpus;
    j["mems"] = c.mems;
}

inline void from_json(const nlohmann::json &j, ResourceCPU &c)
{
    c.shares = j.at("shares").get<uint64_t>();
    c.quota = j.at("quota").get<int64_t>();
    c.period = j.at("period").get<uint64_t>();
    c.realtimeRuntime = j.at("realtimeRuntime").get<int64_t>();
    c.realtimePeriod = j.at("realtimePeriod").get<int64_t>();
    c.cpus = j.at("cpus").get<std::string>();
    c.mems = j.at("mems").get<std::string>();
}

struct Resources
{
    ResourceMemory memory;
    ResourceCPU cpu;
};

inline void to_json(nlohmann::json &j, const Resources &r)
{
    j["cpu"] = r.cpu;
    j["memory"] = r.memory;
}

inline void from_json(const nlohmann::json &j, Resources &r)
{
    r.cpu = j.at("cpu").get<ResourceCPU>();
    r.memory = j.at("memory").get<ResourceMemory>();
}

struct NameSpace
{
    std::string linuxNamespace;
};

inline void to_json(nlohmann::json &j, const NameSpace &n)
{
    j = nlohmann::json::object();
    j["type"] = n.linuxNamespace;
}

inline void from_json(const nlohmann::json &j, NameSpace &n)
{
    n.linuxNamespace = j.at("type").get<std::string>();
}

inline void from_json(nlohmann::json &j, std::vector<NameSpace> &n)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            NameSpace n_tmp;
            from_json(item, n_tmp);
            n.push_back(n_tmp);
        }
    }
}

struct Linux
{
    std::vector<NameSpace> namespaces;
    std::vector<IDMap> uidMappings;
    std::vector<IDMap> gidMappings;
    std::optional<Seccomp> seccomp;
    std::optional<Resources> resources;
    std::vector<std::string> capabilities;
    std::string cgroupsPath;
    std::string rootfsPropagation;
};

inline void to_json(nlohmann::json &j, const Linux &l)
{
    j["namespaces"] = l.namespaces;
    j["uidMappings"] = l.uidMappings;
    j["gidMappings"] = l.gidMappings;
    if (!l.cgroupsPath.empty())
    {
        j["cgroupsPath"] = l.cgroupsPath;
    }
    if (!l.rootfsPropagation.empty())
    {
        j["rootfsPropagation"] = l.rootfsPropagation;
    }
    if (l.seccomp)
    {
        j["seccomp"] = l.seccomp;
    }
    if (l.resources)
    {
        j["resources"] = l.resources;
    }
    if (!l.capabilities.empty())
    {
        j["capabilities"] = l.capabilities;
    }
}

inline void from_json(const nlohmann::json &j, Linux &l)
{
    l.namespaces = j.at("namespaces").get<std::vector<NameSpace>>();
    l.uidMappings = j.at("uidMappings").get<std::vector<IDMap>>();
    l.gidMappings = j.at("gidMappings").get<std::vector<IDMap>>();

    if (j.contains("seccomp"))
    {
        l.seccomp = j.at("seccomp").get<Seccomp>();
    }
    if (j.contains("resources"))
    {
        l.resources = j.at("resources").get<Resources>();
    }

    l.cgroupsPath = j.at("cgroupsPath").get<std::string>();
    l.capabilities = j.at("capabilities").get<std::vector<std::string>>();
}

struct Mount
{
    std::string source;
    std::string destination;
    std::string type;
    std::optional<std::vector<std::string>> options;
};

inline void to_json(nlohmann::json &j, const Mount &m)
{
    j["source"] = m.source;
    j["destination"] = m.destination;
    j["type"] = m.type;
    if (m.options)
    {
        j["options"] = m.options;
    }
}

inline void from_json(const nlohmann::json &j, Mount &m)
{
    m.source = j.at("source").get<std::string>();
    m.destination = j.at("destination").get<std::string>();
    m.type = j.at("type").get<std::string>();
    if (j.contains("options"))
    {
        m.options = j.at("options").get<std::vector<std::string>>();
    }
}

inline void from_json(const nlohmann::json &j, std::vector<Mount> &m)
{
    if (j.is_array())
    {
        for (const auto &item : j)
        {
            Mount m_tmp;
            from_json(item, m_tmp);
            m.push_back(m_tmp);
        }
    }
}

struct KMConfigDbata
{
    std::optional<std::string> ociVersion;
    std::optional<std::string> hostname;
    std::optional<std::string> domainname;
    std::optional<std::map<std::string, std::string>> annotations;
    std::optional<Hooks> hooks;
    std::optional<Linux> linuxConfig;
    std::optional<Process> process;
    std::optional<Root> root;
    std::optional<std::vector<Mount>> mounts;
};

inline void to_json(nlohmann::json &j, const KMConfigDbata &c)
{
    if (c.ociVersion)
    {
        j["ociVersion"] = c.ociVersion;
    }

    if (c.hostname)
    {
        j["hostname"] = c.hostname;
    }

    if (c.domainname)
    {
        j["domainname"] = c.domainname;
    }

    if (c.annotations)
    {
        j["annotations"] = c.annotations;
    }

    if (c.root)
    {
        j["root"] = c.root;
    }

    if (c.hooks)
    {
        j["hooks"] = c.hooks;
    }

    if (c.process)
    {
        j["process"] = c.process;
    }

    if (c.linuxConfig)
    {
        j["linux"] = c.linuxConfig;
    }

    if (c.mounts)
    {
        j["mounts"] = c.mounts;
    }
}

inline void from_json(const nlohmann::json &j, KMConfigDbata &c)
{
    if (j.contains("ociVersion"))
    {
        c.ociVersion = j.at("ociVersion").get<std::string>();
    }

    if (j.contains("hostname"))
    {
        c.hostname = j.at("hostname").get<std::string>();
    }

    if (j.contains("domainname"))
    {
        c.domainname = j.at("domainname").get<std::string>();
    }

    if (j.contains("annotations"))
    {
        c.annotations = j.at("annotations").get<std::map<std::string, std::string>>();
    }

    if (j.contains("root"))
    {
        c.root = j.at("root").get<Root>();
    }

    if (j.contains("hooks"))
    {
        c.hooks = j.at("hooks").get<Hooks>();
    }

    if (j.contains("process"))
    {
        c.process = j.at("process").get<Process>();
    }

    if (j.contains("linux"))
    {
        c.linuxConfig = j.at("linux").get<Linux>();
    }

    if (j.contains("mounts"))
    {
        c.mounts = j.at("mounts").get<std::vector<Mount>>();
    }
}

// 容器状态枚举值
enum class Status : int
{
    Created,
    Creating,
    Running,
    Stopped
};

inline void from_json(const nlohmann::json &j, Status &s)
{
    if (j == "created")
    {
        s = Status::Created;
    }
    else if (j == "creating")
    {
        s = Status::Creating;
    }
    else if (j == "running")
    {
        s = Status::Running;
    }
    else if (j == "stopped")
    {
        s = Status::Stopped;
    }
}

// 容器状态
struct State
{
    Status status;
    std::optional<std::map<std::string, std::string>> annotations;
    std::string bundle;
    std::string id;
    std::string ociVersion;
    std::optional<int64_t> pid;
};

inline void from_json(const nlohmann::json &j, State &x)
{
    if (j.contains("annotations"))
    {
        x.annotations = j.at("annotations").get<std::map<std::string, std::string>>();
    }
    else
    {
        x.annotations = std::nullopt;
    }
    x.bundle = j.at("bundle").get<std::string>();
    x.id = j.at("id").get<std::string>();
    x.ociVersion = j.at("ociVersion").get<std::string>();
    x.pid = j.at("pid").get<int64_t>();
    x.status = j.at("status").get<Status>();
}

// crun list 命令输出的容器信息
struct ContainerListItem
{
    int64_t pid;
    std::string bundle;
    std::string created;
    std::string id;
    std::string owner;
    std::string status;
};

inline void from_json(const nlohmann::json &j, ContainerListItem &x)
{
    x.bundle = j.at("bundle").get<std::string>();
    x.created = j.at("created").get<std::string>();
    x.id = j.at("id").get<std::string>();
    x.owner = j.at("owner").get<std::string>();
    x.pid = j.at("pid").get<int64_t>();
    x.status = j.at("status").get<std::string>();
}

inline void from_json(const nlohmann::json &j, std::vector<ContainerListItem> &x)
{
    if (!j.is_array())
    {
        return;
    }

    for (const auto &item : j)
    {
        ContainerListItem e;
        from_json(item, e);
        x.push_back(e);
    }
}

struct KMPSInfoItem
{
    int64_t pid = 0;
    std::string appid;
    std::string cid;
    std::string path;
};

inline void from_json(const nlohmann::json &j, KMPSInfoItem &i)
{
    i.appid = j.at("appid").get<std::string>();
    i.cid = j.at("cid").get<std::string>();
    i.pid = j.at("pid").get<int64_t>();
    i.path = j.at("path").get<std::string>();
}

inline void to_json(nlohmann::json &j, const KMPSInfoItem &i)
{
    j["appid"] = i.appid;
    j["cid"] = i.cid;
    j["pid"] = i.pid;
    j["path"] = i.path;
}

struct KMPSInfoList
{
    std::vector<KMPSInfoItem> infolist;
};

inline void to_json(nlohmann::json &j, const KMPSInfoList &l)
{
    j = l.infolist;
}

inline void from_json(const nlohmann::json &j, KMPSInfoList &l)
{
  l.infolist = j.get<std::vector<KMPSInfoItem>>(); 
}

#endif // __KMCONFIG_DATA_H__