#!/usr/bin/env node
import { spawnSync } from "node:child_process";
import { existsSync, readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const rootDir = dirname(dirname(fileURLToPath(import.meta.url)));

function localCliBin(pkg) {
  const bin = join(rootDir, "node_modules", pkg, "tauri.js");
  return existsSync(bin) ? bin : null;
}

function detectKylinVersion() {
  const forced = process.env.KYLIN_VERSION;
  if (forced === "v10" || forced === "v11") {
    return forced;
  }
  try {
    const content = readFileSync("/etc/os-release", "utf8");
    const match = content.match(/^VERSION_ID="?([^"\n]*)"?/m);
    if (match && match[1].trim().toLowerCase().startsWith("v10")) {
      return "v10";
    }
  } catch {
    // fall through to default
  }
  return "v11";
}

const args = process.argv.slice(2);
const cliArgs = args.length > 0 ? args : ["build"];
const kylinVersion = detectKylinVersion();
const tauriMajor = kylinVersion === "v10" ? 1 : 2;
console.log(`[kylin] 系统版本: ${kylinVersion}，使用 Tauri ${tauriMajor} 构建`);

const env = { ...process.env, KYLIN_VERSION: kylinVersion };

function ensureCargo() {
  const home = process.env.HOME || "";
  const pathDirs = (env.PATH || "").split(":").filter(Boolean);
  for (const dir of [join(home, ".cargo", "bin"), "/usr/local/bin", "/usr/bin"]) {
    if (dir && !pathDirs.includes(dir)) pathDirs.unshift(dir);
  }
  env.PATH = pathDirs.join(":");
  if (!pathDirs.some((dir) => existsSync(join(dir, "cargo")))) {
    console.error("[kylin] 未找到 cargo（Rust 工具链），请先以普通用户执行 scripts/install-deps.sh");
    process.exit(1);
  }
}
ensureCargo();

function versionOverrideArgs(confPath, key) {
  if (!cliArgs.includes("build")) {
    return [];
  }
  const conf = JSON.parse(readFileSync(confPath, "utf8"));
  const baseVersion = key === "package" ? conf.package.version : conf.version;
  const version = `${baseVersion}-${kylinVersion}`;
  console.log(`[kylin] deb 版本: ${version}`);
  return [
    "--config",
    key === "package"
      ? JSON.stringify({ package: { version } })
      : JSON.stringify({ version })
  ];
}

let result;
if (kylinVersion === "v10") {
  const extra = versionOverrideArgs(
    join(rootDir, "src-tauri-v1", "tauri.conf.json"),
    "package"
  );
  const v1Bin = localCliBin("@tauri-apps/cli-v1");
  const [v1Cmd, v1Args] = v1Bin
    ? [process.execPath, [v1Bin, ...cliArgs, ...extra]]
    : ["npx", ["-y", "@tauri-apps/cli@1", ...cliArgs, ...extra]];
  if (!v1Bin) {
    console.warn("[kylin] 本地未安装 @tauri-apps/cli-v1，回退到 npx 在线下载（需要联网；离线环境请先执行 scripts/install-deps.sh）");
  }
  result = spawnSync(v1Cmd, v1Args, {
    stdio: "inherit",
    env,
    cwd: join(rootDir, "src-tauri-v1")
  });
} else {
  const extra = versionOverrideArgs(
    join(rootDir, "src-tauri", "tauri.conf.json"),
    "version"
  );
  const v2Bin = localCliBin("@tauri-apps/cli");
  const [v2Cmd, v2Args] = v2Bin
    ? [process.execPath, [v2Bin, ...cliArgs, ...extra]]
    : ["npx", ["tauri", ...cliArgs, ...extra]];
  if (!v2Bin) {
    console.warn("[kylin] 本地未安装 @tauri-apps/cli，回退到 npx 在线下载（需要联网；离线环境请先执行 scripts/install-deps.sh）");
  }
  result = spawnSync(v2Cmd, v2Args, {
    stdio: "inherit",
    env,
    cwd: rootDir
  });
}

process.exit(result?.status ?? 1);
