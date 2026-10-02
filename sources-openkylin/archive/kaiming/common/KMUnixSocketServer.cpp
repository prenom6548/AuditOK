/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMUnixSocketServer.h"

#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pty.h>
#include <filesystem>

#include "KMFileUtils.h"
#include "KMLogger.h"

namespace fs = std::filesystem;

KMUnixSocketServer::KMUnixSocketServer()
    : m_serverfd(-1)
    , m_efd(-1)
    , m_isCreated(false)
    , m_socketPath(generateRandomPath())
    , m_isVirtualTty(false)
{
    m_isVirtualTty = isatty(fileno(stdin)) == 1 ? false : true;
}

KMUnixSocketServer::KMUnixSocketServer(const std::string &path)
    : m_serverfd(-1)
    , m_efd(-1)
    , m_isCreated(false)
    , m_socketPath(path)
{
}

KMUnixSocketServer::~KMUnixSocketServer()
{
    release();
}

/**
 * @brief 生成一个随机的 socket 路径
 * @return 返回生成的 socket 路径字符串
 */
std::string KMUnixSocketServer::generateRandomPath()
{
    std::string path = "/tmp/kaiming_XXXXXX";
    if (KMFileUtils::genTempName(path) && !fs::exists(path))
    {
        // todo: 确保路径唯一且不与其他应用冲突
        return path.append(".sock");
    }
    else
    {
        KMError("[SERVER] Failed to generate temp socket path.");
        return "/tmp/kaiming_fallback";
    }
}

/**
 * @brief 创建 socket 文件并绑定到指定路径
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::create()
{
    m_serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_serverfd < 0)
    {
        KMError("socket create failed");
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    size_t copyLen = std::min(m_socketPath.size(), sizeof(addr.sun_path) - 1);
    std::copy_n(m_socketPath.c_str(), copyLen, addr.sun_path);
    addr.sun_path[copyLen] = '\0';

    if (bind(m_serverfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        KMError("bind failed");
        return false;
    }

    if (listen(m_serverfd, 1) < 0)
    {
        KMError("listen failed");
        return false;
    }

    m_isCreated = true;

    return true;
}

/**
 * @brief 判断 socket 是否已成功创建
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::isCreate() const
{
    return m_isCreated;
}

/**
 * @brief 获取当前 socket 路径
 * @return 获取 socket 路径
 */
std::string KMUnixSocketServer::getPath() const
{
    return m_socketPath;
}

/**
 * @brief 关闭 socket 并释放资源
 */
void KMUnixSocketServer::release()
{
    if (m_serverfd != -1)
    {
        close(m_serverfd);
        m_serverfd = -1;
    }

    if (m_efd != -1)
    {
        close(m_efd);
        m_efd = -1;
    }

    m_isCreated = false;
    unlink(m_socketPath.c_str());
}

/**
 * @brief 接收文件描述符
 * @param socket，表示用于接收的 socket 文件描述符
 * @return 成功返回接收到的文件描述符，失败返回 -1
 */
int KMUnixSocketServer::receivefd(int socket)
{
    char dummy;
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(int))] = {};
    struct iovec io = { .iov_base = &dummy, .iov_len = sizeof(dummy) };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    ssize_t ret = recvmsg(socket, &msg, 0);
    if (ret <= 0)
    {
        if (ret == 0)
        {
            KMDebug("[SERVER] Client closed the connection during recvmsg.");
        }
        else
        {
            KMDebug("recvmsg error");
        }
        stop();
        return -1;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg || cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS || cmsg->cmsg_len < CMSG_LEN(sizeof(int)))
    {
        KMDebug("[SERVER] Invalid or missing file descriptor in control message.");
        return -1;
    }

    int fd = -1;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
    if (fd < 0 || fcntl(fd, F_GETFD) == -1)
    {
        KMError("[SERVER] Received invalid file descriptor.");
        return -1;
    }

    return fd;
}

/**
 * @brief 从本地终端读取窗口大小，并同步设置到 slave 端的 pty。
 * @param masterfd 主端的 pseudo terminal 文件描述符
 * @param slavefd 从端的 pseudo terminal 文件描述符
 */
void KMUnixSocketServer::syncWindowSize(int masterfd, int slavefd)
{
    struct winsize ws;
    if (ioctl(masterfd, TIOCGWINSZ, &ws) == 0)
    {
        ioctl(slavefd, TIOCSWINSZ, &ws);
    }
}

