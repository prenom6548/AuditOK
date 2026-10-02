#! /usr/bin/env python3

#v2.0版本 
#1、取消filesystem解压场景
#2、安装deb后续两种方式 一种是放入third-party中、一种放到apps-third（解决类似山东通软件安装问题）
#3、需要在安装系统后的后置操作如（添加字体、kms ip、添加证书、免密登录、安全中心设置等操作全部放入.kylin-post-actions）
#   支持自定义编辑（或上传）.kylin-post-actions文件
#4、优化屏幕小界面显示不全等问题
#5、修复桌面图标部分机型不显示问题
#v2.1版本
#优化华为镜像自动识别
#修复华为镜像下无人值守参数设置
#修复百信机器下要求双硬盘模式
#v2.2版本
#优化百信双硬盘策略控件不再依赖于无人值守状态
#新增kms下激活命令更新及sync落盘操作
#取消默认用户名密码主机名设置 
#冗余函数删除  
#修复华为镜像启用双硬盘模式下data-device参数处理问题
#修复华为镜像下取消出厂备份后factory-backup参数处理问题
#v2.3版本
#修复panguw镜像识别华为镜像失败问题
#修复启动kms功能后U盘安装失败问题
#v2.4版本
#修复pangw下grub中无人值守参数问题
#v2.5版本
#修复华为镜像判断中字段添加Wayland
#百信双硬盘模式下格式化数据盘命令优化
#优化镜像中的license文件夹
#v2.6版本
#修复多次封装后.kylin-post-actions文件被刷新问题
#新增kpb补丁集成形式
#新增关闭kid状态处理
#新增hosts文件修改功能
#v2.7版本
#新增字体格式选择功能
#新增third-party和apps-third包选择功能
#新增kylin-post-actions文件自定义功能
#新增自定义功能 --删除squashfs-root文件中的奇安信软件
#v2.8版本
#修复apps-third删除功能异常
#新增自定义文件夹功能
#优化自定义脚本功能
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import os
import subprocess
import configparser
import shutil
import math
import tempfile
import time
import threading
import logging
from pathlib import Path
import zipfile
import re
import sys
import ipaddress

# --- Configuration ---
# Consider moving these to a config file or class
APP_NAME = "ISO镜像定制工具V2.8"
WORK_BASE_DIR = Path("/opt/iso-customizer-work") # Changed from /opt to avoid conflicts
MNT_DIR = WORK_BASE_DIR / "mnt"
LIVECD_DIR = WORK_BASE_DIR / "livecd"
TOOL_DIR = WORK_BASE_DIR / "tool"
SQUASHFS_ROOT = WORK_BASE_DIR / "squashfs-root"
LOG_FILE = Path("/var/log/iso_customizer.log")

# --- Logging Setup ---
def setup_logging():
    """设置日志配置，处理权限问题"""
    try:
        # 尝试创建日志文件目录
        LOG_FILE.parent.mkdir(parents=True, exist_ok=True)
        # 尝试写入日志文件
        LOG_FILE.touch(exist_ok=True)
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(LOG_FILE),
                logging.StreamHandler() # Also log to console
            ]
        )
        logger = logging.getLogger(__name__)
        logger.info("日志系统初始化成功")
        return logger
    except (OSError, PermissionError) as e:
        # 如果无法写入系统日志目录，使用控制台日志
        print(f"警告: 无法写入系统日志文件 {LOG_FILE}: {e}")
        print("将使用控制台日志输出")
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.StreamHandler() # Only console output
            ]
        )
        logger = logging.getLogger(__name__)
        logger.warning("使用控制台日志输出")
        return logger

# 初始化日志
logger = setup_logging()

# --- 添加辅助函数 ---
def resource_path(relative_path):
    """ 获取资源的绝对路径，适用于开发环境和 PyInstaller 打包环境 """
    try:
        # PyInstaller 创建一个临时文件夹并将路径存储在 _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        # 未在 PyInstaller 包中运行，使用脚本所在目录
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)
# --- 辅助函数结束 ---

