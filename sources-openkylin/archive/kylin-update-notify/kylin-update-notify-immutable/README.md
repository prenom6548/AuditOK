# kylin-background-upgrade

## 主要功能：

> 由后台定时器定时调起，访问源服务器检测是否有强制更新与可选更新
>
> 检测到强制更新内容在后台升级，托盘不显示
>
> 如果控制面板的自动检测更新关闭，则后台托盘程序不提醒用户更新
>
> 检测到可选更新弹出交互界面
>
> 点击同意更新按钮，调到控制面板更新页面，托盘程序退出
>
> 点击暂不更新按钮，退出托盘程序

## 注意事项：

> 如果托盘应该出现的情况下没出现，请查看是否收纳。在控制面板>个性化>显示在托盘上的图标中，把更新管理器托盘程序改成显示。
>
> 如果托盘打印信息为：“源管理器updateSource is failed，连接失败,程序退出”，请查看源管理器是否启动，如已启动，请执行: sudo pkill -f start_dbus sudo 
>
> killall kylin-software-properties-service sudo kylin-software-properties-service 后，再启动托盘程序。
>
> 静默升级时，会在命令行打印信息
>
> 右下角提示，在不进行其他操作时，不退出
>

## 启动定时器：

>  定时器与托盘服务名称： kylin-background-upgrade-timer.timer kylin-background-upgrade.service 
>
> 查看定时器是否工作： systemctl --user status  kylin-background-upgrade-timer.timer


