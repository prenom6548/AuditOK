export interface IGist {
  description: string;
  files: IGistFiles;
  history: IGistHistory[];
  id: string;
  owner: IGistUser;
  public: boolean;

  /** 最后更新时间 */
  updated_at: string;
}

export type IGistFiles = {
  [key: string]: IGistFile;
}

export interface IGistFile {
  content: string;
}

export interface IGistUser {
  id: number;
  login: string;
}

export interface IGistHistory {
  user: IGistUser;
  version: string;
}

export interface GistsCreateParams {
  access_token?: string;
  description?: string;
  files: IGistFiles;
  public?: boolean;
}

export interface GistsUpdateParams {
  access_token?: string;
  files?: IGistFiles;
  description?: string;
  public?: boolean;
}
