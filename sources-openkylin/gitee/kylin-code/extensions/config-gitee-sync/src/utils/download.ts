import * as fs from "fs-extra";
import * as https from "https";
import * as url from "url";
import * as zlib from "zlib";

/**
 * 下载文件
 * @param uri 
 * @param savepath 
 */
export function downloadFile(uri: string, savepath: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const { host, path, port } = url.parse(uri);
    const options: https.RequestOptions = { host, path };

    if (port) {
      options.port = +port;
    }

    const file = fs.createWriteStream(savepath);
    file.on("finish", () => {
      file.close();
      resolve();
    });
    
    https.get(options, (res) => {
      if (res.statusCode === 200) {
        let intermediate: zlib.Gunzip | zlib.Inflate | undefined;
        const contentEncoding = res.headers["content-encoding"];
        if (contentEncoding === "gzip") {
          intermediate = zlib.createGunzip();
        } else if (contentEncoding === "deflate") {
          intermediate = zlib.createInflate();
        }

        if (intermediate) {
          res.pipe(intermediate).pipe(file);
        } else {
          res.pipe(file);
        }
      } else {
        reject();
      }
    }).on("error", (err) => {
      file.close();
      fs.remove(savepath).catch().then(() => reject(err));
    });
  });
}