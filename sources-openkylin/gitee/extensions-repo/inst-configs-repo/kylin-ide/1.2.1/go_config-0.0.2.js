const { exec, spawnSync, spawn, execSync } = require('child_process');
const os = require('os');
const fs = require('fs');
//go工具安装
function GoToolsInstall(archName, goVersion, goPath, systemtype,controller) {
    return new Promise(async (resolve, reject) => {
        let installToolsMessage = '';
        let codeProcess = 0
        if (archName.indexOf('longarch64') === -1) {
            installToolsMessage = "正在" + archName + "架构," + systemtype + "系统类型上安装go版本为" + goVersion + "的工具:" + '\n';
            switch (goVersion) {
                case '1.13':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")
                    try {
                      await gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code;
                                return dlv160Install(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code;
                                    return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                        installToolsMessage += results + '\n'
                                        codeProcess += code
                                    }).catch((error) => {
                                        reject(error);
                                    });
                                })
                            })
                            .catch((error) => {
                                reject(error);
                            });
                       
                        resolve([installToolsMessage, codeProcess])

                    } catch (error) {
                        console.log("Go工具安裝失败！")
                    }

                    return
                case '1.14':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")
                    try {

                        await gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code;
                                return dlv161Install(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code;
                                    return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                        installToolsMessage += results + '\n'
                                        codeProcess += code
                                    }).catch((error) => {
                                        reject(error);
                                    });
                                })
                            })
                            .catch((error) => {
                                reject(error);
                            });

                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        console.log("Go工具安裝失败:", error);
                        reject(error);
                    }
                    return
                case '1.15':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")
                    try {
                        await gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code;
                                return dlv161Install(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code;
                                    return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                        installToolsMessage += results + '\n'
                                        codeProcess += code
                                    }).catch((error) => {
                                        reject(error);
                                    });
                                })
                            })
                            .catch((error) => {
                                reject(error);
                            });

                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        console.log("Go工具安裝失败:", error)
                    }
                    return
                default:
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go工具:")
                    installToolsMessage = "正在" + archName + "架构," + systemtype + "系统类型上安装go工具:" + '\n';
                    try {
                        await goplsInstall(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code;
                                return dlvInstall(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code;
                                    return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                        installToolsMessage += results + '\n'
                                        codeProcess += code
                                    }).catch((error) => {
                                        reject(error);
                                    });
                                })
                            })
                            .catch((error) => {
                                reject(error);
                            });
                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        console.log("Go工具安裝失败:", error)
                        reject(error);
                    }
            }
        } else {
            installToolsMessage = "正在" + archName + "架构," + systemtype + "系统类型上安装go版本为" + goVersion + "的工具:" + '\n';
            switch (goVersion) {
                case '1.15':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")
                    try {
                        await  gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code
                                return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code
                                }).catch((error) => {
                                    reject(error);
                                });
                            })
                            .catch((error) => {
                                reject(error);
                            });
                       
                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        reject(error);
                    }

                    return
                case '1.14':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")
                    try {
                        await  gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code
                                return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code
                                }).catch((error) => {
                                    reject(error);
                                });
                            })
                            .catch((error) => {
                                console.error(error);
                                reject(error);
                            });
                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        console.log("Go工具安裝失败:", error);
                        reject(error);
                    }
                    return
                case '1.13':
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go版本为", goVersion, "的工具:")

                    try {
                        await  gopls095Install(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code
                                return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code
                                }).catch((error) => {
                                    reject(error);
                                });
                            })
                            .catch((error) => {
                                console.error(error);
                                reject(error);
                            });
                    } catch (error) {
                        console.log("Go工具安裝失败:", error)
                        reject(error);
                    }

                    return
                default:
                    console.log("正在", archName, "架构,", systemtype, "系统类型上安装go工具:")
                    installToolsMessage = "正在" + archName + "架构," + systemtype + "系统类型上安装go工具" + '\n';
                    try {
                        await goplsInstall(goPath,controller)
                            .then(([results, code,signal]) => {
                                installToolsMessage += results + '\n'
                                codeProcess += code;
                                return goOutLineInstall(goPath,controller). then(([results, code,signal]) => {
                                    installToolsMessage += results + '\n'
                                    codeProcess += code
                                }).catch((error) => {
                                    reject(error);
                                });
                            })
                            .catch((error) => {
                                console.error(error);
                                reject(error);
                            });
                        resolve([installToolsMessage, codeProcess])
                    } catch (error) {
                        console.log("Go工具安裝失败:", error);
                        reject(error);
                    }

            }

        }
    })


}
//判断go路径是否存在
function executableFileExists(filePath) {
    let exists = true;
    try {
        exists = fs.statSync(filePath).isFile();
        if (exists) {
            fs.accessSync(filePath, fs.constants.F_OK | fs.constants.X_OK);
        }
    } catch (e) {
        exists = false;
    }
    return exists;
}
//下载gopls@0.9.5
function gopls095Install(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "golang.org/x/tools/gopls@v0.9.5"],{detached:true,shell:true});
        ls.stderr.on('data', (data) => { console.error(`${data}`); });
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });
        ls.on('close', (code,signal) => {
            console.log(`子进程退出码：${code}`);
            controller.signal.removeEventListener('abort', cancelTask);
            if (code === 0) {
                console.log("gopls@v0.9.5安装成功");
                resolve(["gopls@v0.9.5安装成功", code,signal]);

            } else {
                console.log("gopls@v0.9.5安装失败");
                reject(["gopls@v0.9.5安装失败", code,signal]);
            }

        });
        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })
}
//下载gopls
function goplsInstall(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "golang.org/x/tools/gopls"],{detached:true,shell:true});
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });

        ls.stderr.on('data', (data) => { console.error(`${data}`); });

        ls.on('close', (code,signal) => {
            console.log(`子进程退出码：${code}`);
            controller.signal.removeEventListener('abort', cancelTask);
            if (code === 0) {
                console.log("gopls安装成功");
                resolve(["gopls安装成功", code,signal]);
            }else {
                console.log("gopls安装失败");
                reject(["gopls安装失败", code,signal]);
            }
        });
        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })

}
//下载dlv@1.6.0
function dlv160Install(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "github.com/go-delve/delve/cmd/dlv@v1.6.0"],{detached:true,shell:true});
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });

        ls.stderr.on('data', (data) => { console.error(`${data}`); });

        ls.on('close', (code,signal) => {
            console.log(`子进程退出码：${code}`);
            controller.signal.removeEventListener('abort', cancelTask);
            if (code === 0) {
                console.log("dlv@v1.6.0安装成功");
                resolve(["dlv@v1.6.0安装成功", code,signal]);
            } else {
                console.log("dlv@v1.6.0安装失败");
                reject(["dlv@v1.6.0安装失败", code,signal]);
            }
        });
        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })

}
//下载dlv@1.6.1
function dlv161Install(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "github.com/go-delve/delve/cmd/dlv@v1.6.1"],{detached:true,shell:true});
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });

        ls.stderr.on('data', (data) => { console.error(`${data}`); });

        ls.on('close', (code,signal) => {
            console.log(`子进程退出码：${code}`);
            controller.signal.removeEventListener('abort', cancelTask);
            if (code === 0) {
                console.log("dlv@v1.6.1安装成功");
                resolve(["dlv@v1.6.1安装成功", code,signal])
            }else {
                console.log("dlv@v1.6.1安装失败");
                reject(["dlv@v1.6.1安装失败", code,signal])
            }
        });
        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })

}
//下载dlv
function dlvInstall(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "github.com/go-delve/delve/cmd/dlv"],{detached:true,shell:true});
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });

        ls.stderr.on('data', (data) => { console.error(`${data}`); });

        ls.on('close', (code,signal) => {
            console.log(`子进程退出码：${code}`);
            controller.signal.removeEventListener('abort', cancelTask);
            if (code === 0) {
                console.log("dlv安装成功");
                resolve(["dlv安装成功", code,signal])
            } else {
                console.log("dlv安装失败");
                reject(["dlv安装失败", code,signal])
            }
        });
        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })
}

