import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

const rootDir = dirname(fileURLToPath(import.meta.url));
const kylinVersion = process.env.KYLIN_VERSION === "v10" ? "v10" : "v11";

const tauriV1Alias =
  kylinVersion === "v10"
    ? {
        "@tauri-apps/api/core": resolve(rootDir, "src/lib/tauri-v1/core.ts"),
        "@tauri-apps/api/event": "@tauri-apps/api-v1/event",
        "@tauri-apps/api/window": resolve(rootDir, "src/lib/tauri-v1/window.ts"),
        "@tauri-apps/api/webview": resolve(rootDir, "src/lib/tauri-v1/webview.ts")
      }
    : {};

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: tauriV1Alias
  },
  clearScreen: false,
  server: {
    strictPort: true,
    port: 1420,
    proxy: {
      "/kylin-cloud": {
        target: "https://archive2.kylinos.cn",
        changeOrigin: true,
        secure: false,
        rewrite: (path) => path.replace(/^\/kylin-cloud/, "")
      }
    }
  },
  envPrefix: ["VITE_", "TAURI_"],
  build: {
    target: "es2020",
    minify: false
  }
});
