export const LOCALE_STORAGE_KEY = 'kylinbot-locale';

const DEFAULT_LOCALE = 'zh';

export function loadLocale(): string {
  try {
    return localStorage.getItem(LOCALE_STORAGE_KEY) ?? DEFAULT_LOCALE;
  } catch {
    return DEFAULT_LOCALE;
  }
}

export function saveLocale(locale: string): void {
  try {
    localStorage.setItem(LOCALE_STORAGE_KEY, locale);
  } catch {
    // Ignore storage failures.
  }
}
