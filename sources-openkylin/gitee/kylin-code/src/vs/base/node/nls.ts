/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { join } from '../common/path.js';
import { promises } from 'fs';
import { mark } from '../common/performance.js';
import { ILanguagePack, ILanguagePacks, INLSConfiguration } from '../../nls.js';
import { Promises } from './pfs.js';
import { createHash } from 'crypto';

export interface IResolveNLSConfigurationContext {

	/**
	 * Location where `nls.messages.json` and `nls.keys.json` are stored.
	 */
	readonly nlsMetadataPath: string;

	/**
	 * Path to the user data directory. Used as a cache for
	 * language packs converted to the format we need.
	 */
	readonly userDataPath: string;

	/**
	 * Commit of the running application. Can be `undefined`
	 * when not built.
	 */
	readonly commit: string | undefined;

	/**
	 * Locale as defined in `argv.json` or `app.getLocale()`.
	 */
	readonly userLocale: string;

	/**
	 * Locale as defined by the OS (e.g. `app.getPreferredSystemLanguages()`).
	 */
	readonly osLocale: string;
}

export async function resolveNLSConfiguration({ userLocale, osLocale, userDataPath, commit, nlsMetadataPath }: IResolveNLSConfigurationContext): Promise<INLSConfiguration> {
	mark('code/willGenerateNls');
	// console.log('process.resourcesPath:', process.resourcesPath);

	if (
		process.env['VSCODE_DEV'] ||
		userLocale === 'pseudo' ||
		userLocale.startsWith('en') ||
		!commit ||
		!userDataPath
	) {
		return defaultNLSConfiguration(userLocale, osLocale, nlsMetadataPath);
	}

	try {
		const languagePacks = await getLanguagePackConfigurations(userDataPath, userLocale, nlsMetadataPath);
		if (!languagePacks) {
			return defaultNLSConfiguration(userLocale, osLocale, nlsMetadataPath);
		}

		const resolvedLanguage = resolveLanguagePackLanguage(languagePacks, userLocale);
		if (!resolvedLanguage) {
			return defaultNLSConfiguration(userLocale, osLocale, nlsMetadataPath);
		}

		const languagePack = languagePacks[resolvedLanguage];
		const mainLanguagePackPath = languagePack?.translations?.['vscode'];
		if (
			!languagePack ||
			typeof languagePack.hash !== 'string' ||
			!languagePack.translations ||
			typeof mainLanguagePackPath !== 'string' ||
			!(await Promises.exists(mainLanguagePackPath))
		) {
			return defaultNLSConfiguration(userLocale, osLocale, nlsMetadataPath);
		}

		const languagePackId = `${languagePack.hash}.${resolvedLanguage}`;
		const globalLanguagePackCachePath = join(userDataPath, 'clp', languagePackId);
		const commitLanguagePackCachePath = join(globalLanguagePackCachePath, commit);
		const languagePackMessagesFile = join(commitLanguagePackCachePath, 'nls.messages.json');
		const translationsConfigFile = join(globalLanguagePackCachePath, 'tcf.json');
		const languagePackCorruptMarkerFile = join(globalLanguagePackCachePath, 'corrupted.info');

		if (await Promises.exists(languagePackCorruptMarkerFile)) {
			await promises.rm(globalLanguagePackCachePath, { recursive: true, force: true, maxRetries: 3 }); // delete corrupted cache folder
		}

		const result: INLSConfiguration = {
			userLocale,
			osLocale,
			resolvedLanguage,
			defaultMessagesFile: join(nlsMetadataPath, 'nls.messages.json'),
			languagePack: {
				translationsConfigFile,
				messagesFile: languagePackMessagesFile,
				corruptMarkerFile: languagePackCorruptMarkerFile
			},

			// NLS: below properties are a relic from old times only used by vscode-nls and deprecated
			locale: userLocale,
			availableLanguages: { '*': resolvedLanguage },
			_languagePackId: languagePackId,
			_languagePackSupport: true,
			_translationsConfigFile: translationsConfigFile,
			_cacheRoot: globalLanguagePackCachePath,
			_resolvedLanguagePackCoreLocation: commitLanguagePackCachePath,
			_corruptedFile: languagePackCorruptMarkerFile
		};

		if (await Promises.exists(commitLanguagePackCachePath)) {
			touch(commitLanguagePackCachePath).catch(() => { }); // We don't wait for this. No big harm if we can't touch
			mark('code/didGenerateNls');
			return result;
		}

		const [
			,
			nlsDefaultKeys,
			nlsDefaultMessages,
			nlsPackdata
		]:
			[unknown, Array<[string, string[]]>, string[], { contents: Record<string, Record<string, string>> }]
			//               ^moduleId ^nlsKeys                               ^moduleId      ^nlsKey ^nlsValue
			= await Promise.all([
				promises.mkdir(commitLanguagePackCachePath, { recursive: true }),
				promises.readFile(join(nlsMetadataPath, 'nls.keys.json'), 'utf-8').then(content => JSON.parse(content)),
				promises.readFile(join(nlsMetadataPath, 'nls.messages.json'), 'utf-8').then(content => JSON.parse(content)),
				promises.readFile(mainLanguagePackPath, 'utf-8').then(content => JSON.parse(content)),
			]);

		const nlsResult: string[] = [];

		// We expect NLS messages to be in a flat array in sorted order as they
		// where produced during build time. We use `nls.keys.json` to know the
		// right order and then lookup the related message from the translation.
		// If a translation does not exist, we fallback to the default message.

		let nlsIndex = 0;
		for (const [moduleId, nlsKeys] of nlsDefaultKeys) {
			const moduleTranslations = nlsPackdata.contents[moduleId];
			for (const nlsKey of nlsKeys) {
				nlsResult.push(moduleTranslations?.[nlsKey] || nlsDefaultMessages[nlsIndex]);
				nlsIndex++;
			}
		}

		await Promise.all([
			promises.writeFile(languagePackMessagesFile, JSON.stringify(nlsResult), 'utf-8'),
			promises.writeFile(translationsConfigFile, JSON.stringify(languagePack.translations), 'utf-8')
		]);

		mark('code/didGenerateNls');

		return result;
	} catch (error) {
		console.error('Generating translation files failed.', error);
	}

	return defaultNLSConfiguration(userLocale, osLocale, nlsMetadataPath);
}

