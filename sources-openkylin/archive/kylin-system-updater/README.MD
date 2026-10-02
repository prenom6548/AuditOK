### Software Updater for apt

- 目录架构：

  ```shell
  backend  debian  Makefile  plugin  README.MD
  ```

- 其中分为控制面板插件目录`plugin` and 更新后端目录`backend`，两个模块相互隔离，公用一个包来安装

- GitLab分支介绍

  - backend_dev:后端更新代码维护的分支
  - plugin_dev:控制面板插件维护的分支
  - dev:分支只要负责将后端更新代码和插件的代码进行合并编包测试的分支，最新的代码在此分支上
  - master:负责最终出版本的分支，dev上验证成功后，将代码合并到master上进行编包



- 安装依赖

  ```
  sudo apt install dh-python python3-all python3-distutils-extra gir1.2-snapd-1 apt-clone intltool at-spi2-core -y
  ```

- 



### 后端服务：

- 后端服务主要负责更新、安装、升级等等各种安装和下载的过程处理

- 查看后端日志：`tail -f /var/log/kylin-system-updater/kylin-system-updater.log.1`

- 调试后端代码：

  - 进入backend目录下直接运行`kylin-system-updater`

  - 调试参数

    ```shell
    -d 日志直接输出到终端，不输出到log文件中
    -n 不更新摸板不刷新source.list and important 列表
    -r 替换已经运行的后端程序
    -c 关闭源过滤等各种过滤代码
    
    ```


### 配置文件

- 名称：`system-updater.conf`

- 路径：`/var/lib/kylin-system-updater`

- 配置项：

  ```shell
  #自动更新使用
  [AutoUpgrade]
  #升级列表，自动更新使用
  upgradelist = 
  
  #系统状态
  [SystemStatus]
  #标志是否异常强制关闭
  isabnormalreboot = False
  ```

  

### 文档

#### Aptdaemon

- https://pythonhosted.org/aptdaemon/aptdaemon.client.html?highlight=commit_packages#aptdaemon.client.AptClient.commit_packages

#### python-apt

- https://apt-team.pages.debiahn.net/python-apt/library/apt_pkg.html

#### 方法与信号接口文档

- 参考interface.md 文档

