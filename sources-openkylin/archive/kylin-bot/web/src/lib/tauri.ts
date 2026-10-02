// Tauri detection utilities for KylinBot Desktop.

declare global {
  interface Window {
    __TAURI__?: unknown;
    __TAURI_INTERNALS__?: unknown;
    __KYLINBOT_GATEWAY__?: string;
    isTauri?: boolean;
  }
}

/** Returns true when running inside a Tauri WebView. */
export const isTauri = (): boolean =>
  Boolean(window.isTauri || window.__TAURI_INTERNALS__ || window.__TAURI__);

/** Gateway base URL when running inside Tauri (defaults to localhost). */
export const tauriGatewayUrl = (): string =>
  window.__KYLINBOT_GATEWAY__ ?? 'http://127.0.0.1:42617';

async function getCurrentWindowHandle() {
  if (!isTauri()) return null;
  const dynamicImport = async (specifier: string): Promise<any> =>
    import(/* @vite-ignore */ specifier);

  try {
    const mod = await dynamicImport('@tauri-apps/api/window');
    if (typeof mod.getCurrentWindow === 'function') {
      return mod.getCurrentWindow();
    }
  } catch {
    // Fall through to v2 path.
  }

  const mod = await dynamicImport('@tauri-apps/api/webviewWindow');
  if (typeof mod.getCurrentWindow === 'function') {
    return mod.getCurrentWindow();
  }
  if (typeof mod.getCurrentWebviewWindow === 'function') {
    return mod.getCurrentWebviewWindow();
  }
  return null;
}

export async function closeCurrentWindow(): Promise<void> {
  const currentWindow = await getCurrentWindowHandle();
  await currentWindow?.close();
}

export async function minimizeCurrentWindow(): Promise<void> {
  const currentWindow = await getCurrentWindowHandle();
  await currentWindow?.minimize();
}

export async function toggleMaximizeCurrentWindow(): Promise<void> {
  const currentWindow = await getCurrentWindowHandle();
  await currentWindow?.toggleMaximize();
}

export async function toggleAlwaysOnTopCurrentWindow(): Promise<void> {
  const currentWindow = await getCurrentWindowHandle();
  if (!currentWindow) return;
  const alwaysOnTop = await currentWindow.isAlwaysOnTop();
  await currentWindow.setAlwaysOnTop(!alwaysOnTop);
}

export async function isCurrentWindowAlwaysOnTop(): Promise<boolean> {
  const currentWindow = await getCurrentWindowHandle();
  return (await currentWindow?.isAlwaysOnTop()) ?? false;
}

export async function startCurrentWindowDragging(): Promise<void> {
  const currentWindow = await getCurrentWindowHandle();
  await currentWindow?.startDragging();
}

const CLOSE_ACTION_KEY = 'kylinbot_close_action';

export function getCloseAction(): 'hide' | 'exit' {
  try {
    const stored = localStorage.getItem(CLOSE_ACTION_KEY);
    if (stored === 'hide' || stored === 'exit') {
      return stored;
    }
  } catch {}
  return 'exit';
}

export async function handleCloseButtonClick(): Promise<void> {
  const action = getCloseAction();
  if (action === 'hide') {
    await minimizeCurrentWindow();
  } else {
    await closeCurrentWindow();
  }
}