/**
 * @function generateLanguagePacks
 * 无参数,生成languagepacks.json
 */
async function generateLanguagePacks(nlsMetadataPath: string): Promise<ILanguagePacks> {
	// console.log('nlsMetadataPath', nlsMetadataPath);
	// 读取 vscode-language-pack-zh-hans/package.json文件,读取到language文件)
	const languageExtensionPath = join(nlsMetadataPath, '..', 'extensions', 'vscode-language-pack-zh-hans');
	// console.log('languageExtensionPath', languageExtensionPath);
	const languagePackage = join(languageExtensionPath, 'package.json');
	const requireLanguagePackage = JSON.parse(await promises.readFile(languagePackage, 'utf-8'));
	const localizationContribution = requireLanguagePackage.contributes.localizations[0];
	const languagePackageTranslation = localizationContribution.translations;
	const extensionIdentifier = requireLanguagePackage.publisher + '.' + requireLanguagePackage.name;
	const md5 = createHash('md5');
	const hash = md5.update(localizationContribution.uuid ?? extensionIdentifier.toLowerCase()).update(requireLanguagePackage.version).digest('hex');

	// hash怎么生成的:
	// 1.读取插件下的package.json文件,判断是否有__metadate的uuid,使用如下算法生成
	// const md5 = createHash('md5');
	// md5.update('e4ee7751-6514-4731-9cdb-7580ffa9e70b'||'kylinideteam.vscode-language-pack-zh-hans').update('1.68.3');
	// console.log(md5.digest('hex'));//结果 hash: 'd40608d6c84271a3245f89217f02df0a',
	const languagePackEntry: ILanguagePack = {
		hash: hash,
		label: localizationContribution.localizedLanguageName ?? localizationContribution.languageName,
		extensions: [],
		translations: {}
	};

	// @ts-ignore
	languagePackEntry.extensions[0] = {
		extensionIdentifier: {
			id: extensionIdentifier,//'kylinideteam.vscode-language-pack-zh-hans',
			uuid: 'e4ee7751-6514-4731-9cdb-7580ffa9e70b'//插件商店随机生成??
		},
		version: requireLanguagePackage.version
	};

	for (const element of languagePackageTranslation) {
		languagePackEntry.translations[element.id] = join(languageExtensionPath, element.path);
	}
	return {
		'zh-cn': languagePackEntry
	};
}

