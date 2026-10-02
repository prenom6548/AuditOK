import * as micromatch from "micromatch";
import * as vscode from "vscode";

import { CaseInsensitiveMap, CaseInsensitiveSet } from "../collections";
import { CONFIGURATION_EXCLUDED_EXTENSIONS, CONFIGURATION_EXTENSIONS_AUTOUPDATE, CONFIGURATION_KEY } from "../constants";
import { localize } from "../i18n";
import { IExtension, ISyncedItem } from "../types/SyncingTypes";
import { IExtensionMeta } from "../types/VSCodeWebAPITypes";
import { getExtensionById, getVSCodeSetting } from "../utils/vscodeAPI";
import { getLatestVSIXVersion, queryExtensions } from "../utils/vscodeWebApi";
import * as Toast from "./toast";


/**
 * Represents the options of synchronization.
 */
interface ISyncOptions {
  /**
   * The extensions to add, update or remove.
   */
  extensions: IExtension[];

  /**
   * The current progress of this synchronization process.
   */
  progress: number;

  /**
   * The total progress of this synchronization process.
   */
  total: number;

  /**
   * Sets a value indicating whether `Syncing` should show the progress indicator. Defaults to `false`.
   */
  showIndicator?: boolean;
}

/**
 * VSCode extension wrapper.
 */
export class Extension {
  private static _instance: Extension;

  private constructor() {
  }

  /**
   * Creates an instance of the singleton class `Extension`.
   */
  public static create(): Extension {
    if (!Extension._instance) {
      Extension._instance = new Extension();
    }
    return Extension._instance;
  }

  /**
   * Gets all installed extensions (Disabled extensions aren't included).
   *
   * @param excludedPatterns The glob patterns of the extensions that should be excluded.
   */
  public getAll(excludedPatterns: string[] = []): IExtension[] {
    let item: IExtension;
    const result: IExtension[] = [];
    for (const ext of vscode.extensions.all) {
      if (
        !ext.packageJSON.isBuiltin
        && !excludedPatterns.some((pattern) => micromatch.isMatch(ext.id, pattern, { nocase: true }))
      ) {
        item = {
          id: ext.id,
          name: ext.packageJSON.name,
          publisher: ext.packageJSON.publisher,
          version: ext.packageJSON.version
        };
        result.push(item);
      }
    }
    return result.sort((a, b) => (a.id || "").localeCompare(b.id || ""));
  }

  /**
   * 同步扩展 (add, update or remove).
   *
   * @param extensions Extensions to be synced.
   * @param showIndicator Whether to show the progress indicator. Defaults to `false`.
   */
  public async sync(extensions: IExtension[], showIndicator: boolean = false): Promise<ISyncedItem> {
    const diff = await this._getDifferentExtensions(extensions);

    // Add, update or remove extensions.
    const { added, updated, removed, total } = diff;
    const result = { extension: {} } as ISyncedItem;
    const tasks = [
      this._addExtensions.bind(this, {
        extensions: added,
        progress: 0,
        total,
        showIndicator
      }),
      this._updateExtensions.bind(this, {
        extensions: updated,
        progress: added.length,
        total,
        showIndicator
      }),
      this._removeExtensions.bind(this, {
        extensions: removed,
        progress: added.length + updated.length,
        total,
        showIndicator
      })
    ];

    for (const task of tasks) {
      const value = await task();
      Object.assign(result.extension as Object, value);
    }

    if (showIndicator) {
      Toast.clearSpinner();
    }

    // Added since VSCode v1.20.
    // await this.updateObsolete(added, updated, removed);

    return result;
  }

  /**
   * 安装扩展
   * @param extension 
   */
  public async installExtension(extension: IExtension) {
    try {
      await vscode.commands.executeCommand('workbench.extensions.installExtension', extension.id);
      return true;
    } catch (error) {
      console.error(error);
    }

    return false;
  }

  /**
   * 卸载扩展
   * @param extension 
   */
  public async uninstallExtension(extension: IExtension) {
    try {
      await vscode.commands.executeCommand('workbench.extensions.uninstallExtension', extension.id);
      return true;
    } catch (error) {
      console.error(error);
    }

    return false;
  }

  /**
   * 更新扩展
   * @param extension 
   */
  public async updateExtension(extension: IExtension) {
    await this.uninstallExtension(extension);
    await this.installExtension(extension);
  }

