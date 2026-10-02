import { format, getNormalizedVSCodeLocale } from "../utils";
import { NormalizedLocale } from "../types/NormalizedLocale";
import { readJsonSync } from "fs-extra";
import * as path from "path";

const DEFAULT_LOCALE_FILENAME = 'package.nls.json';

let instance: I18n;

class I18n {
  private static instance: I18n;

  private _bundle: Record<string, string>;
  private _extensionPath: string;
  private _locale: NormalizedLocale;

  private constructor(extensionPath: string) {
    this._extensionPath = extensionPath;
    this._locale = getNormalizedVSCodeLocale();
    this._bundle = this.prepare();
  }

  public static create(extensionPath: string): I18n {
    if (!I18n.instance || I18n.instance._extensionPath !== extensionPath) {
      I18n.instance = new I18n(extensionPath);
    }

    return I18n.instance;
  }

  public get locale(): NormalizedLocale {
    return this._locale;
  }

  public localize(key: string, ...templateValues: any[]): string {
    const message = this._bundle[key];

    if (templateValues.length > 0) {
      return format(message, ...templateValues);
    }

    return message;
  }

  private prepare() {
    const filename = (this.locale === NormalizedLocale.EN_US) ? DEFAULT_LOCALE_FILENAME : `package.nls.${this.locale}.json`;

    let bundle: Record<string, string>;
    try {
      bundle = readJsonSync(
        path.resolve(this._extensionPath, filename)
        , { encoding: 'utf8' }
      );
    } catch (error) {
      bundle = readJsonSync(
        path.resolve(this._extensionPath, DEFAULT_LOCALE_FILENAME),
        { encoding: "utf8" }
      );
    }

    return bundle;
  }
}

/**
 * 初始化 i18n
 * @param extensionPath 
 */
export function setup(extensionPath: string): void {
  instance = I18n.create(extensionPath)
}

/**
 * 获取vscode的语言版本（EN_US | ZH_CN）
 */
export function locale(): NormalizedLocale {
  return instance.locale;
}

/**
 * 获取本地化的文本信息
 * @param key message的key
 * @param templateValues 
 */
export function localize(key: string, ...templateValues: any[]): string {
  return instance.localize(key, ...templateValues);
}