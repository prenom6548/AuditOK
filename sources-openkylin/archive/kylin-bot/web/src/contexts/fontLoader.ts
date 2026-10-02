import type { MonoFont, UiFont } from './ThemeContextDef';

function setDataAttr(name: string, value: string): void {
  document.documentElement.setAttribute(name, value);
}

export function loadUiFont(font: UiFont): void {
  setDataAttr('data-ui-font', font);
}

export function loadMonoFont(font: MonoFont): void {
  setDataAttr('data-mono-font', font);
}
