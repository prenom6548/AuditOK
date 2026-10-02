/*----------------------------------------------------------------------------
* Copyright (C) 2023, KylinSoft Co., Ltd.All rights reserved.
* 创建工具条，工具条具有显示隐藏功能，在工具条上创建快捷按钮并能响应动作。
* --------------------------------------------------------------------------- */

import './media/toolbarpart.css';
import { localize } from '../../../../nls.js';
import { ICommandService } from '../../../../platform/commands/common/commands.js';
import { IInstantiationService } from '../../../../platform/instantiation/common/instantiation.js';
import { IStorageService } from '../../../../platform/storage/common/storage.js';
import { IThemeService, registerThemingParticipant } from '../../../../platform/theme/common/themeService.js';
import { Part } from '../../../../workbench/browser/part.js';
import { MENUBAR_SELECTION_BACKGROUND, MENUBAR_SELECTION_BORDER, TITLE_BAR_ACTIVE_BACKGROUND, TITLE_BAR_BORDER, WORKBENCH_BACKGROUND } from '../../../../workbench/common/theme.js';
import { IWorkbenchLayoutService, Parts } from '../../../../workbench/services/layout/browser/layoutService.js';
import { URI } from '../../../../base/common/uri.js';
import { registerColor, transparent } from '../../../../platform/theme/common/colorRegistry.js';
import { Color } from '../../../../base/common/color.js';
import { addDisposableListener, EventHelper, EventType, $, getWindow } from '../../../../base/browser/dom.js';
import { registerSingleton, InstantiationType } from '../../../../platform/instantiation/common/extensions.js';
import { FileAccess } from '../../../../base/common/network.js';
import { IToolBarService } from '../../../../workbench/services/toolbar/common/toolBarService.js';
import { StandardMouseEvent } from '../../../../base/browser/mouseEvent.js';
import { IAction, toAction } from '../../../../base/common/actions.js';
import { IContextMenuService } from '../../../../platform/contextview/browser/contextView.js';
import { ToggleToolBarVisibilityAction } from '../../../../workbench/browser/actions/layoutActions.js';
import { Codicon } from '../../../../base/common/codicons.js';
import { ThemeIcon } from '../../../../base/common/themables.js';
import { isWeb } from '../../../../base/common/platform.js';
import { disposeIfDisposable } from '../../../../base/common/lifecycle.js';
import { GestureEvent } from '../../../../base/browser/touch.js';
import { IContextKeyService } from '../../../../platform/contextkey/common/contextkey.js';
// import { isMacintosh } from '../../../../base/common/platform.js';
// import { OpenFileFolderAction, OpenFolderAction } from 'vs/workbench/browser/actions/workspaceActions';
interface ToolItem {
	url: URI;
	command: string;
	arg: string;
	title: string;
	icon?: ThemeIcon;
	className?: string;
}

export class ToolbarPart extends Part implements IToolBarService {
	declare readonly _serviceBrand: undefined;

	readonly minimumWidth: number = 0;
	readonly maximumWidth: number = Number.POSITIVE_INFINITY;
	readonly minimumHeight: number = 34;
	readonly maximumHeight: number = 34;

	//要插入的 创建工程 的配置
	private static readonly createProjectItem: ToolItem = {
		url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/createProject.svg'),
		command: 'project.createProject',
		arg: '',
		title: localize('newProject', 'New Project'),
		className: 'create-project'
	};

	//要插入的 创建工程 的配置
	private static readonly drawioItem: ToolItem = {
		url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/drawio.svg'),
		command: 'hediet.vscode-drawio.newDiagram',
		arg: '',
		title: localize('newDrawioDiagram', 'New Draw.io Diagram'),
		className: 'drawio'
	};

