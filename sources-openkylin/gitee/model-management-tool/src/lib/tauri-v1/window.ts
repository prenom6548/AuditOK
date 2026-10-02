import { appWindow } from "@tauri-apps/api-v1/window";

export function getCurrentWindow() {
  return appWindow;
}