class ISOCustomizer:
    def __init__(self, root):
        self.root = root
        self.root.title(APP_NAME)
        
        # Check root privileges early but don't quit immediately
        self.has_root_privileges = os.geteuid() == 0
        if not self.has_root_privileges:
            logger.warning("Application started without root privileges.")
            # Show warning but don't quit - let user decide
            messagebox.showwarning("警告", "程序需要root权限才能正常工作。\n某些功能可能无法使用。")

        # Set window icon
        try:
            # 图标文件名 (必须与 install.sh 中 --add-data 添加的文件名一致)
            icon_file = "iso-customizer.png"
            # 获取图标文件的正确路径 (打包后也能找到)
            icon_path = resource_path(icon_file)
            if os.path.exists(icon_path):
                # 使用 PhotoImage 加载 PNG 图标
                img = tk.PhotoImage(file=icon_path)
                # 设置窗口图标, True 表示也为将来可能的子窗口设置默认图标
                self.root.iconphoto(True, img)
                logger.info(f"成功设置窗口图标: {icon_path}")
            else:
                 logger.warning(f"窗口图标文件未找到: {icon_path}")
        except tk.TclError as e:
            # 处理加载图片时可能发生的 Tcl 错误 (例如格式不支持)
            logger.warning(f"无法设置窗口图标 (TclError): {e}")
        except Exception as e:
            # 捕获其他可能的异常
             logger.warning(f"设置窗口图标时发生意外错误: {e}")

        # --- Member Variables ---
        # 移除手动华为机器选择，改为自动检测
        self.is_huawei_detected = False  # 自动检测的华为机器标志
        self.enable_unattended = tk.BooleanVar(value=False)
        self.auto_login = tk.BooleanVar(value=True)
        self.factory_backup = tk.BooleanVar(value=True)
        self.is_reboot = tk.BooleanVar(value=True)
        self.remove_qax = tk.BooleanVar(value=False)  # 是否删除奇安信杀毒软件
        self.custom_squashfs_packages = []  # 自定义squashfs软件包列表

        # 添加安全中心设置变量
        self.disable_firewall = tk.BooleanVar(value=False)
        self.disable_network_protection = tk.BooleanVar(value=False)
        self.disable_app_protection = tk.BooleanVar(value=False)
        self.disable_device_security = tk.BooleanVar(value=False)
        self.disable_signature = tk.BooleanVar(value=False)
        self.disable_process_protection = tk.BooleanVar(value=False)  # 新增进程保护选项
        self.disable_file_protection = tk.BooleanVar(value=False)
        self.disable_kid = tk.BooleanVar(value=False)  # 新增关闭kid状态选项

        # 添加百信双硬盘策略变量（独立于无人值守）
        self.baixinx_dual_disk = tk.BooleanVar(value=False)
        self.baixinx_disk_size = tk.StringVar(value="1000")

        # 添加自定义后置脚本标志
        self.post_actions_modified = False
        self.custom_post_actions_script = ""

        self.iso_path_var = tk.StringVar()
        self.district_name = tk.StringVar()
        self.username_var = tk.StringVar(value="")
        self.password_var = tk.StringVar(value="")
        self.hostname_var = tk.StringVar(value="")
        self.data_device = tk.StringVar()
        self.data_fs_type_var = tk.StringVar()

        self.deb_listbox_third_party = None
        self.deb_listbox_apps_third = None
        self.output_text = None
        self.progress_var = tk.StringVar(value="准备就绪")
        self.start_button = None # To disable/enable during processing
        self.worker_thread = None
        self.custom_licenses = [] # Added to store licenses from popup
        self.custom_kpb_patches = [] # Added to store kpb patch files
        self.custom_hosts_entries = [] # Added to store hosts entries [(ip, domain), ...]
        self.custom_personalization_items = []  # 自定义Personalization文件夹内容（文件或文件夹路径）
        self.custom_functions_status_var = tk.StringVar(value="无自定义配置") # Added for status display
        self._iso_prepared = False  # 标志：ISO是否已经准备好（已执行第0-3步）
        self._iso_prep_thread = None  # ISO准备线程
        self._iso_prep_in_progress = False  # 标志：ISO准备是否正在进行中
        # DEB文件路径映射：{listbox: {显示名称: 完整路径}}
        self._deb_path_mapping = {
            'third_party': {},
            'apps_third': {}
        }

        # --- Setup ---
        try:
            self.setup_directories()
        except Exception as e:
            logger.error(f"Failed to setup directories: {e}")
            if not self.has_root_privileges:
                messagebox.showerror("错误", f"无法创建工作目录: {e}\n请使用root权限运行程序。")
                self.root.quit()
                return
            else:
                messagebox.showerror("错误", f"创建工作目录失败: {e}\n请检查权限或磁盘空间。")
                self.root.quit()
                return
                
        self.style = ttk.Style()
        self.configure_styles()

        # --- GUI Creation ---
        self.main_frame = ttk.Frame(self.root, padding="20")
        self.main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.root.rowconfigure(0, weight=1)
        self.root.columnconfigure(0, weight=1)
        self.create_main_widgets()

        # Define processing steps (for progress reporting)
        self.steps = [
            "清理旧目录", "挂载ISO", "复制镜像文件", "卸载ISO",
            "安装/配置包和授权", "配置无人值守",
            "生成MD5", "封装最终镜像"
        ]

        # 设置窗口大小
      

    def configure_styles(self):
        # Example style configuration (can be expanded)
        self.style.configure('Enabled.TLabel', foreground='black')
        self.style.configure('Disabled.TLabel', foreground='gray')
        # Add more styles as needed

    def setup_directories(self):
        """Creates necessary working directories."""
        logger.info("Setting up working directories...")
        try:
            for dir_path in [WORK_BASE_DIR, MNT_DIR, LIVECD_DIR, TOOL_DIR]:
                dir_path.mkdir(parents=True, exist_ok=True)
                # Set appropriate permissions if needed, e.g., os.chmod(dir_path, 0o755)
            logger.info(f"Working directories created/verified under {WORK_BASE_DIR}")
        except OSError as e:
            logger.error(f"Failed to create directory: {e}")
            if not self.has_root_privileges:
                raise RuntimeError(f"无法创建工作目录: {e}\n请使用root权限运行程序。")
            else:
                raise RuntimeError(f"创建工作目录失败: {e}\n请检查权限或磁盘空间。")

    def _cleanup_working_dirs(self):
        """Cleans up temporary working directories."""
        logger.info("Cleaning up working directories...")
        # Unmount first if necessary
        self._unmount_iso()
        # Remove directories
        for dir_path in [MNT_DIR, LIVECD_DIR, TOOL_DIR, SQUASHFS_ROOT]:
             if dir_path.exists():
                try:
                    # Use shutil.rmtree for non-empty dirs
                    logger.debug(f"Removing directory: {dir_path}")
                    shutil.rmtree(dir_path, ignore_errors=True)
                except Exception as e:
                    logger.warning(f"Could not remove directory {dir_path}: {e}")

    def _unmount_iso(self):
        """Unmounts the ISO if it is mounted."""
        if MNT_DIR.is_mount():
            logger.info(f"Unmounting {MNT_DIR}...")
            try:
                # No sudo needed, running as root
                subprocess.run(['umount', '-lf', str(MNT_DIR)], check=False, capture_output=True)
                logger.info(f"Successfully unmounted {MNT_DIR}")
            except Exception as e:
                logger.error(f"Failed to unmount {MNT_DIR}: {e}")
                # Don't raise here, cleanup should proceed if possible

    def create_main_widgets(self):
        """Creates the main GUI layout and widgets."""
        # Using grid layout manager consistently
        inner_main_frame = ttk.Frame(self.main_frame)
        inner_main_frame.grid(row=0, column=0, sticky=(tk.N, tk.S, tk.E, tk.W))
        self.main_frame.rowconfigure(0, weight=1)
        self.main_frame.columnconfigure(0, weight=1)

        left_frame = ttk.Frame(inner_main_frame)
        right_frame = ttk.Frame(inner_main_frame)
        left_frame.grid(row=0, column=0, sticky=(tk.N, tk.S, tk.W), padx=(0, 10), pady=10)
        right_frame.grid(row=0, column=1, sticky=(tk.N, tk.S, tk.E, tk.W), pady=10)

        inner_main_frame.columnconfigure(0, weight=1) # Adjust weights as needed
        inner_main_frame.columnconfigure(1, weight=2)
        inner_main_frame.rowconfigure(0, weight=1)

        # --- Left Frame Widgets ---
        row_idx = 0
        # ISO Selection
        iso_frame = ttk.LabelFrame(left_frame, text="ISO文件选择", padding="10")
        iso_frame.grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=5)
        iso_frame.columnconfigure(0, weight=1)
        iso_frame.columnconfigure(1, weight=0)

        # 创建一个子框架来包含ISO文件路径和选择按钮
        iso_path_frame = ttk.Frame(iso_frame)
        iso_path_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 5))
        iso_path_frame.columnconfigure(0, weight=1)
        iso_path_frame.columnconfigure(1, weight=0)

        self.iso_path_var = tk.StringVar()
        ttk.Entry(iso_path_frame, textvariable=self.iso_path_var, width=40).grid(row=0, column=0, sticky=(tk.W, tk.E), padx=(0, 5))
        ttk.Button(iso_path_frame, text="浏览", command=self.select_iso).grid(row=0, column=1)

        # 添加华为机器检测状态显示
        huawei_status_frame = ttk.Frame(iso_frame)
        huawei_status_frame.grid(row=1, column=0, sticky=tk.W, pady=(5, 0))
        
        ttk.Label(huawei_status_frame, text="镜像类型: ").pack(side=tk.LEFT)
        self.huawei_status_label = ttk.Label(huawei_status_frame, text="未选择镜像", 
                                           foreground="gray")
        self.huawei_status_label.pack(side=tk.LEFT)

        row_idx += 1

        # DEB Selection (third-party)
        deb_frame_third_party = ttk.LabelFrame(left_frame, text="DEB包选择(third-party)", padding="10")
        deb_frame_third_party.grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=5)
        # Configure columns for this frame
        deb_frame_third_party.columnconfigure(0, weight=0) # Column for Add button
        deb_frame_third_party.columnconfigure(1, weight=0) # Column for Clear button
        deb_frame_third_party.columnconfigure(2, weight=1) # Column for Checkbox (takes remaining space)

        # 创建列表框和滚动条的容器框架
        listbox_frame_third_party = ttk.Frame(deb_frame_third_party)
        listbox_frame_third_party.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S))
        listbox_frame_third_party.columnconfigure(0, weight=1)
        listbox_frame_third_party.rowconfigure(0, weight=1)

        # 增加列表框高度从3改为6
        self.deb_listbox_third_party = tk.Listbox(listbox_frame_third_party, height=6, width=50)
        self.deb_listbox_third_party.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 添加滚动条
        scrollbar_third_party = ttk.Scrollbar(listbox_frame_third_party, orient="vertical", command=self.deb_listbox_third_party.yview)
        scrollbar_third_party.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.deb_listbox_third_party.config(yscrollcommand=scrollbar_third_party.set)

        # Place buttons in specific columns, aligned left
        ttk.Button(deb_frame_third_party, text="添加", command=lambda: self.select_debs(self.deb_listbox_third_party)).grid(row=1, column=0, pady=(5,0), padx=(0,10), sticky=tk.W)
        ttk.Button(deb_frame_third_party, text="删除", command=lambda: self.delete_selected_deb(self.deb_listbox_third_party, "third-party")).grid(row=1, column=1, pady=(5,0), padx=(0,6), sticky=tk.W)
        ttk.Button(deb_frame_third_party, text="清空", command=lambda: self.clear_all_debs(self.deb_listbox_third_party, "third-party")).grid(row=1, column=2, pady=(5,0), padx=(0,6), sticky=tk.W)
        #ttk.Checkbutton(deb_frame_third_party, text="华为机器", variable=self.is_huawei).grid(row=1, column=2, sticky=tk.E, pady=(5,0))
        row_idx += 1

        # DEB包选择(apps-third)
        deb_frame_apps_third = ttk.LabelFrame(left_frame, text="DEB包选择(apps-third)", padding="10")
        deb_frame_apps_third.grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=5)
        # Configure columns identically to the frame above
        deb_frame_apps_third.columnconfigure(0, weight=0) # Column for Add button
        deb_frame_apps_third.columnconfigure(1, weight=0) # Column for Clear button
        deb_frame_apps_third.columnconfigure(2, weight=1) # Empty column to take remaining space

        # 创建列表框和滚动条的容器框架
        listbox_frame_apps_third = ttk.Frame(deb_frame_apps_third)
        listbox_frame_apps_third.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S))
        listbox_frame_apps_third.columnconfigure(0, weight=1)
        listbox_frame_apps_third.rowconfigure(0, weight=1)

        # 增加列表框高度从3改为6
        self.deb_listbox_apps_third = tk.Listbox(listbox_frame_apps_third, height=4, width=50)
        self.deb_listbox_apps_third.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # 添加滚动条
        scrollbar_apps_third = ttk.Scrollbar(listbox_frame_apps_third, orient="vertical", command=self.deb_listbox_apps_third.yview)
        scrollbar_apps_third.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.deb_listbox_apps_third.config(yscrollcommand=scrollbar_apps_third.set)

        # Place buttons in the same columns as above, aligned left
        ttk.Button(deb_frame_apps_third, text="添加", command=lambda: self.select_debs(self.deb_listbox_apps_third)).grid(row=1, column=0, pady=(5,0), padx=(0,10), sticky=tk.W)
        ttk.Button(deb_frame_apps_third, text="删除", command=lambda: self.delete_selected_deb(self.deb_listbox_apps_third, "apps-third")).grid(row=1, column=1, pady=(5,0), padx=(0,6), sticky=tk.W)
        ttk.Button(deb_frame_apps_third, text="清空", command=lambda: self.clear_all_debs(self.deb_listbox_apps_third, "apps-third")).grid(row=1, column=2, pady=(5,0), padx=(0,6), sticky=tk.W)
        row_idx += 1

        # Output Config - 进一步增加上方的间距
        output_cfg_frame = ttk.LabelFrame(left_frame, text="输出配置", padding="10")
        output_cfg_frame.grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=(25, 5))
        ttk.Label(output_cfg_frame, text="区县名称:").grid(row=0, column=0, padx=(0, 5), sticky=tk.W)
        district_entry = ttk.Entry(output_cfg_frame, textvariable=self.district_name, width=20)
        district_entry.grid(row=0, column=1, sticky=tk.W)
        district_entry.bind('<KeyRelease>', self.update_output_preview)
        ttk.Label(output_cfg_frame, text="输出文件名预览:").grid(row=1, column=0, padx=(0, 5), sticky=tk.W)
        self.output_preview_label = ttk.Label(output_cfg_frame, text="")
        self.output_preview_label.grid(row=1, column=1, sticky=tk.W)
        self.update_output_preview() # Initial preview
        row_idx += 1

        # Progress Label - 增加上方间距
        ttk.Label(left_frame, textvariable=self.progress_var, anchor=tk.W).grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=(20, 10))
        row_idx += 1

        # Action Buttons
        button_frame = ttk.Frame(left_frame)
        button_frame.grid(row=row_idx, column=0, sticky=(tk.W, tk.E), pady=20)
        button_frame.columnconfigure(0, weight=1)
        button_frame.columnconfigure(1, weight=1)
        self.start_button = ttk.Button(button_frame, text="开始制作", command=self.start_customization_thread)
        self.start_button.grid(row=0, column=0, padx=(0, 5), sticky=tk.EW)
        ttk.Button(button_frame, text="退出", command=self.root.quit).grid(row=0, column=1, padx=(5, 0), sticky=tk.EW)
        row_idx += 1

        # --- Right Frame Widgets ---
        right_frame.rowconfigure(1, weight=1) # Make output text expand
        right_frame.columnconfigure(0, weight=1)

        # Unattended Setup
        unattended_frame = ttk.LabelFrame(right_frame, text="镜像自定义设置", padding="10")
        unattended_frame.grid(row=0, column=0, sticky=(tk.N, tk.W, tk.E), padx=5, pady=5)
        # 在unattended_frame中配置列
        unattended_frame.columnconfigure(0, weight=0)  # 左侧按钮区域
        unattended_frame.columnconfigure(1, weight=0)  # 预留功能按钮区域
        unattended_frame.columnconfigure(2, weight=1)  # 设置状态显示区域（占据剩余空间）

        # 左侧按钮区域
        buttons_frame = ttk.Frame(unattended_frame)
        buttons_frame.grid(row=0, column=0, sticky=(tk.N, tk.W), padx=(0, 5))

        ttk.Button(buttons_frame, text="设置无人值守", 
                   command=self.open_unattended_settings).pack(anchor=tk.W, pady=5)

        # 添加授权文件按钮
        ttk.Button(buttons_frame, text="添加授权文件", 
                   command=self.open_custom_functions_window).pack(anchor=tk.W, pady=5)

        # 添加配置KMS IP按钮
        ttk.Button(buttons_frame, text="配置 KMS   IP", 
                   command=self.open_kms_config_window).pack(anchor=tk.W, pady=5)

        # 添加字体按钮
        ttk.Button(buttons_frame, text="添加系统字体", 
                   command=self.open_fonts_window).pack(anchor=tk.W, pady=5)

        # 添加安全中心设置按钮
        ttk.Button(buttons_frame, text="设置安全中心", 
                   command=self.open_security_settings).pack(anchor=tk.W, pady=5)

        ttk.Button(buttons_frame, text="自定义配置键", 
                   command=self.open_custom_config_window).pack(anchor=tk.W, pady=5)

        # 添加预留功能按钮区域
        reserved_buttons_frame = ttk.Frame(unattended_frame)
        reserved_buttons_frame.grid(row=0, column=1, sticky=(tk.N, tk.W), padx=(10, 10))  # 增加两侧内边距

        # 添加5个预留功能按钮
        ttk.Button(reserved_buttons_frame, text="添加kpb补丁", 
                   command=self.open_kpb_patches_window).pack(anchor=tk.W, pady=5)
        
        ttk.Button(reserved_buttons_frame, text="修改域名文件", 
                   command=self.open_hosts_window).pack(anchor=tk.W, pady=5)
        
        ttk.Button(reserved_buttons_frame, text="管理squashfs", 
                   command=self.manage_squashfs_packages).pack(anchor=tk.W, pady=5)
        
        ttk.Button(reserved_buttons_frame, text="预留功能键四", 
                   command=lambda: self.show_reserved_function_message("预留功能四")).pack(anchor=tk.W, pady=5)
        
        ttk.Button(reserved_buttons_frame, text="预留功能键五", 
                   command=lambda: self.show_reserved_function_message("预留功能五")).pack(anchor=tk.W, pady=5)



        # 右侧设置状态显示区域
        settings_status_frame = ttk.LabelFrame(unattended_frame, text="当前设置状态", padding="5")
        settings_status_frame.grid(row=0, column=2, sticky=(tk.N, tk.S, tk.E, tk.W), padx=(15, 0))  # 只保留水平内边距
        
        # 创建带滚动条的文本框容器
        status_container = ttk.Frame(settings_status_frame)
        status_container.pack(fill=tk.BOTH, expand=True, pady=(0, 0))  # 移除容器的顶部内边距
        status_container.columnconfigure(0, weight=1)
        status_container.rowconfigure(0, weight=1)

        # 创建设置状态文本框，使用更美观的样式
        self.settings_status_text = tk.Text(status_container, height=10, width=30, wrap='word', 
                                           bg='#f8f8f8', fg='#333333', 
                                           font=('TkDefaultFont', 9),
                                           relief=tk.FLAT, padx=5, pady=5,
                                           state='disabled')
        self.settings_status_text.grid(row=0, column=0, sticky=(tk.N, tk.S, tk.E, tk.W))

        # 添加滚动条
        status_scrollbar = ttk.Scrollbar(status_container, orient='vertical', command=self.settings_status_text.yview)
        status_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.settings_status_text['yscrollcommand'] = status_scrollbar.set

        # 初始化设置状态显示
        self.update_settings_status()

        # Execution Output/Log
        output_display_frame = ttk.LabelFrame(right_frame, text="执行回显", padding="10")
        output_display_frame.grid(row=1, column=0, sticky=(tk.N, tk.S, tk.E, tk.W), padx=5, pady=5)
        output_display_frame.rowconfigure(0, weight=1)
        output_display_frame.columnconfigure(0, weight=1)

        self.output_text = tk.Text(output_display_frame, height=15, wrap='word', state='disabled', bg='white', fg='black')
        self.output_text.grid(row=0, column=0, sticky=(tk.N, tk.S, tk.E, tk.W))
        scrollbar = ttk.Scrollbar(output_display_frame, orient='vertical', command=self.output_text.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.output_text['yscrollcommand'] = scrollbar.set

    # --- GUI Interaction Methods ---

    def _check_iso_selected(self):
        """检查是否已选择ISO文件"""
        iso_path = self.iso_path_var.get().strip()
        if not iso_path or not Path(iso_path).exists():
            messagebox.showwarning("提示", "请先选择ISO基础镜像")
            return False
        return True

    def _check_iso_prepared(self):
        """检查ISO文件是否已准备完成（用于添加操作）"""
        # 首先检查是否已选择ISO
        iso_path = self.iso_path_var.get().strip()
        if not iso_path or not Path(iso_path).exists():
            messagebox.showwarning("提示", "请先选择ISO基础镜像")
            return False
        
        # 检查ISO是否正在准备中
        if self._iso_prep_in_progress or (self._iso_prep_thread and self._iso_prep_thread.is_alive()):
            messagebox.showwarning("提示", "请等待ISO文件准备完成")
            return False
        
        # 检查ISO是否已准备完成
        if not self._iso_prepared:
            messagebox.showwarning("提示", "请等待ISO文件准备完成")
            return False
        
        return True

    def select_iso(self):
        # 检查是否有ISO准备流程正在进行中
        if self._iso_prep_in_progress or (self._iso_prep_thread and self._iso_prep_thread.is_alive()):
            messagebox.showwarning(
                "提示", 
                "ISO文件正在准备中，请等待当前流程完成后再选择新的ISO文件。\n\n"
                "当前流程完成后，您可以重新选择ISO文件。"
            )
            return  # 直接返回，不允许选择新文件
        
        filepath = filedialog.askopenfilename(title="选择ISO文件", filetypes=[("ISO镜像", "*.iso")])
        if filepath:
            
            self.iso_path_var.set(filepath)
            
            # 自动检测是否为华为机器镜像
            self._detect_huawei_iso(filepath)
            
            # 重置准备标志，因为选择了新的ISO文件
            self._iso_prepared = False
            self._iso_prep_in_progress = True
            
            # 清空输出并开始准备ISO（执行第0-3步）
            self.output_text.config(state='normal')
            self.output_text.delete('1.0', tk.END)
            self.output_text.config(state='disabled')
            self.update_progress(message="正在准备ISO文件...")
            
            # 在新线程中执行ISO准备步骤（第0-3步）
            self._iso_prep_thread = threading.Thread(target=self._prepare_iso_worker, daemon=True)
            self._iso_prep_thread.start()
            
            self.update_output_preview() # Update preview when ISO changes
            logger.info(f"ISO file selected: {filepath}")

    def _detect_huawei_iso(self, iso_path):
        """
        通过ISO文件名自动检测是否为华为版本
        检测关键字：kirin, 9006, 9000, pangu, wayland, panguw
        """
        iso_filename = os.path.basename(iso_path).lower()
        huawei_keywords = ['kirin', '9006', '9000', 'pangu', 'wayland']
        
        # 检查文件名是否包含华为相关关键字
        is_huawei = any(keyword in iso_filename for keyword in huawei_keywords)
        
        # 检查是否包含panguw关键字
        is_panguw = 'panguw' in iso_filename
        
        if is_huawei != self.is_huawei_detected:
            self.is_huawei_detected = is_huawei
            
        # 设置panguw标志
        if not hasattr(self, 'is_panguw_detected'):
            self.is_panguw_detected = False
        self.is_panguw_detected = is_panguw
        
        if is_huawei:
                logger.info(f"检测到华为版本镜像: {iso_filename}")
                self.append_output(f"✓ 自动检测到华为版本镜像")
                # 更新华为机器状态标签
                if hasattr(self, 'huawei_status_label'):
                    self.huawei_status_label.config(text="华为版本镜像 (已自动检测)", 
                                                   foreground="green")
        else:
                logger.info(f"检测到非华为版本镜像: {iso_filename}")
                self.append_output(f"✓ 检测到非华为版本镜像")
                # 更新非华为机器状态标签
                if hasattr(self, 'huawei_status_label'):
                    self.huawei_status_label.config(text="非华为版本镜像", 
                                                   foreground="blue")
        
        if is_panguw:
            logger.info(f"检测到panguw版本镜像: {iso_filename}")
            self.append_output(f"✓ 检测到panguw版本镜像")
            
        # 更新设置状态显示
        self.update_settings_status()

    def select_debs(self, listbox):
        """选择DEB包并添加到列表框（只显示文件名）"""
        if not self._check_iso_prepared():
            return
        
        filepaths = filedialog.askopenfilenames(
            title="选择DEB包",
            filetypes=[("DEB包", "*.deb"), ("所有文件", "*.*")]
        )
        
        if not filepaths:
            return
        
        # 确定是哪个列表
        mapping_key = 'third_party' if listbox == self.deb_listbox_third_party else 'apps_third'
        
        for filepath in filepaths:
            # 只显示文件名，不显示完整路径
            filename = Path(filepath).name
            # 检查是否已经在列表中（通过文件名检查）
            items = list(listbox.get(0, tk.END))
            if filename not in items:
                # 只显示文件名
                listbox.insert(tk.END, filename)
                # 存储文件名到完整路径的映射
                self._deb_path_mapping[mapping_key][filename] = filepath
        
        # 更新设置状态显示
        self.update_settings_status()
        logger.info(f"DEB files added: {filepaths}")

    def _scan_and_display_deb_files(self):
        """扫描ISO中的third-party和apps-third目录，显示其中的DEB文件"""
        try:
            # 在主线程中更新UI
            def update_ui():
                # 扫描third-party目录（包括子目录如common）
                third_party_dir = LIVECD_DIR / "third-party"
                if third_party_dir.exists() and third_party_dir.is_dir():
                    # 扫描根目录
                    deb_files = list(third_party_dir.glob("*.deb"))
                    # 扫描common子目录（华为机器）
                    common_subdir = third_party_dir / "common"
                    if common_subdir.exists():
                        deb_files.extend(common_subdir.glob("*.deb"))
                    
                    for deb_file in deb_files:
                        deb_path = str(deb_file)
                        filename = deb_file.name
                        # 检查是否已经在列表中（通过文件名检查）
                        items = list(self.deb_listbox_third_party.get(0, tk.END))
                        if filename not in items:
                            # 只显示文件名
                            self.deb_listbox_third_party.insert(tk.END, filename)
                            # 存储文件名到完整路径的映射
                            self._deb_path_mapping['third_party'][filename] = deb_path
                    if deb_files:
                        self.append_output(f"发现 {len(deb_files)} 个 third-party DEB文件")
                        logger.info(f"Found {len(deb_files)} DEB files in third-party directory")
                
                # 扫描apps-third目录（包括子目录如debs）
                apps_third_dir = LIVECD_DIR / "apps-third"
                if apps_third_dir.exists() and apps_third_dir.is_dir():
                    # 扫描根目录和debs子目录
                    deb_files = list(apps_third_dir.glob("*.deb"))
                    debs_subdir = apps_third_dir / "debs"
                    if debs_subdir.exists():
                        deb_files.extend(debs_subdir.glob("*.deb"))
                    
                    for deb_file in deb_files:
                        deb_path = str(deb_file)
                        filename = deb_file.name
                        # 仅显示Default=yes的包（已禁用的不显示）
                        if not self._is_apps_third_package_enabled(deb_path):
                            continue
                        # 检查是否已经在列表中（通过文件名检查）
                        items = list(self.deb_listbox_apps_third.get(0, tk.END))
                        if filename not in items:
                            # 只显示文件名
                            self.deb_listbox_apps_third.insert(tk.END, filename)
                            # 存储文件名到完整路径的映射
                            self._deb_path_mapping['apps_third'][filename] = deb_path
                    if deb_files:
                        self.append_output(f"发现 {len(deb_files)} 个 apps-third DEB文件")
                        logger.info(f"Found {len(deb_files)} DEB files in apps-third directory")
                
                # 更新设置状态显示
                self.update_settings_status()
            
            # 在主线程中执行UI更新
            self.root.after(0, update_ui)
            
        except Exception as e:
            logger.error(f"Failed to scan DEB files: {e}")
            self.append_output(f"警告: 扫描DEB文件时出错: {e}")

    def _get_deb_package_name(self, deb_path):
        """从DEB文件获取包名"""
        try:
            result = subprocess.run(
                ["dpkg-deb", "-f", str(deb_path), "Package"],
                capture_output=True, text=True, check=True
            )
            return result.stdout.strip()
        except (subprocess.CalledProcessError, FileNotFoundError) as e:
            logger.warning(f"无法获取DEB包名: {deb_path}: {e}")
            return None

    def _get_apps_third_cfg_path(self, package_name):
        """获取apps-third包对应的cfg文件路径"""
        return LIVECD_DIR / "apps-third" / "resources" / package_name / f"{package_name}.cfg"

    def _parse_cfg_default_line(self, line):
        """解析cfg中的Default/default行，返回(key, value)或None"""
        stripped = line.strip()
        if '=' not in stripped:
            return None
        key, value = stripped.split('=', 1)
        if key.lower() == 'default':
            return key, value.strip()
        return None

    def _set_apps_third_default(self, package_name, default='no'):
        """设置apps-third包cfg中的Default/default参数（兼容大小写）"""
        cfg_file = self._get_apps_third_cfg_path(package_name)
        if not cfg_file.exists():
            logger.warning(f"apps-third cfg文件不存在: {cfg_file}")
            return False
        try:
            lines = cfg_file.read_text(encoding='utf-8').splitlines()
            updated = False
            new_lines = []
            for line in lines:
                parsed = self._parse_cfg_default_line(line)
                if parsed:
                    key, _ = parsed
                    new_lines.append(f'{key}={default}')
                    updated = True
                else:
                    new_lines.append(line)
            if not updated:
                new_lines.append(f'Default={default}')
            cfg_file.write_text('\n'.join(new_lines) + '\n', encoding='utf-8')
            logger.info(f"已更新apps-third配置 default={default}: {cfg_file}")
            return True
        except Exception as e:
            logger.error(f"更新apps-third cfg失败: {cfg_file}: {e}")
            return False

    def _is_apps_third_package_enabled(self, deb_path):
        """检查apps-third包是否启用(default/Default=yes)"""
        package_name = self._get_deb_package_name(deb_path)
        if not package_name:
            return True
        cfg_file = self._get_apps_third_cfg_path(package_name)
        if not cfg_file.exists():
            return True
        try:
            for line in cfg_file.read_text(encoding='utf-8').splitlines():
                parsed = self._parse_cfg_default_line(line)
                if parsed:
                    _, value = parsed
                    return value.lower() == 'yes'
            return True
        except Exception as e:
            logger.warning(f"读取apps-third cfg失败: {cfg_file}: {e}")
            return True

    def _disable_apps_third_deb(self, deb_path):
        """禁用apps-third包：将cfg中Default=yes改为no（不删除deb文件）"""
        package_name = self._get_deb_package_name(deb_path)
        if not package_name:
            self.append_output(f"警告: 无法识别DEB包名，跳过禁用: {Path(deb_path).name}")
            return False
        if self._set_apps_third_default(package_name, 'no'):
            self.append_output(f"已禁用apps-third包: {package_name} (Default=no)")
            return True
        self.append_output(f"警告: 未找到配置，无法禁用: {package_name}")
        return False

    def _is_deb_in_apps_third_dir(self, deb_file):
        """检查DEB文件是否在ISO的apps-third目录中"""
        apps_third_dir = (LIVECD_DIR / "apps-third").resolve()
        return str(deb_file.resolve()).startswith(str(apps_third_dir))

    def delete_selected_deb(self, listbox, directory_type):
        """删除选中的DEB文件"""
        selected_indices = listbox.curselection()
        if not selected_indices:
            messagebox.showwarning("提示", "请先选择要删除的DEB文件")
            return
        
        mapping_key = 'third_party' if listbox == self.deb_listbox_third_party else 'apps_third'
        is_apps_third = mapping_key == 'apps_third'
        
        # 确认删除
        if is_apps_third:
            confirm_msg = "确定要删除选中的apps-third包吗？\n将仅把对应cfg中的Default=yes改为no，不会删除deb文件。"
        else:
            confirm_msg = "确定要删除选中的DEB文件吗？\n文件将从列表和ISO目录中删除。"
        if not messagebox.askyesno("确认", confirm_msg):
            return
        
        deleted_count = 0
        disabled_count = 0
        removed_from_list_count = 0
        failed_count = 0
        
        # 从后往前删除，避免索引变化问题
        for index in reversed(selected_indices):
            filename = listbox.get(index)  # 获取文件名
            # 从映射中获取完整路径
            deb_path = self._deb_path_mapping[mapping_key].get(filename, filename)  # 如果映射中没有，使用文件名作为路径（兼容旧格式）
            
            try:
                deb_file = Path(deb_path).resolve()  # 获取绝对路径
                if is_apps_third:
                    if deb_file.exists() and self._is_deb_in_apps_third_dir(deb_file):
                        if self._disable_apps_third_deb(deb_path):
                            disabled_count += 1
                        else:
                            failed_count += 1
                    else:
                        logger.info(f"Removed external apps-third DEB from list: {deb_path}")
                        removed_from_list_count += 1
                elif deb_file.exists():
                    # 检查文件是否在ISO工作目录中（third-party目录）
                    third_party_dir = (LIVECD_DIR / "third-party").resolve()
                    is_in_iso_dir = str(deb_file).startswith(str(third_party_dir))
                    
                    if is_in_iso_dir:
                        deb_file.unlink()
                        logger.info(f"Deleted DEB file from ISO directory: {deb_path}")
                        deleted_count += 1
                    else:
                        logger.info(f"Removed external DEB file from list (not in ISO dir): {deb_path}")
                        removed_from_list_count += 1
                else:
                    logger.warning(f"DEB file not found, removing from list: {deb_path}")
                    removed_from_list_count += 1
                
                # 从列表中删除
                listbox.delete(index)
                # 从映射中删除
                if filename in self._deb_path_mapping[mapping_key]:
                    del self._deb_path_mapping[mapping_key][filename]
                
            except Exception as e:
                logger.error(f"Failed to delete DEB file {deb_path}: {e}")
                failed_count += 1
                # 即使删除失败，也从列表中移除
                try:
                    listbox.delete(index)
                except:
                    pass
                self.append_output(f"删除文件失败: {deb_path} - {e}")
        
        # 显示结果
        if deleted_count > 0 or disabled_count > 0 or removed_from_list_count > 0:
            if is_apps_third and disabled_count > 0:
                self.append_output(f"已禁用 {disabled_count} 个apps-third包 ({directory_type})")
            elif deleted_count > 0:
                self.append_output(f"已删除 {deleted_count} 个DEB文件 ({directory_type})")
            if removed_from_list_count > 0:
                self.append_output(f"已从列表移除 {removed_from_list_count} 个文件")
            if failed_count > 0:
                self.append_output(f"操作失败 {failed_count} 个文件")
        
        # 更新设置状态显示
        self.update_settings_status()
        logger.info(f"Delete deb: deleted={deleted_count}, disabled={disabled_count}, removed={removed_from_list_count} ({directory_type})")

    def clear_all_debs(self, listbox, directory_type):
        """清空所有DEB文件"""
        mapping_key = 'third_party' if listbox == self.deb_listbox_third_party else 'apps_third'
        is_apps_third = mapping_key == 'apps_third'
        items_count = listbox.size()
        apps_third_dir = LIVECD_DIR / "apps-third"

        if is_apps_third:
            if items_count == 0 and not (apps_third_dir.exists() and any(apps_third_dir.iterdir())):
                messagebox.showinfo("提示", "apps-third 目录已经是空的")
                return
            confirm_msg = "确定要清空 apps-third 吗？\n将删除镜像中整个 apps-third 文件夹及其全部内容。"
            if not messagebox.askyesno("确认", confirm_msg):
                return
            try:
                if apps_third_dir.exists():
                    shutil.rmtree(apps_third_dir)
                    logger.info(f"已删除整个apps-third目录: {apps_third_dir}")
                    self.append_output("已清空 apps-third 文件夹及全部内容")
                listbox.delete(0, tk.END)
                self._deb_path_mapping[mapping_key].clear()
                messagebox.showinfo("成功", "已清空 apps-third 文件夹")
            except Exception as e:
                logger.error(f"清空apps-third目录失败: {e}")
                self.append_output(f"清空 apps-third 失败: {e}")
                messagebox.showerror("错误", f"清空 apps-third 失败: {e}")
            self.update_settings_status()
            return

        if items_count == 0:
            messagebox.showinfo("提示", "列表已经是空的")
            return

        confirm_msg = f"确定要删除所有 {items_count} 个DEB文件吗？\n文件将从列表和ISO目录中删除。"
        if not messagebox.askyesno("确认", confirm_msg):
            return
        
        deleted_count = 0
        removed_from_list_count = 0
        failed_count = 0
        
        # 获取所有文件名
        all_filenames = list(listbox.get(0, tk.END))
        
        for filename in all_filenames:
            # 从映射中获取完整路径
            deb_path = self._deb_path_mapping[mapping_key].get(filename, filename)
            
            try:
                deb_file = Path(deb_path).resolve()
                if deb_file.exists():
                    third_party_dir = (LIVECD_DIR / "third-party").resolve()
                    is_in_iso_dir = str(deb_file).startswith(str(third_party_dir))
                    
                    if is_in_iso_dir:
                        deb_file.unlink()
                        logger.info(f"Deleted DEB file from ISO directory: {deb_path}")
                        deleted_count += 1
                    else:
                        logger.info(f"Removed external DEB file from list (not in ISO dir): {deb_path}")
                        removed_from_list_count += 1
                else:
                    logger.warning(f"DEB file not found, removing from list: {deb_path}")
                    removed_from_list_count += 1
            except Exception as e:
                logger.error(f"Failed to delete DEB file {deb_path}: {e}")
                failed_count += 1
                self.append_output(f"删除文件失败: {deb_path} - {e}")
        
        # 清空列表
        listbox.delete(0, tk.END)
        # 清空映射
        self._deb_path_mapping[mapping_key].clear()
        
        # 显示结果
        if deleted_count > 0 or removed_from_list_count > 0:
            if deleted_count > 0:
                self.append_output(f"已删除 {deleted_count} 个DEB文件 ({directory_type})")
            if removed_from_list_count > 0:
                self.append_output(f"已从列表移除 {removed_from_list_count} 个文件")
            if failed_count > 0:
                self.append_output(f"操作失败 {failed_count} 个文件")
                messagebox.showwarning("警告", f"成功删除 {deleted_count} 个文件，移除 {removed_from_list_count} 个文件，{failed_count} 个文件处理失败")
            elif deleted_count > 0:
                messagebox.showinfo("成功", f"已成功删除所有 {deleted_count} 个DEB文件")
        
        # 更新设置状态显示
        self.update_settings_status()
        logger.info(f"Cleared all: deleted={deleted_count}, removed={removed_from_list_count} ({directory_type})")

    def _prepare_iso_worker(self):
        """准备ISO文件：执行第0-3步（清理、挂载、复制、卸载）"""
        try:
            self.append_output("\n--- 开始准备ISO文件 ---")
            
            # Step 0: Cleanup from previous runs
            self.update_progress(message="清理旧目录...")
            self._cleanup_working_dirs()
            self.setup_directories() # Recreate clean dirs

            # Step 1: Mount ISO
            self.update_progress(message="挂载ISO...")
            self._mount_iso_worker()

            # Step 2: Copy ISO contents
            self.update_progress(message="复制镜像文件...")
            self._copy_iso_contents_worker()

            # Step 3: Unmount ISO
            self.update_progress(message="卸载ISO...")
            self._unmount_iso() # Unmount early, work on copied files
            
            # 扫描并显示ISO中的DEB文件
            self.update_progress(message="扫描DEB文件...")
            self._scan_and_display_deb_files()
            
            self._iso_prepared = True
            self._iso_prep_in_progress = False  # 标记准备完成
            self.update_progress(message="ISO文件准备完成")
            self.append_output("--- ISO文件准备完成 ---\n")
            logger.info("ISO preparation completed successfully")
            
        except Exception as e:
            logger.exception("ISO preparation failed!")
            self.append_output(f"\n--- 错误 ---")
            self.append_output(f"ISO文件准备失败: {e}")
            self.update_progress(message=f"ISO准备失败: {e}")
            self._iso_prepared = False
            self._iso_prep_in_progress = False  # 即使失败也要重置标志
            # 在主线程中显示错误对话框
            self.root.after(0, lambda: messagebox.showerror("错误", f"ISO文件准备失败:\n{e}"))

    def open_unattended_settings(self):
        """打开无人值守安装设置的弹出窗口"""
        if not self._check_iso_selected():
            return
        settings_win = tk.Toplevel(self.root)
        settings_win.title("无人值守安装设置")
        settings_win.geometry("500x500")  # 进一步增加窗口高度
        settings_win.transient(self.root)  # 设置为主窗口的子窗口
        settings_win.grab_set()  # 模态窗口
        
        # 主框架 - 使用grid布局更好控制
        main_frame = ttk.Frame(settings_win, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # 添加启用无人值守的复选框
        enable_frame = ttk.Frame(main_frame)
        enable_frame.pack(fill=tk.X, pady=(0, 10))
        
        # 创建一个临时变量来存储启用状态
        temp_enable = tk.BooleanVar(value=self.enable_unattended.get())
        
        # 创建百信双硬盘策略变量（如果不存在）
        if not hasattr(self, 'baixinx_dual_disk'):
            self.baixinx_dual_disk = tk.BooleanVar(value=False)
        if not hasattr(self, 'baixinx_disk_size'):
            self.baixinx_disk_size = tk.StringVar(value="1000")
        
        # 创建临时变量
        temp_baixinx = tk.BooleanVar(value=self.baixinx_dual_disk.get())
        
        # 定义一个函数来切换控件状态
        def toggle_controls():
            state = tk.NORMAL if temp_enable.get() else tk.DISABLED
            # 遍历所有需要启用/禁用的控件，但排除百信双硬盘策略框架
            for widget in left_frame.winfo_children() + right_frame.winfo_children() + options_frame.winfo_children():
                try:
                    widget.configure(state=state)
                except tk.TclError:
                    pass  # 有些控件可能不支持状态设置
            
            # 百信双硬盘策略控件状态不受无人值守安装影响，所以不调用toggle_baixinx_controls
        
        # 定义百信双硬盘策略切换函数（独立于无人值守安装）
        def toggle_baixinx_controls():
            try:
                # 只根据百信双硬盘策略本身的状态来控制输入框
                baixinx_disk_size_entry.configure(state=tk.NORMAL if temp_baixinx.get() else tk.DISABLED)
            except:
                pass  # 如果控件还未创建，忽略错误
        
        # 创建复选框并绑定切换函数
        enable_check = ttk.Checkbutton(enable_frame, text="启用无人值守安装", 
                                      variable=temp_enable,
                                      command=toggle_controls)
        enable_check.pack(anchor=tk.W)
        
        # 创建左右两列布局
        content_frame = ttk.Frame(main_frame)
        content_frame.pack(fill=tk.X, pady=(0, 10))
        
        left_frame = ttk.Frame(content_frame)
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 10))
        
        right_frame = ttk.Frame(content_frame)
        right_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # 左侧选项
        ttk.Label(left_frame, text="用户名:").pack(anchor=tk.W)
        ttk.Entry(left_frame, textvariable=self.username_var).pack(fill=tk.X, pady=2)
        
        ttk.Label(left_frame, text="密码:").pack(anchor=tk.W)
        ttk.Entry(left_frame, textvariable=self.password_var).pack(fill=tk.X, pady=2)
        
        ttk.Label(left_frame, text="主机名:").pack(anchor=tk.W)
        ttk.Entry(left_frame, textvariable=self.hostname_var).pack(fill=tk.X, pady=2)
        
        # 右侧选项
        ttk.Label(right_frame, text="数据盘(华为机器单硬盘不要选！):").pack(anchor=tk.W)
        ttk.Entry(right_frame, textvariable=self.data_device).pack(fill=tk.X, pady=2)
        
        ttk.Label(right_frame, text="数据盘分区格式:").pack(anchor=tk.W)
        ttk.Entry(right_frame, textvariable=self.data_fs_type_var).pack(fill=tk.X, pady=2)
        
        # 添加百信双硬盘策略选项
        baixinx_frame = ttk.LabelFrame(main_frame, text="百信双硬盘策略", padding="10")
        baixinx_frame.pack(fill=tk.X, pady=(0, 10))
        
        # 百信双硬盘策略复选框
        baixinx_check = ttk.Checkbutton(baixinx_frame, text="启用百信双硬盘策略", 
                                       variable=temp_baixinx,
                                       command=toggle_baixinx_controls)
        baixinx_check.pack(anchor=tk.W, pady=(0, 5))
        
        # 硬盘大小输入框
        size_frame = ttk.Frame(baixinx_frame)
        size_frame.pack(fill=tk.X)
        
        ttk.Label(size_frame, text="目标硬盘大小(GB):").pack(side=tk.LEFT, padx=(0, 5))
        baixinx_disk_size_entry = ttk.Entry(size_frame, textvariable=self.baixinx_disk_size, width=10)
        baixinx_disk_size_entry.pack(side=tk.LEFT)
        ttk.Label(size_frame, text="(请输入目标数据盘的大小，单位为GB)").pack(side=tk.LEFT, padx=(5, 0))
        
        # 其他选项
        options_frame = ttk.LabelFrame(main_frame, text="其他选项", padding="10")
        options_frame.pack(fill=tk.X, pady=(0, 10))
        
        ttk.Checkbutton(options_frame, text="自动登录", variable=self.auto_login).pack(side=tk.LEFT, padx=5)
        ttk.Checkbutton(options_frame, text="出厂备份", variable=self.factory_backup).pack(side=tk.LEFT, padx=5)
        ttk.Checkbutton(options_frame, text="安装后重启", variable=self.is_reboot).pack(side=tk.LEFT, padx=5)
        
        # 按钮框架 - 固定在窗口底部
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(10, 0))
        
        # 修改确定按钮，保存所有状态
        def on_ok():
            self.enable_unattended.set(temp_enable.get())
            self.baixinx_dual_disk.set(temp_baixinx.get())
            settings_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        def on_cancel():
            settings_win.destroy()
        
        # 添加取消和确定按钮
        ttk.Button(btn_frame, text="取消", command=on_cancel).pack(side=tk.RIGHT, padx=(5, 0))
        ttk.Button(btn_frame, text="确定", command=on_ok).pack(side=tk.RIGHT)
        
        # 初始设置控件状态
        toggle_controls()
        toggle_baixinx_controls()  # 单独设置百信双硬盘策略控件状态
        
        # 等待窗口关闭
        self.root.wait_window(settings_win)

    def update_output_preview(self, event=None):
        """Updates the preview label for the output ISO filename."""
        iso_filename = Path(self.iso_path_var.get()).stem # Get filename without extension
        district = self.district_name.get().strip()
        # Sanitize district name for filename (allow alphanumeric, hyphen, underscore)
        safe_district = re.sub(r'[^\w-]', '', district) if district else "custom"

        if iso_filename:
            preview = f"{iso_filename}-{safe_district}.iso"
        else:
            preview = f"base-{safe_district}.iso"
        self.output_preview_label.config(text=preview)

    def get_output_filename(self):
        """Generates the final output filename based on preview."""
        # Reuse logic from preview, ensuring it's up-to-date
        self.update_output_preview()
        filename = self.output_preview_label.cget("text")
        # Basic validation for the generated name
        if not filename or not filename.endswith(".iso") or len(filename) < 5:
             # Fallback name if generation fails
             timestamp = time.strftime("%Y%m%d-%H%M%S")
             return f"custom-iso-{timestamp}.iso"
        return filename

    def append_output(self, text):
        """Appends text to the output Text widget in a thread-safe way."""
        def _append():
            self.output_text.configure(state='normal')
            self.output_text.insert(tk.END, str(text) + '\n')
            self.output_text.see(tk.END)
            self.output_text.configure(state='disabled')
        # Schedule the GUI update in the main thread
        self.root.after(0, _append)

    def update_progress(self, step_index=None, message=None):
        """Updates the progress label in a thread-safe way."""
        if message is None and step_index is not None and step_index < len(self.steps):
            message = f"进行中: {self.steps[step_index]}"
        elif message is None:
            message = "处理中..."

        def _update():
            self.progress_var.set(message)
        self.root.after(0, _update)

    # --- Input Validation ---

    def validate_inputs(self):
        """验证输入是否完整和有效。"""
        logger.info("Validating inputs...")
        iso_path = self.iso_path_var.get()
        if not iso_path or not Path(iso_path).is_file():
            messagebox.showerror("错误", "请选择一个有效的ISO文件路径。")
            logger.error("Validation failed: Invalid ISO path.")
            return False

        district = self.district_name.get().strip()
        if district and not re.match(r'^[\w-]{2,20}$', district):
             messagebox.showerror("错误", "区县名称只能包含字母、数字、下划线和连字符（2-20个字符）。")
             logger.error(f"Validation failed: Invalid district name '{district}'.")
             return False

        logger.info("Input validation successful.")
        return True # 所有验证通过

    def start_customization_thread(self):
        """Starts the customization process in a background thread."""
        # Check root privileges before starting
        if not self.has_root_privileges:
            messagebox.showerror("错误", "此操作需要root权限。\n请使用sudo或以root权限运行程序。")
            logger.error("Attempted to start customization without root privileges.")
            return
            
        if not self.validate_inputs():
            return

        if self.worker_thread and self.worker_thread.is_alive():
            messagebox.showwarning("警告", "一个定制任务已经在运行中。")
            return
        
        # 检查ISO是否正在准备中
        if self._iso_prep_thread and self._iso_prep_thread.is_alive():
            response = messagebox.askyesno("提示", "ISO文件正在准备中，是否等待完成后继续？")
            if not response:
                return

        # Disable button, clear output (但保留ISO准备时的输出)
        self.start_button.config(state=tk.DISABLED)
        self.update_progress(message="正在开始制作...")
        logger.info("Starting customization process in background thread.")

        # Start the worker thread
        self.worker_thread = threading.Thread(target=self._customization_worker, daemon=True)
        self.worker_thread.start()

    def _customization_worker(self):
        """The main worker function running in a separate thread."""
        start_time = time.time()
        success = False
        output_filename = self.get_output_filename()
        final_iso_path = WORK_BASE_DIR / output_filename # Output to work dir first

        try:
            # 等待ISO准备完成（如果还在准备中）
            if self._iso_prep_thread and self._iso_prep_thread.is_alive():
                self.append_output("等待ISO文件准备完成...")
                self.update_progress(message="等待ISO文件准备...")
                self._iso_prep_thread.join(timeout=60)  # 最多等待60秒
                if self._iso_prep_thread.is_alive():
                    raise RuntimeError("ISO文件准备超时")
            
            # 检查ISO是否已准备好
            if not self._iso_prepared:
                raise RuntimeError("ISO文件尚未准备完成，请先选择ISO文件")
            
            # Step 4: 处理所有自定义配置 (包括DEB包、授权文件、字体、KMS和无人值守安装)
            self.update_progress(4) # "安装/配置包和授权"
            self._process_customizations_worker()

            # Step 5: Generate MD5 sums (原来的第6步)
            self.update_progress(5) # "生成MD5"
            self._generate_md5sum_worker()

            # Step 6: Generate Final ISO (原来的第7步)
            self.update_progress(6) # "封装最终镜像"
            self._generate_iso_worker(final_iso_path)

            success = True

        except Exception as e:
            logger.exception("Customization failed!") # Log full traceback
            self.append_output(f"\n--- 错误 ---")
            self.append_output(f"处理失败: {e}")
            self.update_progress(message=f"失败: {e}")
            # Attempt cleanup even on failure
            #self._cleanup_working_dirs()

        finally:
            # Re-enable button in the main thread
            self.root.after(0, lambda: self.start_button.config(state=tk.NORMAL))
            end_time = time.time()
            duration = end_time - start_time
            logger.info(f"Customization finished in {duration:.2f} seconds. Success: {success}")
            if success:
                self.update_progress(message="定制完成！")
                self.append_output(f"\n--- 成功 ---")
                self.append_output(f"镜像已生成: {final_iso_path}")
                messagebox.showinfo("成功", f"镜像已生成:\n{final_iso_path}")
            else:
                 messagebox.showerror("失败", "ISO镜像定制失败，请检查回显和日志。")

    def _process_customizations_worker(self):
        """处理所有自定义配置，包括DEB包、授权文件、字体、KMS和无人值守安装等"""
        self.append_output("\n--- 开始处理自定义配置 ---")
        
        # 检查是否需要解压squashfs的操作
        need_squashfs = False
        apps_third_filenames = list(self.deb_listbox_apps_third.get(0, tk.END))
        if self.is_huawei_detected and apps_third_filenames:
            need_squashfs = True
        if self.remove_qax.get():
            need_squashfs = True
        if self.custom_squashfs_packages:
            need_squashfs = True
        
        # 如果需要squashfs操作，先统一解压
        if need_squashfs:
            self.update_progress(message="解压文件系统...")
            self._extract_filesystem()
        
        try:
            # 1. 处理DEB包
            self.update_progress(message="处理DEB包...")
            self._process_deb_packages()
            
            # 2. 处理授权文件
            self.update_progress(message="处理授权文件...")
            self._process_custom_licenses()
            
            # 3. 处理字体文件
            self.update_progress(message="处理字体文件...")
            self._process_font_files()
            
            # 3.5. 处理kpb补丁文件
            self.update_progress(message="处理kpb补丁文件...")
            self._process_kpb_patches()
            
            # 3.6. 处理自定义Personalization文件夹
            self.update_progress(message="处理自定义文件夹...")
            self._process_personalization_files()
            
            # 4. 处理自定义后置脚本（与安全中心等一并写入.kylin-post-actions末尾）
            self.update_progress(message="更新后置操作脚本...")
            self._update_post_actions_script()
            
            # 6. 配置无人值守安装 (整合原来的第5步)
            if self.enable_unattended.get():
                self.update_progress(message="配置无人值守安装...")
                self._configure_unattended_worker()
            else:
                self.append_output("无人值守安装未启用，跳过配置")
            
            # 7. 管理squashfs软件包（如果已解压squashfs）
            if self.remove_qax.get() or self.custom_squashfs_packages:
                self.update_progress(message="管理squashfs软件包...")
                
                # 删除奇安信软件
                if self.remove_qax.get():
                    self._remove_qax_antivirus_worker()
                
                # 安装自定义软件包
                if self.custom_squashfs_packages:
                    self._install_custom_squashfs_packages_worker()
            else:
                self.append_output("未选择squashfs软件管理操作，跳过")
            
            # 如果需要squashfs操作，统一打包和清理
            if need_squashfs:
                self.update_progress(message="重新打包文件系统...")
                self._repack_filesystem_worker()
                self.update_progress(message="更新文件系统大小...")
                self._update_filesystem_size_worker()
                self._cleanup_squashfs_root_worker()
            
            self.append_output("所有自定义配置处理完成")
            logger.info("All customizations processed successfully")
            
        except Exception as e:
            logger.error(f"Failed during customization processing: {e}")
            raise RuntimeError(f"处理自定义配置失败: {e}")

    def _process_deb_packages(self):
        """处理DEB包 (third-party和apps-third)"""
        # 获取DEB包列表，从映射中提取完整路径
        def extract_paths(filenames, mapping_key):
            """从文件名列表中提取完整路径"""
            paths = []
            for filename in filenames:
                # 从映射中获取完整路径
                full_path = self._deb_path_mapping[mapping_key].get(filename, filename)
                paths.append(full_path)
            return paths
        
        third_party_filenames = list(self.deb_listbox_third_party.get(0, tk.END))
        apps_third_filenames = list(self.deb_listbox_apps_third.get(0, tk.END))
        debs_to_install_third_party = extract_paths(third_party_filenames, 'third_party')
        debs_to_install_apps_third = extract_paths(apps_third_filenames, 'apps_third')
        
        # 华为机器特殊处理
        if self.is_huawei_detected and debs_to_install_apps_third:
            self.append_output("\n华为机器需要特殊处理apps-third包...")
            self._process_huawei_apps_third(debs_to_install_apps_third)
            # 清空apps-third列表，因为已经在特殊处理中安装了
            #debs_to_install_apps_third = []
        
        if not debs_to_install_third_party and not debs_to_install_apps_third:
            self.append_output("未选择DEB包或已通过特殊方式处理，跳过常规安装步骤")
            return
        
        # 常规处理流程（非华为机器或只有third-party包）
        self.append_output("\n--- 处理DEB包 ---")
        
        # --- 处理third-party DEB包 ---
        if debs_to_install_third_party:
            self.append_output(f"准备复制 {len(debs_to_install_third_party)} 个 third-party DEB包...")
            
            # 确定目标目录
            target_debs_dir_third_party = LIVECD_DIR / "third-party"
            if self.is_huawei_detected:
                target_debs_dir_third_party = target_debs_dir_third_party / "common"
            
            # 确保目录存在 - 修复：使用更明确的日志和错误处理
            try:
                target_debs_dir_third_party.mkdir(parents=True, exist_ok=True)
                logger.info(f"确保目标目录存在: {target_debs_dir_third_party}")
                self.append_output(f"创建目标目录: {target_debs_dir_third_party}")
            except Exception as e:
                logger.error(f"创建目标目录失败: {e}")
                self.append_output(f"错误: 创建目标目录失败: {e}")
                raise RuntimeError(f"无法创建目标目录 {target_debs_dir_third_party}: {e}")
            
            # 复制DEB包
            for deb_path in debs_to_install_third_party:
                if Path(deb_path).is_file():
                    deb_file = Path(deb_path)
                    target_file = target_debs_dir_third_party / deb_file.name
                    
                    # 检查源文件和目标文件是否是同一个文件
                    if deb_file.resolve() == target_file.resolve():
                        logger.info(f"文件已在目标目录中，跳过复制: {deb_path}")
                        self.append_output(f"文件已在目标目录中，跳过: {deb_file.name}")
                    else:
                        logger.info(f"Copying DEB: {deb_path} to {target_debs_dir_third_party}")
                        shutil.copy2(deb_path, target_debs_dir_third_party)
                        self.append_output(f"已复制: {deb_file.name}")
                else:
                    logger.warning(f"DEB file not found, skipping: {deb_path}")
                    self.append_output(f"警告: third-party DEB文件未找到，跳过: {Path(deb_path).name}")
        
        # --- 处理apps-third DEB包 ---
        if debs_to_install_apps_third:
            self.append_output(f"\n准备处理 {len(debs_to_install_apps_third)} 个 apps-third DEB包...")
            
            # 确定目标目录
            target_debs_dir_apps_third = LIVECD_DIR / "apps-third"
            
            # 创建必要的目录结构
            target_debs_dir_apps_third.mkdir(parents=True, exist_ok=True)
            debs_dir = target_debs_dir_apps_third / "debs"
            debs_dir.mkdir(parents=True, exist_ok=True)
            resources_dir = target_debs_dir_apps_third / "resources"
            resources_dir.mkdir(parents=True, exist_ok=True)
            apks_dir = target_debs_dir_apps_third / "apks"
            apks_dir.mkdir(parents=True, exist_ok=True)
            
            # 先检查哪些文件已经在目标目录中（避免删除后无法复制）
            files_already_in_target = set()
            files_to_copy = []
            
            for deb_path in debs_to_install_apps_third:
                if Path(deb_path).is_file():
                    deb_file = Path(deb_path).resolve()
                    target_file = (debs_dir / Path(deb_path).name).resolve()
                    
                    if deb_file == target_file:
                        # 文件已经在目标目录中，保留它
                        files_already_in_target.add(Path(deb_path).name)
                        logger.info(f"文件已在目标目录中，将保留: {Path(deb_path).name}")
                    else:
                        files_to_copy.append(deb_path)
                else:
                    logger.warning(f"DEB file not found, skipping: {deb_path}")
                    self.append_output(f"警告: apps-third DEB文件未找到，跳过: {Path(deb_path).name}")
            
            # 清理debs目录中将要被替换的同名文件
            if files_to_copy:
                self.append_output("准备复制新增的apps-third DEB包...")
                for deb_path in files_to_copy:
                    deb_file = Path(deb_path)
                    target_file = debs_dir / deb_file.name
                    if target_file.exists() and deb_file.resolve() != target_file.resolve():
                        target_file.unlink()
                        logger.info(f"删除同名文件以便替换: {deb_file.name}")
            
            # 删除Packages文件(如果存在)，因为需要重新生成
            packages_file = target_debs_dir_apps_third / "Packages"
            if packages_file.exists():
                packages_file.unlink()
                logger.info("删除旧的Packages文件，将重新生成")
            
            # 复制DEB包到debs目录
            for deb_path in files_to_copy:
                deb_file = Path(deb_path)
                target_file = debs_dir / deb_file.name
                
                # 检查源文件和目标文件是否是同一个文件
                if deb_file.resolve() == target_file.resolve():
                    logger.info(f"文件已在目标目录中，跳过复制: {deb_path}")
                    self.append_output(f"文件已在目标目录中，跳过: {deb_file.name}")
                else:
                    logger.info(f"Copying DEB: {deb_path} to {debs_dir}")
                    shutil.copy2(deb_path, debs_dir)
                    self.append_output(f"已复制: {deb_file.name}")
            
            # 显示保留的文件
            if files_already_in_target:
                self.append_output(f"保留 {len(files_already_in_target)} 个已在目标目录中的文件")
                for filename in files_already_in_target:
                    logger.info(f"保留文件: {filename}")
            
            # 执行dpkg-scanpackages命令
            self.append_output("执行dpkg-scanpackages命令...")
            
            # 切换到apps-third目录
            original_dir = os.getcwd()
            os.chdir(str(target_debs_dir_apps_third))
            
            try:
                # 执行命令并捕获输出
                result = subprocess.run(
                    ["dpkg-scanpackages", "debs/"], 
                    capture_output=True, 
                    text=True, 
                    check=True
                )
                
                # 将输出写入Packages文件
                with open("Packages", "w") as f:
                    f.write(result.stdout)
                
                # 解析输出，提取Package名称
                package_names = []
                current_package = None
                
                for line in result.stdout.splitlines():
                    if line.startswith("Package: "):
                        current_package = line.replace("Package: ", "").strip()
                        package_names.append(current_package)
                
                self.append_output(f"找到 {len(package_names)} 个软件包: {', '.join(package_names)}")
                
                # 仅对列表中选中的包创建/更新配置(Default=yes)
                enabled_package_names = set()
                for deb_path in debs_to_install_apps_third:
                    pkg_name = self._get_deb_package_name(deb_path)
                    if pkg_name:
                        enabled_package_names.add(pkg_name)
                
                for pkg_name in enabled_package_names:
                    # 创建资源目录
                    pkg_resource_dir = resources_dir / pkg_name
                    pkg_resource_dir.mkdir(parents=True, exist_ok=True)
                    
                    cfg_file = pkg_resource_dir / f"{pkg_name}.cfg"
                    if cfg_file.exists():
                        self._set_apps_third_default(pkg_name, 'yes')
                        self.append_output(f"已更新配置: {pkg_name}.cfg (Default=yes)")
                    else:
                        with open(cfg_file, "w") as f:
                            f.write(f"Name={pkg_name}\n")
                            f.write(f"Name_en={pkg_name}\n")
                            f.write(f"Name_bo={pkg_name}\n")
                            f.write("Type=deb\n")
                            f.write("Default=yes\n")
                        self.append_output(f"已创建配置: {pkg_name}.cfg")
                
            except subprocess.CalledProcessError as e:
                logger.error(f"dpkg-scanpackages命令执行失败: {e}")
                self.append_output(f"错误: dpkg-scanpackages命令执行失败: {e}")
                self.append_output(f"错误输出: {e.stderr}")
            except Exception as e:
                logger.error(f"处理apps-third包时出错: {e}")
                self.append_output(f"错误: 处理apps-third包时出错: {e}")
            finally:
                # 返回原目录
                os.chdir(original_dir)
            
            self.append_output("apps-third DEB包处理完成")
        
        
        
    def _process_huawei_apps_third(self, deb_files):
        """华为机器特殊处理apps-third包"""
        self.append_output("开始华为机器特殊处理apps-third包...")
        
        # 检查是否有preinstalled-apps包
        preinstalled_apps_path = None
        
        # 弹出对话框让用户选择preinstalled-apps包
        self.append_output("华为机器需要preinstalled-apps包来安装apps-third包")
        preinstalled_apps_path = filedialog.askopenfilename(
            title="请选择preinstalled-apps包",
            filetypes=[("DEB包", "*.deb")]
        )
        
        if not preinstalled_apps_path:
            self.append_output("未选择preinstalled-apps包，无法继续处理apps-third包")
            logger.warning("华为机器未选择preinstalled-apps包，无法处理apps-third包")
            return
        
        self.append_output(f"已选择preinstalled-apps包: {os.path.basename(preinstalled_apps_path)}")
        squashfs_root= LIVECD_DIR / "casper" / "squashfs-root"
        # 1. 复制preinstalled-apps包到squashfs-root/opt/（squashfs已在_process_customizations_worker中统一解压）
        opt_dir = squashfs_root / "opt"
        opt_dir.mkdir(exist_ok=True)
        
        try:
            preinstalled_file = Path(preinstalled_apps_path)
            target_file = opt_dir / preinstalled_file.name
            
            # 检查源文件和目标文件是否是同一个文件
            if preinstalled_file.resolve() == target_file.resolve():
                logger.info(f"preinstalled-apps包已在目标目录中，跳过复制: {preinstalled_apps_path}")
                self.append_output(f"preinstalled-apps包已在目标目录中，跳过: {preinstalled_file.name}")
            else:
                shutil.copy2(preinstalled_apps_path, opt_dir)
                self.append_output(f"已复制preinstalled-apps包到: {opt_dir}/{preinstalled_file.name}")
            preinstalled_filename = preinstalled_file.name
        except Exception as e:
            self.append_output(f"复制preinstalled-apps包失败: {e}")
            logger.error(f"复制preinstalled-apps包到{opt_dir}失败: {e}")
            return
        
        # 3. 复制apps-third包到squashfs-root/opt/
        apps_third_dir = squashfs_root / "opt" / "apps-third"
        apps_third_dir.mkdir(exist_ok=True)
        
        for deb_path in deb_files:
            try:
                deb_file = Path(deb_path)
                target_file = apps_third_dir / deb_file.name
                
                # 检查源文件和目标文件是否是同一个文件
                if deb_file.resolve() == target_file.resolve():
                    logger.info(f"文件已在目标目录中，跳过复制: {deb_path}")
                    self.append_output(f"文件已在目标目录中，跳过: {deb_file.name}")
                else:
                    shutil.copy2(deb_path, apps_third_dir)
                    self.append_output(f"已复制apps-third包: {deb_file.name}")
            except Exception as e:
                self.append_output(f"复制apps-third包失败: {os.path.basename(deb_path)}: {e}")
                logger.error(f"复制apps-third包{deb_path}失败: {e}")
        
        # 4. chroot并安装preinstalled-apps
        self.append_output("准备chroot环境...")
        
        try:
            # 挂载必要的文件系统
            subprocess.run(["mount", "-t", "proc", "none", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["mount", "-t", "sysfs", "none", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["mount", "-t", "devpts", "none", f"{squashfs_root}/dev/pts"], check=True)
            
            # 准备chroot命令
            preinstalled_path_in_chroot = f"/opt/{preinstalled_filename}"
            chroot_cmd = f"chroot {squashfs_root} dpkg -i {preinstalled_path_in_chroot}"
            
            self.append_output(f"执行命令: {chroot_cmd}")
            result = subprocess.run(chroot_cmd, shell=True, check=True, capture_output=True, text=True)
            
            self.append_output("preinstalled-apps安装完成")
            if result.stdout:
                self.append_output(f"输出: {result.stdout}")
            
            # 卸载文件系统
            subprocess.run(["umount", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/dev/pts"], check=True)
            
        except subprocess.CalledProcessError as e:
            self.append_output(f"chroot安装失败: {e}")
            if e.stderr:
                self.append_output(f"错误输出: {e.stderr}")
            logger.error(f"chroot安装preinstalled-apps失败: {e}")
            
            # 确保卸载文件系统
            subprocess.run(["umount", "-lf", f"{squashfs_root}/proc"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/sys"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/dev/pts"], check=False)
            return
        
        # 5. 打包和清理将在_process_customizations_worker中统一处理
        
        self.append_output("华为机器apps-third包处理完成")

    def _repack_filesystem_worker(self):
        
        """Repacks the modified filesystem."""
        source_dir = LIVECD_DIR / "casper" / "squashfs-root"
        output_file = LIVECD_DIR / "casper" / "filesystem.squashfs"
        logger.info(f"Repacking {source_dir} into {output_file}")
        self.append_output(f"正在重新打包系统文件到 {output_file.name}...")

        if not source_dir.is_dir():
            raise NotADirectoryError(f"Source directory not found: {source_dir}")

        # Remove old squashfs file first
        if output_file.exists():
            logger.debug(f"Removing existing {output_file}")
            output_file.unlink()

        # mksquashfs command (no sudo)
        # Add -comp xz for better compression, -b 1M for block size, -processors 0 for all cores
        command = ['mksquashfs', str(source_dir), str(output_file)]
        self._run_command(command, step_name="重新打包系统")
        logger.info("Filesystem repacked successfully.")

    def _update_filesystem_size_worker(self):

        """Updates the filesystem.size file."""
        target_dir = LIVECD_DIR / "casper" / "squashfs-root"
        size_file = LIVECD_DIR / "casper" / "filesystem.size"
        logger.info(f"Calculating size of {target_dir} for {size_file}")
        self.append_output(f"正在计算 {target_dir.name} 大小...")

        if not target_dir.is_dir():
             logger.warning(f"Cannot update filesystem.size, source dir missing: {target_dir}")
             self.append_output(f"警告: 无法更新filesystem.size，源目录不存在。")
             return # 正确缩进的 return

        try:
            # Use du command to get size
            result = subprocess.run(
                ['du', '-sx', '--block-size=1', str(target_dir)],
                capture_output=True, text=True, check=True, encoding='utf-8'
            )
            size = result.stdout.split()[0]
            logger.info(f"Calculated size: {size}")
            size_file.write_text(size + '\n', encoding='utf-8')
            self.append_output(f"filesystem.size 更新为: {size}")
            logger.info(f"Updated {size_file} successfully.")
        except Exception as e: # 正确缩进的 except 块
            logger.error(f"Failed to update filesystem.size: {e}")
            raise RuntimeError(f"更新 filesystem.size 失败: {e}")
    
    def _process_custom_licenses(self):
        """处理自定义授权文件"""
        if not self.custom_licenses:
            self.append_output("未添加自定义授权文件，跳过此步骤")
            return
        
        self.append_output("开始处理自定义授权文件...")
        
        # 确保目标目录存在
        license_dir = LIVECD_DIR / "licenses"
        license_dir.mkdir(exist_ok=True)
        
        for license_file in self.custom_licenses:
            try:
                # 检查文件是否存在
                if not os.path.exists(license_file):
                    self.append_output(f"警告: 授权文件不存在: {license_file}")
                    continue
                
                # 处理ZIP文件
                if license_file.lower().endswith('.zip'):
                    self.append_output(f"解压授权文件: {license_file}")
                    
                    # 创建临时目录用于解压
                    with tempfile.TemporaryDirectory() as temp_dir:
                        # 解压ZIP文件到临时目录
                        with zipfile.ZipFile(license_file, 'r') as zip_ref:
                            zip_ref.extractall(temp_dir)
                
                        # 查找并复制.kyinfo和LICENSE文件
                        kyinfo_found = False
                        license_found = False
                        
                        # 递归查找所有文件
                        for root, dirs, files in os.walk(temp_dir):
                            for file in files:
                                file_path = os.path.join(root, file)
                                file_name = os.path.basename(file_path)
                                
                                # 处理.kyinfo文件
                                if file_name.endswith('.kyinfo'):
                                    # 直接复制到LIVECD_DIR根目录
                                    dest_path = os.path.join(LIVECD_DIR, file_name)
                                    shutil.copy2(file_path, dest_path)
                                    self.append_output(f"已复制授权信息文件: {file_name}")
                                    kyinfo_found = True
                                    logger.info(f"Copied license info file: {file_path} to {dest_path}")
                                
                                # 处理LICENSE文件
                                elif file_name.upper() == 'LICENSE':
                                    # 直接复制到LIVECD_DIR根目录
                                    dest_path = os.path.join(LIVECD_DIR, file_name)
                                    shutil.copy2(file_path, dest_path)
                                    self.append_output(f"已复制授权文件: {file_name}")
                                    license_found = True
                                    logger.info(f"Copied license file: {file_path} to {dest_path}")
                        
                        # 检查是否找到了授权文件
                        if not (kyinfo_found or license_found):
                            self.append_output(f"警告: 在ZIP文件中未找到授权文件: {os.path.basename(license_file)}")
                
                # 处理单个文件
                else:
                    # 获取文件名
                    file_name = os.path.basename(license_file)
                    
                    # 复制到目标目录
                    if file_name.endswith('.kyinfo'):
                        # .kyinfo文件复制到LIVECD_DIR根目录
                        dest_path = os.path.join(LIVECD_DIR, file_name)
                        shutil.copy2(license_file, dest_path)
                        self.append_output(f"已复制授权信息文件: {file_name}")
                    elif file_name.upper() == 'LICENSE':
                        # LICENSE文件复制到LIVECD_DIR根目录
                        dest_path = os.path.join(LIVECD_DIR, file_name)
                        shutil.copy2(license_file, dest_path)
                        self.append_output(f"已复制授权文件: {file_name}")
                    else:
                        # 其他文件复制到licenses目录
                        dest_path = os.path.join(license_dir, file_name)
                        shutil.copy2(license_file, dest_path)
                        self.append_output(f"已复制授权相关文件: {file_name}")
        
            except Exception as e:
                self.append_output(f"处理授权文件时出错: {os.path.basename(license_file)}: {e}")
                logger.error(f"Error processing license file {license_file}: {e}")
        
        # 检查licenses文件夹是否为空，如果为空则删除
        try:
            if license_dir.exists() and not any(license_dir.iterdir()):
                license_dir.rmdir()
                logger.info("删除了空的licenses文件夹")
        except Exception as e:
            logger.warning(f"删除空licenses文件夹时出错: {e}")
        
        self.append_output("自定义授权文件处理完成")

    def _process_kpb_patches(self):
        """处理kpb补丁文件"""
        if not hasattr(self, 'custom_kpb_patches') or not self.custom_kpb_patches:
            self.append_output("未添加kpb补丁文件，跳过此步骤")
            return
        
        self.append_output("开始处理kpb补丁文件...")
        
        # 确保目标目录存在（在镜像根目录创建patches目录）
        patches_dir = LIVECD_DIR / "patches"
        patches_dir.mkdir(exist_ok=True)
        
        for kpb_file in self.custom_kpb_patches:
            try:
                # 检查文件是否存在
                if not os.path.exists(kpb_file):
                    self.append_output(f"警告: kpb补丁文件不存在: {kpb_file}")
                    continue
                
                # 获取文件名
                file_name = os.path.basename(kpb_file)
                
                # 复制到patches目录
                dest_path = patches_dir / file_name
                shutil.copy2(kpb_file, dest_path)
                self.append_output(f"已复制kpb补丁文件: {file_name}")
                logger.info(f"Copied kpb patch file: {kpb_file} to {dest_path}")
        
            except Exception as e:
                self.append_output(f"处理kpb补丁文件时出错: {os.path.basename(kpb_file)}: {e}")
                logger.error(f"Error processing kpb patch file {kpb_file}: {e}")
        
        # 检查patches文件夹是否为空，如果为空则删除
        try:
            if patches_dir.exists() and not any(patches_dir.iterdir()):
                patches_dir.rmdir()
                logger.info("删除了空的patches文件夹")
        except Exception as e:
            logger.warning(f"删除空patches文件夹时出错: {e}")
        
        self.append_output("kpb补丁文件处理完成")

    def _process_personalization_files(self):
        """将用户选择的文件/文件夹复制到镜像根目录的custom文件夹，保留原有层级"""
        if not getattr(self, 'custom_personalization_items', None):
            self.append_output("未添加自定义文件夹内容，跳过此步骤")
            return

        self.append_output("开始处理自定义文件夹...")
        personalization_dir = LIVECD_DIR / "custom"
        personalization_dir.mkdir(parents=True, exist_ok=True)

        copied_count = 0
        for item_path in self.custom_personalization_items:
            src = Path(item_path)
            if not src.exists():
                self.append_output(f"警告: 路径不存在，跳过: {item_path}")
                logger.warning(f"Personalization source not found: {item_path}")
                continue

            dest = personalization_dir / src.name
            try:
                if src.is_file():
                    shutil.copy2(src, dest)
                    self.append_output(f"已复制文件: {src.name}")
                    logger.info(f"Copied personalization file: {src} -> {dest}")
                    copied_count += 1
                elif src.is_dir():
                    if dest.exists():
                        shutil.rmtree(dest)
                    shutil.copytree(src, dest)
                    self.append_output(f"已复制文件夹: {src.name}（保留原有层级）")
                    logger.info(f"Copied personalization directory: {src} -> {dest}")
                    copied_count += 1
                else:
                    self.append_output(f"警告: 不支持的路径类型，跳过: {item_path}")
            except Exception as e:
                self.append_output(f"复制失败: {src.name}: {e}")
                logger.error(f"Failed to copy personalization item {src}: {e}")

        self.append_output(f"自定义文件夹处理完成，已复制 {copied_count} 项到 custom/")
        logger.info(f"Personalization processing done, copied {copied_count} items")

    def _process_font_files(self):
        """处理字体文件"""
        if not hasattr(self, 'custom_fonts') or not self.custom_fonts:
            self.append_output("未选择字体文件，跳过此步骤")
            return
        
        self.append_output(f"\n--- 处理字体文件 ---")
        self.append_output(f"准备处理 {len(self.custom_fonts)} 个字体文件...")
        
        # 创建字体目标目录
        fonts_dir = LIVECD_DIR / "fonts"
        fonts_dir.mkdir(parents=True, exist_ok=True)
        
        # 处理每个字体文件
        for font_path in self.custom_fonts:
            font_path = Path(font_path)
            if font_path.is_file():
                if font_path.suffix.lower() == '.ttf':
                    # 直接复制TTF文件
                    logger.info(f"Copying TTF font: {font_path} to {fonts_dir}")
                    shutil.copy2(font_path, fonts_dir)
                    self.append_output(f"已复制字体: {font_path.name}")
                elif font_path.suffix.lower() == '.zip':
                    # 解压ZIP文件
                    self.append_output(f"解压字体包: {font_path.name}")
                    try:
                        with zipfile.ZipFile(font_path, 'r') as zip_ref:
                            # 只提取TTF文件
                            for file in zip_ref.namelist():
                                if file.lower().endswith('.ttf'):
                                    zip_ref.extract(file, fonts_dir)
                                    self.append_output(f"已提取字体: {file}")
                    except Exception as e:
                        logger.error(f"Failed to extract font zip: {e}")
                        self.append_output(f"警告: 解压字体包失败: {font_path.name}, 错误: {e}")
                else:
                    logger.warning(f"Unsupported font file format: {font_path}")
                    self.append_output(f"警告: 不支持的字体文件格式: {font_path.name}")
            else:
                logger.warning(f"Font file not found: {font_path}")
                self.append_output(f"警告: 字体文件未找到: {font_path.name}")
        
        self.append_output("字体文件处理完成")

    def _run_command(self, command, cwd=None, step_name="执行命令", check=True, capture=True):
        """Executes a shell command, logs output, and updates GUI."""
        cmd_str = ' '.join(command) if isinstance(command, list) else command
        logger.info(f"Executing [{step_name}] in {cwd or '.'}: {cmd_str}")
        self.append_output(f"\n> {cmd_str}") # Add newline before command

        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE, # 标准缩进
            stderr=subprocess.STDOUT, # 标准缩进
            text=True,
            encoding='utf-8',
            errors='replace',
            shell=isinstance(command, str), # Use shell=True if command is a string
            cwd=cwd
        )

        while True:
            line = process.stdout.readline()
            if not line and process.poll() is not None:
                break
            if line:
                line_strip = line.strip()
                self.append_output(line_strip)
                logger.debug(f"[{step_name} Output]: {line_strip}") # Log command output at debug level

        rc = process.poll()
        logger.info(f"Command ({step_name}) finished with exit code {rc}")
        if check and rc != 0:
            error_msg = f"{step_name} 失败 (退出码: {rc})"
            self.append_output(f"错误: {error_msg}")
            raise subprocess.CalledProcessError(rc, cmd_str)
        return rc

    # --- Worker Helper Methods ---

    def _mount_iso_worker(self):
        """Mounts the selected ISO."""
        iso_path = self.iso_path_var.get()
        logger.info(f"Mounting {iso_path} to {MNT_DIR}")
        try:
            # No sudo needed
            self._run_command(['mount', '-o', 'loop,ro', iso_path, str(MNT_DIR)], step_name="挂载ISO")
            logger.info("ISO mounted successfully.")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to mount ISO: {e}")
            raise RuntimeError(f"挂载ISO失败: {e}") # Re-raise with clearer message

    def _copy_iso_contents_worker(self):
        """Copies contents from mounted ISO to livecd dir using rsync for progress."""
        logger.info(f"Copying files from {MNT_DIR} to {LIVECD_DIR}")
        self.append_output(f"正在复制ISO内容到 {LIVECD_DIR}...")

        # Ensure LIVECD_DIR exists and is empty
        if LIVECD_DIR.exists():
            shutil.rmtree(LIVECD_DIR)
        LIVECD_DIR.mkdir()

        # Use rsync for potentially better progress reporting and efficiency
        command = ['rsync', '-ah', '--info=progress2', str(MNT_DIR) + '/', str(LIVECD_DIR)]
        logger.info(f"Executing: {' '.join(command)}")
        self.append_output(f"\n> {' '.join(command)}")

        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')

        last_percent = -1
        while True:
            line = process.stdout.readline()
            if not line and process.poll() is not None:
                break
            if line:
                line_strip = line.strip()
                self.append_output(line_strip) # Show rsync output directly
                logger.debug(f"[Copy Output]: {line_strip}")
                # Try to parse progress percentage
                match = re.search(r'(\d+)%', line_strip)
                if match:
                    try:
                        percent = int(match.group(1))
                        if percent != last_percent:
                             self.update_progress(message=f"复制文件: {percent}%")
                             last_percent = percent
                    except ValueError:
                        pass # Ignore if percentage parsing fails

        rc = process.poll()
        if rc != 0:
            logger.error(f"rsync failed with exit code {rc}")
            raise RuntimeError(f"复制ISO内容失败 (rsync 退出码: {rc})")
        logger.info("ISO contents copied successfully.")
        self.update_progress(message="文件复制完成")


    def _configure_unattended_worker(self):
        """Configures unattended installation files (grub.cfg, ky_install.conf)."""
        logger.info("Configuring unattended installation...")
        self.append_output("配置无人值守安装...")

        try:
            # --- Modify ky_install.conf ---
            if self.is_huawei_detected:
                ky_install_config_path = LIVECD_DIR / "kyple-installer.cfg"
            else:
                ky_install_config_path = LIVECD_DIR / "ky-installer.cfg"


            if not ky_install_config_path.is_file():
                logger.warning(f"Unattended config file not found: {ky_install_config_path}")
                self.append_output(f"警告: 未找到无人值守配置文件 {ky_install_config_path}")
                # Decide if this is a fatal error or just a warning
                # raise FileNotFoundError(f"找不到无人值守配置文件: {ky_install_config_path}")
            else:
                logger.info(f"Modifying {ky_install_config_path}")
                config_lines = ky_install_config_path.read_text(encoding='utf-8').splitlines()

                # --- 获取 GUI 输入值 ---
                hostname = self.hostname_var.get().strip()
                username = self.username_var.get().strip()
                password = self.password_var.get().strip() # 获取原始密码
                data_device = self.data_device.get().strip()
                data_fs_type = self.data_fs_type_var.get().strip()

                # --- 构建仅包含非空输入的 updates 字典 ---
                updates = {}
                if hostname:
                    updates['hostname'] = hostname
                if username:
                    updates['username'] = username
                if password:
                    updates['password'] = f'@ByteArray({password})' # 仅在密码非空时添加并格式化
                
                # 处理data-device参数：如果启用了双硬盘策略，必须保证data-device值为空
                is_dual_disk_enabled = hasattr(self, 'baixinx_dual_disk') and self.baixinx_dual_disk.get()
                if is_dual_disk_enabled:
                    # 启用双硬盘策略时，强制设置data-device为空
                    updates['data-device'] = ""
                    logger.info("双硬盘策略已启用，data-device设置为空")
                    self.append_output("双硬盘策略已启用，data-device设置为空")
                else:
                    # 未启用双硬盘策略时，正常处理data-device
                    if data_device:
                        updates['data-device'] = data_device
                        logger.info(f"未启用双硬盘策略，data-device设置为: {data_device}")

                # 布尔值总是添加，因为它们来自复选框
                updates['autologin'] = '1' if self.auto_login.get() else '0'
                updates['factory-backup'] = '1' if self.factory_backup.get() else '0'
                updates['reboot'] = '1' if self.is_reboot.get() else '0'

                logger.info(f"Updates based on non-empty inputs: {updates}")
                logger.info(f"Data partition filesystem type from GUI: '{data_fs_type}'")

                # 标志：是否需要修改文件系统类型相关设置
                modify_fs_type_settings = bool(data_fs_type) # 如果 data_fs_type 非空则为 True
 
                new_lines = []
                updated_keys = set()
                # Flags to track if specific lines were handled to avoid duplicate processing
                data_mount_line_updated = False
                custom_data_line_updated = False
                # Flags for NTFS specific changes
                disk_custom_found = False
                in_setting_section = False
                filesystem_line_updated = False

                for line in config_lines:
                    stripped_line = line.strip()
                    # Skip empty lines and comments
                    if not stripped_line or stripped_line.startswith('#'):
                        new_lines.append(line)
                        continue

                    # --- 检查是否进入或离开 [setting] 段 ---
                    if stripped_line.startswith('[') and stripped_line.endswith(']'):
                        if stripped_line == '[setting]':
                            in_setting_section = True
                            logger.debug("Entered [setting] section.")
                        else:
                            in_setting_section = False # 离开 [setting] 或进入其他段

                    # --- 处理 custom-data 行 (仅当需要修改 FS 类型时才修改 fs=) ---
                    if 'custom-data=' in line:
                        logger.debug(f"Found potential custom-data line: {line}")
                        # 提取 key 和 value (带引号)
                        key_part, value_part_quoted = line.split('=', 1)
                        # 去掉引号获取实际值
                        value_part = value_part_quoted.strip().strip('"')
                        # 按分号分割参数
                        params = value_part.split(';')
                        new_params = []
                        fs_updated_in_custom = False
                        for param in params:
                            stripped_param = param.strip()
                            if modify_fs_type_settings and stripped_param.startswith('fs='):
                                new_params.append(f"fs={data_fs_type}")
                                fs_updated_in_custom = True
                                logger.debug(f"  Updated fs= parameter within custom-data to {data_fs_type}")
                            elif stripped_param: # 保留其他非空参数
                                new_params.append(stripped_param)

                        # 如果原始值中没有 fs=，则添加它 (如果需要且允许修改)
                        
                        # 重构 custom-data 的值和整行
                        new_value = ';'.join(new_params) # Join parameters with semicolon
                        # 确保如果值不为空，则以分号结尾
                        if new_value and not new_value.endswith(';'):
                            new_value += ';'

                        # 保留原始缩进
                        indent = line[:len(line) - len(line.lstrip())]
                        new_lines.append(f'{indent}{key_part.strip()}="{new_value}"')
                        custom_data_line_updated = True
                        logger.info(f"Updated custom-data line: {new_lines[-1]}")
                        continue

                    # --- 处理 [setting] 段内的 FileSystem 行 (仅当 FS 类型为 ntfs 时修改) ---
                    if modify_fs_type_settings and data_fs_type == "ntfs" and in_setting_section and stripped_line.startswith('FileSystem='):
                        logger.debug(f"Found FileSystem line in [setting]: {line}")
                        key_part, value_part_quoted = line.split('=', 1)
                        value_part = value_part_quoted.strip().strip('"')
                        fs_list = [fs.strip() for fs in value_part.split(';') if fs.strip()] # 获取现有列表

                        # 如果选择的是 ntfs 且列表中没有 ntfs，则添加
                        if data_fs_type == "ntfs" and "ntfs" not in fs_list:
                            fs_list.append("ntfs") # data_fs_type 已经是 "ntfs"
                            logger.info("Added 'ntfs' to FileSystem list.")

                        # 重构 FileSystem 行
                        new_value = ';'.join(fs_list)
                        indent = line[:len(line) - len(line.lstrip())]
                        new_lines.append(f'{indent}{key_part.strip()}="{new_value}"')
                        filesystem_line_updated = True
                        continue # 处理完 FileSystem 行，跳到下一行

                    # --- 处理 disk-custom 行 ---
                    # 如果用户指定了任何数据盘分区格式，则将 disk-custom 设为 1
                    if stripped_line.startswith('disk-custom='):
                        logger.debug(f"Found disk-custom line: {line}")
                        disk_custom_found = True
                        if modify_fs_type_settings: # 只要指定了分区格式 (modify_fs_type_settings is True)
                            new_lines.append("disk-custom=true") # 强制设为 1
                            logger.info("Set disk-custom=1 because a data filesystem type was specified.")
                        else:
                            new_lines.append(line) # 未指定分区格式，保留原始行
                        continue

                    # Handle simple key=value updates
                    if '=' in line and not custom_data_line_updated: # 避免重复处理 custom-data
                        key = line.split('=', 1)[0].strip()
                        if key in updates:
                            new_lines.append(f"{key}={updates[key]}")
                            updated_keys.add(key)
                            continue # Move to next line after simple update

                    # --- Special handling for mount=/data line ---
                    # 仅在未处理 custom-data、该行包含 mount=/data 且允许修改 FS 类型时执行
                    if not custom_data_line_updated and 'mount=/data' in stripped_line:
                        logger.debug(f"Found potential standalone mount=/data line: {line}")
                        parts = stripped_line.split()
                        new_parts = []
                        fs_updated = False
                        for part in parts:
                            stripped_part = part.strip()
                            if modify_fs_type_settings and stripped_part.startswith('fs='):
                                new_parts.append(f"fs={data_fs_type}")
                                fs_updated = True
                                logger.debug(f"  Updated fs= parameter within mount=/data to {data_fs_type}")
                            else:
                                new_parts.append(stripped_part) # 保留其他部分
                        # If fs= was not found, add it
                        if not fs_updated: # 仅在 mount=/data 行本身没有 fs= 时添加
                            new_parts.append(f"fs={data_fs_type}")
                            logger.debug(f"  Added fs= parameter: {data_fs_type}")

                        # 重构 mount=/data 的值和整行
                        new_value = ';'.join(new_parts)
                        # 保留原始缩进
                        indent = line[:len(line) - len(line.lstrip())]
                        new_lines.append(f'{indent}{new_value}')
                        data_mount_line_updated = True
                        logger.info(f"Updated standalone mount=/data line: {new_lines[-1]}")
                        continue # Move to next line after handling mount=/data

                    # 保留未被以上逻辑处理的行
                    new_lines.append(line)

                # --- 循环结束后处理 ---
                # 如果指定了任何分区格式 (且非空) 但文件中没有 disk-custom 行，则添加它
                if modify_fs_type_settings and not disk_custom_found:
                    new_lines.append("disk-custom=true")
                    logger.info("Added disk-custom=true because it was missing and NTFS was selected.")

                ky_install_config_path.write_text('\n'.join(new_lines) + '\n', encoding='utf-8')
                self.append_output("ky_install.conf 更新完成。")
                logger.info("ky_install.conf updated.")

            # --- Modify grub.cfg ---
            if self.is_huawei_detected:
                # 华为机器使用不同的 grub.cfg 路径
                grub_cfg_path = LIVECD_DIR / "EFI" / "kylin" / "grub.cfg"
            else:
                # 标准路径
                grub_cfg_path = LIVECD_DIR / "boot" / "grub" / "grub.cfg"

            if not grub_cfg_path.is_file():
                logger.warning(f"grub.cfg not found: {grub_cfg_path}")
                self.append_output(f"警告: 未找到 grub.cfg 文件 {grub_cfg_path}")
            else:
                logger.info(f"Modifying {grub_cfg_path}")
                content = grub_cfg_path.read_text(encoding='utf-8')
                
                # 根据机器类型使用不同的正则表达式
                if self.is_huawei_detected:
                    # 华为机器特殊处理 - 使用更精确的模式匹配
                    huawei_pattern = r'(menuentry\s+"Install Kylin-Desktop V10 SP1"\s+\{\s*\n\s*linux\s+/casper/vmlinuz\s+)([^}]*?)(\s*\n\s*initrd)'
                    if re.search(huawei_pattern, content, re.DOTALL):
                        # 根据是否检测到panguw决定使用哪个参数
                        if hasattr(self, 'is_panguw_detected') and self.is_panguw_detected:
                            content = re.sub(huawei_pattern, r'\1\2 test-mod\3', content, flags=re.DOTALL)
                            self.append_output("已为华为panguw机器添加test-mod参数")
                            logger.info("Added test-mod parameter to Huawei panguw machine grub entry")
                        else:
                            content = re.sub(huawei_pattern, r'\1\2 automatic\3', content, flags=re.DOTALL)
                            self.append_output("已为华为机器添加无人值守参数")
                            logger.info("Added  parameter to Huawei machine grub entry")
                    else:
                        # 如果没有找到特定模式，使用通用方法
                        if hasattr(self, 'is_panguw_detected') and self.is_panguw_detected:
                            content = re.sub(r'(linux\s+/casper/vmlinuz.*?)(quiet|splash|security=|$)', r'\1 test-mod \2', content, flags=re.MULTILINE)
                            self.append_output("使用通用方法为华为panguw机器添加test-mod参数")
                            logger.info("Used generic method to add test-mod parameter for Huawei panguw machine")
                        else:
                            content = re.sub(r'(linux\s+/casper/vmlinuz.*?)(quiet|splash|security=|$)', r'\1 automatic \2', content, flags=re.MULTILINE)
                            self.append_output("使用通用方法为华为机器添加无人值守参数")
                            logger.info("Used generic method to add automatic parameter for Huawei machine")
                    
                    # 华为机器特殊处理：删除 oem-config/enable=true 参数
                    content = re.sub(r'\s*oem-config/enable=true\s*', ' ', content)
                    self.append_output("已为华为机器删除 oem-config/enable=true 参数")
                    logger.info("Removed oem-config/enable=true parameter for Huawei machine")
                    
                    # 华为机器特殊处理：如果取消出厂备份，删除 factory-backup 参数
                    if not self.factory_backup.get():
                        content = re.sub(r'\s*factory-backup\s*', ' ', content)
                        self.append_output("已为华为机器删除 factory-backup 参数")
                        logger.info("Removed factory-backup parameter for Huawei machine")
                else:
                    # 非华为机器使用通用处理
                    if hasattr(self, 'is_panguw_detected') and self.is_panguw_detected:
                        content = re.sub(r'(linux\s+/casper/vmlinuz.*?)(quiet|splash|security=|$)', r'\1 test-mod \2', content, flags=re.MULTILINE)
                        self.append_output("已为非华为panguw机器添加test-mod参数")
                        logger.info("Added test-mod parameter to non-Huawei panguw machine")
                    else:
                        content = re.sub(r'(linux\s+/casper/vmlinuz.*?)(quiet|splash|security=|$)', r'\1 automatic \2', content, flags=re.MULTILINE)
                        self.append_output("已为非华为机器添加无人值守参数")
                        logger.info("Added automatic parameter to non-Huawei machine")
                
                # 移除特定菜单项
                content = re.sub(r'^\s*menuentry\s+"Try to fix grub for local system".*?^\s*}\s*$', '', content, flags=re.DOTALL | re.MULTILINE)
                content = re.sub(r'^\s*menuentry\s+"Try Kylin-Desktop V10-SP1 without installing".*?^\s*}\s*$', '', content, flags=re.DOTALL | re.MULTILINE)
                
                grub_cfg_path.write_text(content, encoding='utf-8')
                self.append_output("grub.cfg 更新完成。")
                logger.info("grub.cfg updated.")


            logger.info("Unattended configuration finished.")

        except Exception as e:
            logger.error(f"Failed to configure unattended install: {e}")
            # 're' should be imported at the top of the file now
            raise RuntimeError(f"配置无人值守失败: {e}")

    def _generate_md5sum_worker(self):
        """Generates the md5sum.txt file for the livecd contents."""
        md5_file = LIVECD_DIR / "md5sum.txt"
        logger.info(f"Generating MD5 checksums in {md5_file}")
        self.append_output("正在生成MD5校验文件...")

        # Use find and md5sum command
        # Exclude boot.cat and the md5sum.txt itself
        command = f"find . -type f -print0 | xargs -0 md5sum | grep -vE 'isolinux/boot.cat|./md5sum.txt' > {md5_file.name}"
        # Run this command with shell=True and cwd=LIVECD_DIR
        self._run_command(command, cwd=str(LIVECD_DIR), step_name="生成MD5校验")
        logger.info("MD5 checksum file generated.")

    def _generate_iso_worker(self, final_iso_path: Path):
        """Generates the final ISO image using genisoimage."""
        logger.info(f"Generating final ISO image: {final_iso_path}")
        self.append_output(f"正在封装最终镜像到 {final_iso_path}...")

        # Determine genisoimage arguments based on presence of isolinux.bin
        isolinux_bin = LIVECD_DIR / "isolinux" / "isolinux.bin"
        efi_img = LIVECD_DIR / "boot" / "grub" / "efi.img" # Check if exists?

        # Base arguments
        iso_command = [
            'genisoimage', '-sysid', 'LINUX',
            '-joliet', '-joliet-long', '-full-iso9660-filenames',
            '-rational-rock', '-translation-table', '-udf',
            '-input-charset', 'utf-8', # Ensure UTF-8 input
            '-cache-inodes', '-allow-multidot', '-untranslated-filenames',
            '-verbose', # Get more output
            '-o', str(final_iso_path)
        ]

        # Add El Torito boot options
        if isolinux_bin.exists():
            logger.debug("isolinux.bin found, adding El Torito options.")
            iso_command.extend([
                '-volid', 'KYLIN_CUSTOM_ISO', # Example Volume ID
                '-no-emul-boot', '-boot-load-size', '4', '-boot-info-table',
                '-eltorito-boot', 'isolinux/isolinux.bin',
                '-eltorito-catalog', 'isolinux/boot.cat'
            ])
        else:
            logger.debug("isolinux.bin not found, using different volid.")
            iso_command.extend(['-volid', 'KYLIN_V10SP1_CUSTOM']) # Example

        # Add EFI boot option (always?)
        if efi_img.exists():
             logger.debug("efi.img found, adding EFI boot option.")
             iso_command.extend(['-eltorito-alt-boot', '-no-emul-boot', '-efi-boot', 'boot/grub/efi.img'])
        else:
             logger.warning(f"EFI boot image not found: {efi_img}")
             self.append_output(f"警告: 未找到EFI启动镜像 {efi_img.name}")


        # Add source directory at the end
        iso_command.append('.')

        # Execute the command
        self._run_command(iso_command, cwd=str(LIVECD_DIR), step_name="封装镜像")
        logger.info(f"Final ISO image generated successfully: {final_iso_path}")

    def open_custom_functions_window(self):
        """打开授权文件选择窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开授权文件选择窗口")
        custom_win = tk.Toplevel(self.root)
        custom_win.title("添加授权文件")
        custom_win.geometry("450x300")  # 减小窗口大小
        custom_win.transient(self.root)
        custom_win.grab_set()  # 模态窗口

        # --- 授权文件选择框架 ---
        main_frame = ttk.Frame(custom_win, padding="10")  # 减小内边距
        main_frame.pack(fill=tk.BOTH, expand=True)

        license_frame = ttk.LabelFrame(main_frame, text="授权文件列表", padding="5")  # 减小内边距
        license_frame.pack(fill=tk.BOTH, expand=True)
        license_frame.columnconfigure(0, weight=1)
        license_frame.rowconfigure(0, weight=1)  # 让列表框可以扩展

        # 使用局部变量存储授权文件列表
        popup_license_listbox = tk.Listbox(license_frame, height=8)  # 减小高度
        popup_license_listbox.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        
        # 添加滚动条
        scrollbar = ttk.Scrollbar(license_frame, orient='vertical', command=popup_license_listbox.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S), pady=5)
        popup_license_listbox['yscrollcommand'] = scrollbar.set

        # 填充当前已选授权文件
        for lic in self.custom_licenses:
            popup_license_listbox.insert(tk.END, lic)

        # 按钮框架
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(10, 0))

        # 选择授权文件的辅助函数
        def select_licenses_popup():
            if not self._check_iso_prepared():
                return
            filepaths = filedialog.askopenfilenames(title="选择授权文件")
            if filepaths:
                current_items = set(popup_license_listbox.get(0, tk.END))
                added_count = 0
                for fp in filepaths:
                    if fp not in current_items:
                        popup_license_listbox.insert(tk.END, fp)
                        added_count += 1
                if added_count > 0:
                    logger.info(f"添加了 {added_count} 个新授权文件")

        # 左侧放操作按钮
        action_frame = ttk.Frame(btn_frame)
        action_frame.pack(side=tk.LEFT)
        
        ttk.Button(action_frame, text="添加", command=select_licenses_popup).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="清空", command=lambda: popup_license_listbox.delete(0, tk.END)).pack(side=tk.LEFT, padx=5)

        # 右侧放确定按钮
        ttk.Button(btn_frame, text="确定", command=lambda: on_ok()).pack(side=tk.RIGHT, padx=5)

        # --- 确定按钮功能 ---
        def on_ok():
            # 保存选择的授权文件
            self.custom_licenses = list(popup_license_listbox.get(0, tk.END))
            logger.info(f"保存了 {len(self.custom_licenses)} 个授权文件")
            custom_win.destroy()
            self.update_settings_status()  # 添加这一行

        # 等待窗口关闭
        self.root.wait_window(custom_win)

    def open_kms_config_window(self):
        """打开KMS配置窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开KMS配置窗口")
        kms_win = tk.Toplevel(self.root)
        kms_win.title("KMS配置")
        kms_win.geometry("400x250")  # 增加窗口高度，确保按钮完全显示
        kms_win.transient(self.root)
        kms_win.grab_set()  # 模态窗口
        
        # 创建KMS配置变量
        if not hasattr(self, 'kms_enabled'):
            self.kms_enabled = tk.BooleanVar(value=False)
        if not hasattr(self, 'kms_ip_var'):
            self.kms_ip_var = tk.StringVar(value="")
        
        # 主框架
        main_frame = ttk.Frame(kms_win, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # KMS设置框架
        kms_frame = ttk.LabelFrame(main_frame, text="KMS服务器设置", padding="10")
        kms_frame.pack(fill=tk.BOTH, expand=True)
        
        # 启用KMS复选框
        ttk.Checkbutton(kms_frame, text="启用KMS激活", 
                       variable=self.kms_enabled).pack(anchor=tk.W, pady=5)
        
        # IP地址输入框
        ip_frame = ttk.Frame(kms_frame)
        ip_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(ip_frame, text="服务器IP:").pack(side=tk.LEFT, padx=(0, 10))
        ttk.Entry(ip_frame, textvariable=self.kms_ip_var, width=25).pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        # 按钮框架 - 确保有足够的空间
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(20, 0))
        
        # 确定按钮功能
        def on_ok():
            # 验证IP地址格式（如果启用了KMS）
            if self.kms_enabled.get():
                ip = self.kms_ip_var.get().strip()
                if not ip:
                    messagebox.showerror("错误", "请输入KMS服务器IP地址")
                    return
                
                # 简单的IP地址格式验证
                ip_pattern = r"^(\d{1,3}\.){3}\d{1,3}$"
                if not re.match(ip_pattern, ip):
                    messagebox.showerror("错误", "请输入有效的IP地址格式 (例如: 192.168.1.1)")
                    return
            
            logger.info(f"KMS配置已保存: 启用={self.kms_enabled.get()}, IP={self.kms_ip_var.get()}")
            kms_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        # 取消按钮功能
        def on_cancel():
            logger.info("取消KMS配置")
            kms_win.destroy()
        
        # 先添加取消按钮，再添加确定按钮（这样确定按钮会在右边）
        cancel_btn = ttk.Button(btn_frame, text="取消", width=10, command=on_cancel)
        cancel_btn.pack(side=tk.RIGHT, padx=5)
        
        ok_btn = ttk.Button(btn_frame, text="确定", width=10, command=on_ok)
        ok_btn.pack(side=tk.RIGHT, padx=5)
        
        # 确保窗口大小足够显示所有内容
        kms_win.update_idletasks()
        min_height = main_frame.winfo_reqheight() + 50  # 添加一些额外空间
        current_width = kms_win.winfo_width()
        kms_win.geometry(f"{current_width}x{min_height}")
        
        # 设置最小窗口大小
        kms_win.minsize(400, min_height)
        
        # 将窗口居中显示在主窗口上
        self.center_window(kms_win, self.root)
        
        # 等待窗口关闭
        self.root.wait_window(kms_win)

    def center_window(self, window, parent=None):
        """将窗口居中显示在父窗口上"""
        window.update_idletasks()
        
        # 获取窗口尺寸
        window_width = window.winfo_width()
        window_height = window.winfo_height()
        
        # 如果有父窗口，则居中于父窗口
        if parent:
            parent_x = parent.winfo_rootx()
            parent_y = parent.winfo_rooty()
            parent_width = parent.winfo_width()
            parent_height = parent.winfo_height()
            
            x = parent_x + (parent_width - window_width) // 2
            y = parent_y + (parent_height - window_height) // 2
        # 否则居中于屏幕
        else:
            screen_width = window.winfo_screenwidth()
            screen_height = window.winfo_screenheight()
            
            x = (screen_width - window_width) // 2
            y = (screen_height - window_height) // 2
        
        # 设置窗口位置
        window.geometry(f"+{x}+{y}")

    def open_fonts_window(self):
        """打开字体添加窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开字体添加窗口")
        fonts_win = tk.Toplevel(self.root)
        fonts_win.title("添加字体")
        fonts_win.geometry("450x300")  # 与授权文件窗口大小一致
        fonts_win.transient(self.root)
        fonts_win.grab_set()  # 模态窗口

        # --- 字体选择框架 ---
        main_frame = ttk.Frame(fonts_win, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        fonts_frame = ttk.LabelFrame(main_frame, text="字体文件列表", padding="5")
        fonts_frame.pack(fill=tk.BOTH, expand=True)
        fonts_frame.columnconfigure(0, weight=1)
        fonts_frame.rowconfigure(0, weight=1)  # 让列表框可以扩展

        # 创建或获取字体文件列表
        if not hasattr(self, 'custom_fonts'):
            self.custom_fonts = []

        # 使用局部变量存储字体文件列表
        popup_fonts_listbox = tk.Listbox(fonts_frame, height=8, selectmode=tk.EXTENDED)
        popup_fonts_listbox.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        
        # 添加滚动条
        scrollbar = ttk.Scrollbar(fonts_frame, orient='vertical', command=popup_fonts_listbox.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S), pady=5)
        popup_fonts_listbox['yscrollcommand'] = scrollbar.set

        # 填充当前已选字体文件
        for font in self.custom_fonts:
            popup_fonts_listbox.insert(tk.END, font)

        # 按钮框架
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(10, 0))

        # 选择字体文件的辅助函数
        def select_fonts_popup():
            if not self._check_iso_prepared():
                return
            filepaths = filedialog.askopenfilenames(
                title="选择字体文件", 
                filetypes=[
                    ("TTF字体文件", "*.ttf"),
                    ("ZIP字体包", "*.zip"),
                    ("TTF字体包", "*.TTF"),
                    ("所有文件", "*.*")
                ]
            )
            if filepaths:
                for fp in filepaths:
                    # 检查文件是否已在列表中
                    if fp not in popup_fonts_listbox.get(0, tk.END):
                        popup_fonts_listbox.insert(tk.END, fp)
                        logger.info(f"添加字体文件: {fp}")

        # 删除选中字体文件
        def remove_selected_fonts():
            selected_indices = popup_fonts_listbox.curselection()
            if selected_indices:
                # 从后往前删除，避免索引变化
                for i in sorted(selected_indices, reverse=True):
                    logger.info(f"删除字体文件: {popup_fonts_listbox.get(i)}")
                    popup_fonts_listbox.delete(i)

        # 左侧放操作按钮
        action_frame = ttk.Frame(btn_frame)
        action_frame.pack(side=tk.LEFT)
        
        ttk.Button(action_frame, text="添加", command=select_fonts_popup).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="删除", command=remove_selected_fonts).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="清空", command=lambda: popup_fonts_listbox.delete(0, tk.END)).pack(side=tk.LEFT, padx=5)

        # 右侧放确定按钮
        ttk.Button(btn_frame, text="确定", command=lambda: on_ok()).pack(side=tk.RIGHT, padx=5)

        # --- 确定按钮功能 ---
        def on_ok():
            # 保存选择的字体文件
            self.custom_fonts = list(popup_fonts_listbox.get(0, tk.END))
            logger.info(f"保存了 {len(self.custom_fonts)} 个字体文件")
            fonts_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        # 等待窗口关闭
        self.root.wait_window(fonts_win)

    def open_security_settings(self):
        """打开安全中心设置窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开安全中心设置窗口")
        security_win = tk.Toplevel(self.root)
        security_win.title("安全中心设置")
        security_win.geometry("450x500")  # 增加窗口高度以容纳新选项
        security_win.transient(self.root)
        security_win.grab_set()  # 模态窗口

        # 创建变量（如果不存在）
        if not hasattr(self, 'disable_firewall'):
            self.disable_firewall = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_app_protection'):
            self.disable_app_protection = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_network_protection'):
            self.disable_network_protection = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_device_security'):
            self.disable_device_security = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_signature'):
            self.disable_signature = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_process_protection'):
            self.disable_process_protection = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_file_protection'):
            self.disable_file_protection = tk.BooleanVar(value=False)
        if not hasattr(self, 'disable_kid'):
            self.disable_kid = tk.BooleanVar(value=False)

        # 主框架 - 减少内边距
        main_frame = ttk.Frame(security_win, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # 安全中心设置框架
        security_frame = ttk.LabelFrame(main_frame, text="安全中心设置选项", padding="10")
        security_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # 添加复选框
        ttk.Checkbutton(security_frame, text="关闭防火墙", 
                       variable=self.disable_firewall).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭应用保护", 
                       variable=self.disable_app_protection).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭网络保护", 
                       variable=self.disable_network_protection).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭设备安全", 
                       variable=self.disable_device_security).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭进程保护", 
                       variable=self.disable_process_protection).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭文件保护", 
                       variable=self.disable_file_protection).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭签名状态", 
                       variable=self.disable_signature).pack(anchor=tk.W, pady=5)
        
        ttk.Checkbutton(security_frame, text="关闭kid状态", 
                       variable=self.disable_kid).pack(anchor=tk.W, pady=5)

        # 按钮框架 - 放在底部，确保有足够空间
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(30, 10))  # 增加上方间距

        # 确定按钮功能
        def on_ok():
            logger.info(f"安全中心设置已保存: 防火墙={self.disable_firewall.get()}, "
                       f"应用保护={self.disable_app_protection.get()}, "
                       f"网络保护={self.disable_network_protection.get()}, "
                       f"设备安全={self.disable_device_security.get()}, "
                       f"进程保护={self.disable_process_protection.get()}, "
                       f"文件保护={self.disable_file_protection.get()}, "
                       f"签名状态={self.disable_signature.get()}, "
                       f"kid状态={self.disable_kid.get()}")
            security_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        # 添加按钮 - 使用更大的宽度
        ttk.Button(btn_frame, text="确定", width=15, command=on_ok).pack(side=tk.RIGHT, padx=10)
        
        # 设置最小窗口大小，确保按钮可见
        security_win.update_idletasks()
        min_height = main_frame.winfo_reqheight() + 50
        security_win.minsize(450, min_height)
        
        # 将窗口居中显示
        self.center_window(security_win, self.root)
        
        # 等待窗口关闭
        self.root.wait_window(security_win)

    def update_settings_status(self):
        """更新设置状态显示区域的内容"""
        # 启用文本框编辑
        self.settings_status_text.config(state='normal')
        # 清空当前内容
        self.settings_status_text.delete('1.0', tk.END)
        
        # 添加镜像类型状态
        self.settings_status_text.insert(tk.END, " 镜像类型: ", "section")
        if self.is_huawei_detected:
            self.settings_status_text.insert(tk.END, "华为版本 (自动检测)\n", "enabled")
        else:
            iso_path = self.iso_path_var.get()
            if iso_path:
                self.settings_status_text.insert(tk.END, "非华为版本\n", "enabled")
            else:
                self.settings_status_text.insert(tk.END, "未选择镜像\n", "disabled")
        
        # 添加无人值守设置状态
        self.settings_status_text.insert(tk.END, "\n 无人值守: ", "section")
        if self.enable_unattended.get():
            self.settings_status_text.insert(tk.END, "已启用\n", "enabled")
            self.settings_status_text.insert(tk.END, f"  • 用户名: {self.username_var.get()}\n", "item")
            self.settings_status_text.insert(tk.END, f"  • 主机名: {self.hostname_var.get()}\n", "item")
            self.settings_status_text.insert(tk.END, f"  • 自动登录: {'是' if self.auto_login.get() else '否'}\n", "item")
            self.settings_status_text.insert(tk.END, f"  • 出厂备份: {'是' if self.factory_backup.get() else '否'}\n", "item")
            self.settings_status_text.insert(tk.END, f"  • 安装后重启: {'是' if self.is_reboot.get() else '否'}\n", "item")
            
            # 数据盘信息
            data_device = self.data_device.get().strip()
            data_fs_type = self.data_fs_type_var.get().strip()
            if data_device or data_fs_type:
                self.settings_status_text.insert(tk.END, f"  • 数据盘: {data_device or '未设置'}\n", "item")
                self.settings_status_text.insert(tk.END, f"  • 分区格式: {data_fs_type or '未设置'}\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未启用\n", "disabled")
        
        # 添加百信双硬盘策略状态（独立显示）
        self.settings_status_text.insert(tk.END, "\n 百信双硬盘策略: ", "section")
        if self.baixinx_dual_disk.get():
            disk_size = self.baixinx_disk_size.get().strip()
            self.settings_status_text.insert(tk.END, f"已启用 ({disk_size}GB)\n", "enabled")
        else:
            self.settings_status_text.insert(tk.END, "未启用\n", "disabled")
        
        # 添加KMS设置状态
        self.settings_status_text.insert(tk.END, "\n KMS配置: ", "section")
        if hasattr(self, 'kms_enabled') and self.kms_enabled.get():
            self.settings_status_text.insert(tk.END, "已启用\n", "enabled")
            kms_ip = self.kms_ip_var.get().strip() if hasattr(self, 'kms_ip_var') else ""
            self.settings_status_text.insert(tk.END, f"  • 服务器IP: {kms_ip or '未设置'}\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未启用\n", "disabled")
        
        # 添加字体设置状态
        self.settings_status_text.insert(tk.END, "\n 字体: ", "section")
        if hasattr(self, 'custom_fonts') and self.custom_fonts:
            self.settings_status_text.insert(tk.END, f"已添加 {len(self.custom_fonts)} 个\n", "enabled")
        else:
            self.settings_status_text.insert(tk.END, "未添加\n", "disabled")
        
        # 添加授权文件状态
        self.settings_status_text.insert(tk.END, "\n 授权文件: ", "section")
        if self.custom_licenses:
            self.settings_status_text.insert(tk.END, f"已添加 {len(self.custom_licenses)} 个\n", "enabled")
        else:
            self.settings_status_text.insert(tk.END, "未添加\n", "disabled")
        
        # 添加kpb补丁状态
        self.settings_status_text.insert(tk.END, "\n kpb补丁: ", "section")
        if hasattr(self, 'custom_kpb_patches') and self.custom_kpb_patches:
            self.settings_status_text.insert(tk.END, f"已添加 {len(self.custom_kpb_patches)} 个\n", "enabled")
        else:
            self.settings_status_text.insert(tk.END, "未添加\n", "disabled")
        
        # 添加hosts文件状态
        self.settings_status_text.insert(tk.END, "\n hosts文件: ", "section")
        if hasattr(self, 'custom_hosts_entries') and self.custom_hosts_entries:
            self.settings_status_text.insert(tk.END, f"已添加 {len(self.custom_hosts_entries)} 条\n", "enabled")
            for ip, domain in self.custom_hosts_entries[:3]:  # 最多显示3条
                self.settings_status_text.insert(tk.END, f"  • {ip}    {domain}\n", "item")
            if len(self.custom_hosts_entries) > 3:
                self.settings_status_text.insert(tk.END, f"  • ... 还有 {len(self.custom_hosts_entries) - 3} 条\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未添加\n", "disabled")
        
        # 添加安全中心设置状态
        self.settings_status_text.insert(tk.END, "\n 安全中心设置: ", "section")
        if (hasattr(self, 'disable_firewall') and self.disable_firewall.get() or
            hasattr(self, 'disable_network_protection') and self.disable_network_protection.get() or
            hasattr(self, 'disable_app_protection') and self.disable_app_protection.get() or
            hasattr(self, 'disable_device_security') and self.disable_device_security.get() or
            hasattr(self, 'disable_process_protection') and self.disable_process_protection.get() or
            hasattr(self, 'disable_file_protection') and self.disable_file_protection.get() or
            hasattr(self, 'disable_signature') and self.disable_signature.get() or
            hasattr(self, 'disable_kid') and self.disable_kid.get()):
            self.settings_status_text.insert(tk.END, "已修改\n", "enabled")
            if hasattr(self, 'disable_firewall') and self.disable_firewall.get():
                self.settings_status_text.insert(tk.END, "  • 防火墙: 已关闭\n", "item")
            if hasattr(self, 'disable_network_protection') and self.disable_network_protection.get():
                self.settings_status_text.insert(tk.END, "  • 网络保护: 已关闭\n", "item")
            if hasattr(self, 'disable_app_protection') and self.disable_app_protection.get():
                self.settings_status_text.insert(tk.END, "  • 应用保护: 已关闭\n", "item")
            if hasattr(self, 'disable_device_security') and self.disable_device_security.get():
                self.settings_status_text.insert(tk.END, "  • 设备安全: 已关闭\n", "item")
            if hasattr(self, 'disable_process_protection') and self.disable_process_protection.get():
                self.settings_status_text.insert(tk.END, "  • 进程保护: 已关闭\n", "item")
            if hasattr(self, 'disable_file_protection') and self.disable_file_protection.get():
                self.settings_status_text.insert(tk.END, "  • 文件保护: 已关闭\n", "item")
            if hasattr(self, 'disable_signature') and self.disable_signature.get():
                self.settings_status_text.insert(tk.END, "  • 签名状态: 已关闭\n", "item")
            if hasattr(self, 'disable_kid') and self.disable_kid.get():
                self.settings_status_text.insert(tk.END, "  • kid状态: 已关闭\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未修改\n", "disabled")
        
        # 添加自定义配置状态（脚本 + 文件夹）
        self.settings_status_text.insert(tk.END, "\n 自定义配置: ", "section")
        has_script = bool(getattr(self, 'custom_post_actions_script', '').strip())
        has_folder = bool(self.custom_personalization_items)
        if has_script or has_folder:
            self.settings_status_text.insert(tk.END, "已配置\n", "enabled")
            if has_folder:
                self.settings_status_text.insert(tk.END, f"  • 自定义文件夹: {len(self.custom_personalization_items)} 项\n", "item")
                for item_path in self.custom_personalization_items[:3]:
                    item = Path(item_path)
                    item_type = "文件夹" if item.is_dir() else "文件"
                    self.settings_status_text.insert(tk.END, f"    [{item_type}] {item.name}\n", "item")
                if len(self.custom_personalization_items) > 3:
                    self.settings_status_text.insert(tk.END, f"    ... 还有 {len(self.custom_personalization_items) - 3} 项\n", "item")
            else:
                self.settings_status_text.insert(tk.END, "  • 自定义文件夹: 未添加\n", "item")
            if has_script:
                self.settings_status_text.insert(tk.END, "  • 自定义脚本: 已配置\n", "item")
            else:
                self.settings_status_text.insert(tk.END, "  • 自定义脚本: 未配置\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未配置\n", "disabled")
        
        # 添加DEB包状态
        self.settings_status_text.insert(tk.END, "\n DEB包: ", "section")
        third_party_count = self.deb_listbox_third_party.size()
        apps_third_count = self.deb_listbox_apps_third.size()
        
        if third_party_count > 0 or apps_third_count > 0:
            self.settings_status_text.insert(tk.END, "已添加\n", "enabled")
            if third_party_count > 0:
                self.settings_status_text.insert(tk.END, f"  • third-party: {third_party_count} 个\n", "item")
            if apps_third_count > 0:
                self.settings_status_text.insert(tk.END, f"  • apps-third: {apps_third_count} 个\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未添加\n", "disabled")
        
        # 添加squashfs软件管理状态
        self.settings_status_text.insert(tk.END, "\n squashfs软件管理: ", "section")
        has_squashfs_ops = self.remove_qax.get() or self.custom_squashfs_packages
        if has_squashfs_ops:
            self.settings_status_text.insert(tk.END, "已配置\n", "enabled")
            if self.remove_qax.get():
                self.settings_status_text.insert(tk.END, "  • 删除奇安信: 已启用\n", "item")
            if self.custom_squashfs_packages:
                self.settings_status_text.insert(tk.END, f"  • 自定义软件包: {len(self.custom_squashfs_packages)} 个\n", "item")
        else:
            self.settings_status_text.insert(tk.END, "未配置\n", "disabled")
        
        # 禁用文本框编辑
        self.settings_status_text.config(state='disabled')
        
        # 配置标签样式
        self.settings_status_text.tag_configure("title", font=("TkDefaultFont", 10, "bold"), justify='center')
        self.settings_status_text.tag_configure("section", font=("TkDefaultFont", 9, "bold"))
        self.settings_status_text.tag_configure("enabled", foreground="#008800")
        self.settings_status_text.tag_configure("disabled", foreground="#888888")
        self.settings_status_text.tag_configure("item", foreground="#333333", lmargin1=20, lmargin2=20)

    def _update_post_actions_script(self):
        """更新.kylin-post-actions脚本文件，添加自定义配置"""
        try:
            post_actions_file = LIVECD_DIR / ".kylin-post-actions"
            
            # 如果文件不存在，创建一个基本的脚本框架
            if not post_actions_file.exists():
                with open(post_actions_file, 'w') as f:
                    f.write("#!/bin/bash\n\n")
                    f.write("# 此文件由ISO镜像定制工具生成，用于系统安装后的自定义操作\n\n")
                    f.write("# 脚本开始\necho \"执行后置操作脚本\"\n\n")
                    f.write("# 脚本结束\nexit 0\n")  # 确保脚本有正确的结束
                # 新创建的文件不算是用户上传的
                self.post_actions_modified = False
            
            # 读取现有内容
            with open(post_actions_file, 'r') as f:
                content = f.read()
            
            # 检查文件是否已经有正确的结束
            if not content.strip().endswith("exit 0") and not "exit 0" in content:
                content = content.rstrip() + "\n\n# 确保脚本正常退出\nexit 0\n"
            
            # 准备要添加的内容（先不添加标记，等确认有实际内容后再添加）
            new_content = ""
            
            # 添加百信双硬盘策略脚本
            if hasattr(self, 'baixinx_dual_disk') and self.baixinx_dual_disk.get():
                disk_size = self.baixinx_disk_size.get().strip() if hasattr(self, 'baixinx_disk_size') else "1000"
                if disk_size:
                    new_content += f"\n# 百信双硬盘策略脚本\n"
                    new_content += "# 创建百信双硬盘分区脚本\n"
                    new_content += "cat > /tmp/baixinx_dual_disk.sh << 'EOF'\n"
                    new_content += "#!/bin/bash\n"
                    new_content += "#set -e\n\n"
                    new_content += "#卸载分区\n"
                    new_content += "do_umount()\n"
                    new_content += "{\n"
                    new_content += "    device=$1\n"
                    new_content += "    i=0\n"
                    new_content += "    while [ $i -lt 3 ];do\n"
                    new_content += "        if cat /proc/mounts |grep $device > /dev/null; then\n"
                    new_content += "            umount ${device}* || true # |和 ||或，|如第一个条件满足还是会运行下个命令，||如第一个条件满足则不会运行第二个命令\n"
                    new_content += "        else\n"
                    new_content += "            break\n"
                    new_content += "        fi\n"
                    new_content += "        i=$((i+1))\n"
                    new_content += "    done\n"
                    new_content += "    swapoff -a  #关闭所有交换分区\n"
                    new_content += "    if cat /proc/mounts |grep $device > /dev/null; then\n"
                    new_content += "        echo \"Error: can't umount partition in $device\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n\n"
                    new_content += "}\n\n"
                    
                    new_content += "begin_partition()\n"
                    new_content += "{\n"
                    new_content += "    device=$1\n"
                    new_content += "    string_comment=\"# /data1 was on DEVICE during installation\"\n"
                    new_content += "    string_work=\"UUID=VALUE /data1           ext4    rw,user,x-gvfs-show,nosuid,nofail 0       2\"\n\n"
                    new_content += "    dd if=/dev/zero of=${device} bs=512 count=1 > /dev/null\n"
                    new_content += "    sync\n"
                    new_content += "    sleep 3\n"
                    new_content += "    echo \"\n"
                    new_content += "    g\n"
                    new_content += "    n\n"
                    new_content += "    1\n"
                    new_content += "    \n"
                    new_content += "    \n"
                    new_content += "    w\n"
                    new_content += "    \" | fdisk ${device} > /dev/null\n"
                    new_content += "    if [ ! $? -eq 0 ];then\n"
                    new_content += "        echo \"Error: could not make partition!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n"
                    new_content += "    partprobe ${device}\n"
                    new_content += "    if [ ! $? -eq 0 ];then\n"
                    new_content += "        echo \"Error: failed to run partprobe!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n"
                    new_content += "    sleep 3\n"
                    new_content += "    mkfs.ext4 -L \"数据盘\" ${device}1\n"
                    new_content += "    if [ ! $? -eq 0 ];then\n"
                    new_content += "        echo \"Error: could not make filesystem!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n"
                    new_content += "    sync\n"
                    new_content += "    dev_uuid=`blkid -s UUID ${device}1 -o value`\n"
                    new_content += "    string_comment=$(echo $string_comment|sed -e \"s#DEVICE#${device}1#\")\n"
                    new_content += "    string_work=$(echo $string_work|sed -e \"s/VALUE/${dev_uuid}/\")\n"
                    new_content += "    echo ${string_comment} >> /etc/fstab\n"
                    new_content += "    echo ${string_work} >> /etc/fstab\n"
                    new_content += "    mkdir -p /data1\n"
                    new_content += "    mount ${device}1 /data1\n"
                    new_content += "    chmod 777 /data1\n"
                    new_content += "    umount /data1\n"
                    new_content += "    if [ ! $? -eq 0 ];then\n"
                    new_content += "        echo \"Error: could not chmod data!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n\n"
                    new_content += "}\n\n"
                    
                    new_content += "do_updategrub()\n"
                    new_content += "{\n"
                    new_content += "    sudo  /bin/bash -c \"update-grub\"\n"
                    new_content += "    if [ ! $? -eq 0 ];then\n"
                    new_content += "        echo \"Error: failed to update-grub!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n"
                    new_content += "}\n\n"
                    
                    new_content += "do_partition()\n"
                    new_content += "{\n"
                    new_content += "    device=$1\n"
                    new_content += "    echo \"in do_partition device is $device\"\n"
                    new_content += "    do_umount $device\n"
                    new_content += "    begin_partition $device\n"
                    new_content += "    do_updategrub\n"
                    new_content += "    \n"
                    new_content += "}\n\n"
                    
                    new_content += "print_help()\n"
                    new_content += "{\n"
                    new_content += "  echo \"Usage: $0 DiskSize unit as GB like $0 1000\"\n"
                    new_content += "}\n\n"
                    
                    new_content += "if [ ! $# -eq 1 ];then\n"
                    new_content += "  print_help\n"
                    new_content += "  exit 1\n"
                    new_content += "fi\n\n"
                    
                    new_content += "target_size=$1\n"
                    new_content += "LANGUAGE=en_US fdisk -l 2>/dev/null|grep \"Disk /dev/sd[a-z]\" |awk '{print $2 \" \"$5}'|\n"
                    new_content += "while read device size\n"
                    new_content += "do\n"
                    new_content += "  device=$(echo $device |cut -f 1 -d ':')\n"
                    new_content += "  size_gb=$((${size}/1000/1000/1000))\n"
                    new_content += "  echo \"Find devie match: $device $size_gb $target_size\"\n"
                    new_content += "  if [ $size_gb -eq ${target_size} ];then\n"
                    new_content += "    target_device=\"$device\"\n"
                    new_content += "    if cat /proc/mounts|grep \"target\"|grep \"$target_device\" > /dev/null;then\n"
                    new_content += "        echo \"Error: detected System use this disk, abort!\"\n"
                    new_content += "        exit 1\n"
                    new_content += "    fi\n"
                    new_content += "    do_partition $target_device\n"
                    new_content += "    echo \"Success: make data disk done!\"\n"
                    new_content += "    break\n"
                    new_content += "  fi\n"
                    new_content += "done\n"
                    new_content += "EOF\n\n"
                    
                    # 设置脚本权限并执行
                    new_content += "# 设置脚本权限并执行百信双硬盘策略\n"
                    new_content += "chmod +x /tmp/baixinx_dual_disk.sh\n"
                    new_content += f"echo \"执行百信双硬盘策略，目标硬盘大小: {disk_size}GB\"\n"
                    new_content += f"/tmp/baixinx_dual_disk.sh {disk_size}\n"
                    new_content += "if [ $? -eq 0 ]; then\n"
                    new_content += "    echo \"百信双硬盘策略执行成功\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"百信双硬盘策略执行失败\"\n"
                    new_content += "fi\n"
                    new_content += "# 清理临时脚本\n"
                    new_content += "rm -f /tmp/baixinx_dual_disk.sh\n\n"
            
            # 添加KMS配置
            if hasattr(self, 'kms_enabled') and self.kms_enabled.get():
                kms_ip = self.kms_ip_var.get().strip() if hasattr(self, 'kms_ip_var') else ""
                if kms_ip:
                    new_content += f"\n# 配置KMS服务器\n"
                    new_content += "\nexport QT_QPA_PLATFORM='offscreen'\n"
                    new_content += "kylin-activation    -install\n"
                    # 在while循环前添加ping检查
                    new_content += f"# 检查KMS服务器 {kms_ip} 是否可达\n"
                    new_content += f"if ping -c 3 -W 5 {kms_ip} >/dev/null 2>&1; then\n"
                    new_content += "    echo \"KMS服务器可达，继续执行激活\"\n"
                    new_content += "    while [ $? -ne  0 ] \n"
                    new_content += "    do\n"
                    new_content += "         sleep 5\n"
                    new_content += "         kylin-activation   -install\n" 
                    new_content += "    done\n"
                    new_content += "else\n"
                    new_content += f"    echo \"KMS服务器 {kms_ip} 不可达，跳过激活\"\n"
                    new_content += "fi\n"
                    new_content += "sync\n"
                    # 使用纯shell脚本更新KMS配置
                    new_content += "# 更新 /usr/share/kylin-activation/activation_conf.ini 文件\n"
                    new_content += "if [ -f /usr/share/kylin-activation/activation_conf.ini ]; then\n"
                    
                    # 检查并更新 [Server] ServerIp
                    new_content += "    # 检查并更新 [Server] ServerIp\n"
                    new_content += "    if grep -q '\\[Server\\]' /usr/share/kylin-activation/activation_conf.ini; then\n"
                    new_content += "        if grep -q 'ServerIp=' /usr/share/kylin-activation/activation_conf.ini; then\n"
                    new_content += f"            sed -i 's/ServerIp=.*/ServerIp={kms_ip}/' /usr/share/kylin-activation/activation_conf.ini\n"
                    new_content += "            echo \"已更新 [Server] ServerIp 配置\"\n"
                    new_content += "        fi\n"  # 闭合内部if
                    new_content += "    fi\n"  # 闭合外部if
                    
                    # 检查并更新 [KMS_Activation] serverip
                    new_content += "\n    # 检查并更新 [KMS_Activation] serverip\n"
                    new_content += "    if grep -q '\\[KMS_Activation\\]' /usr/share/kylin-activation/activation_conf.ini; then\n"
                    new_content += "        if grep -q 'serverip=' /usr/share/kylin-activation/activation_conf.ini; then\n"
                    new_content += f"            sed -i 's/serverip=.*/serverip={kms_ip}/' /usr/share/kylin-activation/activation_conf.ini\n"
                    new_content += "            echo \"已更新 [KMS_Activation] serverip 配置\"\n"
                    new_content += "        fi\n"  # 闭合内部if
                    new_content += "    fi\n"  # 闭合外部if
                    
                    # 检查并更新 [KMS_Activation] ServerIp
                    new_content += "\n    # 检查并更新 [KMS_Activation] ServerIp\n"
                    new_content += "    if grep -q '\\[KMS_Activation\\]' /usr/share/kylin-activation/activation_conf.ini; then\n"
                    new_content += "        if grep -q 'ServerIp=' /usr/share/kylin-activation/activation_conf.ini && grep -A10 '\\[KMS_Activation\\]' /usr/share/kylin-activation/activation_conf.ini | grep -q 'ServerIp='; then\n"
                    new_content += f"            sed -i '/\\[KMS_Activation\\]/,/\\[/ s/ServerIp=.*/ServerIp={kms_ip}/' /usr/share/kylin-activation/activation_conf.ini\n"
                    new_content += "            echo \"已更新 [KMS_Activation] ServerIp 配置\"\n"
                    new_content += "        fi\n"  # 闭合内部if
                    new_content += "    fi\n"  # 闭合外部if

                    new_content += "fi\n"  # 闭合最外层if
                    
            # 添加字体配置
            if hasattr(self, 'custom_fonts') and self.custom_fonts:
                new_content += "\n# 安装自定义字体\n"
                new_content += "mkdir -p /usr/share/fonts/custom\n"
                # 不再需要循环每个字体文件，直接使用通配符复制所有字体
                new_content += "# 复制所有TTF字体文件\n"
                new_content += "cp -f /cdrom/fonts/*.ttf /usr/share/fonts/custom/ 2>/dev/null\n"
                new_content += "cp -f /cdrom/fonts/*.TTF /usr/share/fonts/custom/ 2>/dev/null\n"
                new_content += "chmod 644 /usr/share/fonts/custom/*\n"
                new_content += "fc-cache -fv\n"
            
            # 添加安全中心设置
            security_settings_added = False

            if (hasattr(self, 'disable_firewall') and self.disable_firewall.get() or
                hasattr(self, 'disable_network_protection') and self.disable_network_protection.get() or
                hasattr(self, 'disable_app_protection') and self.disable_app_protection.get() or
                hasattr(self, 'disable_device_security') and self.disable_device_security.get() or
                hasattr(self, 'disable_process_protection') and self.disable_process_protection.get() or
                hasattr(self, 'disable_file_protection') and self.disable_file_protection.get() or
                hasattr(self, 'disable_signature') and self.disable_signature.get() or
                hasattr(self, 'disable_kid') and self.disable_kid.get()):
                security_settings_added = True
                
                # 配置安全中心设置
                new_content += "\n# 配置安全中心设置\n"
                
                # 关闭签名状态
                if hasattr(self, 'disable_signature') and self.disable_signature.get():
                    new_content += "# 关闭签名状态\n"
                    new_content += "if [ -f /usr/bin/setsignstatus ]; then\n"
                    new_content += "    /usr/bin/setsignstatus off\n"
                    new_content += "    echo \"签名状态已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到 setsignstatus 命令\"\n"
                    new_content += "fi\n\n"
                
                # 关闭防火墙 - 修复配置文件路径
                if hasattr(self, 'disable_firewall') and self.disable_firewall.get():
                    new_content += "# 关闭防火墙\n"
                    new_content += "if [ -f /etc/kylin-firewall/kylin-firewall.conf ]; then\n"
                    new_content += "    sed -i 's/Mode=.*/Mode=off/' /etc/kylin-firewall/kylin-firewall.conf\n"
                    new_content += "    echo \"防火墙已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到防火墙配置文件\"\n"
                    new_content += "fi\n\n"
            
                # 关闭应用保护
                if hasattr(self, 'disable_app_protection') and self.disable_app_protection.get():
                    new_content += "# 关闭应用保护\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_exectl =.*/kysec_exectl = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"应用保护已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
            
                # 关闭网络保护 - 修复格式
                if hasattr(self, 'disable_network_protection') and self.disable_network_protection.get():
                    new_content += "# 关闭网络保护\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_netctl =.*/kysec_netctl = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"网络保护已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
            
                # 关闭设备安全
                if hasattr(self, 'disable_device_security') and self.disable_device_security.get():
                    new_content += "# 关闭设备安全\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_devctl =.*/kysec_devctl = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"设备安全已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
                
                # 关闭进程保护
                if hasattr(self, 'disable_process_protection') and self.disable_process_protection.get():
                    new_content += "# 关闭进程保护\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_ppro =.*/kysec_ppro = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"进程保护已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
                
                # 关闭文件保护
                if hasattr(self, 'disable_file_protection') and self.disable_file_protection.get():
                    new_content += "# 关闭文件保护\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_fpro =.*/kysec_fpro = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"文件保护已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
                
                # 关闭kid状态
                if hasattr(self, 'disable_kid') and self.disable_kid.get():
                    new_content += "# 关闭kid状态\n"
                    new_content += "if [ -f /etc/kysec/kysec.conf ]; then\n"
                    new_content += "    sed -i 's/kysec_kid =.*/kysec_kid = 0/' /etc/kysec/kysec.conf\n"
                    new_content += "    echo \"kid状态已关闭\"\n"
                    new_content += "else\n"
                    new_content += "    echo \"未找到安全中心配置文件\"\n"
                    new_content += "fi\n\n"
                
            # 添加hosts文件条目
            if hasattr(self, 'custom_hosts_entries') and self.custom_hosts_entries:
                new_content += "# 添加hosts文件条目\n"
                new_content += "if [ -f /etc/hosts ]; then\n"
                for ip, domain in self.custom_hosts_entries:
                    # 转义特殊字符以便在sed中使用
                    ip_escaped = ip.replace('/', '\\/').replace('.', '\\.')
                    domain_escaped = domain.replace('/', '\\/').replace('.', '\\.')
                    # 检查是否已存在该域名或IP的条目，如果存在则先删除
                    new_content += f"    # 删除已存在的 {domain} 或 {ip} 条目（如果存在）\n"
                    new_content += f"    sed -i '/{domain_escaped}/d' /etc/hosts\n"
                    new_content += f"    sed -i '/{ip_escaped}/d' /etc/hosts\n"
                    # 添加新条目（使用多个空格分隔，符合hosts文件格式）
                    new_content += f"    echo \"{ip}    {domain}\" >> /etc/hosts\n"
                new_content += "    echo \"hosts文件条目已添加\"\n"
                new_content += "else\n"
                new_content += "    echo \"未找到hosts文件\"\n"
                new_content += "fi\n\n"
                
            # 提取镜像中已有的用户自定义脚本，与本次新增内容合并成一块
            user_block_pattern = (
                r'\n*# === 用户自定义脚本开始 ===\n?(.*?)# === 用户自定义脚本结束 ===\s*'
            )
            existing_scripts = [
                part.strip()
                for part in re.findall(user_block_pattern, content, flags=re.S)
                if part.strip()
            ]
            content = re.sub(user_block_pattern, '\n', content, flags=re.S)

            new_user_script = (getattr(self, 'custom_post_actions_script', '') or '').strip()
            merged_scripts = list(existing_scripts)
            if new_user_script and new_user_script not in existing_scripts:
                merged_scripts.append(new_user_script)
            merged_user_script = "\n\n".join(merged_scripts).strip()

            if new_content or merged_user_script:
                logger.info("更新后置操作脚本")
                content = re.sub(r'\n#\s*确保脚本正常退出\s*\nexit\s+[0-9]+\s*$', '', content)
                content = re.sub(r'\nexit\s+[0-9]+\s*$', '', content)
                content = content.rstrip() + "\n"

                if new_content:
                    has_marker = "# === 以下内容由ISO镜像定制工具自动添加 ===" in content
                    if has_marker:
                        content += "\n" + new_content
                    else:
                        content += "\n# === 以下内容由ISO镜像定制工具自动添加 ===\n" + new_content
                    logger.info("已添加自动生成的配置到后置脚本")

                if merged_user_script:
                    content += "\n# === 用户自定义脚本开始 ===\n"
                    content += merged_user_script + "\n"
                    content += "# === 用户自定义脚本结束 ===\n"
                    if new_user_script and existing_scripts:
                        self.append_output("已将本次自定义脚本与镜像中原有自定义脚本合并到.kylin-post-actions末尾")
                    else:
                        self.append_output("已将用户自定义脚本追加到.kylin-post-actions末尾")
                    logger.info("已更新.kylin-post-actions中的用户自定义脚本（合并保留原有内容）")

                content += "\n# 确保脚本正常退出\nexit 0\n"

                with open(post_actions_file, 'w') as f:
                    f.write(content)

                os.chmod(post_actions_file, 0o755)

                self.append_output(f"已更新后置操作脚本: {post_actions_file}")
                logger.info(f"Updated post-actions script: {post_actions_file}")
        except Exception as e:
            logger.error(f"Failed to update post-actions script: {e}")
            self.append_output(f"更新后置操作脚本失败: {e}")

    def show_reserved_function_message(self, function_name):
        """显示预留功能的提示信息"""
        logger.info(f"点击了{function_name}按钮")
        messagebox.showinfo("功能预留", f"{function_name}功能正在开发中，敬请期待！")

    def open_custom_config_window(self):
        """打开自定义配置窗口（自定义文件夹 + 自定义脚本）"""
        if not self._check_iso_selected():
            return
        logger.info("打开自定义配置窗口")
        config_win = tk.Toplevel(self.root)
        config_win.title("自定义配置键")
        config_win.transient(self.root)
        config_win.grab_set()

        main_frame = ttk.Frame(config_win, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=(10, 0))

        # --- 自定义文件夹 ---
        folder_frame = ttk.LabelFrame(main_frame, text="自定义文件夹", padding="5")
        folder_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        folder_frame.columnconfigure(0, weight=1)
        folder_frame.rowconfigure(1, weight=1)

        ttk.Label(
            folder_frame,
            text="制作镜像时会复制到镜像根目录的 custom 文件夹下，上传文件夹时保留原有层级。",
            wraplength=620
        ).grid(row=0, column=0, columnspan=2, sticky=tk.W, padx=5, pady=(0, 5))

        popup_items = list(self.custom_personalization_items)
        popup_listbox = tk.Listbox(folder_frame, height=6, selectmode=tk.EXTENDED)
        popup_listbox.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        folder_scroll = ttk.Scrollbar(folder_frame, orient='vertical', command=popup_listbox.yview)
        folder_scroll.grid(row=1, column=1, sticky=(tk.N, tk.S), pady=5)
        popup_listbox['yscrollcommand'] = folder_scroll.set

        def refresh_listbox():
            popup_listbox.delete(0, tk.END)
            for item_path in popup_items:
                item = Path(item_path)
                item_type = "文件夹" if item.is_dir() else "文件"
                popup_listbox.insert(tk.END, f"[{item_type}] {item_path}")

        def append_paths(paths):
            added = 0
            for fp in paths:
                if fp and fp not in popup_items:
                    popup_items.append(fp)
                    added += 1
                    logger.info(f"添加自定义文件/文件夹: {fp}")
            if added:
                refresh_listbox()

        def add_files():
            if not self._check_iso_prepared():
                return
            config_win.grab_release()
            filepaths = filedialog.askopenfilenames(
                title="选择文件",
                filetypes=[("所有文件", "*.*")]
            )
            config_win.grab_set()
            config_win.lift()
            config_win.focus_force()
            if not filepaths:
                return
            append_paths(filepaths)

        def add_folder():
            if not self._check_iso_prepared():
                return
            config_win.grab_release()
            folder = filedialog.askdirectory(title="选择文件夹")
            config_win.grab_set()
            config_win.lift()
            config_win.focus_force()
            if not folder:
                return
            append_paths([folder])

        def remove_selected():
            selected_indices = popup_listbox.curselection()
            if not selected_indices:
                return
            for i in sorted(selected_indices, reverse=True):
                logger.info(f"移除自定义项: {popup_items[i]}")
                del popup_items[i]
            refresh_listbox()

        def clear_folders():
            popup_items.clear()
            refresh_listbox()

        refresh_listbox()

        folder_btn_frame = ttk.Frame(folder_frame)
        folder_btn_frame.grid(row=2, column=0, columnspan=2, sticky=tk.W, pady=(0, 5), padx=5)
        ttk.Button(folder_btn_frame, text="添加文件", command=add_files).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(folder_btn_frame, text="添加文件夹", command=add_folder).pack(side=tk.LEFT, padx=5)
        ttk.Button(folder_btn_frame, text="删除", command=remove_selected).pack(side=tk.LEFT, padx=5)
        ttk.Button(folder_btn_frame, text="清空", command=clear_folders).pack(side=tk.LEFT, padx=5)

        # --- 自定义脚本 ---
        script_frame = ttk.LabelFrame(main_frame, text="自定义脚本", padding="5")
        script_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, pady=(8, 0))
        script_frame.columnconfigure(0, weight=1)
        script_frame.rowconfigure(1, weight=1)

        ttk.Label(
            script_frame,
            text="此处填写本次新增脚本。制作时会与镜像中已有的自定义脚本合并成一块，追加到 .kylin-post-actions 最后，不会覆盖之前的内容。",
            wraplength=620
        ).grid(row=0, column=0, columnspan=2, sticky=tk.W, padx=5, pady=(0, 5))

        script_text = tk.Text(script_frame, wrap='none', height=10, undo=True)
        script_text.grid(row=1, column=0, sticky=(tk.N, tk.S, tk.E, tk.W), padx=5, pady=5)
        y_scroll = ttk.Scrollbar(script_frame, orient='vertical', command=script_text.yview)
        y_scroll.grid(row=1, column=1, sticky=(tk.N, tk.S), pady=5)
        x_scroll = ttk.Scrollbar(script_frame, orient='horizontal', command=script_text.xview)
        x_scroll.grid(row=2, column=0, sticky=(tk.W, tk.E), padx=5)
        script_text.config(yscrollcommand=y_scroll.set, xscrollcommand=x_scroll.set)
        if getattr(self, 'custom_post_actions_script', ''):
            script_text.insert('1.0', self.custom_post_actions_script)

        def clear_script():
            script_text.delete('1.0', tk.END)

        ttk.Button(script_frame, text="清空脚本", command=clear_script).grid(
            row=3, column=0, sticky=tk.W, padx=5, pady=(0, 5)
        )

        def on_ok():
            self.custom_personalization_items = list(popup_items)
            self.custom_post_actions_script = script_text.get('1.0', 'end-1c')
            self.post_actions_modified = bool(self.custom_post_actions_script.strip())
            logger.info(
                f"保存自定义配置: 文件夹={len(self.custom_personalization_items)} 项, "
                f"脚本长度={len(self.custom_post_actions_script)}"
            )
            config_win.destroy()
            self.update_settings_status()

        ttk.Button(btn_frame, text="确定", command=on_ok).pack(side=tk.RIGHT, padx=5)
        ttk.Button(btn_frame, text="取消", command=config_win.destroy).pack(side=tk.RIGHT, padx=5)

        config_win.update_idletasks()
        req_w = max(config_win.winfo_reqwidth(), 680)
        req_h = max(config_win.winfo_reqheight() + 20, 560)
        config_win.geometry(f"{req_w}x{req_h}")
        config_win.minsize(620, 480)
        self.center_window(config_win, self.root)

        self.root.wait_window(config_win)

    def manage_squashfs_packages(self):
        """打开管理squashfs软件窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开管理squashfs软件窗口")
        manage_win = tk.Toplevel(self.root)
        manage_win.title("管理squashfs软件")
        manage_win.geometry("550x500")
        manage_win.resizable(False, False)  # 禁止调整大小
        manage_win.transient(self.root)
        manage_win.grab_set()  # 模态窗口
        
        # 主框架
        main_frame = ttk.Frame(manage_win, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)
        main_frame.columnconfigure(0, weight=1)
        
        # 标题
        title_label = ttk.Label(main_frame, text="管理squashfs软件", font=("TkDefaultFont", 12, "bold"))
        title_label.pack(pady=(0, 10))
        
        # 选项框架
        options_frame = ttk.LabelFrame(main_frame, text="操作选项", padding="10")
        options_frame.pack(fill=tk.X, pady=(0, 10))
        
        # 删除奇安信软件选项
        remove_qax_var = tk.BooleanVar(value=self.remove_qax.get())
        remove_qax_checkbox = ttk.Checkbutton(options_frame, text="删除奇安信杀毒软件", variable=remove_qax_var)
        remove_qax_checkbox.pack(anchor=tk.W, pady=5)
        
        # squashfs中安装自定义软件包列表框架
        packages_frame = ttk.LabelFrame(main_frame, text="squashfs中安装自定义软件", padding="5")
        packages_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        packages_frame.columnconfigure(0, weight=1)
        packages_frame.rowconfigure(0, weight=1)
        
        # 软件包列表
        packages_listbox = tk.Listbox(packages_frame, height=6, selectmode=tk.EXTENDED)
        packages_listbox.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        
        # 滚动条
        packages_scrollbar = ttk.Scrollbar(packages_frame, orient='vertical', command=packages_listbox.yview)
        packages_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S), pady=5)
        packages_listbox['yscrollcommand'] = packages_scrollbar.set
        
        # 填充当前已选软件包
        for pkg in self.custom_squashfs_packages:
            packages_listbox.insert(tk.END, os.path.basename(pkg))
        
        # 软件包操作按钮框架
        pkg_btn_frame = ttk.Frame(packages_frame)
        pkg_btn_frame.grid(row=1, column=0, columnspan=2, pady=5)
        
        def add_package():
            """添加软件包"""
            if not self._check_iso_prepared():
                return
            filepaths = filedialog.askopenfilenames(
                title="选择DEB软件包",
                filetypes=[("DEB包", "*.deb")]
            )
            if filepaths:
                for fp in filepaths:
                    if fp not in self.custom_squashfs_packages:
                        self.custom_squashfs_packages.append(fp)
                        packages_listbox.insert(tk.END, os.path.basename(fp))
                        logger.info(f"添加squashfs软件包: {fp}")
        
        def remove_package():
            """删除选中的软件包"""
            selected_indices = packages_listbox.curselection()
            if selected_indices:
                # 从后往前删除，避免索引变化
                for i in sorted(selected_indices, reverse=True):
                    pkg_name = packages_listbox.get(i)
                    # 从列表中移除对应的完整路径
                    self.custom_squashfs_packages = [pkg for pkg in self.custom_squashfs_packages 
                                                     if os.path.basename(pkg) != pkg_name]
                    packages_listbox.delete(i)
                    logger.info(f"删除squashfs软件包: {pkg_name}")
        
        def clear_packages():
            """清空所有软件包"""
            if packages_listbox.size() > 0:
                if messagebox.askyesno("确认", "确定要清空所有软件包吗？"):
                    self.custom_squashfs_packages.clear()
                    packages_listbox.delete(0, tk.END)
        
        ttk.Button(pkg_btn_frame, text="添加", command=add_package).pack(side=tk.LEFT, padx=5)
        ttk.Button(pkg_btn_frame, text="删除", command=remove_package).pack(side=tk.LEFT, padx=5)
        ttk.Button(pkg_btn_frame, text="清空", command=clear_packages).pack(side=tk.LEFT, padx=5)
        
        # 按钮框架 - 确保在窗口底部可见
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=(10, 0))
        
        # 确定按钮
        def on_ok():
            self.remove_qax.set(remove_qax_var.get())
            # 列表中的软件包会自动安装，无需额外检查
            logger.info(f"squashfs软件管理设置: 删除奇安信={self.remove_qax.get()}, 自定义软件包数量={len(self.custom_squashfs_packages)}")
            manage_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        # 确保按钮可见，使用grid布局更可靠
        ttk.Button(btn_frame, text="确定", command=on_ok, width=12).grid(row=0, column=0, padx=5, pady=5)
        ttk.Button(btn_frame, text="取消", command=manage_win.destroy, width=12).grid(row=0, column=1, padx=5, pady=5)
        btn_frame.columnconfigure(0, weight=1)
        btn_frame.columnconfigure(1, weight=1)

    def _remove_qax_antivirus_worker(self):
        """执行删除奇安信杀毒软件的操作（squashfs已在_process_customizations_worker中统一解压）"""
        logger.info("开始删除奇安信杀毒软件")
        self.append_output("开始删除奇安信杀毒软件...")
        
        squashfs_root = LIVECD_DIR / "casper" / "squashfs-root"
        
        # 1. 挂载文件系统
        self.append_output("准备chroot环境...")
        
        try:
            # 挂载必要的文件系统
            subprocess.run(["mount", "-t", "proc", "none", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["mount", "-t", "sysfs", "none", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["mount", "-t", "devpts", "none", f"{squashfs_root}/dev/pts"], check=True)
            
            # 3. 执行删除命令（参考1466行，替换为dpkg -P qax）
            chroot_cmd = f"chroot {squashfs_root} dpkg -P qaxsafe"
            
            self.append_output(f"执行命令: {chroot_cmd}")
            result = subprocess.run(chroot_cmd, shell=True, check=True, capture_output=True, text=True)
            
            self.append_output("奇安信杀毒软件删除完成")
            if result.stdout:
                self.append_output(f"输出: {result.stdout}")
            
            # 4. 卸载文件系统（参考1476行）
            subprocess.run(["umount", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/dev/pts"], check=True)
            
        except subprocess.CalledProcessError as e:
            self.append_output(f"删除奇安信杀毒失败: {e}")
            if e.stderr:
                self.append_output(f"错误输出: {e.stderr}")
            logger.error(f"删除奇安信杀毒失败: {e}")
            
            # 确保卸载文件系统
            subprocess.run(["umount", "-lf", f"{squashfs_root}/proc"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/sys"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/dev/pts"], check=False)
            return
        
        # 5. 打包和清理将在_process_customizations_worker中统一处理
        
        self.append_output("奇安信杀毒软件删除完成")

    def _install_custom_squashfs_packages_worker(self):
        """安装自定义squashfs软件包（squashfs已在_process_customizations_worker中统一解压）"""
        if not self.custom_squashfs_packages:
            return
        
        logger.info(f"开始安装自定义squashfs软件包，共{len(self.custom_squashfs_packages)}个")
        self.append_output(f"开始安装自定义squashfs软件包，共{len(self.custom_squashfs_packages)}个...")
        
        squashfs_root = LIVECD_DIR / "casper" / "squashfs-root"
        opt_dir = squashfs_root / "opt"
        opt_dir.mkdir(exist_ok=True)
        
        # 复制软件包到squashfs-root/opt/
        installed_packages = []
        for pkg_path in self.custom_squashfs_packages:
            try:
                pkg_file = Path(pkg_path)
                target_file = opt_dir / pkg_file.name
                
                # 检查源文件和目标文件是否是同一个文件
                if pkg_file.resolve() == target_file.resolve():
                    logger.info(f"软件包已在目标目录中，跳过复制: {pkg_path}")
                    self.append_output(f"软件包已在目标目录中，跳过: {pkg_file.name}")
                    installed_packages.append(pkg_file.name)
                else:
                    shutil.copy2(pkg_path, opt_dir)
                    installed_packages.append(pkg_file.name)
                    self.append_output(f"已复制软件包到: {opt_dir}/{pkg_file.name}")
                    logger.info(f"已复制软件包: {pkg_path} -> {opt_dir}/{pkg_file.name}")
            except Exception as e:
                self.append_output(f"复制软件包失败 {os.path.basename(pkg_path)}: {e}")
                logger.error(f"复制软件包失败 {pkg_path}: {e}")
        
        if not installed_packages:
            self.append_output("没有成功复制任何软件包，跳过安装")
            return
        
        # 挂载文件系统并安装
        self.append_output("准备chroot环境...")
        
        try:
            # 挂载必要的文件系统
            subprocess.run(["mount", "-t", "proc", "none", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["mount", "-t", "sysfs", "none", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["mount", "-t", "devpts", "none", f"{squashfs_root}/dev/pts"], check=True)
            
            # 安装每个软件包
            for pkg_filename in installed_packages:
                pkg_path_in_chroot = f"/opt/{pkg_filename}"
                chroot_cmd = f"chroot {squashfs_root} dpkg -i {pkg_path_in_chroot}"
                
                self.append_output(f"执行命令: {chroot_cmd}")
                result = subprocess.run(chroot_cmd, shell=True, check=True, capture_output=True, text=True)
                
                self.append_output(f"软件包 {pkg_filename} 安装完成")
                if result.stdout:
                    self.append_output(f"输出: {result.stdout}")
            
            # 卸载文件系统
            subprocess.run(["umount", f"{squashfs_root}/proc"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/sys"], check=True)
            subprocess.run(["umount", f"{squashfs_root}/dev/pts"], check=True)
            
        except subprocess.CalledProcessError as e:
            self.append_output(f"安装自定义软件包失败: {e}")
            if e.stderr:
                self.append_output(f"错误输出: {e.stderr}")
            logger.error(f"安装自定义软件包失败: {e}")
            
            # 确保卸载文件系统
            subprocess.run(["umount", "-lf", f"{squashfs_root}/proc"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/sys"], check=False)
            subprocess.run(["umount", "-lf", f"{squashfs_root}/dev/pts"], check=False)
            return
        
        # 打包和清理将在_process_customizations_worker中统一处理
        self.append_output(f"自定义软件包安装完成，共安装{len(installed_packages)}个")

    def open_kpb_patches_window(self):
        """打开kpb补丁选择窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开kpb补丁选择窗口")
        kpb_win = tk.Toplevel(self.root)
        kpb_win.title("添加kpb补丁")
        kpb_win.geometry("450x300")
        kpb_win.transient(self.root)
        kpb_win.grab_set()  # 模态窗口

        # --- kpb补丁选择框架 ---
        main_frame = ttk.Frame(kpb_win, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        kpb_frame = ttk.LabelFrame(main_frame, text="kpb补丁文件列表", padding="5")
        kpb_frame.pack(fill=tk.BOTH, expand=True)
        kpb_frame.columnconfigure(0, weight=1)
        kpb_frame.rowconfigure(0, weight=1)  # 让列表框可以扩展

        # 创建或获取kpb补丁文件列表
        if not hasattr(self, 'custom_kpb_patches'):
            self.custom_kpb_patches = []

        # 使用局部变量存储kpb补丁文件列表
        popup_kpb_listbox = tk.Listbox(kpb_frame, height=8, selectmode=tk.EXTENDED)
        popup_kpb_listbox.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=5, pady=5)
        
        # 添加滚动条
        scrollbar = ttk.Scrollbar(kpb_frame, orient='vertical', command=popup_kpb_listbox.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S), pady=5)
        popup_kpb_listbox['yscrollcommand'] = scrollbar.set

        # 填充当前已选kpb补丁文件
        for kpb in self.custom_kpb_patches:
            popup_kpb_listbox.insert(tk.END, kpb)

        # 按钮框架
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(10, 0))

        # 选择kpb补丁文件的辅助函数
        def select_kpb_patches_popup():
            if not self._check_iso_prepared():
                return
            filepaths = filedialog.askopenfilenames(
                title="选择kpb补丁文件", 
                filetypes=[("KPB补丁文件", "*.kpb")]
            )
            if filepaths:
                for fp in filepaths:
                    # 检查文件是否已在列表中
                    if fp not in popup_kpb_listbox.get(0, tk.END):
                        popup_kpb_listbox.insert(tk.END, fp)
                        logger.info(f"添加kpb补丁文件: {fp}")

        # 删除选中kpb补丁文件
        def remove_selected_kpb_patches():
            selected_indices = popup_kpb_listbox.curselection()
            if selected_indices:
                # 从后往前删除，避免索引变化
                for i in sorted(selected_indices, reverse=True):
                    logger.info(f"删除kpb补丁文件: {popup_kpb_listbox.get(i)}")
                    popup_kpb_listbox.delete(i)

        # 左侧放操作按钮
        action_frame = ttk.Frame(btn_frame)
        action_frame.pack(side=tk.LEFT)
        
        ttk.Button(action_frame, text="添加", command=select_kpb_patches_popup).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="删除", command=remove_selected_kpb_patches).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="清空", command=lambda: popup_kpb_listbox.delete(0, tk.END)).pack(side=tk.LEFT, padx=5)

        # 右侧放确定按钮
        ttk.Button(btn_frame, text="确定", command=lambda: on_ok()).pack(side=tk.RIGHT, padx=5)

        # --- 确定按钮功能 ---
        def on_ok():
            # 保存选择的kpb补丁文件
            self.custom_kpb_patches = list(popup_kpb_listbox.get(0, tk.END))
            logger.info(f"保存了 {len(self.custom_kpb_patches)} 个kpb补丁文件")
            kpb_win.destroy()
            self.update_settings_status()  # 更新设置状态显示
        
        # 等待窗口关闭
        self.root.wait_window(kpb_win)

    def open_hosts_window(self):
        """打开hosts文件配置窗口"""
        if not self._check_iso_selected():
            return
        logger.info("打开hosts文件配置窗口")
        hosts_win = tk.Toplevel(self.root)
        hosts_win.title("修改hosts文件")
        hosts_win.geometry("550x400")
        hosts_win.transient(self.root)
        hosts_win.grab_set()  # 模态窗口

        # IP地址验证函数（支持IPv4和IPv6）
        def validate_ip_address(ip_str):
            """验证IP地址格式，支持IPv4和IPv6"""
            if not ip_str or not ip_str.strip():
                return False, "IP地址不能为空"
            ip_str = ip_str.strip()
            try:
                # 尝试解析为IPv4或IPv6地址
                ipaddress.ip_address(ip_str)
                return True, None
            except ValueError:
                return False, f"'{ip_str}' 不是有效的IP地址（IPv4或IPv6）"

        # 主框架
        main_frame = ttk.Frame(hosts_win, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # 说明标签
        info_label = ttk.Label(main_frame, text="添加IP地址和域名映射（格式：IP地址    域名，支持IPv4和IPv6）", 
                              font=("", 9))
        info_label.pack(anchor=tk.W, pady=(0, 10))

        # 创建滚动框架用于存放hosts条目
        canvas_frame = ttk.Frame(main_frame)
        canvas_frame.pack(fill=tk.BOTH, expand=True)

        # 创建Canvas和滚动条
        canvas = tk.Canvas(canvas_frame, height=200)
        scrollbar = ttk.Scrollbar(canvas_frame, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )

        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # 存储所有行的引用
        entry_rows = []

        # 创建或获取hosts条目列表
        if not hasattr(self, 'custom_hosts_entries'):
            self.custom_hosts_entries = []

        # 添加新行的函数
        def add_entry_row(ip="", domain=""):
            row_frame = ttk.Frame(scrollable_frame)
            row_frame.pack(fill=tk.X, pady=2, padx=5)

            # IP地址输入框
            ip_var = tk.StringVar(value=ip)
            ip_entry = ttk.Entry(row_frame, textvariable=ip_var, width=18)
            ip_entry.pack(side=tk.LEFT, padx=5)

            # 域名输入框
            domain_var = tk.StringVar(value=domain)
            domain_entry = ttk.Entry(row_frame, textvariable=domain_var, width=25)
            domain_entry.pack(side=tk.LEFT, padx=5)

            # 删除按钮
            def remove_row():
                row_frame.destroy()
                entry_rows.remove((ip_var, domain_var, row_frame))
                # 更新滚动区域
                canvas.configure(scrollregion=canvas.bbox("all"))

            remove_btn = ttk.Button(row_frame, text="删除", width=8, command=remove_row)
            remove_btn.pack(side=tk.LEFT, padx=5)

            entry_rows.append((ip_var, domain_var, row_frame))
            # 更新滚动区域
            canvas.configure(scrollregion=canvas.bbox("all"))

        # 添加按钮函数
        def add_new_row():
            add_entry_row()

        # 如果已有条目，先加载它们
        for ip, domain in self.custom_hosts_entries:
            add_entry_row(ip, domain)

        # 如果没有条目，至少添加一行
        if not entry_rows:
            add_entry_row()

        # 按钮框架
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=(10, 0))

        # 左侧放添加按钮
        ttk.Button(btn_frame, text="+ 添加一行", command=add_new_row).pack(side=tk.LEFT, padx=5)

        # 右侧放确定和取消按钮
        action_frame = ttk.Frame(btn_frame)
        action_frame.pack(side=tk.RIGHT)
        
        ttk.Button(action_frame, text="确定", command=lambda: on_ok()).pack(side=tk.LEFT, padx=5)
        ttk.Button(action_frame, text="取消", command=lambda: hosts_win.destroy()).pack(side=tk.LEFT, padx=5)

        # 确定按钮功能
        def on_ok():
            # 收集所有有效的条目（IP和域名都不为空，且IP格式正确）
            valid_entries = []
            invalid_ips = []
            
            for ip_var, domain_var, _ in entry_rows:
                ip = ip_var.get().strip()
                domain = domain_var.get().strip()
                
                # 如果IP和域名都为空，跳过
                if not ip and not domain:
                    continue
                
                # 如果只填了一个，给出警告
                if not ip or not domain:
                    invalid_ips.append(f"IP: '{ip}', 域名: '{domain}' - 条目不完整（IP和域名都必须填写）")
                    continue
                
                # 验证IP地址格式
                is_valid, error_msg = validate_ip_address(ip)
                if not is_valid:
                    invalid_ips.append(f"IP: '{ip}', 域名: '{domain}' - {error_msg}")
                    continue
                
                # IP和域名都有效，添加到列表
                valid_entries.append((ip, domain))
            
            # 如果有无效的IP地址，显示错误消息并阻止保存
            if invalid_ips:
                error_message = "以下条目的IP地址格式不正确或条目不完整：\n\n"
                error_message += "\n".join(f"• {item}" for item in invalid_ips)
                error_message += "\n\n请修正后重试。"
                messagebox.showerror("IP地址格式错误", error_message)
                logger.warning(f"hosts条目验证失败: {invalid_ips}")
                return  # 阻止保存
            
            # 如果没有有效条目，提示用户
            if not valid_entries:
                messagebox.showwarning("警告", "没有有效的hosts条目。\n请至少添加一条完整的IP地址和域名映射。")
                return
            
            # 保存有效的条目
            self.custom_hosts_entries = valid_entries
            logger.info(f"保存了 {len(self.custom_hosts_entries)} 个hosts条目")
            hosts_win.destroy()
            self.update_settings_status()  # 更新设置状态显示

        # 等待窗口关闭
        self.root.wait_window(hosts_win)

    def create_deb_selection_frame(self, parent):
        """创建DEB包选择框架"""
        deb_frame = ttk.LabelFrame(parent, text="DEB包选择", padding="10")
        
        # 创建third-party框架
        deb_frame_third_party = ttk.Frame(deb_frame)
        deb_frame_third_party.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # 添加标签
        ttk.Label(deb_frame_third_party, text="third-party:").grid(row=0, column=0, sticky=tk.W)
        
        # 创建列表框和滚动条
        list_frame = ttk.Frame(deb_frame_third_party)
        list_frame.grid(row=0, column=1, columnspan=2, sticky=(tk.W, tk.E))
        
        # 进一步增加列表框高度从8改为12
        self.deb_listbox_third_party = tk.Listbox(list_frame, height=12, width=40)
        self.deb_listbox_third_party.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=self.deb_listbox_third_party.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.deb_listbox_third_party.config(yscrollcommand=scrollbar.set)
        
        # 添加按钮
        ttk.Button(deb_frame_third_party, text="添加", 
                   command=lambda: self.select_debs(self.deb_listbox_third_party)).grid(row=1, column=0, pady=(5,0), padx=(0,10), sticky=tk.W)
        
        # 修改清空按钮，添加更新状态的功能
        def clear_third_party():
            self.deb_listbox_third_party.delete(0, tk.END)
            self.update_settings_status()  # 更新设置状态显示
        
        ttk.Button(deb_frame_third_party, text="清空", 
                   command=clear_third_party).grid(row=1, column=1, pady=(5,0), padx=(0,6), sticky=tk.W)
        
        # 创建apps-third框架
        deb_frame_apps_third = ttk.Frame(deb_frame)
        deb_frame_apps_third.pack(fill=tk.BOTH, expand=True)
        
        # 添加标签
        ttk.Label(deb_frame_apps_third, text="apps-third:").grid(row=0, column=0, sticky=tk.W)
        
        # 创建列表框和滚动条
        list_frame_apps = ttk.Frame(deb_frame_apps_third)
        list_frame_apps.grid(row=0, column=1, columnspan=2, sticky=(tk.W, tk.E))
        
        # 进一步增加列表框高度从8改为12
        self.deb_listbox_apps_third = tk.Listbox(list_frame_apps, height=12, width=40)
        self.deb_listbox_apps_third.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar_apps = ttk.Scrollbar(list_frame_apps, orient="vertical", command=self.deb_listbox_apps_third.yview)
        scrollbar_apps.pack(side=tk.RIGHT, fill=tk.Y)
        self.deb_listbox_apps_third.config(yscrollcommand=scrollbar_apps.set)
        
        # 添加按钮
        ttk.Button(deb_frame_apps_third, text="添加", 
                   command=lambda: self.select_debs(self.deb_listbox_apps_third)).grid(row=1, column=0, pady=(5,0), padx=(0,10), sticky=tk.W)
        
        ttk.Button(deb_frame_apps_third, text="清空", 
                   command=lambda: self.clear_all_debs(self.deb_listbox_apps_third, "apps-third")).grid(row=1, column=1, pady=(5,0), padx=(0,6), sticky=tk.W)
        
        return deb_frame

    def _is_squashfs_extracted(self):
        """检查squashfs是否已经解压"""
        squashfs_root = LIVECD_DIR / "casper" / "squashfs-root"
        return squashfs_root.exists() and squashfs_root.is_dir()

    def _extract_filesystem(self):
        """Unsquashes the main filesystem."""
        # 如果已经解压，跳过
        if self._is_squashfs_extracted():
            logger.info("squashfs已经解压，跳过解压步骤")
            self.append_output("squashfs已经解压，跳过解压步骤")
            return
        
        squashfs_file = LIVECD_DIR / "casper" / "filesystem.squashfs"
        logger.info(f"Unsquashing {squashfs_file} ")
        self.append_output(f"正在解压 {squashfs_file.name}...") # Use squashfs_file.name

        if not squashfs_file.is_file():
            raise FileNotFoundError(f"Filesystem file not found: {squashfs_file}")

        command = [
            'unsquashfs',
            str(squashfs_file)         # Input file path needs to be absolute or relative to cwd
        ]
        # Run the command with the CWD set to the casper directory
        self._run_command(command, cwd=str(LIVECD_DIR / "casper"), step_name="解压系统文件")
        logger.info("Filesystem unsquashed successfully.")


        
    def _cleanup_squashfs_root_worker(self):
        """Removes the temporary unpacked filesystem directory."""
        target_dir = LIVECD_DIR / "casper" / "squashfs-root"
        logger.info(f"Removing temporary directory {target_dir}")
        self.append_output(f"正在删除临时解压目录 {target_dir.name}...")
        if target_dir.exists():
            try:
                shutil.rmtree(target_dir)
                logger.info("Temporary directory removed.")
                self.append_output("临时目录已删除。")
            except Exception as e:
                 logger.error(f"Failed to remove {target_dir}: {e}")
                 # Log error but don't necessarily fail the whole process
                 self.append_output(f"警告: 删除临时目录失败: {e}")
        else:
            logger.info("Temporary directory already removed.")



if __name__ == "__main__":
    # 修复主执行块的 try...except 和内部缩进
    try:
        root = tk.Tk()
        # Make sure log file is writable by root
        try: # 内部 try 块开始
            # Attempt to create/touch the log file to check permissions early
            LOG_FILE.touch(exist_ok=True)
        except OSError as e: # 内部 except 块
             print(f"Error: Cannot write to log file {LOG_FILE}. Check permissions or change LOG_FILE path. Error: {e}")

        logger.info("Application starting.")

        app = ISOCustomizer(root)
        # Only run mainloop if initialization didn't quit
        if root.winfo_exists(): 
             root.mainloop() 
        logger.info("Application finished.")
    except Exception as e:
        print(f"应用程序启动失败: {e}")
        logger.exception("Application failed to start")