  /**
   * Gets the extensions that will be added, updated or removed.
   */
  private async _getDifferentExtensions(extensions: IExtension[]): Promise<{
    added: IExtension[];
    removed: IExtension[];
    updated: IExtension[];
    total: number;
  }> {
    const result = {
      added: [] as IExtension[],
      removed: [] as IExtension[],
      updated: [] as IExtension[],
      get total() {
        return this.added.length + this.removed.length + this.updated.length;
      }
    };
    if (extensions) {
      // 1. Auto update extensions: Query the latest extensions.
      let queriedExtensions: CaseInsensitiveMap<string, IExtensionMeta> = new CaseInsensitiveMap();
      const autoUpdateExtensions = getVSCodeSetting<boolean>(
        CONFIGURATION_KEY,
        CONFIGURATION_EXTENSIONS_AUTOUPDATE
      );
      if (autoUpdateExtensions) {
        queriedExtensions = await queryExtensions(extensions.map((ext) => ext.id));
      }

      // Find added & updated extensions.
      const reservedExtensionIDs = new CaseInsensitiveSet<string>();
      for (const ext of extensions) {
        // 2. Auto update extensions: Update to the latest version.
        if (autoUpdateExtensions) {
          const extensionMeta = queriedExtensions.get(ext.id);
          if (extensionMeta) {
            const latestVersion = getLatestVSIXVersion(extensionMeta);
            if (latestVersion != null) {
              ext.version = latestVersion;
            }
          }
        }

        const localExtension = getExtensionById(ext.id);
        if (localExtension) {
          if (localExtension.packageJSON.version === ext.version) {
            // Reserved.
            reservedExtensionIDs.add(ext.id);
          } else {
            // Updated.
            result.updated.push(ext);
          }
        } else {
          // Added.
          result.added.push(ext);
        }
      }

      // Find removed extensions, but don't remove the extensions that are excluded.
      // Here's the trick: since the `extensions.json` are always synchronized after the `settings.json`,
      // We can safely get the patterns from VSCode.
      const patterns = getVSCodeSetting<string[]>(CONFIGURATION_KEY, CONFIGURATION_EXCLUDED_EXTENSIONS);
      const localExtensions: IExtension[] = this.getAll(patterns);
      for (const ext of localExtensions) {
        if (!reservedExtensionIDs.has(ext.id)) {
          // Removed.
          result.removed.push(ext);
        }
      }

      // Release resources.
      queriedExtensions.clear();
      reservedExtensionIDs.clear();
    }
    return result;
  }

  /**
   * Adds extensions.
   */
  private async _addExtensions(options: ISyncOptions): Promise<{ added: IExtension[]; addedErrors: IExtension[];}> {
    const { extensions, progress, showIndicator = false, total } = options;

    let steps: number = progress;
    const result = { added: [] as IExtension[], addedErrors: [] as IExtension[] };
    for (const item of extensions) {
      try {
        steps++;

        if (showIndicator) {
          Toast.showSpinner(localize("toast.settings.installing.extension", item.id), steps, total);
        }
        await this.installExtension(item);

        result.added.push(item);
      } catch (error) {
        result.addedErrors.push(item);
      }
    }
    return result;
  }

  /**
   * Updates extensions.
   */
  private async _updateExtensions(options: ISyncOptions): Promise<{updated: IExtension[]; updatedErrors: IExtension[];}> {
    const { extensions, progress, showIndicator = false, total } = options;

    let steps: number = progress;
    const result = { updated: [] as IExtension[], updatedErrors: [] as IExtension[] };
    for (const item of extensions) {
      try {
        steps++;

        if (showIndicator) {
          Toast.showSpinner(localize("toast.settings.installing.extension", item.id), steps, total);
        }
        await this.updateExtension(item);

        result.updated.push(item);
      } catch (error) {
        result.updatedErrors.push(item);
      }
    }
    return result;
  }

  /**
   * Removes extensions.
   */
  private async _removeExtensions(options: ISyncOptions): Promise<{ removed: IExtension[]; removedErrors: IExtension[];}> {
    const { extensions, progress, showIndicator = false, total } = options;

    let steps: number = progress;
    const result = { removed: [] as IExtension[], removedErrors: [] as IExtension[] };
    for (const item of extensions) {
      try {
        steps++;

        if (showIndicator) {
          Toast.showSpinner(localize("toast.settings.uninstalling.extension", item.id), steps, total);
        }
        await this.uninstallExtension(item);

        result.removed.push(item);
      } catch (error) {
        result.removedErrors.push(item);
      }
    }
    return result;
  }
}
