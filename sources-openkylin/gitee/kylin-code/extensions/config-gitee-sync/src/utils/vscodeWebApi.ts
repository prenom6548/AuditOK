import fetch from "node-fetch";
import { CaseInsensitiveMap } from "../collections";
import {
  ExtensionAssetType,
  IExtensionMeta,
  IExtensionVersion,
  QueryFilterType,
  QueryFlag
} from "../types/VSCodeWebAPITypes";
import { getVSCodeEdition } from "./vscodeAPI";
import { VSCodeEdition } from "../types/VSCodeEdition";


/**
 * 查询扩展元数据（meta data）
 *
 * @param {string[]} ids The id list of extensions. The id is in the form of: `publisher.name`.
 * @param {string} [proxy] The proxy settings.
 */
export async function queryExtensions(ids: string[]): Promise<CaseInsensitiveMap<string, IExtensionMeta>> {
  const result = new CaseInsensitiveMap<string, IExtensionMeta>();
  if (ids.length > 0) {
    let api = "https://open-vsx.org/vscode/gallery/extensionquery";
    const edition = getVSCodeEdition();
    switch (edition) {
      case VSCodeEdition.STANDARD:
      case VSCodeEdition.INSIDERS:
      case VSCodeEdition.EXPLORATION:
        api = "https://marketplace.visualstudio.com/_apis/public/gallery/extensionquery";
        break;

      case VSCodeEdition.VSCODIUM:
      case VSCodeEdition.OSS:
      case VSCodeEdition.KYLINIDE:
      case VSCodeEdition.KYLINCODE:
      case VSCodeEdition.KYLINCODEDEV:
        api = "https://open-vsx.org/vscode/gallery/extensionquery";
        break;

      default:
        api = "https://open-vsx.org/vscode/gallery/extensionquery";
        break;
    }

    const data = {
      filters: [
        {
          criteria: ids.map((name) => {
            return {
              filterType: QueryFilterType.NAME,
              value: name
            };
          })
        }
      ],
      flags: QueryFlag.LATEST_VERSION_WITH_FILES
    };

    try {
      // 这里必须添加'user-agent'
      const headers = {
        "user-agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36 Edg/87.0.664.66",
        "content-Type": "application/json",
        "accept": "application/json;api-version=5.1-preview.1"
      };
      const resp = await fetch(api, {
        method: "POST",
        headers,
        body: JSON.stringify(data)
      });
      if (resp.ok) {
        const { results } = await resp.json();
        if (results.length > 0) {
          (results[0].extensions || []).forEach((extension: IExtensionMeta) => {
            // Use extension's fullname as the key.
            result.set(`${extension.publisher.publisherName}.${extension.extensionName}`, extension);
          });
        }
      }
    }
    catch (err) {
    }
  }
  return result;
}

/**
 * 获取最新版本的扩展文件（VSIX file）
 *
 * @param {IExtensionMeta} extensionMeta The extension's meta object.
 */
export function getLatestVSIXVersion(extensionMeta: IExtensionMeta): string | undefined {
  const versionMeta = extensionMeta.versions[0];
  return versionMeta ? versionMeta.version : undefined;
}

/**
 * Gets the VSIX download URL.
 *
 * @deprecated The download speed of this URL is too slow.
 *
 * @param {IExtensionVersion} version The extension's version object.
 */
export function getVSIXDownloadURL(version: IExtensionVersion): string | undefined {
  const files = version.files;
  if (files) {
    const file = files.find((f) => (f.assetType === ExtensionAssetType.SERVICES_VSIXPACKAGE));
    if (file) {
      return file.source;
    }
  }
  return;
}
