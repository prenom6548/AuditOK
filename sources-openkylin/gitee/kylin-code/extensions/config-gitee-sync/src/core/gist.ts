
import { CONFIGURATION_KEY, CONFIGURATION_POKA_YOKE_THRESHOLD } from "../constants";
import { localize } from "../i18n";
import fetch, { Response } from 'node-fetch';
import { ISetting, SettingType } from "../types/SyncingTypes";
import { diff, getVSCodeSetting, createError } from "../utils";
import * as Toast from "./toast";
import { GistsCreateParams, GistsUpdateParams, IGist, IGistFile, IGistFiles } from "../types/GiteeTypes";
import pick = require("lodash.pick");
import { parse } from "../utils/jsonc";
import * as vscode from "vscode";

const GIST_DESCRIPTION = (vscode.env.appName ? vscode.env.appName : "IDE") + "'s Settings - gitee sync";
const BASE_API = "https://gitee.com/api/v5";

export class Gist {
  private static _instance: Gist;

  private _token?: string;
  private _timeout = 20000;

  private constructor(token?: string) {
    this._token = token;
  }

  public static create(token?: string): Gist {
    if (!Gist._instance || Gist._instance.token != token) {
      Gist._instance = new Gist(token)
    }
    return Gist._instance;
  }

  public get token() {
    return this._token;
  }

  /**
   * 获取授权的用户信息
   */
  public async user(): Promise<{ id: number; name: string }> {
    const url = `${BASE_API}/user?access_token=${this.token}`
    try {
      let resp = await fetch(url, {
        timeout: this._timeout
      });
      this.checkStatus(resp);
      let data = await resp.json();
      return {
        id: data.id,
        name: data.login
      };
    } catch (error) {
      throw this.createError(error);
    }
  }

  public async get(id: string, showIndicator: boolean = false): Promise<IGist> {
    const url = `${BASE_API}/gists/${id}?access_token=${this.token}`
    showIndicator && Toast.showSpinner(localize("toast.settings.checking.remote"));

    try {
      const resp = await fetch(url, {
        timeout: this._timeout
      });
      this.checkStatus(resp);
      showIndicator && Toast.clearSpinner();
      const data = resp.json();
      return data;
    } catch (error) {
      const err = this.createError(error);
      showIndicator && Toast.statusError(localize("toast.settings.downloading.failed", error.message));
      throw err;
    }
  }

  /**
   * 获取全部的gists
   */
  public async getAll(): Promise<IGist[]> {
    const url = `${BASE_API}/gists?access_token=${this.token}`;
    try {
      const resp = await fetch(url, {
        timeout: this._timeout
      });
      this.checkStatus(resp);
      const gists: IGist[] = await resp.json();
      const extensionsRemoteFilename = `${SettingType.Extensions}.json`;
      return gists
        .filter((gist) => gist.description === GIST_DESCRIPTION || gist.files[extensionsRemoteFilename])
        .sort((a, b) => new Date(a.updated_at).getTime() - new Date(b.updated_at).getTime());
    } catch (error) {
      throw this.createError(error);
    }
  }

  public async delete(id: string): Promise<void> {
    const url = `${BASE_API}/gists/${id}?access_token=${this.token}`
    try {
      const resp = await fetch(url, { method: "DELETE", timeout: this._timeout });
      this.checkStatus(resp);
      return;
    } catch (error) {
      throw this.createError(error);
    }
  }

  public async update(id: string, content: GistsUpdateParams): Promise<IGist> {
    const url = `${BASE_API}/gists/${id}`;

    Object.assign(content, { access_token: this.token })

    try {
      const resp = await fetch(url, {
        method: "PATCH",
        body: JSON.stringify(content),
        headers: { 'Content-Type': 'application/json' },
        timeout: this._timeout
      });
      this.checkStatus(resp);
      const data = resp.json();
      return data;
    } catch (error) {
      throw this.createError(error);
    }
  }

  /**
   * 判断指定的gist是否存在
   * @param id
   */
  public async exists(id: string): Promise<false | IGist> {
    if (!id) { return false; }

    try {
      const gist = await this.get(id);
      if (this.token) {
        const user = await this.user();
        if (user.id === gist.owner.id) {
          return gist;
        }
        return false;
      }
      return gist;
    } catch (error) {

    }
    return false;
  }

  /**
   * 创建一个新的gist
   * @param content
   */
  public async create(content: GistsCreateParams): Promise<IGist> {
    const url = `${BASE_API}/gists`

    Object.assign(content, { access_token: this.token })

    try {
      const resp = await fetch(url, {
        method: "POST",
        body: JSON.stringify(content),
        headers: { 'Content-Type': 'application/json' },
        timeout: this._timeout,
      });
      this.checkStatus(resp);
      const data = await resp.json();
      return data;
    } catch (error) {
      throw this.createError(error);
    }
  }

