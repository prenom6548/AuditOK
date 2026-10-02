/*----------------------------------------------------------------------------
* Copyright (C) 2023, KylinSoft Co., Ltd.All rights reserved.
*
* 创建工具条服务类。
* --------------------------------------------------------------------------- */

import { createDecorator } from '../../../../platform/instantiation/common/instantiation.js';

export const IToolBarService = createDecorator<IToolBarService>('toolbarService');

export interface IToolBarService {

	readonly _serviceBrand: undefined;

}
