const os = require('os');
const fs = require('fs');
const { promises } = require('dns');
var os_arch; //arch x86_64
var os_org;      //Kylin
var os_type;     // Desktop Server
var os_release;// eg:2203
var kylin_release_id;
var os_version_id; // V10
var os_sp;//sp
var os_custom; //hwe
var pretty_name;
var milestone;
var dist_id;
var pretty_name;
var os_info;
const kyinfoPath = '/etc/.kyinfo';
const osReleasePath = '/etc/os-release';
//检查系统架构
function getArch() {
  const arch = os.arch();
  let res = '';
  switch (arch) {
    case 'x64':
    case 'x86':
    case "x86_64": res = 'x86'; break;
    case 'arm':
    case 'arm64':
    case 'aarch64': res = 'arm'; break;
    case 'loongarch': res = 'loongarch'; break;
    default: res = arch;
  }
  return res;
}

function getoskey() {
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2403') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2403";
  }
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2203') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2203";
  }
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2303') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2303";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2203') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2203";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2303') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2303";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2403') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2403";
  }
  return "undefined"
}
function getosInterkey() {
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2403') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2403_inter";
  }
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2203') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2203_inter";
  }
  if (os_arch == 'x86' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2303') && os_version_id == 'v10') {
    return "x86_kylin_desktop_v10_2303_inter";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2203') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2203_inter";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2303') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2303_inter";
  }
  if (os_arch == 'arm' && os_org == 'kylin' && os_type == 'desktop' && os_release.includes('2403') && os_version_id == 'v10') {
    return "arm_kylin_desktop_v10_2403_inter";
  }
  return "undefined"
}

function getFileData() {
  //1、架构
  //2、是否是麒麟系统
  //3、是桌面系统还是服务器系统
  //4、是否是V10
  //5、是否带sp，是sp几
  //6、小版本号
  //7、定制版字符
  os_arch = getArch();
  /*分类型进行处理
  1.存在.kyinfo的系统 ，取值dist_id 中 架构之前的信息
  2.不在.kyinfo的系统，但是存在os-release  的kylin系统
  3.不是kylin的系统，但是存在os-release
  */
  let data;
  let releasedata;
  if (fs.existsSync(kyinfoPath) && fs.existsSync(osReleasePath)) {
    os_org = "kylin";
    try {
      let version_id;
      data = fs.readFileSync(kyinfoPath, 'utf8');
      releasedata = fs.readFileSync(osReleasePath, 'utf8');
      // 使用 split 方法将文件内容分割成行数组
      const lines = data.split('\n');
      // 遍历行数组 获取系统架构
      lines.forEach((line) => {
        line = line.replace(/ /g, "");
        if (line.startsWith("milestone=")) {
          milestone = line.slice(10).toLocaleLowerCase();
        } else if (line.startsWith("dist_id=")) {
          dist_id = line.slice(8).toLocaleLowerCase();
        }
      });//end lines

      const releasedatalines = releasedata.split('\n');
      releasedatalines.forEach((line) => {
        if (line.startsWith("PRETTY_NAME=")) {
          pretty_name = line.slice(12).toLocaleLowerCase().replace(/"/g, '');
        } else if (line.startsWith("VERSION_ID=")) {
          version_id = line.slice(11).toLocaleLowerCase().replace(/"/g, '');
        }
        if (line.startsWith("KYLIN_RELEASE_ID=")) {
          kylin_release_id = line.slice(17).toLocaleLowerCase().replace(/"/g, '');
        }
      });
      if (version_id) {
        os_version_id = version_id;
      }
      os_type = dist_id.includes('desktop') ? "desktop" : "server";
      if (kylin_release_id) {
        os_release = kylin_release_id;
      }

      if (dist_id.includes('edu')) {
        os_custom = 'edu';
      } else {
        os_custom = 'gen';
      }
      if (dist_id.includes('hwe')) {
        os_custom += 'hwe';
      }
      if (dist_id.includes('gfb')) {
        os_custom += 'gfb'
      }
    } catch (err) {
      os_info = "";
      console.log(err);
    }
  } else
    if (fs.existsSync(osReleasePath)) {
      try {
        let version_id;
        releasedata = fs.readFileSync(osReleasePath, 'utf8');
        const releasedatalines = releasedata.split('\n');
        releasedatalines.forEach((line) => {
          if (line.startsWith("PRETTY_NAME=")) {
            pretty_name = line.slice(12).toLocaleLowerCase().replace(/"/g, '');
            pretty_name = pretty_name.replace(/ /g, "_")
          } else if (line.startsWith("VERSION_ID=")) {
            version_id = line.slice(11).toLocaleLowerCase().replace(/"/g, '');
          } else if (line.startsWith("ID=")) {
            os_org = line.slice(3).toLocaleLowerCase().replace(/"/g, '');
          } else if (line.startsWith("VERSION_CODENAME=")) {
            os_custom = line.slice(17).toLocaleLowerCase().replace(/"/g, '');
          } else if (line.startsWith('NAME=')) {
            os_org = line.slice(5).toLocaleLowerCase().replace(/"/g, '')
          }
        });
        if (version_id) {
          os_version_id = version_id;
        }
        if (pretty_name.includes('openkylin'))
          os_type = "desktop";

      } catch (err) {
        os_info = "";
      }
    }

}

async function checkInternet() {
  process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";
  try {
    const response = await fetch('https://toolhub.kylinos.cn:9000');
    if (response.ok) {
      return true;
    } else {
      return false;
    }
  } catch (error) {
    return false;
  }
}
async function getosinfo() {
  let interflag = false;
  let osinfo = "";
  interflag = await checkInternet();
  delete process.env.NODE_TLS_REJECT_UNAUTHORIZED;
  if (interflag == true) {
    osinfo = osinfo + getosInterkey();
  } else {
    osinfo = osinfo + getoskey();
  }
  return osinfo;
}

function showosinfo() {
  let osData = '';
  if (fs.existsSync(kyinfoPath)) {
    if (os_arch) { osData += os_arch; }
    let indexR = dist_id.indexOf("release");
    let info = dist_id.substring(0, indexR - 1);
    info = info.replace(/-?\b\d{4}\b/, '');
    osData += "_" + info + "_" + os_release;
    osData = osData.replace(/^_+|_+$/g, '');
    osData = osData.replace(/-/g, "_");
    osData = osData.replace(/_+$/, '');
    osData = osData + "(根据os-release和.kyinfo信息)";
    return osData;
  }
  else {
    //架构
    if (os_arch) { osData += os_arch; }
    //组织
    if (os_org) { osData += "_" + os_org; }
    //操作系统类型
    if (os_type) { osData += "_" + os_type; }
    if (pretty_name) { osData += "_" + pretty_name; }

    if (os_custom) { osData += "_" + os_custom; }
    osData = osData.replace(/^_+|_+$/g, '');
    return osData;
  }
}

function getsupportlist() {
  let strArry = ["X86 银河麒麟操作系统 桌面版 V10 2203",
    "X86 银河麒麟操作系统 桌面版 V10 2303",
    "X86 银河麒麟操作系统 桌面版 V10 2403",
    "ARM 银河麒麟操作系统 桌面版 V10 2203",
    "ARM 银河麒麟操作系统 桌面版 V10 2303",
    "ARM 银河麒麟操作系统 桌面版 V10 2403"
  ];
  return strArry;
}

async function getoscontent() {
  let osinfo = getosinfo();
  const content = [await getosinfo(), showosinfo(), getsupportlist()];
  return content;
}

getFileData();

module.exports = {
  getosinfo,
  showosinfo,
  getsupportlist,
  getoscontent
}

