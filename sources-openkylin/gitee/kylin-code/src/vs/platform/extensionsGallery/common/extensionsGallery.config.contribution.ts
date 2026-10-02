/*---------------------------------------------------------------------------------------------
 *  Copyright (c) kylinIDETeam. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { localize } from '../../../nls.js';
import { ConfigurationScope, Extensions as ConfigurationExtensions, IConfigurationRegistry } from '../../configuration/common/configurationRegistry.js';
import { Registry } from '../../registry/common/platform.js';


const configurationRegistry = Registry.as<IConfigurationRegistry>(ConfigurationExtensions.Configuration);
configurationRegistry.registerConfiguration({
	id: 'extensionsGallery',
	order: 199,
	title: localize('extensionsGalleryConfigurationTitle', "extensionsGallery"),
	type: 'object',
	properties: {
		'extensionsGallery.serviceUrl': {
			type: 'string',
			pattern: '^(?:https?|ftp):\/\/(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,6}|^(?:https?|ftp):\/\/(?:\d{1,3}\.){3}\d{1,3}|^(?:https?|ftp):\/\/localhost(?::\d+)?(?:\/[^\s]*)?|^$',
			default: '',
			scope: ConfigurationScope.APPLICATION,
			description: localize('extensionsGalleryServiceUrl', "Configure Extensions Gallery ServiceUrl.Sample:https://open-vsx.org/vscode/gallery"),
			restricted: true
		},
		'extensionsGallery.itemUrl': {
			type: 'string',
			pattern: '^(?:https?|ftp):\/\/(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,6}|^(?:https?|ftp):\/\/(?:\d{1,3}\.){3}\d{1,3}|^(?:https?|ftp):\/\/localhost(?::\d+)?(?:\/[^\s]*)?|^$',
			default: '',
			scope: ConfigurationScope.APPLICATION,
			description: localize('extensionsGalleryItemUrl', "Configure Extensions Gallery ItemUrl.Sample:https://open-vsx.org/vscode/item"),
			restricted: true,

		},
		'extensionsGallery.trustDomains': {
			type: 'array',
			items: {
				type: 'string'
			},
			default: [],
			description: localize('extensionsGalleryTrustDomains', "Configure Trusted Domains,Sample:https://open-vsx.org"),
			scope: ConfigurationScope.RESOURCE
		}
	},
});

export interface IExtensionsGalleryConfiguration {
	serviceUrl: string;
	itemUrl: string;
	trustDomains: string[];
}
