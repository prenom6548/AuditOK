/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "ondeviceengine.h"

#include <kylin-ai/plugins/ai-engines/ondevicenlpengine.h>
#include <nlohmann/json.hpp>
#include <ondeviceengine.h>
// #include "base64_transformer.h"
#include <chrono>
#include <iostream>
#include <thread>
std::string res_all = "";
std::string res_all2 = "";
void ocrPrintResult(const ai_engine::gen_ai::nlp::TextGenerationResult result) {

    nlohmann::json bodys = nlohmann::json::parse(result.text.c_str());
    printf("1 result.text:%s\n", result.text.c_str());
    res_all += bodys["result"].get<std::string>().c_str();
    printf("2 res_all:%s\n", res_all.c_str());
}

int main(int argc, char **argv) {
    ai_engine::gen_ai::nlp::OndeviceNlpEngine *nlp_p_ =
        new ai_engine::gen_ai::nlp::OndeviceNlpEngine();
    ai_engine::result::AiEngineBaseResult res;
    nlp_p_->setTextGenerationResultCallback(ocrPrintResult);
    std::string model_info = nlp_p_->modelInfo();
    printf("main:model_info:%s\n", model_info.c_str());

    ai_engine::EngineError error2;
    std::string model_name = "Qwen-2.5-3b_1.0";

    std::string message = "写一篇介绍triton的文章";
    std::string chatPrompt1 =
        R"({"task_id": -1, "messages": [{"role": "system", "content": "无"},{"role": "user", "content": "写一篇介绍triton的文章"}],"verbose": true}')";
    std::string chatPrompt2 = R"({"task_id": -1,
	"messages" : 
	[
		{
			"content" : "请根据输入的文档内容，对用户的提问进行解答。",
			"role" : "system"
		},
		{
			"content" : "如何使用看图",
			"role" : "user"
		},
		{
			"content" : "<h2>概述</h2><p>看图是一款简单易用，界面友好，支持打开多种图像格式的图像查看器。在查看图像的同时，还可以对图像进行简单的编辑和处理，支持图像缩放比例选择、自适应窗口显示、图像裁剪、图像镜像翻转等功能。</p><br>根据上述内容，可以得知看图这款软件支持图像缩放比例选择、自适应窗口显示、图像裁剪、图像镜像翻转等功能。\n\n<h2>打开方式</h2><ul><li>“<strong>开始菜单</strong>”<imgsrc=\"image/1.png\"alt=\"\"/>&gt;“<strong>看图</strong>”<imgsrc=\"image/2.png\"alt=\"\"/>&gt;；</li><li>选中图像&gt;双击&gt;默认打开&gt;“<strong>看图</strong>”<imgsrc=\"image/2.png\"alt=\"\"/>&gt;；</li><li>选中图像&gt;右键单击&gt;“<strong>打开方式</strong>”&gt;“<strong>看图</strong>”<imgsrc=\"image/2.png\"alt=\"\"/>&gt;；</li><li>“<strong>任务栏</strong>”&gt;“<strong>搜索</strong>”<imgsrc=\"image/5.png\"alt=\"\"/>&gt;“<strong>看图</strong>”<imgsrc=\"image/2.png\"alt=\"\"/>&gt;选择“<strong>打开</strong>”。</li></ul>根据上述内容，可以得知看图的打开方式有四种，分别是：1. “开始菜单”>“看图”；2. 选中图像>双击>默认打开>“看图”；3. 选中图像>右键单击>“打开方式”>“看图”；4. “任务栏”>“搜索”>“看图”>选择“打开”。\n\n<h2>基本操作</h2><h3>打开图片</h3><p>​首次打开看图，点击“<strong>载入图片</strong>”或者点击“<strong>打开</strong>”可打开图片。</p><br>根据上述内容，可以得知首次打开看图，点击“载入图片”或者点击“打开”可打开图片。\n\n<h2>编辑图片</h2><p>看图工具栏功能说明</p><p>|二级菜单|功能|描述||-------|----|----||<imgsrc=\"image/21.png\"alt=\"\"/>|调整试图比例|放大、缩小图像||<imgsrc=\"image/22.png\"alt=\"\"/>|适应窗口|调整图像适应窗口||<imgsrc=\"image/23.png\"alt=\"\"/>|原尺寸|以原尺寸显示图像||<imgsrc=\"image/24.png\"alt=\"\"/>|向右旋转|图像顺时针旋转90°||<imgsrc=\"image/25.png\"alt=\"\"/>|水平镜像|水平翻转图像||<imgsrc=\"image/26.png\"alt=\"\"/>|垂直镜像|垂直翻转图像||<imgsrc=\"image/27.png\"alt=\"\"/>|剪裁|自定义剪裁图像||<imgsrc=\"image/31.png\"alt=\"\"/>|OCR|文字识别||<imgsrc=\"image/28.png\"alt=\"\"/>|侧边栏|打开/关闭侧边栏||<imgsrc=\"image/29.png\"alt=\"\"/>|删除图像|删除图像||<imgsrc=\"image/30.png\"alt=\"\"/>|重命名|重命名图像||<imgsrc=\"image/32.png\"alt=\"\"/>|在文件夹中显示|显示图像所在文件夹位置。</p><br>根据上述内容，可以得知看图工具栏功能有：调整试图比例、适应窗口、原尺寸、向右旋转、水平镜像、垂直镜像、剪裁、OCR、侧边栏、删除图像、重命名、在文件夹中显示。",
			"role" : "assistant"
		},
		{
			"content" : "<h1>安全中心</h1><h2>概述</h2><h3>产品简介</h3><p>安全中心是一款基于麒麟安全框架KYSEC的管理工具，提供系统安全体检、账户保护配置、防火墙配置、联网控制、病毒防护、应用来源检查、应用执行控制、应用防护和外设管控等功能，保障系统运行环境的安全和稳定。</p><h3>产品亮点</h3><p>安全中心集安全体检、账户保护、网络保护、病毒防护、应用保护和设备安全等功能于一体，全面保障系统运行环境的安全。</p><ul><li>安全体检：提供账户安全、运行安全、系统安全等多维度的扫描与一键修复，可自动更新漏洞补丁，及时发现系统安全隐患；</li><li>账户保护：提供系统账户密码强度检查和账户锁定机制，实现对系统账户的统一管控，提升系统账户安全防御能力，有效防止密码被暴力破解；</li><li>网络保护：提供防火墙和应用联网控制功能，实时防护未知应用网络行为，阻断主动外联及其它异常网络活动，提高网络访问安全性；</li><li>病毒防护：提供杀毒软件的统一管理入口，能够兼容多款主流的病毒防护软件，实时防护病毒入侵，保护系统安全；</li><li>应用控制：提供应用来源检查和应用执行控制功能，阻止未知软件、应用程序的非法安装和恶意执行，避免木马病毒攻击，保障系统运行环境的安全可靠；</li><li>应用防护：提供进程防杀死、内核模块防卸载和文件防篡改功能，保护系统关键文件完整性，阻止系统关键应用服务异常中断。</li><li>设备安全：提供外部设备接入的多重防护控制功能，有效防止系统重要数据意外丢失和恶意盗取。</li></ul><h2>使用入门</h2><h3>软件位置</h3><p>点击操作系统“开始菜单”，选择并点击“安全中心”，打开安全中心软件界面，如图1所示。</p><p><imgsrc=\"image/1.png\"alt=\"图1开始菜单（打开。安全中心）-big\"/></p><p>点击操作系统“开始菜单”，再点击“设置”，再点击“安全”，显示安全中心首页。点击不同模块的“查看详情”按钮，可以直接进入安全中心软件对应模块界面，如图2所示。</p><p><imgsrc=\"image/2.png\"alt=\"图2设置（打开安全中心）-big\"/></p><h3>获取帮助</h3><p>安全中心软件界面中点击菜单图标，点击“帮助”按钮，弹出“用户手册”弹窗，显示安全中心的帮助手册相关信息，如图3所示。</p><p><imgsrc=\"image/3.png\"alt=\"图3用户手册-big\"/></p><h3>模式支持</h3><p>安全中心软件界面显示模式随系统主题变化，开始菜单—&gt;设置—&gt;个性化—&gt;主题进行模式的切换。如图4至5所示。</p><p><imgsrc=\"image/4.png\"alt=\"图4浅色模式-big\"/></p><p><imgsrc=\"image/5.png\"alt=\"图5深色模式-big\"/></p><h3>语言支持</h3><p>安全中心软件界面默认跟随系统语言设置，支持中文、英文和藏文，如图6至8所示。</p><p><imgsrc=\"image/6.png\"alt=\"图6中文显示-big\"/></p><p><imgsrc=\"image/7.png\"alt=\"图7英文显示-big\"/></p><p><imgsrc=\"image/8.png\"alt=\"图8藏文显示-big\"/></p><h2>安全体检</h2><p>系统从未体检时，安全体检首页显示如图9所示。</p><p><imgsrc=\"image/9.png\"alt=\"图9安全体检首页（从未体检）-big\"/></p><p>系统体检过后，安全体检首页显示上次体检时间、体检持续时间、发现风险问题项数和体检扫描项数等信息，如图10所。【操作方法】\n步骤1：在开始菜单中，搜索框中输入关键字找到安全中心软件，然后鼠标点击“安全中心”软件图标，即可打开安全中心软件。\n步骤2：在安全中心软件首页，点击该界面右上角“自定义”按钮，然后在“切换系统安全模式”弹窗界面，勾选“关闭”选项，再点击“确认”按钮即可。。统上安装指定的软件包且硬件支持安全内存时，安全中心将提供安全内存功能。点击左侧菜单栏中“安全内存”标签页进入，如图73所示。</p><p><imgsrc=\"image/72.png\"alt=\"图73安全内存首页-big\"/></p><h2>附录：常见问题及处理方法(FAQ)</h2><h5>一、安全体检</h5><ul><li>如何进行应用完整性的修复？</li></ul><p>答：在安全体检中暂不提供修复方案，用户可在应用保护-应用执行控制-高级设置中查看文件列表，对已篡改的文件进行认证或解除，对已损坏的文件则需重新安装对应的软件。</p><ul><li>如何进行文件完整性的修复？</li></ul><p>答：在安全体检中暂不支持修复方案，用户可在扫描结束页查看风险详情，根据详情提示，手动对相应文件进行修改。</p><ul><li>如何进行漏洞更新和系统更新？</li></ul><p>答:当安全体检提示系统未更新或漏洞未更新时，用户可根据提示前往“设置”，进行系统或漏洞更新。当“设置”无法更新时，用户可根据提示手动安装对应版本的漏洞包，以修复漏洞风险项。</p><ul><li>为什么修复进程保护后，应用保护-应用防护控制仍为关闭状态？</li></ul><p>答：应用保护中的应用防护控制包含三个功能（进程保护，模块防卸载，文件保护），当且仅当三个功能均开启时，应用防护控制才显示为开启状态。在安全体检中进行三个模块的修复，即可开启应用程序控制。</p><ul><li>为什么进行安全修复后，无属主和属组的文件消失了？</li></ul><p>答：当检测出属主与属组异常文件时，判定该文件存在被调用获取管理员权限风险，修复后，会将这些文件删除。为避免误删文件，用户可在修复前查看风险详情，对无属主与无属组文件进行检查。</p><h5>二、账户保护</h5><ul><li>密码输错次。\nquestion: 如何使用安全中心\n",
			"role" : "user"
		}
	]}')";
    std::string chatPrompt3 =
        R"({"task_id": 1, "messages": [{"role": "system", "content": "你是一个AIPC的意图识别助理，你的任务是按照KYLIN_INTENT_DETECT的要求，结合上下文对话记录和用户输入进行意图识别和槽位信息抽取。"},{"role": "user", "content": "请帮我调低电脑的亮度"}],"verbose": true}')";

    res = nlp_p_->chat(chatPrompt1);
    nlp_p_->stopChat();

    return 0;
}
