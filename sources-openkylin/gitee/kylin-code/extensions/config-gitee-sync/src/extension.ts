import { ExtensionContext } from 'vscode';
import { SyncService } from './sync';


export function activate(context: ExtensionContext) {
	SyncService.init(context);
}

export function deactivate() {
	SyncService.uninit();
}
