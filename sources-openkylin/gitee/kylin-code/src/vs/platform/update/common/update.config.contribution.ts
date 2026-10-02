/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

// import { isWeb, isWindows } from '../../../base/common/platform.js';
import { localize } from '../../../nls.js';
import { ConfigurationScope, Extensions as ConfigurationExtensions, IConfigurationRegistry } from '../../configuration/common/configurationRegistry.js';
import { Registry } from '../../registry/common/platform.js';

const configurationRegistry = Registry.as<IConfigurationRegistry>(ConfigurationExtensions.Configuration);
configurationRegistry.registerConfiguration({
	id: 'update',
	order: 15,
	title: localize('updateConfigurationTitle', "Update"),
	type: 'object',
	properties: {
		'update.mode': {
			type: 'string',
			enum: ['enable', 'disable'],
			default: 'disable',
			scope: ConfigurationScope.APPLICATION,
			description: localize('updateMode', "Configure whether you receive automatic updates. Requires a restart after change. The updates are fetched from a Kylin online service.").replace('微软', 'Kylin'),
			tags: ['usesOnlineServices'],
			enumDescriptions: [
				localize('enable', "Enable updates."),
				localize('disable', "Disable automatic background update checks."),
				// localize('start', "Check for updates only on startup. Disable automatic background update checks."),
				// localize('default', "Enable automatic update checks. Code will check for updates automatically and periodically.")
			],
			policy: {
				name: 'UpdateMode',
				minimumVersion: '1.67',
			}
		},
		// 'update.channel': {
		// 	type: 'string',
		// 	default: 'default',
		// 	scope: ConfigurationScope.APPLICATION,
		// 	description: localize('updateMode', "Configure whether you receive automatic updates. Requires a restart after change. The updates are fetched from a Kylin-Code online service."),
		// 	deprecationMessage: localize('deprecated', "This setting is deprecated, please use '{0}' instead.", 'update.mode')
		// },
		// 'update.enableWindowsBackgroundUpdates': {
		// 	type: 'boolean',
		// 	default: true,
		// 	scope: ConfigurationScope.APPLICATION,
		// 	title: localize('enableWindowsBackgroundUpdatesTitle', "Enable Background Updates on Windows"),
		// 	description: localize('enableWindowsBackgroundUpdates', "Enable to download and install new Kylin-Code versions in the background on Windows."),
		// 	included: isWindows && !isWeb
		// },
		// 'update.showReleaseNotes': {
		// 	type: 'boolean',
		// 	default: false,
		// 	scope: ConfigurationScope.APPLICATION,
		// 	description: localize('showReleaseNotes', "Show Release Notes after an update. The Release Notes are fetched from a online service."),
		// 	tags: ['usesOnlineServices']
		// }
	}
});
