import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import tailwindcss from '@tailwindcss/vite';
import path from 'path';

const DEFAULT_DEV_BACKEND = 'http://127.0.0.1:42617';

// Production assets are served by the Rust gateway via rust-embed under /_app/.
// During local development, Vite serves the frontend directly and proxies API/ws
// traffic to a live KylinBot backend so edits are visible immediately.
export default defineConfig(({ command }) => {
  // 允许通过环境变量切换开发态后端地址；未配置时回退到本地默认端口。
  const backendTarget = process.env.KYLINBOT_DEV_BACKEND ?? DEFAULT_DEV_BACKEND;
  // serve=本地开发；build=生产打包。用于控制 base 与 dev proxy。
  const isDev = command === 'serve';

  return {
    base: isDev ? '/' : '/_app/',
    // 关闭 Tailwind v4 内置 optimize（其内部目标浏览器偏现代），
    // 改由 postcss.config.js + build.cssTarget 统一做旧内核兼容处理。
    plugins: [react(), tailwindcss({ optimize: false })],
    resolve: {
      alias: {
        '@': path.resolve(__dirname, './src'),
      },
    },
    server: isDev
      ? {
          host: '127.0.0.1',
          port: 5173,
          strictPort: true,
          proxy: {
            '/api': {
              target: backendTarget,
              changeOrigin: true,
            },
            '/ws': {
              target: backendTarget.replace(/^http/, 'ws'),
              ws: true,
              changeOrigin: true,
            },
            '/pair': {
              target: backendTarget,
              changeOrigin: true,
            },
            '/health': {
              target: backendTarget,
              changeOrigin: true,
            },
            '/admin': {
              target: backendTarget,
              changeOrigin: true,
            },
          },
        }
      : undefined,
    optimizeDeps: {
      // Qt 5.12(.12) WebEngine ~ Chromium 69：smol-toml 等会使用 ?. / Object.hasOwn，
      // 须在依赖预构建阶段降级至 chrome69；否则 dev（及旧内核）无法运行。
      include: ['smol-toml'],
      esbuildOptions: {
        target: 'chrome69',
      },
    },
    build: {
      outDir: 'dist',
      // Qt 5.12 WebEngine ~ Chromium 69，JS/CSS 按该基线输出。
      target: ['chrome69', 'edge79', 'firefox78', 'safari13'],
      cssTarget: 'chrome69',
      // 继续使用 LightningCSS 压缩；配合 cssTarget 产出兼容 CSS。
      cssMinify: 'lightningcss',
    },
  };
});
