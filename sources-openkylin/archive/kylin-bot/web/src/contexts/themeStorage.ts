import type { ColorThemeId } from './colorThemes';
import type { AccentColor, MonoFont, ThemeMode, UiFont } from './ThemeContextDef';

export const STORAGE_KEY = 'kylinbot-theme-settings';

interface StoredThemeSettings {
  theme: ThemeMode;
  accent: AccentColor;
  colorTheme: ColorThemeId;
  uiFont: UiFont;
  monoFont: MonoFont;
  uiFontSize: number;
  monoFontSize: number;
}

const DEFAULT_SETTINGS: StoredThemeSettings = {
  theme: 'light',
  accent: 'cyan',
  colorTheme: 'default-light',
  uiFont: 'system',
  monoFont: 'jetbrains',
  uiFontSize: 15,
  monoFontSize: 14,
};

export function loadStored(): StoredThemeSettings {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      return DEFAULT_SETTINGS;
    }
    const parsed = JSON.parse(raw) as Partial<StoredThemeSettings>;
    return {
      theme: parsed.theme ?? DEFAULT_SETTINGS.theme,
      accent: parsed.accent ?? DEFAULT_SETTINGS.accent,
      colorTheme: parsed.colorTheme ?? DEFAULT_SETTINGS.colorTheme,
      uiFont: parsed.uiFont ?? DEFAULT_SETTINGS.uiFont,
      monoFont: parsed.monoFont ?? DEFAULT_SETTINGS.monoFont,
      uiFontSize: parsed.uiFontSize ?? DEFAULT_SETTINGS.uiFontSize,
      monoFontSize: parsed.monoFontSize ?? DEFAULT_SETTINGS.monoFontSize,
    };
  } catch {
    return DEFAULT_SETTINGS;
  }
}
