/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef UNIX_SOCKET_SERVER_H
#define UNIX_SOCKET_SERVER_H

#include <string>
#include <atomic>
#include <future>

class KMUnixSocketServer
{
public:
    KMUnixSocketServer();
    explicit KMUnixSocketServer(const std::string &path);
    ~KMUnixSocketServer();

    /**
     * @brief 创建 socket 文件并绑定到指定路径
     * @return 成功 true，失败 false
     */
    bool create();

    /**
     * @brief 启动 socket 监听并处理客户端连接
     * @return 成功 true，失败 false
     */
    bool run();

    /**
     * @brief 等待 socket 服务器完成处理
     * @return 成功 true，失败 false
     */
    bool wait();

    /**
     * @brief 关闭 socket 并释放资源
     */
    void release();

    /**
     * @brief 停止
     * @return 成功 true，失败 false
     */
    bool stop();

    /**
     * @brief 判断 socket 是否已成功创建
     * @return 成功 true，失败 false
     */
    bool isCreate() const;

    /**
     * @brief 获取当前 socket 路径
     * @return 获取 socket 路径
     */
    std::string getPath() const;

private:
    /**
     * @brief 开始接收文件描述符
     * @return 成功 true,失败 false
     */
    bool start();

    /**
     * @brief 接收文件描述符
     * @param socket，表示用于接收的 socket 文件描述符
     * @return 成功返回接收到的文件描述符，失败返回 -1
     */
    int receivefd(int socket);

    /**
     * @brief 处理单个连接
     * @param connfd，连接的 socket 文件描述符
     * @note 此函数在连接建立后调用，用于执行具体的处理逻辑
     * @return 成功 true，失败 false
     */
    bool handleConnection(int connfd);

    /**
     * @brief 生成一个随机的 socket 路径
     * @return 返回生成的 socket 路径字符串
     */
    std::string generateRandomPath();

    /**
     * @brief 从本地终端读取窗口大小，并同步设置到 slave 端的 pty。
     * @param masterfd 主端的 pseudo terminal 文件描述符
     * @param slavefd 从端的 pseudo terminal 文件描述符
     */
    void syncWindowSize(int masterd, int slavefd);

    /**
     * @brief 设置终端为原始模式（raw mode），用于标准输入
     * @param ori 保存原始终端设置，用于后续恢复
     */
    void setupTerminal(struct termios &ori);

    /**
     * @brief 恢复终端原始设置（调用前保存过）
     * @param ori 原始终端设置结构
     */
    void restoreTerminal(const struct termios &ori);

    /**
     * @brief 检查文件描述符是否合法（非负且有效）
     * @param fd 要检查的文件描述符
     * @return true 表示合法，false 表示无效
     */
    bool validateFD(int fd);

    /**
     * @brief 去除 ANSI 控制序列（如颜色、光标移动等）
     * @param input 原始文本
     * @return 移除控制序列后的纯文本
     */
    std::string removeAnsi(const std::string &input);

    /**
     * @brief 处理 pty 输出，根据是否为交互终端进行不同方式处理（写终端 or 缓存处理）
     * @param buffer 当前从 pty 读出的原始数据指针
     * @param n 实际读出的字节数
     * @param isInteractiveShell 是否处于交互终端环境（tty）
     * @param outputBuffer 用于非交互模式下的行缓冲输出（按行）
     * @param lastChunk 上一轮读取的数据块（用于与当前块拼接找 exit code）
     * @param currChunk 当前读取到的数据块（用于与上一块拼接）
     * @param exitflag  匹配退出码的标志字符串，例如 "kaiming application exit code: "
     */
    void processPtyOutput(char *buffer, ssize_t n, bool isInteractiveShell, std::string &outputBuffer, std::string &lastChunk, std::string &currChunk, std::string exitflag);

    /**
     * @brief 尝试从给定的输出行中解析退出码
     * @param line      要检查的输出行（建议传入已移除 ANSI 控制符的字符串）
     * @param exitflag  匹配退出码的标志字符串，例如 "kaiming application exit code: "
     * @param code      [输出] 若成功解析则填充为对应的退出码
     * @return true     找到退出码并成功解析为整数
     * @return false    未找到标志或解析失败
     */
    bool extractExitCode(const std::string &line, const std::string &exitflag, std::string &code);

    /**
     * @brief 检查终端窗口尺寸是否变化，如有变化则更新从端 pty 的窗口设置
     * @param fd 从端 pty 文件描述符
     * @param last_ws 上次记录的窗口尺寸（用于比较）
     */
    void updateWindowSizeIfChanged(int fd, struct winsize &last_ws);

    /**
     * @brief 去除字符串首尾空白字符（空格、制表符、换行符等）
     * @param s 待处理的字符串
     */
    static void trim(std::string& s);

private:
    int m_serverfd;
    int m_efd; // 用于事件通知的文件描述符
    bool m_isCreated;
    bool m_isVirtualTty;
    std::string m_socketPath;
    std::future<bool> m_fut;

public:
    std::string exitCode;
    std::string errorInfo;
};

#endif // UNIX_SOCKET_SERVER_H