/**
 * @brief 处理单个连接
 * @param connfd，连接的 socket 文件描述符
 * @note 此函数在连接建立后调用，用于执行具体的处理逻辑
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::handleConnection(int connfd)
{
    int ptyfd = receivefd(connfd);
    if (!validateFD(ptyfd))
    {
        close(connfd);
        return false;
    }

    syncWindowSize(STDIN_FILENO, ptyfd);

    struct termios ori;
    setupTerminal(ori);

    struct termios tty;
    if (tcgetattr(ptyfd, &tty) == 0)
    {
        tty.c_lflag |= (ECHO | ICANON);
        tty.c_oflag |= OPOST;
        tcsetattr(ptyfd, TCSANOW, &tty);
    }

    struct winsize last_ws = {};
    ioctl(STDIN_FILENO, TIOCGWINSZ, &last_ws);

    char buffer[256] = { 0 };
    std::string outputBuffer;
    std::string lastChunk, currChunk;

    bool bRet = true;
    bool ptyClosed = false;
    const std::string exitflag = "kaiming application exit code:";
    bool isInteractiveShell = !getenv("TERM") ? false : true;

    while (!ptyClosed)
    {
        updateWindowSizeIfChanged(ptyfd, last_ws);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ptyfd, &fds);
        FD_SET(m_efd, &fds);

        int maxfd = std::max({ ptyfd, m_efd }) + 1;

        // 非交互tty环境不需要监控STDIN_FILENO
        if (isatty(fileno(stdin)) == 1)
        {
            FD_SET(STDIN_FILENO, &fds);
            maxfd = std::max({ ptyfd, m_efd, STDIN_FILENO }) + 1;
        }

        int ret = select(maxfd, &fds, nullptr, nullptr, nullptr);
        if (ret == -1)
        {
            KMError("[Server] select error");
            break;
        }

        if (FD_ISSET(ptyfd, &fds))
        {
            ssize_t n = read(ptyfd, buffer, sizeof(buffer));
            if (n > 0)
            {
                processPtyOutput(buffer, n, isInteractiveShell, outputBuffer, lastChunk, currChunk, exitflag);
            }
            else
            {
                ptyClosed = true;

                std::string combined = lastChunk + currChunk;

                if (extractExitCode(combined, exitflag, exitCode))
                {
                    if (exitCode != "0")
                    {
                        bRet = false;
                    }
                }
                break;
            }
        }

        if (FD_ISSET(m_efd, &fds))
        {
            eventfd_t value;
            eventfd_read(m_efd, &value);
            KMDebug("[SERVER] stop success.");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fds))
        {
            ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (n > 0)
            {
                ssize_t result = write(ptyfd, buffer, n);
                if (result == -1)
                {
                    KMError("Failed to write to pty: " + std::string(strerror(errno)));
                }
            }
        }
    }

    //非交互输出缓冲区剩余
    if (!isInteractiveShell && !outputBuffer.empty())
    {
        outputBuffer.erase(std::remove(outputBuffer.begin(), outputBuffer.end(), '\r'), outputBuffer.end());
        outputBuffer = removeAnsi(outputBuffer);
        outputBuffer += '\n';
        ssize_t result = write(STDOUT_FILENO, outputBuffer.c_str(), outputBuffer.size());
        if (result == -1)
        {
            KMError("Failed to write to stdout: " + std::string(strerror(errno)));
        }
    }

    close(ptyfd);
    close(connfd);
    restoreTerminal(ori);

    return bRet;
}

/**
 * @brief 启动 socket 监听并处理客户端连接
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::run()
{
    if (!m_isCreated)
        return false;

    m_fut = std::async(std::launch::async, [&]() {
        return start();
    });

    return true;
}

/**
 * @brief 开始接收文件描述符
 * @return 成功 true,失败 false
 */
bool KMUnixSocketServer::start()
{
    bool bRet = false;
    m_efd = eventfd(0, EFD_NONBLOCK);
    if (m_efd == -1)
    {
        KMError("eventfd create failed");
        return bRet;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_serverfd, &fds);
    FD_SET(m_efd, &fds);
    int maxfd = std::max(m_serverfd, m_efd) + 1;
    int ret = select(maxfd, &fds, nullptr, nullptr, nullptr);
    if (ret == -1)
    {
        KMError("select error");
        return bRet;
    }

    if (FD_ISSET(m_efd, &fds))
    {
        eventfd_t value;
        eventfd_read(m_efd, &value);
        KMDebug("[SERVER] stop success .");
        bRet = true;
    }

    if (FD_ISSET(m_serverfd, &fds))
    {
        int connfd = accept(m_serverfd, nullptr, nullptr);
        if (connfd < 0)
        {
            KMError("Unix socket server accept failed");
            return bRet;
        }

        bRet = handleConnection(connfd);
    }

    return bRet;
}

/**
 * @brief 停止
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::stop()
{
    if (m_efd != -1)
    {
        eventfd_write(m_efd, 1);
    }

    return true;
}

/**
 * @brief 等待 socket 服务器完成处理
 * @return 成功 true，失败 false
 */
bool KMUnixSocketServer::wait()
{
    if (!m_fut.valid())
    {
        return true;
    }

    return m_fut.get();
}

/**
 * @brief 去除 ANSI 控制序列（如颜色、光标移动等）
 * @param input 原始文本
 * @return 移除控制序列后的纯文本
 */
std::string KMUnixSocketServer::removeAnsi(const std::string &input)
{
    static const std::regex ansi_regex("\033\\[[0-9;]*[A-Za-z]");
    return std::regex_replace(input, ansi_regex, "");
}