//下载go-outline
function goOutLineInstall(goPath,controller) {
    return new Promise((resolve, reject) => {
        const ls = spawn(goPath, ["get", "-v", "github.com/ramya-rao-a/go-outline"],{detached:true,shell:true});
        ls.stdout.on('data', (data) => { console.log(`stdout: ${data}`); });

        ls.stderr.on('data', (data) => { console.error(`${data}`); });

        ls.on('close', (code,signal) => {
            controller.signal.removeEventListener('abort', cancelTask);
            console.log(`子进程退出码：${code}`);
            if (code === 0) {
                console.log("go-outline安装成功");
                resolve(["go-outline安装成功", code,signal])
            }  else {
                console.log("go-outline安装失败");
                reject(["go-outline安装失败", code,signal])
            }
        });

        const cancelTask=()=>{
            if(ls.pid){
                execSync(`kill -9  -${ls.pid}`);
            }
        }
        controller.signal.addEventListener('abort', cancelTask);
    })
}
//获取go路径以及版本号
function getGoPath() {
    const p = "";
    const defaultPathsForGo =
        process.platform === 'win32'
            ? ['C:\\Program Files\\Go\\bin\\go.exe', 'C:\\Program Files (x86)\\Go\\bin\\go.exe']
            : ['/usr/bin/go', '/bin/go', '/usr/local/go/bin/go', '/usr/local/bin/go', '/usr/lib/go-1.15/bin/go', '/usr/lib/go-1.14/bin/go', '/usr/lib/go-1.13/bin/go'];
    for (const p of defaultPathsForGo) {
        if (executableFileExists(p)) {

            return p;
        }

    }
    return p;
}
//设置go代理
function setting(goPath) {
    process.env.GO111MODULE = "on";
    process.env.GOPROXY = "https://goproxy.cn,direct";
    exec(`${goPath} env -w GO111MODULE=on  && ${goPath} env -w GOPROXY=https://goproxy.cn,direct`, (error, stdout, stderr) => {

        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
        //   console.log(`stdout: ${stdout}`);
        console.log("Go代理设置成功!")
        //   console.log(`stderr: ${stderr}`);  
    })

}
//检查系统类型
function checkSystemType() {
    let cmdResult = spawnSync('ls', ['/usr/bin/dpkg']);
    if (cmdResult.stdout !== null) {
        let libcResult = spawnSync('dpkg-query', ['-W', 'libc6']);

        if (libcResult.stdout !== null) {
            if (libcResult.stdout.toString() !== '') {
                return 'deb';
            }

        }
    }
    cmdResult = spawnSync('ls', ['/usr/bin/rpm']);
    if (cmdResult.stdout !== null) {
        let libcResult = spawnSync('rpm', ['-qa', 'glibc']);
        if (libcResult.stdout !== null) {
            if (libcResult.stdout.toString() !== '') {
                return 'rpm';
            }

        }
    }
    return 'unknown';

}


