const { execSync, spawn } = require('child_process');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const os = require('os');

function executeCommand(command) {
    try {
        const output = execSync(command, { encoding: 'utf-8' });
        return output;
    } catch (error) {
        throw new Error(`Error executing command: ${error.message}`);
    }
}

class KylingToolStoreApiProvider {
    apiUrl = "http://toolhub.kylinos.cn:8000";
    tool_download_url = "/tool_download_url";
    tool_latest_version_url = "/tool_latest_version";
    tool_name = 'KCoder';
    archString = '';
    TOKEN = "re*wm0ff@kfw=a8l&ii";
    controller;
    downloadPath = '';
    cancelflag = false;

    constructor(controller) {
        this.controller = controller
        console.log('apiurl:', this.apiUrl);
        console.log('arch:', process.arch);
        this.archString = this.getArchString();
        this.controller.signal.addEventListener("abort", () => {
            this.cancelflag = true;
        })
    }

    getArchString() {
        const osArch = process.arch;
        switch (osArch) {
            case "x86":
            case "x64":
            case "x86_64":
                return "x86_64";
            case "arm64":
            case "aarch64":
                return "arm64";
            case "mips64":
                return "mips64";
            case "loong64":
            case "longarch64":
                return "longarch64";
            case "alpha":
                return "alpha";
            default:
                return "x86_64";
        }
    }
    checkDir(dir) {
        if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir, { recursive: true }); // 递归创建目录
        }
    }
    spawnCommand(command) {
        return new Promise((resolve, reject) => {
            const child = spawn('sh', ['-c', command], { detached: true });

            let stdout = '';
            let stderr = '';

            child.stdout.on('data', (data) => {
                stdout += data.toString();
            });

            child.stderr.on('data', (data) => {
                stderr += data.toString();
            });

            child.on('close', (code) => {
                clearTimeout(timer);
                this.controller.signal.removeEventListener('abort', cancelTask);
                if (code === 0) {
                    resolve({ stdout, stderr });
                } else {
                    reject(new Error(`Command failed with exit code ${code} `));
                }
            });
            const timer = setTimeout(() => {
                reject(new Error('命令执行超时'));
                if (child && child.pid) {
                    execSync(`kill -9 -${child.pid}`);
                }
            }, 1200000);
            const cancelTask = () => {
                if (child.pid) {
                    reject(new Error('abort'))
                    execSync(`kill -9 -${child.pid}`);
                }

            }
            this.controller.signal.addEventListener('abort', cancelTask);
        });
    }
    async getNewestVersion() {
        return await this.getLatestVersion();
    }

    async installLatestExtension() {
        console.log("installLatestExtension");
        try {
            const vsixPath = await this.downloadFile();
            return this.installNewVersion(vsixPath);
        } catch {
            return false;
        }
    }

    // 安装新版本
    async installNewVersion(vsixPath) {
        console.log('[update] start installNewVersion:');
        try {
            // 安装 VSIX 包
            if (this.cancelflag) {
                return false;
            }
            const command = `kylin-ide --install-extension ${vsixPath}`
            const res = await this.spawnCommand(command);
            console.log('installNewVersion', res);
            return true;
        } catch (error) {
            console.log('[update] Failed to installNewVersion.Case:' + JSON.stringify(error));
            return false;
        }

    }

    async getLatestVersion() {
        console.log("[update] getLatestVersion");
        const paramsObj = {
            tool_name: this.tool_name,
            token: this.TOKEN
        };

        const searchParams = new URLSearchParams(paramsObj);
        const url = this.apiUrl + this.tool_latest_version_url;
        const options = {
            method: 'GET',
            headers: {
                'accept': 'application/json',
            }
        };
        console.log(`${url}?${searchParams}`);
        try {
            process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";
            const response = await fetch(`${url}?${searchParams}`, options);
            delete process.env.NODE_TLS_REJECT_UNAUTHORIZED;
            if (!response.ok) {
                throw new Error(`Failed to fetch latest version: ${response.statusText}`);
            }

            const responseJson = await response.json();
            if (responseJson.code !== 200) {
                throw new Error(`Request success, but get latest version failed. Response:\n ${responseJson.message}`);
            }
            if (responseJson.code == 200 && responseJson.data == undefined) {
                throw new Error(`Request success, but get latest version failed. data is undefined`);
            }

            let newestVersion = ''; // 获取最新版本号
            Object.keys(responseJson.data).forEach(arch => {
                if (arch === this.archString || arch.includes(this.archString)) {
                    if (responseJson.data[arch].toLowerCase().startsWith('v')) {
                        newestVersion = responseJson.data[arch].toLowerCase().replace('v', '');
                    } else {
                        newestVersion = responseJson.data[arch].toLowerCase();
                    }
                }
            });

            const updateInfo = {
                version: newestVersion
            }
            return updateInfo;
        } catch (error) {
            if (error instanceof Error) {
                throw new Error(`${error.message}`);
            } else {
                console.log(`[update] Error fetching latest version. Response:\n ${JSON.stringify(error)}`);
                throw new Error(`Error fetching latest version`);
            }
        }
    }

    async getDownloadURL() {
        console.log("[update] getDownloadURL");
        const paramsObj = {
            tool_name: this.tool_name,
            token: this.TOKEN,
            //   arch: this.archString,
        };
        let options = {
            headers: {
                'accept': 'application/json',
            }
        }
        const searchParams = new URLSearchParams(paramsObj);
        const url = this.apiUrl + this.tool_download_url;
        try {
            process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";
            const response = await fetch(`${url}?${searchParams}`, options);
            delete process.env.NODE_TLS_REJECT_UNAUTHORIZED;
            if (!response.ok) {
                console.log("Failed to fetch downloadURL");
                throw new Error(`Failed to fetch downloadURL: ${response.statusText}`);
            }

            const responseJson = await response.json();
            if (responseJson.code !== 200) {
                throw new Error(`Request success, but get downloadURL failed. Response:\n ${responseJson.message}`);
            }
            let downloadUrl = responseJson.data.url;
            let md5 = responseJson.data.md5;

            return { url: downloadUrl, md5: md5 };
        } catch (error) {
            if (error instanceof Error) {
                console.log(`[update] : ${JSON.stringify(error)}`);
                throw new Error(`${error.message}`);
            } else {
                console.log(`[update] Error fetching downloadURL. Response:\n ${JSON.stringify(error)}`);
                throw new Error(`Error fetching downloadURL`);
            }
        }
    }

    async downloadFile() {
        console.log("[update] downloadFile");
        let newFileName = '';
        let downloadPath = ""
        let downloadFilePath = '';
        let fileStream;
        let downloadUrl = '';
        let expectedMd5 = '';
        try {
            this.downloadPath = path.join(os.homedir(), "./.config/Kylin-IDE/installconfig/ai");
            this.checkDir(this.downloadPath);
            downloadPath = this.downloadPath;
            const { url, md5 } = await this.getDownloadURL();// 此处将获取的下载地址的错误也同时抛出到最后的catch中
            downloadUrl = url;
            expectedMd5 = md5;
            const options = {
                method: 'GET'
            };
            process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";
            const response = await fetch(downloadUrl, options);
            delete process.env.NODE_TLS_REJECT_UNAUTHORIZED;
            console.log("[update] after download file.");
            if (!response.ok) {
                console.log('[update] Failed to fetch download file.');
                throw new Error(`Failed to fetch download file:${response.statusText}`);
            }
            const contentDisposition = response.headers.get('content-disposition');
            if (contentDisposition) {
                const matches = /filename[^;=\n]*=((['"]).*?\2|[^;\n]*)/.exec(contentDisposition);
                if (matches != null && matches[1]) {
                    newFileName = matches[1].replace(/['"]/g, '');
                    console.log("newFileName:304", newFileName);
                }
            }

            downloadFilePath = path.join(downloadPath, newFileName);
            fileStream = fs.createWriteStream(downloadFilePath);
            // 创建md5的hash算法
            const hash = crypto.createHash('md5');
            // 流式下载文件
            const reader = response.body?.getReader();
            const read = () => {
                reader?.read().then(({ done, value }) => {
                    if (done) {
                        fileStream.close();
                        console.log(`Download complete: ${newFileName}`);
                        return;
                    }
                    // 计算md5
                    hash.update(value);
                    // 写入文件流
                    fileStream.write(value);
                    read();
                });
            }
            read();

            // 等待写入完成
            await new Promise((resolve, reject) => {
                fileStream.on('finish', () => resolve(downloadFilePath));
                fileStream.on('error', reject);
            });

            // 获取计算出的 MD5 哈希值
            const actualMd5 = hash.digest('hex');

            // 校验 MD5 哈希值
            if (actualMd5 !== expectedMd5) {
                throw new Error('File checksum failure, MD5 hash mismatch');
            }
            console.log("update", downloadFilePath);
            return downloadFilePath;
        } catch (error) {
            throw error;
        }

    }
}

class UpdateExtension {
    apiProvider;
    controller;
    cancelflag = false;
    constructor(controller) {
        this.controller = controller;
        this.apiProvider = new KylingToolStoreApiProvider(this.controller);
        this.controller.signal.addEventListener("abort", () => {
            this.cancelflag = true;
            return false;
        })
    }
    getCurrentExtensionVersion(extensionid) {
        let version;
        const command = `kylin-ide --list-extensions --show-versions |grep ^${extensionid}@`;
        try {
            const result = executeCommand(command);
            const lines = result.split('\n');
            lines.forEach(line => {
                const parts = line.split('@');
                if (parts.length === 2) {
                    if (parts[0] === extensionid)
                        version = parts[1];
                }
            });
            return version;
        } catch {
            return undefined;
        }
    }

    /**
     * @description: 比较当前版本和最新版本
     * @param currentVersion 当前版本
     * @param newestVersion 最新版本
     * @returns 1: 当前版本大于最新版本 0: 当前版本等于最新版本 -1: 当前版本小于最新版本
     */
    compareExtensionVersion(currentVersion, newestVersion) {
        let versionObj = new Versions();
        return versionObj.compare(versionObj.fromString(currentVersion), versionObj.fromString(newestVersion));
    }

    async start() {
        console.log('[update] start');
        // 获取当前扩展版本
        const currentVersion = this.getCurrentExtensionVersion("KylinIdeTeam.kcoder");
        // 获取最新版本
        let latestVersion = undefined;
        let result = {
            version: '',
            chesumDownloadUrl: '',
            packageDownloadUrl: ''
        };
        try {
            // 根据provider请求获取最新版本
            result = await this.apiProvider.getNewestVersion();
            if (result && result.version)
                latestVersion = result.version;
        } catch (error) {
            console.log('[update] getlatestVersion failed.Case:' + error);
            latestVersion = undefined;
            if (error.includes("aborted")) {
                return false;
            }
        }
        if (latestVersion != undefined && currentVersion != undefined) {
            console.log('[update] getlatestVersion:', latestVersion)
            if (this.compareExtensionVersion(currentVersion, latestVersion) >= 0) {
                console.log('[update] no update version.');
                return true;
            }
        }
        return this.downloadAndInstallExtension();
    }
    async downloadAndInstallExtension() {
        console.log('[update] downloadAndInstallExtension');
        try {
            return await this.apiProvider.installLatestExtension();
        } catch (error) {
            console.log('[update] Failed to download and install extension.Case:' + JSON.stringify(error));
            if (error instanceof Error) {
                console.log(`Failed to download and install extension.Cause: ${error.message}`);
            } else {
                console.log(`Failed to download and install extension.`);
            }
            return false;
        }
    }
}



class Versions {
    compare(v1, v2) {
        if (typeof v1 === 'string') {
            v1 = fromString(v1);
        }
        if (typeof v2 === 'string') {
            v2 = fromString(v2);
        }

        if (v1.major > v2.major) { return 1; }
        if (v1.major < v2.major) { return -1; }

        if (v1.minor > v2.minor) { return 1; }
        if (v1.minor < v2.minor) { return -1; }

        if (v1.patch > v2.patch) { return 1; }
        if (v1.patch < v2.patch) { return -1; }

        if (v1.pre === undefined && v2.pre !== undefined) { return 1; }
        if (v1.pre !== undefined && v2.pre === undefined) { return -1; }

        if (v1.pre !== undefined && v2.pre !== undefined) {
            return v1.pre.localeCompare(v2.pre);
        }

        return 0;
    }

    from(major, minor, patch = "0", pre = "0") {
        return {
            major: typeof major === 'string' ? parseInt(major, 10) : major,
            minor: typeof minor === 'string' ? parseInt(minor, 10) : minor,
            patch: patch === undefined || patch === null ? 0 : typeof patch === 'string' ? parseInt(patch, 10) : patch,
            pre: pre,
        };
    }

    fromString(version) {
        const [ver, pre] = version.split('-');
        const [major, minor, patch] = ver.split('.');
        return this.from(major, minor, patch, pre);
    }
}

async function main(controller) {
    return new Promise((resolve, reject) => {
        let UpdateExtensionObject = new UpdateExtension(controller);
        UpdateExtensionObject.start().then(res => {
            if (res == true) {
                console.log("UpdateExtensionObject.start", "success", res);
                resolve(1);
            } else {
                console.log("UpdateExtensionObject.start then", "error");
                reject(0);
            }
        }).catch(err => {
            console.log("UpdateExtensionObject.start", "error");
            reject(0);
        });
    })
}
module.exports = {
    main
}