/**
 * @brief 处理 pty 输出，根据是否为交互终端进行不同方式处理（写终端 or 缓存处理）
 * @param buffer 当前从 pty 读出的原始数据指针
 * @param n 实际读出的字节数
 * @param isInteractiveShell 是否处于交互终端环境（tty）
 * @param outputBuffer 用于非交互模式下的行缓冲输出（按行）
 * @param lastChunk 上一轮读取的数据块（用于与当前块拼接找 exit code）
 * @param currChunk 当前读取到的数据块（用于与上一块拼接）
 */
void KMUnixSocketServer::processPtyOutput(char *buffer, ssize_t n, bool isInteractiveShell, std::string &outputBuffer, std::string &lastChunk, std::string &currChunk, std::string exitflag)
{
    lastChunk = std::move(currChunk);
    currChunk.assign(buffer, n);

    if (isInteractiveShell)
    {
        std::string buf(currChunk);
        std::string tmpCode;
        if (extractExitCode(buf, exitflag, tmpCode))
        {
            exitCode = tmpCode;

            // 去除 lastChunk 前后空白
            trim(lastChunk);
            errorInfo = lastChunk;

            return;
        }
        if (!m_isVirtualTty)
        {
            // 没有退出码或退出码为0 → 输出当前buffer
            ssize_t w = write(STDOUT_FILENO, buffer, n);
            if (w == -1)
                KMError("Failed to write to stdout: " + std::string(strerror(errno)));
        }

        return;
    }

    // 非交互模式逐行处理
    outputBuffer.append(buffer, n);
    size_t pos = 0;
    while ((pos = outputBuffer.find('\n')) != std::string::npos)
    {
        std::string line = outputBuffer.substr(0, pos + 1);
        std::string clean = removeAnsi(line);
        //line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::string tmpCode;
        if (extractExitCode(clean, exitflag, tmpCode))
        {
            exitCode = tmpCode;
        }
        else
        {
            if (!m_isVirtualTty)
            {
                ssize_t w = write(STDOUT_FILENO, line.c_str(), line.size());
                if (w == -1)
                    KMError("Failed to write to stdout: " + std::string(strerror(errno)));
            }
        }
        outputBuffer.erase(0, pos + 1);
    }
}

/**
 * @brief 尝试从给定的输出行中解析退出码
 * @param line      要检查的输出行（建议传入已移除 ANSI 控制符的字符串）
 * @param exitflag  匹配退出码的标志字符串，例如 "kaiming application exit code: "
 * @param code      [输出] 若成功解析则填充为对应的退出码
 * @return true     找到退出码并成功解析为整数
 * @return false    未找到标志或解析失败
 */
bool KMUnixSocketServer::extractExitCode(const std::string &line, const std::string &exitflag, std::string &code)
{
    auto pos = line.find(exitflag);
    if (pos == std::string::npos)
        return false;

    // 提取退出码部分
    std::string num = line.substr(pos + exitflag.size());

    //去掉首尾空白字符和 \r
    trim(num);
    //num = std::regex_replace(num, std::regex(R"(^\s+|\s+$)"), "");
    code = num;
    return true;
}

/**
 * @brief 检查文件描述符是否合法（非负且有效）
 * @param fd 要检查的文件描述符
 * @return true 表示合法，false 表示无效
 */
bool KMUnixSocketServer::validateFD(int fd)
{
    if (fd < 0 || fcntl(fd, F_GETFD) == -1)
    {
        KMDebug("[SERVER] Invalid file descriptor received.");
        return false;
    }
    return true;
}

/**
 * @brief 设置终端为原始模式（raw mode），用于标准输入
 * @param ori 保存原始终端设置，用于后续恢复
 */
void KMUnixSocketServer::setupTerminal(struct termios &ori)
{
    tcgetattr(STDIN_FILENO, &ori);
    struct termios raw = ori;
    cfmakeraw(&raw);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/**
 * @brief 恢复终端原始设置（调用前保存过）
 * @param ori 原始终端设置结构
 */
void KMUnixSocketServer::restoreTerminal(const struct termios &ori)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &ori);
}

/**
 * @brief 检查终端窗口尺寸是否变化，如有变化则更新从端 pty 的窗口设置
 * @param fd 从端 pty 文件描述符
 * @param last_ws 上次记录的窗口尺寸（用于比较）
 */
void KMUnixSocketServer::updateWindowSizeIfChanged(int ptyfd, struct winsize &last_ws)
{
    struct winsize current_ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &current_ws) == 0)
    {
        if (current_ws.ws_col != last_ws.ws_col || current_ws.ws_row != last_ws.ws_row)
        {
            ioctl(ptyfd, TIOCSWINSZ, &current_ws);
            last_ws = current_ws;
        }
    }
}

/**
 * @brief 去除字符串首尾空白字符（空格、制表符、换行符等）
 * @param s 待处理的字符串
 */
void KMUnixSocketServer::trim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](unsigned char ch) {
                                        return !std::isspace(ch);
                                    }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) {
                             return !std::isspace(ch);
                         })
                .base(),
            s.end());
}