async function main(controller) {
    return new Promise((resolve, reject) => {
        let goVersion = "";
        let goPath = "";
        const archName = os.arch();
        goPath = getGoPath();
        if (goPath === '') {
            resolve("go路径找不到");
            return;
        }
        if (goPath.indexOf('go-1.15') !== -1) {
            goVersion = "1.15";
        } else if (goPath.indexOf('go-1.14') !== -1) {
            goVersion = "1.14";
        } else if (goPath.indexOf('go-1.13') !== -1) {
            goVersion = "1.13";
        } else {

            let cmdResult = spawnSync('go', ['version']);
            if (cmdResult && cmdResult.stdout !== null && cmdResult.stdout.toString().indexOf("1.13") !== -1) {
                goVersion = '1.13';
            }
        }
        const systemtype = checkSystemType();


        let p = new Promise((resolve, reject) => {
            setting(goPath)
            setTimeout(() => {
                resolve();
            }, 1000);

            // reject("dddd");
        })
        p.then(async (value) => {
            if (goPath !== "") {
                GoToolsInstall(archName, goVersion, goPath, systemtype,controller)
                    .then(([results, code]) => {
                        resolve([results, code])
                    }).catch((error) => {
                        reject(error);
                     });
            } else {
                reject("找不到go路径")
            }


        })
    })

}

module.exports = {
    main
}
