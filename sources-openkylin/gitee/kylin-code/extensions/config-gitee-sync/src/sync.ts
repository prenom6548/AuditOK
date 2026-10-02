import * as vscode from "vscode";
import { AutoSyncService, Gist, showReloadBox, statusFatal, statusInfo, Syncing, VSCodeSetting } from "./core";
import { localize, setup } from "./i18n";
import { ISyncedItem } from "./types/SyncingTypes";
import { registerCommand } from "./utils";

export class SyncService {
  private static instance: SyncService;

  private context: vscode.ExtensionContext;
  private isReady = false;
  private isSynchronizing = false;

  private autoSyncService?: AutoSyncService;
  private syncing?: Syncing;
  private vscodeSetting?: VSCodeSetting;

  private constructor(context: vscode.ExtensionContext) {
    this.context = context;
  }

  private static create(context: vscode.ExtensionContext) {
    if (!SyncService.instance) {
      SyncService.instance = new SyncService(context);
    }

    return SyncService.instance;
  }

  public static init(context: vscode.ExtensionContext) {
    SyncService.create(context);
    SyncService.instance.initSyncing();
    SyncService.instance.initCommands();
    SyncService.instance.initAutoSync();
  }

  public static uninit() {
    SyncService.instance.stopAutoSyncService();
  }

  /**
   * 初始化扩展
   * @param context
   */
  private initSyncing() {
    try {
      // i18n
      setup(this.context.extensionPath);
      this.syncing = Syncing.create();
      this.vscodeSetting = VSCodeSetting.create();

      this.isReady = true;

    } catch (err) {
      console.error("initSyncing:", err);
      this.isReady = false;
      statusFatal(localize("error.initialization", err.message))
    }
  }

  /**
   * 注册命令
   * @param context
   */
  private initCommands() {
    registerCommand(this.context, "configGiteeSync.uploadSettings", this.uploadVSCodeSettings.bind(this));
    registerCommand(this.context, "configGiteeSync.downloadSettings", this.downloadVSCodeSettings.bind(this));
    registerCommand(this.context, "configGiteeSync.openSettings", this.openSyncingSettings.bind(this));
  }

  /**
   * 初始化自动同步
   */
  private initAutoSync() {
    if (this.isReady) {
      setTimeout(async () => {
        const syncingSettings = this.syncing!.loadSettings();
        if (syncingSettings.auto_sync && syncingSettings.token && syncingSettings.id) {
          this.autoSyncService = AutoSyncService.create();
          this.autoSyncService.synchronize(syncingSettings);
          this.autoSyncService.start();
        }
      }, 3000);
    }
  }

  private async uploadVSCodeSettings() {
    if (this.isReady && !this.isSynchronizing) {
      this.isSynchronizing = true;
      this.stopAutoSyncService();

      try {
        const syncingSettings = await this.syncing!.prepareUploadSettings(true);
        const api = Gist.create(syncingSettings.token);
        const settings = await this.vscodeSetting!.getSettings(true, true);
        const gist = await api.findAndUpdate(syncingSettings.id, settings, true, true);
        if (gist.id !== syncingSettings.id) {
          await this.syncing!.saveSettings({ ...syncingSettings, id: gist.id });
        }

        // Synchronizes the last modified time.
        for (const setting of settings) {
          await this.vscodeSetting!.saveLastModifiedTime(setting, gist.updated_at);
        }

        statusInfo(localize("toast.settings.uploaded"));
      } catch (error) {

      } finally {
        this.isSynchronizing = false;
        this.autoSyncService?.start();
      }
    }
  }

  private async downloadVSCodeSettings() {
    if (this.isReady && !this.isSynchronizing) {
      this.isSynchronizing = true;
      this.pauseAutoSyncService();

      try {
        const syncingSettings = await this.syncing!.prepareDownloadSettings(true);
        const api = Gist.create(syncingSettings.token);
        try {
          const gist = await api.get(syncingSettings.id, true);
          const syncedItems = await this.vscodeSetting!.saveSettings(gist, true);
          statusInfo(localize("toast.settings.downloaded"));
          if (this.isExtensionsSynced(syncedItems)) {
            showReloadBox();
          }
        } catch (error) {
          if (error.code === 401) {
            this.syncing!.clearGiteeToken();
          } else if (error.code === 404) {
            this.syncing!.clearGistID();
          }
        }
      } catch (error) {

      } finally {
        this.isSynchronizing = false;
        this.resumeAutoSyncService();
      }
    }
  }

  private openSyncingSettings() {
    if (this.isReady) {
      this.syncing!.openSettings();
    }
  }


  private isExtensionsSynced(syncedItems: { updated: ISyncedItem[]; removed: ISyncedItem[] }): boolean {
    for (const item of syncedItems.updated) {
      if (item.extension && (
        item.extension.added.length > 0
        || item.extension.removed.length > 0)
        || item.extension!.updated.length > 0) {

        return true;
      }
    }
    return false;
  }

  private pauseAutoSyncService() {
    this.autoSyncService?.pause();
  }

  private resumeAutoSyncService() {
    this.autoSyncService?.resume();
  }

  private stopAutoSyncService() {
    this.autoSyncService?.stop();
  }
}
