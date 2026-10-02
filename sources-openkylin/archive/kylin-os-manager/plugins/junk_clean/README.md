# 垃圾清理

## 清理项介绍

* 系统垃圾

    * 安装包缓存

        ```
        清理路径：
        - /var/cache/apt/archives/ (目录)

        扫描和清理该路径下 ".deb" 结尾的文件
        ```

    * 日志文件

        ```
        清理路径：
        - ~/.log/ (目录)

        扫描和清理该路径下的文件
        ```

    * QQ

        ```
        清理路径：
        - ~/.config/QQ/nt_qq_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/nt_data/Emoji/marketface (目录)

        扫描和删除该目录
        ```

    * 卸载残留

        ```
        扫描执行命令: dpkg -l | grep '^rc' | awk '{print $2}'
        清理执行命令: dpkg -P 扫描出来的包名
        ```

    * 系统缓存

        ```
        不扫描
        清理执行命令: echo 3 > /proc/sys/vm/drop_caches
        ```

    * 缩略图缓存

        ```
        扫描路径：
        - ~/.cache/thumbnails (目录)

        扫描和清理该路径下的所有文件
        ```

    * 微信

        ```
        清理路径：
        Universal 版本微信
        - ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/cache (目录)
        - ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/temp (目录)
        - ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/msg/file/ (目录)
        - ~/文档/xwechat_files/wxid_xxxxxxxxxxxxxx_xxxx/msg/video/ (目录)
        扫描和删除上述四个目录

        网页版本微信
        - ~/.conf/weixin/wechat/users/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/message (目录)
        扫描和删除该目录

        企业微信
        - ~/文档/WXWork/xxxxxxxxxxxxxxxx/Cache (目录)
        扫描和删除该目录

        kmre 版本微信
        ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Cache/
        ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Temp
        ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Sns/Cache/
        ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/File
        ~/.cxoffice/wechat/dosdevices/c:/users/crossover/My Documents/WeChat Files/wxid_9228272283014/FileStorage/Video
        扫描和删除上述五个目录
        ```

* 上网垃圾

    * 齐安信浏览器

        ```
        清理路径:
        -  ~/.cache/qaxbrowser/Default (目录)

        清理时直接删除选中的该路径下的目录
        ```

    * 齐安信浏览器 cookies 信息

        ```
        清理路径：
        - ~/.config/qaxbrowser/Default/Cookies (文件)

        清理时直接删除该文件
        ```


* 使用痕迹

    * 文件记录

        ```
        清理路径:
        - ~/.loacl/share/recently-used.xbel (文件)

        该文件是一个 xml 格式的文件，用于记录用户在桌面环境中最近使用的文件和应用程序。包含了最近打开、编辑或浏览文件的列表，以及最近启动过的应用程序
        ```

    * 命令行历史记录

        ```
        清理路径:
        - ~/.bash_history (文件)

        清理时直接删除该文件
        ```