/**
 * The `languagepacks.json` file is a JSON file that contains all metadata
 * about installed language extensions per language. Specifically, for
 * core (`vscode`) and all extensions it supports, it points to the related
 * translation files.
 *
 * The file is updated whenever a new language pack is installed or removed.
 */
async function getLanguagePackConfigurations(userDataPath: string, userLocale: string, nlsMetadataPath: string): Promise<ILanguagePacks | undefined> {
	const configFile = join(userDataPath, 'languagepacks.json');
	try {
		if (userLocale !== 'zh-cn') {
			return JSON.parse(await promises.readFile(configFile, 'utf-8'));
		}
		if (!await Promises.exists(configFile)) {
			console.log('languagepacks.json not exist');
			const languagePack = generateLanguagePacks(nlsMetadataPath);
			return languagePack;
		} else {
			const config = JSON.parse(await promises.readFile(configFile, 'utf-8'));

			const locale = resolveLanguagePackLanguage(config, userLocale);
			if (!locale) {
				return config;
			}
			const packConfig = config[locale];
			if (!packConfig || typeof packConfig.hash !== 'string') {
				return config;
			}
			if (!(packConfig.translations.vscode).startsWith(process.resourcesPath)) {
				console.log('[wpl]: resourcesPath incorrect then generateLanguagePacks');
				const languagePack = generateLanguagePacks(nlsMetadataPath);
				return languagePack;
			}
			if (packConfig.hash !== '9483d8789026fdfc34df4768731699eb' && locale === 'zh-cn') {
				const languagePack = generateLanguagePacks(nlsMetadataPath);
				return languagePack;
			}
			return config;
		}
		// return JSON.parse(await promises.readFile(configFile, 'utf-8'));
	} catch (err) {
		console.log('getLanguagePackConfigurations error:' + err);
		return undefined; // Do nothing. If we can't read the file we have no language pack config.
	}
}

function resolveLanguagePackLanguage(languagePacks: ILanguagePacks, locale: string | undefined): string | undefined {
	try {
		while (locale) {
			if (languagePacks[locale]) {
				return locale;
			}

			const index = locale.lastIndexOf('-');
			if (index > 0) {
				locale = locale.substring(0, index);
			} else {
				return undefined;
			}
		}
	} catch (error) {
		console.error('Resolving language pack configuration failed.', error);
	}

	return undefined;
}

function defaultNLSConfiguration(userLocale: string, osLocale: string, nlsMetadataPath: string): INLSConfiguration {
	mark('code/didGenerateNls');

	return {
		userLocale,
		osLocale,
		resolvedLanguage: 'en',
		defaultMessagesFile: join(nlsMetadataPath, 'nls.messages.json'),

		// NLS: below 2 are a relic from old times only used by vscode-nls and deprecated
		locale: userLocale,
		availableLanguages: {}
	};
}

//#region fs helpers

function touch(path: string): Promise<void> {
	const date = new Date();

	return promises.utimes(path, date, date);
}

//#endregion
