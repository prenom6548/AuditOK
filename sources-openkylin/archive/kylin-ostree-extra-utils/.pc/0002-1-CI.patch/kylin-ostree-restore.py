import gi
import traceback
import os,subprocess
gi.require_version('OSTree', '1.0')
from gi.repository import GLib, Gio, OSTree,GObject


def is_restore():
    with open('/proc/cmdline') as f:
        content = f.read()
        if "ostree-factory-restore" in content:
            print("restore")
            return True
    print("no restore")
    return False

def main():
    # 在这里编写主进程的代码
    print("这是主进程")
    ##先判断是不是恢复出厂模式
    if is_restore() == True:  ##是恢复出厂设置
        ##fix mount
        args = ["mount", "-o", "remount,rw",'/sysroot/ostree/deploy/openkylin/var']
        subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True) ##删除非pin、非hotfix部署
        
        sysroot = OSTree.Sysroot.new(None)
        sysroot.set_mount_namespace_in_use()
        sysroot.initialize()
        sysroot.load()
        old_deployments = sysroot.get_deployments()
        # 部署的列表
        for i in range(len(old_deployments)-1,-1,-1):
            deployment = old_deployments[i]
            index = deployment.get_index()
            # 判断当前部署不是pin，则删除该部署
            if deployment.is_pinned() != True:
                
                print(f"NO-PIN-index=",index) 
                args = ["ostree", "admin", "undeploy",str(index)]
                subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True) ##删除非pin、非hotfix部署
                print(f"Successfully undeployed")  
            ##删除分支后实际列表已变，但old_deployments未变，所以取消pin、创建新的pin需要重新reload

        ##重新load一次
        sysroot = OSTree.Sysroot.new(None)
        sysroot.set_mount_namespace_in_use()
        sysroot.initialize()
        sysroot.load()
        deployment_list = sysroot.get_deployments()
        for deployment in deployment_list:
            # 判断当前部署是pin，则取消该部署pin并重新部署该分支
            if deployment.is_pinned() == True:
                sysroot.deployment_set_pinned(deployment,False)
                osname = deployment.get_osname()
                bootcsum = deployment.get_csum()
                origin = deployment.get_origin()
                ret,my_deployment = sysroot.deploy_tree(osname,bootcsum,origin,deployment,None,None)
                #print(f"Successfully tree")
                sysroot.simple_write_deployment(osname,my_deployment,None,OSTree.SysrootSimpleWriteDeploymentFlags.RETAIN,None)
                # print(f"Successfully write")
                sysroot.deployment_set_pinned(my_deployment,True)
                subprocess.run(["/bin/plymouth display-message","--text='系统还原完成.....'"], stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
                print(f"Successfully pin")
                break
        ###更新grub
        subprocess.run(["update-grub"], stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)

if __name__ == "__main__":
    main()