  /**
   * 创建一个新的gist设置
   * @param files
   * @param isPublic
   */
  public createSettings(files = {}, isPublic = false): Promise<IGist> {
    return this.create({
      description: GIST_DESCRIPTION,
      files,
      public: isPublic
    });
  }

  /**
   * 更新gist
   * @param id
   * @param uploads
   * @param upsert
   * @param showIndicator
   */
  public async findAndUpdate(
    id: string,
    uploads: ISetting[],
    upsert = true,
    showIndicator = false
  ): Promise<IGist> {
    showIndicator && Toast.showSpinner(localize("toast.settings.uploading"));

    try {
      let result: IGist;
      const exists = await this.exists(id);

      const localGist: { files: any } = { files: {} };
      for (const item of uploads) {
        if (item.content) {
          localGist.files[item.remoteFilename] = { content: item.content };
        }
      }

      if (exists) {
        localGist.files = this.getModifiedFiles(localGist.files, exists.files);
        if (localGist.files) {
          const threshold = getVSCodeSetting<number>(CONFIGURATION_KEY, CONFIGURATION_POKA_YOKE_THRESHOLD);
          if (threshold > 0) {
            const localFiles = { ...localGist.files };
            const remoteFiles = pick(exists.files, Object.keys(localFiles));

            const changes = this.diffSettings(localFiles, remoteFiles);
            if (changes >= threshold) {
              const okButton = localize("pokaYoke.continue.upload");
              const message = localize("pokaYoke.continue.upload.message");
              const selection = await Toast.showConfirmBox(
                message,
                okButton,
                localize("pokaYoke.cancel")
              );
              if (selection !== okButton) {
                throw createError(localize("error.abort.synchronization"));
              }
            }
          }

          result = await this.update(id, localGist);
        } else {
          // 没有改变
          result = exists;
        }
      } else {
        if (upsert) {
          result = await this.createSettings(localGist.files);
        } else {
          throw createError(localize("error.gist.notfound", id));
        }
      }

      showIndicator && Toast.clearSpinner();
      return result;
    } catch (error) {
      showIndicator && Toast.statusError(localize("toast.settings.uploading.failed", error.message));
      throw error;
    }
  }

  /**
   * 比较本地和服务器的文件列表，返回修改后的文件列表或者undefined
   * @param localFiles  本地文件列表
   * @param remoteFiles 服务器文件列表
   */
  public getModifiedFiles(
    localFiles: IGistFiles,
    remoteFiles?: IGistFiles
  ): IGistFiles | undefined {
    if (!remoteFiles) {
      return localFiles;
    }

    let localFile;
    let remoteFile;
    const result = {} as IGistFiles;
    const recordedKeys = [];

    for (const key of Object.keys(remoteFiles)) {
      localFile = localFiles[key];
      remoteFile = remoteFiles[key];
      if (localFile) {
        if (localFile.content && localFile.content != remoteFile.content) {
          result[key] = localFile;
        }
      } else {
        if (!key.includes(SettingType.Keybindings) && !key.includes(SettingType.Settings)) {
          result[key] = null as any;
        }
      }

      recordedKeys.push(key);
    }

    for (const key of Object.keys(localFiles)) {
      if (!recordedKeys.includes(key)) {
        localFile = localFiles[key];
        if (localFile.content) {
          result[key] = localFile;
        }
      }
    }

    return (Object.keys(result).length === 0) ? undefined : result;
  }


  private createError(error: Error & { status: number }) {
    const { status } = error;
    let message = error.message || localize("error.check.internet");
    if (status === 401) {
      message = localize("error.check.gitee.token");
    } else if (status === 404) {
      message = localize("error.check.gist.id");
    }

    console.error("config-gitee-sync:", error);

    return createError(message, status);
  }

  private diffSettings(localFiles: IGistFiles, remoteFiles: IGistFiles): number {
    const left = this.parseToJSON(localFiles);
    const right = this.parseToJSON(pick(remoteFiles, Object.keys(localFiles)));
    return diff(left, right);
  }

  private parseToJSON(files: IGistFiles): Record<string, any> {
    const extensionsRemoteFilename = `${SettingType.Extensions}.json`;
    let parsed: any;
    let file: IGistFile;
    const result: Record<string, any> = {};
    for (const key of Object.keys(files)) {
      file = files[key];
      if (file) {
        parsed = parse(file.content || "");

        if (key === extensionsRemoteFilename && Array.isArray(parsed)) {
          for (const ext of parsed) {
            if (ext["id"] != null) {
              ext["id"] = ext["id"].toLocaleLowerCase();
            }

            // Only compares id and version.
            delete ext["name"];
            delete ext["publisher"];
            delete ext["uuid"];
          }
        }

        result[key] = parsed;
      }
      else {
        result[key] = file;
      }
    }
    return result;
  }

  private checkStatus(response: Response) {
    if (!response.ok) {
      throw response;
    }
  }
}