	private static readonly toolItems: ToolItem[] = [
		// 新建文本文件
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/newfile.svg'),
			command: 'workbench.action.files.newUntitledFile',
			arg: '',
			title: localize('newFile', 'New Text File') + '(Ctrl+N)',
			icon: Codicon.newFile
		},
		// 新建窗口
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/newWindow.svg'),
			command: 'workbench.action.newWindow',
			arg: '',
			title: localize('newWindow', 'New Window') + '(Ctrl+Shift+N)',
			icon: Codicon.emptyWindow
		},
		// //打开文件夹,有问题
		// {
		// 	url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/create.svg'),
		// 	command: isMacintosh ? 'workbench.action.files.openFileFolder' : 'workbench.action.files.openFolder',
		// 	arg: '',
		// 	title: localize('openFolder', 'Open Folder') + '(Ctrl+K Ctrl+O)',
		// 	icon: Codicon.folderOpened
		// },
		// 撤销
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/cancel.svg'),
			command: 'undo',
			arg: '',
			title: localize('undo', 'Undo') + '(Ctrl+Z)',
			icon: Codicon.discard
		},
		//恢复
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/redo.svg'),
			command: 'redo',
			arg: '',
			title: localize('redo', 'Redo') + '(Ctrl+Y)',
			icon: Codicon.redo
		},
		// 返回
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/back.svg'),
			command: 'workbench.action.navigateBack',
			arg: '',
			title: localize('navigatePrevious', 'Go Back') + '(Ctrl+Alt+-)',
			icon: Codicon.arrowLeft//Codicon.chevronLeft
		},
		//前进
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/goahead.svg'),
			command: 'workbench.action.navigateForward',
			arg: '',
			title: localize('navigateNext', 'Go Forward') + '(Ctrl+Shift+-)',
			icon: Codicon.arrowRight//Codicon.chevronRight
		},
		// 向上移动一行
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/moveLinesUp.svg'),
			command: 'editor.action.moveLinesUpAction',
			arg: '',
			title: localize('moveLinesUp', 'moveLinesUp') + '(Alt+RightArrow)',
			icon: Codicon.arrowUp
		},
		// 向下移动一行
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/moveLinesDown.svg'),
			command: 'editor.action.moveLinesDownAction',
			arg: '',
			title: localize('moveLinesDown', 'moveLinesDown') + '(Alt+LeftArrow)',
			icon: Codicon.arrowDown
		},
		//打开explorer
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/explorer.svg'),
			command: 'workbench.view.explorer',
			arg: '',
			title: localize('explorer', 'explorer') + '(Ctrl+Shift+E)',
			icon: Codicon.files
		},
		//开启搜索
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/search.svg'),
			command: 'workbench.view.search',
			arg: '',
			title: localize('Search', 'Search') + '(Ctrl+Shift+F)',
			icon: Codicon.search
		},
		//打开源代码管理
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/scm.svg'),
			command: 'workbench.view.scm',
			arg: '',
			title: localize('Scm', 'Source Control') + '(Ctrl+Shift+G G)',
			icon: Codicon.sourceControl
		},
		//打开debug
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/debug.svg'),
			command: 'workbench.view.debug',
			arg: '',
			title: localize('Run and Debug', 'Run and Debug') + '(Ctrl+Shift+D)',
			icon: Codicon.debugAlt
		},
		//开启插件
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/extensions.svg'),
			command: 'workbench.view.extensions',
			arg: '',
			title: localize('Extensions', 'Extensions') + '(Ctrl+Shift+X)',
			icon: Codicon.extensions
		},
		//开启关闭终端
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/command.svg'),
			command: 'workbench.action.terminal.toggleTerminal',
			arg: '',
			title: localize('Terminal', 'Terminal') + '(Ctrl+`)',
			icon: Codicon.terminal
		},
		//打开键盘快捷方式
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/keyboardShortcuts.svg'),
			command: 'workbench.action.openGlobalKeybindings',
			arg: '',
			title: localize('KeyboardShortcuts', 'Keyboard Shortcuts') + '(Ctrl+K Ctrl+S)',
			icon: Codicon.keyboard
		},
		//打开设置
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/config-dark.svg'),
			command: 'workbench.action.openSettings',
			arg: '',
			title: localize('Settings', 'Settings') + '(Ctrl+,)',
			icon: Codicon.gear
		},
		//图片问题svg不显示，注释
		// {
		// 	url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/siderbar.svg'),
		// 	command: 'workbench.action.toggleSidebarVisibility',
		// 	arg: '',
		// 	title: localize('Siderbar', 'Siderbar') + '(Ctrl+B)',
		// 	icon: undefined
		// },
		// {
		// 	url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/panel.svg'),
		// 	command: 'workbench.action.togglePanel',
		// 	arg: '',
		// 	title: localize('Panel', 'Panel') + '(Ctrl+J)',
		// 	icon: undefined
		// },
		//放大窗体
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/zoomIn.svg'),
			command: 'workbench.action.zoomIn',
			arg: '',
			title: localize('ZoomIn', 'ZoomIn') + '(Ctrl+=)',
			icon: Codicon.zoomIn
		},
		//缩小窗体
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/zoomOut.svg'),
			command: 'workbench.action.zoomOut',
			arg: '',
			title: localize('ZoomOut', 'ZoomOut') + '(Ctrl+-)',
			icon: Codicon.zoomOut
		},
		//全屏模式
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/screenFull.svg'),
			command: 'workbench.action.toggleFullScreen',
			arg: '',
			title: localize('FullScreen', 'FullScreen') + '(F11)',
			icon: Codicon.screenFull
		},
		//禅模式
		{
			url: FileAccess.asFileUri('vs/workbench/browser/parts/toolbar/media/zenMode.svg'),
			command: 'workbench.action.toggleZenMode',
			arg: '',
			title: localize('ZenMode', 'ZenMode') + '(Ctrl+K Z)',
			icon: Codicon.target
		}
	];

	constructor(
		@IStorageService storageService: IStorageService,
		@IThemeService themeService: IThemeService,
		@IWorkbenchLayoutService layoutService: IWorkbenchLayoutService,
		@IInstantiationService protected readonly instantiationService: IInstantiationService,
		@ICommandService private readonly commandService: ICommandService,
		@IContextMenuService private contextMenuService: IContextMenuService,
		@IContextKeyService private contextKeyService: IContextKeyService,
	) {
		super(Parts.TOOLBAR_PART, { hasTitle: false }, themeService, storageService, layoutService);

		const projectManagerKeys = new Set<string>();
		// 添加监听创建工程插件的key值，是否设置
		projectManagerKeys.add('hasProjectManager');

		const drawioKeys = new Set<string>();
		// 添加监听创建工程插件的key值，是否设置
		drawioKeys.add('hasDrawio');
		// 注册contextKeyService服务的监听事件，监听设置的key是否改变
		this._register(this.contextKeyService.onDidChangeContext(e => {
			if (e.affectsSome(projectManagerKeys)) {
				//创建工程的按钮
				if (this.contextKeyService.getContextKeyValue('hasProjectManager')) {
					var container = this.element.getElementsByClassName('toolbar-title')[0] as HTMLElement;
					// 创建新的div元素
					var newDiv = this.createCreateProjectHtmlElement(ToolbarPart.createProjectItem);

					// 插入到第n个位置，假设n是索引（从0开始）
					var n = 1; // 例如，在第2个div后面插入（索引为1）
					var refDiv = container.children[n];
					container.insertBefore(newDiv, refDiv.nextElementSibling);
				} else {
					const container = this.element.getElementsByClassName('toolbar-title')[0] as HTMLElement;
					const projectManagerElement = this.element.getElementsByClassName(ToolbarPart.createProjectItem.className!)[0] as HTMLElement;
					if (projectManagerElement) {
						container.removeChild(projectManagerElement);
					}
				}
			}
			if (e.affectsSome(drawioKeys)) {
				if (this.contextKeyService.getContextKeyValue('hasDrawio')) {
					const container = this.element.getElementsByClassName('toolbar-title')[0] as HTMLElement;
					// 创建新的div元素
					const newDiv = this.createCreateProjectHtmlElement(ToolbarPart.drawioItem);
					// container.appendChild(newDiv);

					// 插入到第n个位置，假设n是索引（从0开始）
					const n = 1; // 例如，在第2个div后面插入（索引为1）
					const refDiv = container.children[n];
					container.insertBefore(newDiv, refDiv.nextElementSibling);
				} else {
					const container = this.element.getElementsByClassName('toolbar-title')[0] as HTMLElement;
					const draioElement = this.element.getElementsByClassName(ToolbarPart.drawioItem.className!)[0] as HTMLElement;
					if (draioElement) {
						container.removeChild(draioElement);
					}
				}

			}
		}));
	}

	override createContentArea(parent: HTMLElement): HTMLElement {
		this.element = parent;
		const content = document.createElement('div');
		content.classList.add('toolbar-title');
		this.element.appendChild(content);

		//遍历创建item
		ToolbarPart.toolItems.forEach(item => {
			//添加分隔竖线
			if (item.command === 'undo' || item.command === 'workbench.view.explorer' || item.command === 'workbench.action.zoomIn') {
				const toolitem = document.createElement('div');
				toolitem.style.margin = '0 5px 0 5px';
				toolitem.style.height = '23px';
				toolitem.style.border = '0.5px solid';
				toolitem.style.borderColor = '#5C5C5C';
				content.appendChild(toolitem);
			}
			//web平台不显示图标
			if (isWeb && (item.command === 'workbench.action.zoomIn' || item.command === 'workbench.action.zoomOut')) {
				return;
			}
			//创建工具栏目
			this.createToolItem(content, item);
		});
		this.updateStyles();

		// Context menu support 鼠标右键菜单
		this._register(addDisposableListener(parent, EventType.CONTEXT_MENU, e => this.showContextMenu(e)));
		return this.element;
	}

	private createToolItem(parent: HTMLElement, item: ToolItem): void {
		const toolitem = document.createElement('div');
		toolitem.classList.add('tool-item');

		const toolitemicon = document.createElement('div');
		toolitemicon.classList.add('tool-item-icon');

		//没有使用url的图片地址，使用IDE中的默认icon
		if (item.icon === undefined) {
			if (item.className) {
				const img = document.createElement('img');
				img.title = item.title;
				img.classList.add(item.className);
				img.src = item.url.fsPath;
				toolitemicon.appendChild(img);
			}
		} else {
			const node = $(`span`);
			node.title = item.title;
			node.classList.add(...ThemeIcon.asClassNameArray(item.icon));
			toolitemicon.appendChild(node);
		}

		toolitem.appendChild(toolitemicon);
		parent.appendChild(toolitem);
		if (item.command.length > 0) {
			this._register(addDisposableListener(toolitem, EventType.MOUSE_UP, _e => {
				if (item.arg.length > 0) {
					this.commandService.executeCommand(item.command, item.arg);
				}
				else {
					this.commandService.executeCommand(item.command);
				}
			}));
		}
	}

	//创建要插入的'创建工程'html元素
	private createCreateProjectHtmlElement(item: ToolItem): HTMLElement {
		const toolitem = document.createElement('div');
		toolitem.classList.add('tool-item');

		const toolitemicon = document.createElement('div');
		toolitemicon.classList.add('tool-item-icon');

		const iconDiv = document.createElement('div');
		iconDiv.title = item.title;
		iconDiv.classList.add(item.className!);//这里对应css中的class
		toolitemicon.appendChild(iconDiv);

		toolitem.appendChild(toolitemicon);
		if (item.command.length > 0) {
			this._register(addDisposableListener(toolitem, EventType.CLICK, _e => {
				if (item.arg.length > 0) {
					this.commandService.executeCommand(item.command, item.arg);
				} else {
					this.commandService.executeCommand(item.command);
				}
			}));
		}
		return toolitem;
	}

	//显示右键菜单内容
	private showContextMenu(e: MouseEvent | GestureEvent): void {
		EventHelper.stop(e, true);

		const event = new StandardMouseEvent(getWindow(this.element), e);

		let actions: IAction[] | undefined = undefined;
		this.contextMenuService.showContextMenu({
			getAnchor: () => ({ x: event.posx, y: event.posy }),
			getActions: () => {
				actions = this.getContextMenuActions(event);

				return actions;
			},
			onHide: () => {
				if (actions) {
					disposeIfDisposable(actions);
				}
			}
		});
	}

	//add context action 填充鼠标右键菜单的事件
	private getContextMenuActions(_event: StandardMouseEvent): IAction[] {
		const actions: IAction[] = [];

		// Toggle Tool Bar
		actions.push(toAction({ id: ToggleToolBarVisibilityAction.ID, label: localize('hideToolBar', 'Hide Tool Bar'), run: async () => this.instantiationService.invokeFunction(accessor => new ToggleToolBarVisibilityAction().run(accessor)) }));

		// 此处可添加右键菜单
		return actions;
	}

	override updateStyles() {
		super.updateStyles();
		// Part container
		if (this.element) {
			const TOOL_BAR_BACKGROUND = registerColor('toolBar.background', {
				dark: transparent(TITLE_BAR_ACTIVE_BACKGROUND, 0.7),
				light: transparent(TITLE_BAR_ACTIVE_BACKGROUND, 0.8),
				hcDark: Color.black, hcLight: Color.white
			}, localize('toolBarBackground', 'Tool bar background'));
			const toolbarBackground = this.getColor(TOOL_BAR_BACKGROUND, (color, theme) => {
				return color.isOpaque() ? color : color.makeOpaque(WORKBENCH_BACKGROUND(theme));
			}) || '';
			this.element.style.backgroundColor = toolbarBackground;
			if (toolbarBackground && Color.fromHex(toolbarBackground).isLighter()) {
				this.element.classList.add('light');
			} else {
				this.element.classList.remove('light');
			}
			const toolbarBorder = this.getColor(TITLE_BAR_BORDER);
			this.element.style.borderBottom = toolbarBorder ? `1px solid ${toolbarBorder}` : '';
		}
	}

	override layout(width: number, height: number): void {
		super.layoutContents(width, height);
	}

	toJSON(): object {
		return {
			type: Parts.TOOLBAR_PART
		};
	}
}

registerThemingParticipant((theme, collector) => {
	const toolBarHoverBg = theme.getColor(MENUBAR_SELECTION_BACKGROUND);
	if (toolBarHoverBg) {
		collector.addRule(`
	.tool-item-icon:hover {
		background: ${toolBarHoverBg};
		border: 0px solid;
	}
	`);
	}
	const toolBarActiveBg = theme.getColor(TITLE_BAR_ACTIVE_BACKGROUND);
	if (toolBarActiveBg) {
		collector.addRule(`
	.tool-item-icon:active {
		background: ${toolBarActiveBg};
		}
	`);
	}
	const toolBarHoverBd = theme.getColor(MENUBAR_SELECTION_BORDER);
	if (toolBarHoverBd) {
		collector.addRule(`
	.tool-item-icon:hover {
		border-color: ${toolBarHoverBd};
		}
	`);
	}
});

registerSingleton(IToolBarService, ToolbarPart, InstantiationType.Eager);
