import { createContext } from 'react';
import type { ColorThemeId } from './colorThemes';

export type ThemeMode = 'system' | 'dark' | 'light' | 'oled';
export type AccentColor = 'cyan' | 'violet' | 'emerald' | 'amber' | 'rose' | 'blue';
export type UiFont = 'system' | 'inter' | 'ibm-plex-sans' | 'source-serif';
export type MonoFont = 'jetbrains' | 'fira-code' | 'sfmono' | 'monospace';
export type ResolvedTheme = 'dark' | 'light' | 'oled';

export const uiFontStacks: Record<UiFont, string> = {
  system: 'system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif',
  inter: '"Inter", system-ui, sans-serif',
  'ibm-plex-sans': '"IBM Plex Sans", system-ui, sans-serif',
  'source-serif': '"Source Serif 4", Georgia, serif',
};

export const monoFontStacks: Record<MonoFont, string> = {
  jetbrains: '"JetBrains Mono", "SFMono-Regular", Consolas, monospace',
  'fira-code': '"Fira Code", "SFMono-Regular", Consolas, monospace',
  sfmono: '"SFMono-Regular", Menlo, Consolas, monospace',
  monospace: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
};

export interface ThemeContextValue {
  theme: ThemeMode;
  accent: AccentColor;
  colorTheme: ColorThemeId;
  uiFont: UiFont;
  monoFont: MonoFont;
  uiFontSize: number;
  monoFontSize: number;
  resolvedTheme: ResolvedTheme;
  setTheme: (theme: ThemeMode) => void;
  setAccent: (accent: AccentColor) => void;
  setColorTheme: (colorTheme: ColorThemeId) => void;
  setUiFont: (font: UiFont) => void;
  setMonoFont: (font: MonoFont) => void;
  setUiFontSize: (size: number) => void;
  setMonoFontSize: (size: number) => void;
}

export const ThemeContext = createContext<ThemeContextValue>({
  theme: 'system',
  accent: 'blue',
  colorTheme: 'default-dark',
  uiFont: 'system',
  monoFont: 'jetbrains',
  uiFontSize: 15,
  monoFontSize: 14,
  resolvedTheme: 'dark',
  setTheme: () => {},
  setAccent: () => {},
  setColorTheme: () => {},
  setUiFont: () => {},
  setMonoFont: () => {},
  setUiFontSize: () => {},
  setMonoFontSize: () => {},
});
