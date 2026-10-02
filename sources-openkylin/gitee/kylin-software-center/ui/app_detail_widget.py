#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from models.enums import UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH, RESOURCE_SERVER, UBUNTUKYLIN_RES_SCREENSHOT_PATH, \
    PKG_NAME, setLongTextToElideFormat,PageStates,ClickCard
from models.apkinfo import *
from ui.login import *
from ui.popular_app_card import *
from ui.circular_progress_bar import *
import requests
from ui.ahead_review_widget import Ahead_Review_Widget
from ui.all_review_widget import All_Review_Widget
from ui.show_ratings import *
from ui.shot_detail import *
from ui.starwidget import *
from utils.commontools import webbrowser_open_url
import time
import threading
import sip
import gettext

gettext.bindtextdomain("kylin-software-center", "/usr/share/locale")
gettext.textdomain("kylin-software-center")
_ = gettext.gettext


#
class Work_Thread(QThread):
    work_list = []
    get_screen_shot_over = pyqtSignal()
    down_screen_shot = pyqtSignal(str, int, str, str)

    def __init__(self, app_detail_widget, parent=None):
        QThread.__init__(self)
        self.mutex = threading.RLock()
        self.app_detail_widget = app_detail_widget
        self.get_screen_shot_over.connect(self.app_detail_widget.slot_get_screen_shot_over)
        self.down_screen_shot.connect(self.app_detail_widget.get_screen_shot)
        pass

    #
    # 函数名:获取截图
    # Function:get screenshots
    #
    def earn_crenshoots(self, update_time, previewImage):
        self.app_detail_widget.screenshot_path = self.screen_select_path(update_time, previewImage)
        if (self.app_detail_widget.screenshot_path != ""):
            screen_count = len(self.app_detail_widget.app_msg.screenshots)
            self.get_screen_shot_over.emit()

    #
    # 函数名:截图路径选择
    # Function: screenshots select path
    #
    def screen_select_path(self, update_time, previewImage):
        self.app_detail_widget.app_msg.screenshots.clear()
        if os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH):
            scre = self.app_detail_widget.app_msg.pkgname + "_thumbnail1_" + update_time
            filename = scre
            for file in os.listdir(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH):
                if (scre in file):
                    filename = file
                    break
            screen_filename_list = filename.strip("_").split(".")
            screen_filename = screen_filename_list[0]
            if (len(filename.split(".")) < 2):
                self.down_screen_shot.emit(self.app_detail_widget.app_msg.down_preview_image_path,
                                           self.app_detail_widget.app_msg.app_id,
                                           self.app_detail_widget.app_msg.pkgname, update_time)
                return ""
            else:
                last_update_time = screen_filename.split("_")[len(screen_filename.split("_")) - 1]
                if (last_update_time != update_time):
                    self.down_screen_shot.emit(self.app_detail_widget.app_msg.down_preview_image_path,
                                               self.app_detail_widget.app_msg.app_id,
                                               self.app_detail_widget.app_msg.pkgname, update_time)
                    return ""
                else:
                    set = os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + filename)
                    file_list = filename.split(".")
                    file_type = file_list[len(file_list) - 1]
                    if set:
                        if ((UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + scre) not in self.app_detail_widget.app_msg.screenshots):
                            self.app_detail_widget.app_msg.screenshots.append(
                                UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + filename)
                        i = 2
                        while (1):
                            file_name = self.app_detail_widget.app_msg.pkgname + "_thumbnail" + str(i) + "_" + last_update_time + "." + file_type
                            if (os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name)):
                                if ((UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name) not in self.app_detail_widget.app_msg.screenshots):
                                    self.app_detail_widget.app_msg.screenshots.append(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name)
                            else:
                                break
                            i = i + 1
                        if (len(self.app_detail_widget.app_msg.screenshots) < len(previewImage.split(","))):
                            self.down_screen_shot.emit(self.app_detail_widget.app_msg.down_preview_image_path,
                                                       self.app_detail_widget.app_msg.app_id,
                                                       self.app_detail_widget.app_msg.pkgname, update_time)
                            return ""
                        else:
                            return UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH
                    else:
                        self.down_screen_shot.emit(self.app_detail_widget.app_msg.down_preview_image_path,
                                                   self.app_detail_widget.app_msg.app_id,
                                                   self.app_detail_widget.app_msg.pkgname, update_time)
                        return ""
        else:
            os.mkdir(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH)
            self.down_screen_shot.emit(self.app_detail_widget.app_msg.down_preview_image_path,
                                       self.app_detail_widget.app_msg.app_id, self.app_detail_widget.app_msg.pkgname,
                                       update_time)
            return ""

        return UBUNTUKYLIN_RES_SCREENSHOT_PATH

    def run(self):
        while (1):
            if len(self.work_list) == 0:
                time.sleep(0.2)
                continue
            self.mutex.acquire()
            item = self.work_list.pop(0)
            self.mutex.release()
            if (item["action"] == "get_screen_shot"):
                self.earn_crenshoots(item["update_time"], item["previewImage"])


class App_Detail_Widget(QFrame):
    send_download_progress = pyqtSignal(int)
    puase_download = pyqtSignal(Application)
    puase_apk_download = pyqtSignal(ApkInfo)
    need_get_new_rating = pyqtSignal(str, str,str)
    need_get_download_count = pyqtSignal(str, str,str)
    check_all_check_statue = pyqtSignal()
    # zzz
    need_get_app_sign_state = pyqtSignal(int, str)
    # zzz
    get_my_rating = pyqtSignal(str, str)
    get_screen_shot_signal = pyqtSignal(str, int, str, str)
    # get_app_reviews = pyqtSignal(int, str, int)
    get_app_reviews = pyqtSignal(Application,str,int)
    get_app_reviews_apk = pyqtSignal(ApkInfo, str, int)
    # submit_my_review_to_server = pyqtSignal(int, str, str,str)
    submit_my_review_to_server = pyqtSignal(Application,str)
    submit_my_review_to_server_apk = pyqtSignal(ApkInfo,str)
    get_screen_shot_msg_signal = pyqtSignal(str, str)
    set_screenshot_signal = pyqtSignal(QLabel,int)
    get_previewnum = pyqtSignal(str)
    change_screen_shot_size = pyqtSignal()
    reviewpage = ''
    reviewlist1 = []
    Login_status = False
    is_first_time_reload_review = True
    all_review_state = False
    app_msg_change = True
    proportion = 1
    commend_height = 260
    i = 0
    num = 0
    m = 0
    btn_text = ""
    screen_left_time = QTimer()
    screen_right_time = QTimer()
    get_now_first_card_count = 0
    new_now_first_card_count = 0
    screen_widget_x = 0
    screen_widget_y = 0
    get_screen_num = 0

    def __init__(self, w, h, app_msg, ca, need_send_exposure_rate=False,click_card_type=ClickCard.INSIDECARD,parent=None):
        QFrame.__init__(self, parent)
        self.app_msg = app_msg
        self.ca = ca
        self.w = w
        self.h = h
        self.need_send_exposure_rate = need_send_exposure_rate
        self.click_card_type = click_card_type
        self.screen_h_num = 0
        self.card_width = 400
        self.card_height = 281
        self.surface_color_width = 400
        self.surface_color_height = 78
        self.screen_shot_width = 346
        self.screen_shot_height = 230
        self.screen_outer_frame_width = 370
        self.screen_outer_frame_height = 254
        self.kmre_width = 1052
        self.kmre_height = 300
        self.m_parent = parent
        self.discrip_max_height = 60
        self.pre_progress = 0
        self.font = QFont()
        self.font.setPixelSize(20)
        self.m_font = QFontMetrics(self.font)
        self.font2 = QFont()
        self.font2.setPixelSize(14)
        self.m_font2 = QFontMetrics(self.font)
        self.narrow_btn_time = QTimer()
        self.installEventFilter(self)
        self.init_ui()

        # self.wt = Work_Thread(self)
        # self.wt.start()

        # 访问服务器
        self.get_screen_shot_msg_signal.connect(self.m_parent.mw.slot_get_screen_shot_msg)


        self.result_review = {}
        self.first_submit = True
        # self.document = ''
        self.narrow_btn_time.timeout.connect(self.move_btn)
        self.need_get_download_count.connect(self.m_parent.mw.slot_need_get_download_count)
        # zzz
        # self.need_get_app_sign_state.connect(self.m_parent.mw.slot_need_get_app_sign_state)
        # zzz

        self.need_get_new_rating.connect(self.m_parent.mw.slot_need_get_new_rating)
        self.get_my_rating.connect(self.m_parent.mw.slot_get_my_rating)
        self.get_screen_shot_signal.connect(self.m_parent.mw.slot_get_screen_shot)
        self.get_app_reviews.connect(self.m_parent.mw.slot_get_app_reviews)
        self.get_app_reviews_apk.connect(self.m_parent.mw.slot_get_app_reviews)
        self.submit_my_review_to_server.connect(self.m_parent.mw.slot_submit_my_review)
        self.submit_my_review_to_server_apk.connect(self.m_parent.mw.slot_submit_my_review)

        self.get_previewnum.connect(self.m_parent.mw.slot_get_getPreviewNum)
        self.set_stylesheet()
        self.m_parent.mw.sigenal_show_downloading_widget.connect(self.show_downloading_widget)
        self.m_parent.mw.get_screen_shot_signal.connect(self.get_screen_shot_msg)
        self.change_screen_shot_size.connect(self.slot_change_screen_shot)

        self.set_screenshot_signal.connect(self.set_shotscreen)
        self.screen_left_time.timeout.connect(self.slot_screen_left_time_timeout)
        self.screen_right_time.timeout.connect(self.slot_screen_right_time_timeout)


        # self.slot_app_sign_state()

    def get_screen_shot_msg(self, msg):
        if (str(msg["appId"]) == str(self.app_msg.app_id)):
            self.app_msg.init_screen_shot = True
            # self.update_screen_shot_time =
            if(msg["previewImage"] == "null"):
                self.slot_get_screen_shot_over()
                return
            if (hasattr(self, "wt")):
                pass
            else:
                self.wt = Work_Thread(self)
                self.wt.start()
            self.app_msg.screenshots_updatetime = msg["updateTime"]
            self.app_msg.screenshots_url = msg["previewImage"]
            item = {"action": "get_screen_shot", "update_time": msg["updateTime"], "previewImage": msg["previewImage"]}
            self.wt.mutex.acquire()
            self.wt.work_list.append(item)
            self.wt.mutex.release()

    def init_ui(self):
        self.resize(self.w, self.h)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.review_text = ''
        self.submittime = True
        self.timer = QTimer(self)  # 初始化一个定时器
        self.proportion = self.w / 1340
        self.count = 30
        self.timer_go = QTimer(self)
        self.timer_go.setInterval(1000)
        self.timer_go.timeout.connect(self.slot_time_Refresh)
        self.local_lan = QLocale.system().name()
        # self.count1 = 30
        # self.timer_go_first = QTimer(self)
        # self.timer_go_first.setInterval(1000)
        # self.timer_go_first.timeout.connect(self.slot_time_Refresh_first)

        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet(
                "QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color: #ffffff;}")
        else:
            self.setStyleSheet(
                "QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color: #1F2022;}")
        self.scrollArea = QScrollArea(self)
        self.show_scrollArea_widget = QWidget(self)
        self.show_scrollArea_widget.setStyleSheet(".QWidget{border-radius: 4px;}")
        self.show_scrollArea_widget.setWindowFlags(Qt.FramelessWindowHint)
        self.show_scrollArea_widget.setAutoFillBackground(True)
        palette1 = QPalette()
        palette1.setColor(self.backgroundRole(), QColor(0, 222, 123))
        self.show_scrollArea_widget.setPalette(palette1)
        self.scrollArea.setWidget(self.show_scrollArea_widget)
        self.scrollArea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scrollArea.horizontalScrollBar().setEnabled(False)
        self.scrollArea.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.scrollArea.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                     QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                     QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                     QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                     QScrollBar::sub-page:vertical{background-color:transparent;}\
                     QScrollBar::add-page:vertical{background-color:transparent;}\
                     QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                     QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")

        self.show_scrollArea_widget.resize(self.w, self.height())

        self.show_scrollArea_widget.move(int((self.width() - self.show_scrollArea_widget.width()) / 2), 0)
        self.show_scrollArea_widget.hide()
        self.scrollArea.setGeometry(int(self.show_scrollArea_widget.x()), 0,
                                    int(self.width() - int((self.width() - self.show_scrollArea_widget.width()) / 2) - 5),
                                    int(self.height() - 10))
        self.scrollArea.hide()
        self.show_scrollArea_widget.resize(self.w - 5, self.height())

        self.kmre_photo_label = QLabel(self.show_scrollArea_widget)
        self.kmre_photo_label.setStyleSheet("QLabel{border-radius: 12px;}")
        self.kmre_photo_label.resize(1052,300)
        self.kmre_photo_label.resize(self.width()- 50*2,300)
        self.kmre_photo_label.move(50,37)
        self.image_url = "./res/kmre_banner.png"
        self.pix = QPixmap(self.image_url)
        self.fitpixmap_userIcon = self.pix.scaled(self.width()- 50 * 2, 300, Qt.IgnoreAspectRatio,
                                                   Qt.SmoothTransformation)
        self.fitpixmap_userIcon = self.PixmapToRound(self.fitpixmap_userIcon, self.width()- 50 * 2, 300)
        self.kmre_photo_label.setPixmap(self.fitpixmap_userIcon)
        self.kmre_photo_label.hide()

        self.kmre_platform_introduction = QLabel(self.show_scrollArea_widget)
        self.kmre_platform_introduction.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 26px;background-color:transparent;}")
        self.kmre_platform_introduction.setText(_("Platform_introduction"))
        self.kmre_platform_introduction.adjustSize()
        self.kmre_platform_introduction.move(int(self.kmre_photo_label.x()),int(self.kmre_photo_label.y()+self.kmre_photo_label.height()+30))
        self.kmre_platform_introduction.hide()

        self.kmre_platform_introduction_label = QLabel(self.show_scrollArea_widget)
        self.kmre_platform_introduction_label.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
        self.kmre_platform_introduction_label.move(int(self.kmre_platform_introduction.x()),int(self.kmre_platform_introduction.y()+self.kmre_platform_introduction.height()+20))
        # self.kmre_platform_introduction_label.setMaximumWidth(self.width()- 50*2)
        self.kmre_platform_introduction_label.setFixedWidth(self.width()- 50*2)
        self.kmre_platform_introduction_label.setText(self.app_msg.summary_init)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_platform_introduction_label.setWordWrap(True)
        self.kmre_platform_introduction_label.hide()

        self.kmre_technical_features = QLabel(self.show_scrollArea_widget)
        self.kmre_technical_features.setStyleSheet(
            "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 26px;background-color:transparent;}")
        self.kmre_technical_features.setText(_("Technical features"))
        self.kmre_technical_features.adjustSize()
        self.kmre_technical_features.move(int(self.kmre_platform_introduction_label.x()),
                                             int(self.kmre_platform_introduction_label.y() + self.kmre_platform_introduction_label.height() + 30))
        self.kmre_technical_features.hide()

        self.kmre_technical_features_label = QLabel(self.show_scrollArea_widget)
        self.kmre_technical_features_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
        self.kmre_technical_features_label.move(int(self.kmre_technical_features.x()),
                                                   int(self.kmre_technical_features.y() + self.kmre_technical_features.height() + 20))
        self.kmre_technical_features_label.setMaximumWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setFixedWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setText(self.app_msg.description)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_technical_features_label.setWordWrap(True)
        self.kmre_technical_features_label.hide()

        # self.kmre_btn = QPushButton(self.show_scrollArea_widget)
        # self.kmre_btn.setFocusPolicy(Qt.NoFocus)
        # self.kmre_btn.resize(140, 40)
        # self.kmre_btn.setStyleSheet(
        #     "QPushButton{font-size: 20px;font-weight: 400;color: #262626;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #FFFFFF;}QPushButton:hover{background: #FFFFFF;border-radius: 20px;color: #3790FA;font-weight: 400;border:0px;}QPushButton:pressed{background: #FFFFFF;border-radius: 20px;color: #296CD9;font-weight: 400;border:0px;}")
        # self.kmre_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width()-self.kmre_btn.width())/2),self.kmre_photo_label.y() + self.kmre_photo_label.height()-self.kmre_btn.height()-50)
        # self.kmre_btn.setEnabled(True)
        # self.kmre_btn.setCheckable(True)
        # self.kmre_btn.setText(_("download experience"))
        # self.change_kmre_btn_width()
        # self.kmre_btn.hide()

        # self.kmre_tip_label = QLabel(self.show_scrollArea_widget)
        # self.kmre_tip_label.setStyleSheet("QLabel{font-size: 16px;font-weight: 400;color: #FFFFFF;line-height: 20px;background-color:transparent;}")
        # self.kmre_tip_label.setText(_("The installation is successful. Please restart the system and use it!"))
        # self.kmre_tip_label.adjustSize()
        # self.kmre_tip_label.move(
        #     self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_tip_label.width()) / 2),
        #     self.kmre_btn.y() + self.kmre_btn.height() + 6)
        # self.kmre_tip_label.hide()

        self.icon_label = QLabel(self.show_scrollArea_widget)
        self.icon_label.resize(128, 128)
        self.icon_label.move(50, 35)

        self.photo = Photo("", self.icon_label.width(),self.icon_label.height(),self.icon_label)

        self.appname_label = QLabel(self.show_scrollArea_widget)
        self.appname_label.setFixedHeight(26)

        self.appname_label.move(int(self.icon_label.x() + self.icon_label.width() + 21), 42)

        self.app_categories1 = QLabel(self.show_scrollArea_widget)
        self.app_categories1.resize(52, 24)
        self.app_categories1.setAlignment(Qt.AlignCenter)
        self.app_categories1.move(int(self.icon_label.x() + self.icon_label.width() + 21),
                                  int(self.appname_label.y() + self.appname_label.height() + 10))
        self.app_categories1.hide()
        self.app_categories2 = QLabel(self.show_scrollArea_widget)
        self.app_categories2.resize(52, 24)
        self.app_categories2.setAlignment(Qt.AlignCenter)
        self.app_categories2.move(int(self.app_categories1.x() + self.app_categories1.width() + 4), int(self.app_categories1.y()))
        self.app_categories2.hide()
        self.app_categories3 = QLabel(self.show_scrollArea_widget)
        self.app_categories3.resize(52, 24)
        self.app_categories3.setAlignment(Qt.AlignCenter)
        self.app_categories3.move(int(self.app_categories2.x() + self.app_categories2.width() + 4), int(self.app_categories1.y()))
        self.app_categories3.hide()

        self.score_label_small = QLabel(self.show_scrollArea_widget)
        self.score_label_small.setStyleSheet(
            "QLabel{font-size: 16px;font-weight: 500;color: #E6A23B;line-height: 16px;}")
        self.score_label_small.resize(24, 16)

        self.small_starwidget = StarWidget('medium', 3, self.show_scrollArea_widget)

        self.download_num_label = QLabel(self.show_scrollArea_widget)
        self.download_num_label.setFixedHeight(16)

        self.open_wibside_label = QLabel(self.show_scrollArea_widget)
        self.open_wibside_label.setFixedHeight(16)

        self.popularity_label = QLabel(self.show_scrollArea_widget)
        self.popularity_label.setFixedHeight(16)
        if(Globals.NEED_SHOW_POPULARITY == False):
            self.popularity_label.hide()

        self.download_size_label = QLabel(self.show_scrollArea_widget)
        self.download_size_label.setFixedHeight(16)

        self.app_name = QLabel(self.show_scrollArea_widget)
        self.app_name.setFixedHeight(16)
        self.app_name.resize(209, 16)

        self.version_label = QLabel(self.show_scrollArea_widget)
        self.version_label.setFixedHeight(16)

        self.update_time_label = QLabel(self.show_scrollArea_widget)
        self.update_time_label.setFixedHeight(16)

        self.manufacturers_label = QLabel(self.show_scrollArea_widget)
        self.manufacturers_label.setFixedHeight(16)
        self.manufacturers_label.setMaximumWidth(150)

        self.website_label = QLabel(self.show_scrollArea_widget)
        self.website_label.setFixedHeight(16)
        self.website_label.setMaximumWidth(152)

        self.show_shot_detail = QFrame(self)
        self.show_shot_detail.resize(self.width(), self.height())
        self.show_shot_detail.move(0, 0)
        self.show_shot_detail.hide()
        self.show_shot_detail.setStyleSheet(
            "QFrame{background-color:rgba(0, 0, 0, 0.7);border-bottom-left-radius:6px;border-bottom-right-radius:6px;}")


        self.app_safe_detection = QLabel(self.show_scrollArea_widget)
        self.app_safe_detection.setText(_("Safety detection"))

        self.app_sign_bottom = QLabel(self.show_scrollArea_widget)
        self.app_sign_bottom.setFixedHeight(20)
        # self.app_sign.setText(_("软件签名："))
        self.app_sign_bottom.setText(_("Software signature"))
        # self.app_sign.move(self.icon_label.x() + self.icon_label.width() + 20, 124)

        self.manual_detection_bottom = QLabel(self.show_scrollArea_widget)
        self.manual_detection_bottom.setFixedHeight(20)
        # self.manual_detection.setText(_("人工检测："))
        self.manual_detection_bottom.setText(_("Manual detection"))
        # self.manual_detection.move(self.icon_label.x() + self.icon_label.width() + 20, 124)

        self.signature_unit_label = QLabel(self.show_scrollArea_widget)
        self.signature_unit_label.setFixedHeight(20)
        # self.signature_unit_label.setText(_("签名单位："))
        self.signature_unit_label.setText(_("Signature unit:"))

        self.btn = QPushButton(self.show_scrollArea_widget)
        self.btn.setFocusPolicy(Qt.NoFocus)
        self.btn.resize(100, 40)
        self.btn.setStyleSheet(
            "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
        self.btn.move(int(self.scrollArea.width() - self.btn.width() - 50), 36)
        self.btn.hide()


        self.app_screen_shot_list_widget = QWidget(self.show_scrollArea_widget)
        self.app_screen_shot_list_widget.setStyleSheet("QWidget{background-color: transparent;border: 0px solid #3790FA;}")
        self.app_screen_shot_list_widget.installEventFilter(self)
        self.app_screen_shot_list_widget.hide()

        self.show_app_screen_shot_widget = QWidget(self.app_screen_shot_list_widget)
        self.show_app_screen_shot_widget.setStyleSheet(
            "QWidget{background-color: transparent;border: 0px solid #3790FA;}")
        self.show_app_screen_shot_widget.installEventFilter(self)
        self.show_app_screen_shot_widget.show()

        self.screen_card_list_widget = QWidget(self.show_app_screen_shot_widget)
        self.screen_card_list_widget.setStyleSheet("QWidget{background-color: transparent;border: 0px solid #3790FA;}")
        self.screen_card_list_widget.installEventFilter(self)
        self.screen_card_list_widget.move(0,0)
        self.screen_card_list_widget.show()


        self.screen_left_btn = QPushButton(self.app_screen_shot_list_widget)
        self.screen_left_btn.setFocusPolicy(Qt.NoFocus)
        self.screen_left_btn.resize(30, 30)
        if (Globals.THEME_COLOR == 0):
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
        else:
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
        self.screen_left_btn.hide()
        self.screen_left_btn.setEnabled(False)
        self.screen_left_btn.clicked.connect(self.slot_clicked_screen_left_btn)

        self.screen_right_btn = QPushButton(self.app_screen_shot_list_widget)
        self.screen_right_btn.setFocusPolicy(Qt.NoFocus)
        self.screen_right_btn.resize(30, 30)
        if (Globals.THEME_COLOR == 0):
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
        else:
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
        self.screen_right_btn.hide()
        self.screen_right_btn.clicked.connect(self.slot_clicked_screen_right_btn)

        self.app_screenshot_widget_list = []
        # self.app_detail_photo1 = Screenshot(self.card_width, self.card_height,self.show_scrollArea_widget)
        # self.app_detail_photo1.resize(self.card_width, self.card_height)
        # self.app_detail_photo1.hide()
        # self.defualt_photo1 = QLabel(self.app_detail_photo1)
        # self.defualt_photo1.setFixedSize(30, 30)
        # self.defualt_photo1.move(int((self.app_detail_photo1.width() - self.defualt_photo1.width()) / 2),
        #                          int((self.app_detail_photo1.height() - self.defualt_photo1.height()) / 2))
        # self.defualt_photo1.setStyleSheet(
        #     "QLabel{background-image:url(./res/defualt_screen.png);border-radius: 0px;background-color:transparent;}")
        # self.defualt_photo1.hide()
        # self.screen_outer_frame1 = QLabel(self.app_detail_photo1)
        # self.screen_outer_frame1.resize(238, 161)
        # # self.screen_outer_frame1.move(51, 79)
        # self.screen_outer_frame1.setStyleSheet(
        #     "QLabel{border-radius: 0px;border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # self.screen_outer_frame1.hide()
        # self.screen_shot1 = QLabel(self.app_detail_photo1)
        # self.screen_shot1.resize(223, 148)
        # # self.screen_shot1.move(58, 86)
        # self.app_detail_photo1.setFocusPolicy(Qt.NoFocus)
        #
        # self.screen_shot1.hide()
        # self.surface_color1 = QLabel(self.app_detail_photo1)
        # self.surface_color1.resize(337, 78)
        # # self.surface_color1.move(0, 181)
        # if (Globals.THEME_COLOR == 0):
        #     self.surface_color1.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # else:
        #     self.surface_color1.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        #
        # # self.surface_color1.setStyleSheet("QLabel{background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(247,251,251,0), stop:1 #3790FA);border-bottom-left-radius:12px;border-bottom-right-radius:12px;opacity: 0.49;}")
        # self.surface_color1.hide()

        # self.app_detail_photo2 = QPushButton(self.show_scrollArea_widget)
        # self.app_detail_photo2.resize(self.card_width, self.card_height)
        # self.app_detail_photo2.hide()
        # self.app_detail_photo2.setFocusPolicy(Qt.NoFocus)
        # self.defualt_photo2 = QLabel(self.app_detail_photo2)
        # self.defualt_photo2.setFixedSize(30, 30)
        # self.defualt_photo2.move(int((self.app_detail_photo2.width() - self.defualt_photo2.width()) / 2),
        #                          int((self.app_detail_photo2.height() - self.defualt_photo2.height()) / 2))
        #
        # self.defualt_photo2.setStyleSheet(
        #     "QLabel{background-image:url(./res/defualt_screen.png);border-radius: 0px;background-color:transparent;}")
        # self.defualt_photo2.hide()
        # self.screen_outer_frame2 = QLabel(self.app_detail_photo2)
        # self.screen_outer_frame2.resize(238, 161)
        # # self.screen_outer_frame2.move(51, 79)
        # self.screen_outer_frame2.setStyleSheet(
        #     "QLabel{border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # self.screen_outer_frame2.hide()
        # self.screen_shot2 = QLabel(self.app_detail_photo2)
        # self.screen_shot2.setFocusPolicy(Qt.NoFocus)
        # self.screen_shot2.resize(223, 148)
        # # self.screen_shot2.move(58, 86)
        # self.screen_shot2.hide()
        # self.surface_color2 = QLabel(self.app_detail_photo2)
        # self.surface_color2.resize(337, 78)
        # # self.surface_color2.move(0, 181)
        # # self.surface_color2.setStyleSheet(
        # #     "QLabel{background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(247,251,251,0), stop:1 #A8E4D1);border-bottom-left-radius:12px;border-bottom-right-radius:12px;opacity: 0.49;}")
        # if (Globals.THEME_COLOR == 0):
        #     self.surface_color2.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # else:
        #     self.surface_color2.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        #
        # self.surface_color2.hide()
        #
        # self.app_detail_photo3 = QPushButton(self.show_scrollArea_widget)
        # self.app_detail_photo3.resize(self.card_width, self.card_height)
        # self.app_detail_photo3.hide()
        # self.app_detail_photo3.setFocusPolicy(Qt.NoFocus)
        # self.defualt_photo3 = QLabel(self.app_detail_photo3)
        # self.defualt_photo3.setFixedSize(30, 30)
        # self.defualt_photo3.move(int((self.app_detail_photo3.width() - self.defualt_photo3.width()) / 2),
        #                          int((self.app_detail_photo3.height() - self.defualt_photo3.height()) / 2))
        #
        # self.defualt_photo3.setStyleSheet(
        #     "QLabel{background-image:url(./res/defualt_screen.png);border-radius: 0px;background-color:transparent;}")
        # self.defualt_photo3.hide()
        # self.screen_outer_frame3 = QLabel(self.app_detail_photo3)
        # self.screen_outer_frame3.resize(238, 161)
        # # self.screen_outer_frame3.move(51, 79)
        # self.screen_outer_frame3.setStyleSheet(
        #     "QLabel{border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # self.screen_outer_frame3.hide()
        # self.screen_shot3 = QLabel(self.app_detail_photo3)
        # self.screen_shot3.setFocusPolicy(Qt.NoFocus)
        # self.screen_shot3.resize(223, 148)
        # # self.screen_shot3.move(58, 86)
        # self.screen_shot3.hide()
        # self.surface_color3 = QLabel(self.app_detail_photo3)
        # self.surface_color3.resize(337, 78)
        # # self.surface_color3.move(0, 181)
        # # self.surface_color3.setStyleSheet(
        # #     "QLabel{background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(247,251,251,0), stop:1 #A8E4D1);border-bottom-left-radius:12px;border-bottom-right-radius:12px;opacity: 0.49;}")
        # if (Globals.THEME_COLOR == 0):
        #     self.surface_color3.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # else:
        #     self.surface_color3.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        #
        # self.surface_color3.hide()
        #
        # self.app_detail_photo4 = QPushButton(self.show_scrollArea_widget)
        # self.app_detail_photo4.resize(self.card_width, self.card_height)
        # self.app_detail_photo4.hide()
        # self.app_detail_photo4.setFocusPolicy(Qt.NoFocus)
        # self.defualt_photo4 = QLabel(self.app_detail_photo4)
        # self.defualt_photo4.setFixedSize(30, 30)
        # self.defualt_photo4.move(int((self.app_detail_photo4.width() - self.defualt_photo4.width()) / 2),
        #                          int((self.app_detail_photo4.height() - self.defualt_photo4.height()) / 2))
        # self.defualt_photo4.setStyleSheet(
        #     "QLabel{background-image:url(./res/defualt_screen.png);border-radius: 0px;background-color:transparent;}")
        # self.defualt_photo4.hide()
        # self.screen_outer_frame4 = QLabel(self.app_detail_photo4)
        # self.screen_outer_frame4.resize(238, 161)
        # # self.screen_outer_frame4.move(51, 79)
        # self.screen_outer_frame4.setStyleSheet(
        #     "QLabel{border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # self.screen_outer_frame4.hide()
        # self.screen_shot4 = QLabel(self.app_detail_photo4)
        # self.screen_shot4.resize(223, 148)
        # # self.screen_shot4.move(58, 86)
        # self.screen_shot4.setFocusPolicy(Qt.NoFocus)
        # self.screen_shot4.hide()
        # self.surface_color4 = QLabel(self.app_detail_photo4)
        # self.surface_color4.resize(337, 78)
        # # self.surface_color4.move(0, 181)
        # # self.surface_color3.setStyleSheet(
        # #     "QLabel{background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(247,251,251,0), stop:1 #A8E4D1);border-bottom-left-radius:12px;border-bottom-right-radius:12px;opacity: 0.49;}")
        # if (Globals.THEME_COLOR == 0):
        #     self.surface_color4.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        # else:
        #     self.surface_color4.setStyleSheet(
        #         "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        #
        # self.surface_color4.hide()

        self.screen_loading = QLabel(self.show_scrollArea_widget)
        self.loadingmove = QMovie("./res/loadgif2.gif")
        self.loadingmove_dark = QMovie("./res/loadgifdark.gif")
        self.screen_loading.setAttribute(Qt.WA_TranslucentBackground)
        self.screen_loading.resize(40, 40)
        self.screen_loading.setAlignment(Qt.AlignCenter)
        if (Globals.THEME_COLOR == 0):
            self.screen_loading.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.screen_loading.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
        self.screen_loading.hide()

        self.manufacturer_label = QLabel(self.show_scrollArea_widget)
        self.manufacturer_label.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
        self.manufacturer_label.setText(_("Manufacturer"))
        self.manufacturer_label.adjustSize()
        self.manufacturer_label.hide()
        ft = QFont()
        ft.setPixelSize(14)
        self.manufacturer_name_label = QLabel(self.show_scrollArea_widget)
        # self.app_discription_label.installEventFilter(self)
        self.manufacturer_name_label.setFocusPolicy(Qt.NoFocus)
        self.manufacturer_name_label.setFont(ft)
        self.manufacturer_name_label.setContextMenuPolicy(Qt.NoContextMenu)
        self.manufacturer_name_label.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.manufacturer_name_label.hide()

        self.software_introduction = QLabel(self.show_scrollArea_widget)
        self.software_introduction.setStyleSheet(
            "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
        self.software_introduction.setText(_("Software Introduction"))
        self.software_introduction.adjustSize()
        ft = QFont()
        ft.setPixelSize(14)
        self.app_discription_label = QTextEdit(self.show_scrollArea_widget)
        # self.app_discription_label.installEventFilter(self)
        self.app_discription_label.setReadOnly(True)
        self.app_discription_label.setFocusPolicy(Qt.NoFocus)
        self.app_discription_label.setFixedSize(int(1184 * self.proportion), 74)
        self.app_discription_label.setFont(ft)
        self.app_discription_label.setContextMenuPolicy(Qt.NoContextMenu)
        self.app_discription_label.setTextInteractionFlags(Qt.TextSelectableByMouse)
        # self.app_discription_label.installEventFilter(self)
        # self.clipboard = QApplication.clipboard()
        # self.clipboard.selectionChanged.connect(self.textCopy)

        self.show_more_text = QPushButton(self.show_scrollArea_widget)
        self.show_more_text.setFocusPolicy(Qt.NoFocus)
        self.show_more_text.setStyleSheet("QPushButton{border: 0px;font-size: 18px;font-weight: 500;color: #3790FA;line-height: 20px;}")
        self.show_more_text.setText(_("more"))
        self.show_more_text.move(
            int(self.show_scrollArea_widget.width() - self.app_discription_label.x() - self.show_more_text.width()),
            int(self.app_discription_label.y() + self.app_discription_label.height() - self.show_more_text.height() + 1))
        self.show_more_text.hide()
        self.show_more_text.clicked.connect(self.slot_show_more_text)

        self.down_progress = Circular_Progress_Bar(36, 36, self.app_msg.pkgname, self.show_scrollArea_widget)
        self.down_progress.update_progress(self.app_msg.percent)
        self.down_progress.setStyleSheet(
            "QFrame{border:0px solid #ff0000;border-radius: 12px;background-color:transparent;}")
        # self.down_progress.btn.resize(18, 18)
        self.down_progress.btn.setStyleSheet(
            "QPushButton{border:0px solid #ff0000;border-radius: 16px;background-color:transparent;image: url(./res/puase2.png)}")
        self.down_progress.btn.move(int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                    int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
        self.down_progress.move(int(self.btn.x() + self.btn.width() - self.down_progress.width()),
                                int(self.btn.y() + (int(self.btn.height() - self.down_progress.height()) / 2)))
        self.down_progress.hide()
        self.send_download_progress.connect(self.down_progress.GenUpdate)
        self.down_progress.btn.clicked.connect(self.slot_puase_download)
        self.down_progress.cont_btn.clicked.connect(lambda: self.m_parent.mw.re_download_app(self.app_msg))

        self.cancel_btn = QPushButton(self.show_scrollArea_widget)
        self.cancel_btn.setFocusPolicy(Qt.NoFocus)
        self.cancel_btn.resize(36, 36)
        self.cancel_btn.setStyleSheet(
            "QPushButton{background-image:url(./res/cancel2.png);border-radius:15px;}QPushButton:hover{background-image:url(./res/cancel2_hover.png);border-radius:15px;}")
        # self.cancel_btn.setText(_("cancel"))
        self.cancel_btn.move(int(self.down_progress.x() - self.cancel_btn.width() - 8),
                             int(self.down_progress.y() - int((self.cancel_btn.height() - self.down_progress.height()) / 2)))
        self.cancel_btn.hide()
        self.cancel_btn.clicked.connect(lambda: self.m_parent.mw.slot_click_cancel_btn(self, self.app_msg))

        self.line_label = QLabel(self.show_scrollArea_widget)
        self.line_label.resize(self.width() - 50 * 2, 2)


        self.software_rating_label = QLabel(self.show_scrollArea_widget)
        self.software_rating_label.resize(72, 20)

        self.ratings_widget = show_ratings_widget(self, self.show_scrollArea_widget)
        self.ratings_widget.btn_all_review.clicked.connect(self.slot_change_reviewWindow)
        self.ratings_widget.edit_btn.clicked.connect(lambda:self.m_parent.mw.slot_click_edit_btn(self.app_msg,self.ratings_widget.MY_SCORE))

        self.count_of_participation_rating = QLabel(self.show_scrollArea_widget)
        self.count_of_participation_rating.resize(96, 20)

        self.score_label = QLabel(self.show_scrollArea_widget)
        self.score_label.setStyleSheet("QLabel{font-size: 60px;font-weight: 500;color: #E6A23B;line-height: 60px;}")
        self.score_label.setAlignment(Qt.AlignLeft)
        self.score_label.resize(95, 60)

        self.full_score_label = QLabel(self.show_scrollArea_widget)
        self.full_score_label.setStyleSheet(
            "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.full_score_label.resize(51, 20)

        self.average_score_star = StarWidget("big",0,self.show_scrollArea_widget)

        self.editorial_comments_widget = QFrame(self.show_scrollArea_widget)
        # self.editorial_comments_widget.setStyleSheet("QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")


        self.tip = QLabel(self.editorial_comments_widget)
        self.tip.setStyleSheet(
            "QLabel{background-color:transparent;font-size:14px;font-weight: 400;color: #F56C6C;line-height: 20px;border:none;}")
        self.tip.resize(349, 20)

        self.tip.setFocusPolicy(Qt.NoFocus)
        self.tip.hide()

        self.time_tip = QLabel(self.editorial_comments_widget)
        self.time_tip.setStyleSheet(
            "QLabel{background-color:transparent;font-size:14px;font-weight: 400;color: #F56C6C;line-height: 20px;border:none;}")
        self.time_tip.resize(349, 20)

        self.time_tip.setFocusPolicy(Qt.NoFocus)
        self.time_tip.hide()


        self.editorial_comments_label = QTextEdit(self.editorial_comments_widget)
        self.editorial_comments_label.setContextMenuPolicy(Qt.NoContextMenu)

        # self.editorial_comments_label.setPalette(_("请输入评论"))
        # self.editorial_comments_label.setPlaceholderText(_("Please enter your comments"))
        # self.editorial_comments_label.installEventFilter(self)
        self.editorial_comments_label.setStyleSheet(
            "QTextEdit{border:0px;font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
        self.editorial_comments_label.verticalScrollBar().setStyleSheet("QScrollBar:vertical{margin:0px 0px 0px 0px;background-color:transparent;border:0px;width:6px;border-radius: 3px;}\
                     QScrollBar::handle:vertical{background-color:#C2C2C2;width:6px;border-radius:3px;} QScrollBar::handle:vertical:hover{background-color:#8F9399;opacity:0.1;width:6px;border-radius:3px;}  QScrollBar::handle:vertical:pressed{background-color:#8F9399;width:6px;border-radius:3px;}\
                     QScrollBar::sub-line:vertical{subcontrol-origin:margin;border:0px solid red;height:13px}\
                     QScrollBar::up-arrow:vertical{subcontrol-origin:margin;background-color:rgba(0,0,0,0);height:13px}\
                     QScrollBar::sub-page:vertical{background-color:transparent;}\
                     QScrollBar::add-page:vertical{background-color:transparent;}\
                     QScrollBar::down-arrow:vertical{background-color:rgba(0,0,0,0);}\
                     QScrollBar::add-line:vertical{subcontrol-origin:margin;border:0px solid green;height:13px}")
        self.editorial_comments_label.textChanged.connect(self.slot_edit_label_textchanged)
        self.back_text = QLabel(self.editorial_comments_label)
        self.back_text.setStyleSheet(
            "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")
        self.back_text.setAttribute(Qt.WA_TranslucentBackground)
        self.back_text.setText(_("Please enter your comments"))
        self.back_text.adjustSize()
        self.back_text.move(6, 2)
        if (Globals.LOGIN_SUCCESS):
            self.back_text.show()
        else:
            self.back_text.hide()

        self.submit_btn = QPushButton(self.editorial_comments_widget)
        self.submit_btn.setFocusPolicy(Qt.NoFocus)
        self.submit_btn.resize(100, 40)
        self.submit_btn.setStyleSheet(
            "QPushButton{background: #3790FA;border-radius: 20px;border: 1px solid #3790FA;font-size: 20px;text-align:center;font-weight: 400;color: #FFFFFF;line-height: 12px;}QPushButton：hover{background: #40A9FB;border-radius: 20px;border: 1px solid #3790FA;font-size: 20px;font-weight: 400;text-align:center;color: #FFFFFF;line-height: 12px;}QPushButton：pressed{background: #3790FA;border-radius: 20px;border: 1px solid #296CD9;font-size: 20px;font-weight: 400;text-align:center;color: #FFFFFF;line-height: 12px;}")

        self.submit_btn.setText(_("submit"))
        self.submit_btn.hide()
        self.submit_btn.clicked.connect(self.submit_my_review)

        self.editorial_comments_label.hide()

        self.all_review_label = QLabel(self.show_scrollArea_widget)
        self.all_review_label.setFixedHeight(20)
        self.all_review_label.setText(_("ALL REVIEWS"))
        self.all_review_label.hide()
        # 为未登录的状态设置按钮,继承于self.editorial_comments_widget
        self.btn_login_and_register = QPushButton(self.editorial_comments_widget)
        # self.btn_login_and_register.setText(_("登录 | 注册"))
        self.btn_login_and_register.setText(_("Log"))
        self.btn_login_and_register.setFocusPolicy(Qt.NoFocus)
        self.btn_login_and_register.setFixedSize(28, 20)
        self.btn_login_and_register.setStyleSheet(
            "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        # self.btn_login_and_register.move(self.editorial_comments_widget.width() / 2 - 49,self.editorial_comments_widget.height() / 2 - 13)
        self.btn_login_and_register.hide()
        self.btn_login_and_register.clicked.connect(self.slot_login_in)

        self.btn_login_and_register_line = QPushButton(self.editorial_comments_widget)
        # self.btn_login_and_register.setText(_("登录 | 注册"))
        self.btn_login_and_register_line.setText(_("|"))
        self.btn_login_and_register_line.setFocusPolicy(Qt.NoFocus)
        self.btn_login_and_register_line.setFixedSize(1, 20)
        self.btn_login_and_register_line.setStyleSheet(
            "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        # self.btn_login_and_register_line.move(self.editorial_comments_widget.width() / 2 - 49-37,self.editorial_comments_widget.height() / 2 - 10 )
        self.btn_login_and_register_line.hide()
        # self.btn_login_and_register_line.clicked.connect(self.slot_login_in)

        self.btn_login_and_register_register = QPushButton(self.editorial_comments_widget)

        # self.btn_login_and_register_register.setText(_("注册"))
        self.btn_login_and_register_register.setText(_("Reg"))
        self.btn_login_and_register_register.setFocusPolicy(Qt.NoFocus)
        self.btn_login_and_register_register.setFixedSize(28, 20)
        self.btn_login_and_register_register.setStyleSheet(
            "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
        # self.btn_login_and_register_register.move(self.editorial_comments_widget.width() / 2 - 49-37 - 9,self.editorial_comments_widget.height() / 2 - 13)
        self.btn_login_and_register_register.hide()
        self.btn_login_and_register_register.clicked.connect(self.slot_to_register)

        # 全部评论和收起评论的两个按钮
        # self.btn_all_review = QPushButton(self.show_scrollArea_widget)
        # self.btn_all_review.setText(_("View reviews"))
        # self.btn_all_review.setFocusPolicy(Qt.NoFocus)
        # self.btn_all_review.setStyleSheet(
        #     "QPushButton{background: transparent;border:none;font-size: 18px;font-weight: 400;text-align: left;color: #3790FA;line-height: 12px;}")
        # # self.btn_all_review.resize(120,40)
        # if self.local_lan == 'zh_CN':
        #     self.btn_all_review.setFixedSize(len(self.btn_all_review.text()) * 18, 20)
        # else:
        #     self.btn_all_review.setFixedSize(len(self.btn_all_review.text()) * 9, 20)

        self.btn_retract = QPushButton(self)
        self.btn_retract.setFocusPolicy(Qt.NoFocus)
        self.btn_retract.hide()
        if (Globals.THEME_COLOR == 0):
            self.btn_retract.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:50px;image: url(./res/return_shortreview.png)}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked.png)}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked.png)}")
        else:
            self.btn_retract.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:50px;image: url(./res/return_shortreview_dark.png)}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked_dark.png)}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked_dark.png)}")
        # self.btn_retract.setGeometry(1211,416,100,100)
        self.btn_retract.resize(60, 60)
        self.btn_retract.clicked.connect(self.slot_go_back_Top)

        self.btn_close_all_review = QPushButton(self.show_scrollArea_widget)
        self.btn_close_all_review.hide()
        # self.btn_close_all_review.setText(_("Hide reviews"))
        # self.btn_close_all_review.setText(_("收起评论"))
        self.btn_close_all_review.setFocusPolicy(Qt.NoFocus)
        self.btn_close_all_review.setStyleSheet(
            "QPushButton{background: transparent;border:none;font-size: 18px;font-weight: 400;text-align: left;color: #3790FA;line-height: 12px;}")
        self.btn_close_all_review.resize(120, 40)
        # self.btn_close_all_review.clicked.connect(self.slot_change_shortreviewWindow)

        self.reviews_num = QLabel(self.show_scrollArea_widget)
        self.reviews_num.setStyleSheet(
            "QLabel{background: transparent;border:none;font-size: 14px;font-weight: 400;text-align: left;color: #606265;line-height: 20px;}")
        # self.reviews_num.resize(154,20)

        self.reviews_num.hide()

        # 左右滑动评论窗口
        self.editorial_review_widget = QFrame(self.show_scrollArea_widget)
        # self.editorial_review_widget.setStyleSheet("QFrame{background: #fff000;}")

        # 上下滑动评论窗口
        self.all_review_widget = QFrame(self.show_scrollArea_widget)
        # self.all_review_widget.setStyleSheet("QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #12C188;}")

        self.scrollArea.verticalScrollBar().valueChanged.connect(self.slot_load_more_review)
        self.scrollArea.verticalScrollBar().valueChanged.connect(self.slot_show_btn_reacter)

        self.loadinglabel = QLabel(self.show_scrollArea_widget)
        self.loadinglabel.setAttribute(Qt.WA_TranslucentBackground)
        self.loadinglabel.resize(350, 350)
        self.loadinglabel.setAlignment(Qt.AlignCenter)
        if (Globals.THEME_COLOR == 0):
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()
        else:
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
        self.loadinglabel.show()
        self.deal_loading_and_btn_review()
        # Globals.ALL_REVIEWLIST.clear()
        self.reviewlist1.clear()

        # self.get_reviewlist_to_init_review_ui()
        # 检测是否为登录状态
        self.Login_status_check()
        # 展示截图界面
        self.show_sheet_shot = Shot_Detail(self.app_msg.screenshots, self.width(), self.height(), self,
                                           self.show_shot_detail)

        self.kmre_btn = QPushButton(self.show_scrollArea_widget)
        self.kmre_btn.setFocusPolicy(Qt.NoFocus)
        self.kmre_btn.resize(140, 40)
        self.kmre_btn.setStyleSheet(
            "QPushButton{font-size: 20px;font-weight: 400;color: #262626;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #FFFFFF;}QPushButton:hover{background: #FFFFFF;border-radius: 20px;color: #3790FA;font-weight: 400;border:0px;}QPushButton:pressed{background: #FFFFFF;border-radius: 20px;color: #296CD9;font-weight: 400;border:0px;}")
        self.kmre_btn.move(int(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_btn.width()) / 2)),
                           int(self.kmre_photo_label.y() + self.kmre_photo_label.height() - self.kmre_btn.height() - 50))
        self.kmre_btn.setEnabled(True)
        self.kmre_btn.setCheckable(True)
        self.kmre_btn.setText(_("download experience"))
        self.change_kmre_btn_width()
        self.kmre_btn.hide()
        self.kmre_tip_label = QLabel(self.show_scrollArea_widget)
        self.kmre_tip_label.setStyleSheet(
            "QLabel{font-size: 16px;font-weight: 400;color: #FFFFFF;line-height: 20px;background-color:transparent;}")
        self.kmre_tip_label.setText(_("The installation is successful. Please restart the system and use it!"))
        self.kmre_tip_label.adjustSize()
        self.kmre_tip_label.move(
            int(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_tip_label.width()) / 2)),
            int(self.kmre_btn.y() + self.kmre_btn.height() + 6))
        self.kmre_tip_label.hide()
        self.show_kmre_detail()
        self.slot_set_dark_mode()

    def textCopy(self):
        self.app_discription_label.copy()
        command = self.clipboard.text(self.clipboard.Selection)

    def show_kmre_detail(self):
        if(self.app_msg.is_notice == False):
            if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                self.kmre_photo_label.show()
                self.kmre_platform_introduction.show()
                self.kmre_platform_introduction_label.show()
                self.kmre_technical_features.show()
                self.kmre_technical_features_label.show()
                # self.kmre_btn.show()
                self.appname_label.hide()
                self.icon_label.hide()
                self.app_categories1.hide()
                self.app_categories2.hide()
                self.app_categories3.hide()
                self.appname_label.hide()
                self.version_label.hide()
                self.update_time_label.hide()
                self.manufacturers_label.hide()
                self.website_label.hide()
                self.download_num_label.hide()
                self.open_wibside_label.hide()
                self.download_size_label.hide()
                self.score_label_small.hide()
                self.small_starwidget.hide()
                self.all_review_label.hide()
                self.app_name.hide()
                self.manual_detection_bottom.hide()
                self.signature_unit_label.hide()
                self.software_introduction.hide()
                self.app_discription_label.hide()
                self.app_discription_label.move(-3000,-200)
                self.show_more_text.hide()
                self.line_label.hide()
                self.software_rating_label.hide()
                self.ratings_widget.hide()
                self.count_of_participation_rating.hide()
                self.score_label.hide()
                self.editorial_comments_widget.hide()
                self.full_score_label.hide()
                self.app_safe_detection.hide()
                self.app_sign_bottom.hide()
                self.loadinglabel.hide()
                self.average_score_star.hide()
                self.screen_loading.hide()
                self.btn.hide()
                self.photo.hide()
                self.show_scrollArea_widget.resize(self.width(),self.kmre_technical_features_label.y()+self.kmre_technical_features_label.height()+30)
            else:
                self.kmre_photo_label.hide()
                self.kmre_platform_introduction.hide()
                self.kmre_platform_introduction_label.hide()
                self.kmre_technical_features.hide()
                self.kmre_technical_features_label.hide()
                self.kmre_tip_label.hide()
                self.kmre_btn.hide()
                # self.btn.show()
                # self.cancel_btn.show()
                self.appname_label.show()
                self.icon_label.show()
                # self.app_categories1.show()
                # self.app_categories2.show()
                # self.app_categories3.show()
                self.appname_label.show()
                self.version_label.show()
                self.update_time_label.show()
                # self.manufacturers_label.show()
                # self.website_label.show()
                if(self.app_msg.from_ukscdb == True):
                    self.download_num_label.show()
                else:
                    self.download_num_label.hide()
                self.download_size_label.show()
                self.open_wibside_label.hide()
                self.score_label_small.show()
                self.small_starwidget.show()
                # self.all_review_label.show()
                self.app_name.show()
                self.manual_detection_bottom.show()
                # self.signature_unit_label.show()
                self.software_introduction.show()
                self.average_score_star.show()
                self.app_discription_label.show()
                # self.show_more_text.show()
                self.line_label.show()
                self.software_rating_label.show()
                self.ratings_widget.show()
                self.count_of_participation_rating.hide()
                self.score_label.show()
                self.editorial_comments_widget.hide()
                self.full_score_label.show()
                self.app_safe_detection.show()
                self.app_sign_bottom.show()
                # self.loadinglabel.hide()
                # self.screen_loading.show()
                # self.btn.show()
        else:
            self.kmre_photo_label.hide()
            self.kmre_platform_introduction.hide()
            self.kmre_platform_introduction_label.hide()
            self.kmre_technical_features.hide()
            self.kmre_technical_features_label.hide()
            self.kmre_tip_label.hide()
            self.kmre_btn.hide()
            self.appname_label.show()
            self.icon_label.show()
            # self.app_categories1.show()
            # self.app_categories2.show()
            # self.app_categories3.show()
            self.appname_label.show()
            self.version_label.hide()
            self.update_time_label.show()
            # self.manufacturers_label.show()
            # self.website_label.show()
            self.download_num_label.hide()
            self.open_wibside_label.show()
            self.download_size_label.hide()
            self.score_label_small.hide()
            self.small_starwidget.hide()
            self.average_score_star.hide()
            # self.all_review_label.show()
            self.app_name.hide()
            self.manual_detection_bottom.hide()
            # self.signature_unit_label.show()
            self.software_introduction.show()
            self.app_discription_label.show()
            # self.show_more_text.show()
            self.line_label.hide()
            self.software_rating_label.hide()
            self.ratings_widget.hide()
            self.count_of_participation_rating.hide()
            self.score_label.hide()
            self.editorial_comments_widget.hide()
            self.full_score_label.hide()
            self.app_safe_detection.hide()
            self.app_sign_bottom.hide()
            # self.loadinglabel.hide()
            # self.screen_loading.show()
            # self.btn.show()
            self.show_scrollArea_widget.resize(self.width(),self.app_discription_label.y() + self.app_discription_label.height() + 30)
            
    #
    #函数：处理获取点击次数结束
    #
    def slot_get_previe_num_over(self,appid,count = ""):
        if(str(appid) == str(self.app_msg.app_id)):
            if(count == ""):
                self.open_wibside_label.setText(_("Times of opening the official website:")+_("failure"))
                self.open_wibside_label.adjustSize()
            else:
                self.open_wibside_label.setText(_("Times of opening the official website:") + count)
                self.open_wibside_label.adjustSize()

    #
    # 函数：重新设计界面大小
    #
    def re_move(self):
        self.proportion = self.width() / 1340
        self.show_scrollArea_widget.resize(self.width(), self.height())
        self.show_scrollArea_widget.move(int((self.width() - self.show_scrollArea_widget.width()) / 2), 0)
        self.scrollArea.setGeometry(int(self.show_scrollArea_widget.x()), 0,
                                    int(self.width() - int((self.width() - self.show_scrollArea_widget.width()) / 2) - 5),
                                    int(self.height() - 10))

        self.show_scrollArea_widget.resize(self.width() - 5, self.height())
        self.btn.move(int(self.scrollArea.width() - self.btn.width() - 50), 36)
        self.kmre_photo_label.resize(int(self.width() - 50*2),int(((self.width() - 50*2)/self.kmre_width)*self.kmre_height))
        pix = QPixmap(self.image_url)
        fitpixmap_userIcon = self.pix.scaled(self.width() - 50 * 2, self.kmre_photo_label.height(), Qt.IgnoreAspectRatio,
                                                  Qt.SmoothTransformation)
        fitpixmap_userIcon = self.PixmapToRound(fitpixmap_userIcon, self.width() - 50 * 2, self.kmre_photo_label.height())
        self.kmre_photo_label.setPixmap(fitpixmap_userIcon)
        self.kmre_photo_label.hide()

        self.kmre_platform_introduction.move(int(self.kmre_photo_label.x()),
                                             int(self.kmre_photo_label.y() + self.kmre_photo_label.height() + 30))

        self.kmre_platform_introduction_label.move(int(self.kmre_platform_introduction.x()),
                                                   int(self.kmre_platform_introduction.y() + self.kmre_platform_introduction.height() + 20))
        # self.kmre_platform_introduction_label.setMaximumWidth(self.width() - 50 * 2)
        self.kmre_platform_introduction_label.setFixedWidth(self.width() - 50 * 2)
        self.kmre_platform_introduction_label.setText(self.app_msg.summary_init)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_platform_introduction_label.setWordWrap(True)

        self.kmre_technical_features.move(int(self.kmre_platform_introduction_label.x()),
                                          int(self.kmre_platform_introduction_label.y() + self.kmre_platform_introduction_label.height() + 30))

        self.kmre_technical_features_label.move(int(self.kmre_technical_features.x()),
                                                int(self.kmre_technical_features.y() + self.kmre_technical_features.height() + 20))
        self.kmre_technical_features_label.setMaximumWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setFixedWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setText(self.app_msg.description)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_technical_features_label.setWordWrap(True)
        proportion = (self.width() - 50 * 2) / self.kmre_width
        # self.kmre_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_btn.width()) / 2),
        #                    self.kmre_photo_label.y() + self.kmre_photo_label.height() - self.kmre_btn.height() - 50)
        self.kmre_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_btn.width()) / 2),
                           self.kmre_photo_label.y() + self.kmre_photo_label.height() - self.kmre_btn.height() - int(50*proportion))
        self.kmre_tip_label.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - self.kmre_tip_label.width()) / 2),
            self.kmre_btn.y()+self.kmre_btn.height() + 6)
        self.change_kmre_btn_width()

        self.app_screen_shot_list_widget.move(10, self.download_num_label.y() + self.download_num_label.height() + 40)

        ############截图缩放
        self.app_discription_label.setFixedSize(1184 * self.proportion, 100)
        self.app_discription_label.setFixedHeight(self.app_discription_label.document().size().height())

        self.show_more_text.move(
            self.show_scrollArea_widget.width() - self.app_discription_label.x() - self.show_more_text.width(),
            self.app_discription_label.y() + self.app_discription_label.height() - self.show_more_text.height() + 1)
        if (str(self.app_msg.summary_init) != "None"):
            self.setLongTextToElideFormat(self.app_discription_label,self.app_msg.description,str(self.app_msg.summary_init) )
        else:
            self.setLongTextToElideFormat(self.app_discription_label, self.app_msg.description)
        self.show_more_btn()
        if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
            self.down_progress.btn.move(int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                        int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
            self.cancel_btn.move(self.kmre_photo_label.x()+int((self.kmre_photo_label.width() - (self.down_progress.width() + self.cancel_btn.width() + 9))/2),
                                 self.kmre_btn.y() + int((self.kmre_btn.height() - self.cancel_btn.height())/2 ))
            self.down_progress.move(self.cancel_btn.x()+self.cancel_btn.width()+9,self.cancel_btn.y())

        else:
            self.down_progress.btn.move(int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                        int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
            self.down_progress.move(self.btn.x() + self.btn.width() - self.down_progress.width(),
                                    self.btn.y() + (int(self.btn.height() - self.down_progress.height()) / 2))
            self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8,
                                 self.down_progress.y() - int((self.cancel_btn.height() - self.down_progress.height()) / 2))
        self.line_label.resize(self.width() - 50 * 2, 2)
        self.show_sheet_shot.resize(self.width(), self.height())
        self.show_sheet_shot.re_move(self.app_msg.screenshots)
        self.show_shot_detail.resize(self.width(), self.height())
        self.change_screen_shot_size.emit()
        self.move_and_resize()
        # self.show_sheet_shot.shot_pic_change(self.app_msg.screenshots)
        if (hasattr(self, "review_widget")):
            self.review_widget.resize(self.editorial_comments_widget.width() + 31 * 2 + 20 * 2, 260)
            self.review_widget.re_move()
        if (hasattr(self, "all_review")):
            self.all_review.resize(self.editorial_comments_widget.width(), 260)
            self.all_review.re_move()
        pass

    def slot_change_screen_shot(self):
        self.show_screen_shot()

    def init_disconnect(self):
        screen_count = len(self.app_msg.screenshots)
        for i in self.app_screenshot_widget_list:
            try:
                i.clicked.disconnect()
            except Exception:
                pass

    #
    # 函数：隐藏截图窗口
    #
    def hide_screen_shot(self):
        for i in self.app_screenshot_widget_list:
            i.surface_color.hide()
            i.screen_outer_frame.hide()
            i.screen_shot.hide()
            i.defualt_photo.hide()
            i.hide()


    #
    # 函数：重新缩放截图
    #
    def reload_screen(self, card_width, card_height, proportion):
        for i in self.app_screenshot_widget_list:
            i.resize(card_width,card_height)
            i.screen_outer_frame.resize(int(self.screen_outer_frame_width * proportion),
                                            int(self.screen_outer_frame_height * proportion))
            if (Globals.THEME_COLOR == 0):
                i.screen_outer_frame.setStyleSheet(
                    "QLabel{border-image: url(./res/screenshot_2.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            i.screen_shot.resize(int(self.screen_shot_width * proportion),int(self.screen_shot_height * proportion))
            i.surface_color.resize(int(self.surface_color_width * proportion),int(self.surface_color_height * proportion))
            i.surface_color.move(0, i.height() - i.surface_color.height())
            if (Globals.THEME_COLOR == 0):
                i.surface_color.setStyleSheet(
                    "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            else:
                i.surface_color.setStyleSheet(
                    "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
        self.move_and_resize()

    #
    #函数：处理截图的显示
    #
    def set_shotscreen(self,label,num):
        QApplication.processEvents()
        time.sleep(0.1)
        if (not hasattr(self, "pix1") or self.app_msg_change):
            # self.pix1 = QPixmap(self.app_msg.screenshots[i])
            # self.screen_shot1.setPixmap(self.pix1)
            # self.screen_shot1.setScaledContents(True)
            if(self.app_msg.screenshots != None):
                photo_list = []
                photo_list.extend(self.app_msg.screenshots)
                photos = label.children()
                # for get_photo in photos:
                #     sip.delete(get_photo)
                if(num < len(photo_list)):
                    if(len(photos) != 0):
                        photo = photos[0]
                        photo.resize(label.width(),label.height())
                        photo.re_move()
                    else:
                        photo = Photo(photo_list[num], label.width(),
                                      label.height(),
                                      label)
                    photo.shot_photo_change(photo_list[num])
                    # photo.resize_image()
                    photo.show()

    #
    #
    #
    def create_screenshot(self,card_width,card_height,num):
        app_screen_shot = Screenshot(card_width,card_height,num,self,self.screen_card_list_widget)
        app_screen_shot.setFocusPolicy(Qt.NoFocus)
        self.app_screenshot_widget_list.append(app_screen_shot)
        return  app_screen_shot

    #
    # 函数：显示截图
    #
    def show_screen_shot(self):
        # self.hide_screen_shot()
        self.screen_h_num = int((self.width() - 50 * 2) / self.card_width)
        card_width = int((self.width() - 50 * 2 - (self.screen_h_num - 1) * 20) / self.screen_h_num)
        card_height = int((card_width / self.card_width) * self.card_height)
        self.reload_screen(card_width, card_height, (card_width / self.card_width))
        screen_count = len(self.app_msg.screenshots)
        self.screen_card_list_widget.move(0,0)
        self.screen_card_list_widget.resize(0,self.screen_card_list_widget.height())
        self.screen_right_btn.setEnabled(True)
        if (Globals.THEME_COLOR == 0):
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
        else:
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
        self.screen_left_btn.setEnabled(False)
        if (Globals.THEME_COLOR == 0):
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
        else:
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
        if (screen_count == 0):
            self.hide_screen_shot()
            self.move_and_resize()
        else:
            self.get_screen_num = 0
            for i in range(screen_count):
                # if (i == 0):
                self.get_screen_num = self.get_screen_num + 1
                if(len(self.app_screenshot_widget_list) < i+1):
                    app_screen_shot = self.create_screenshot(card_width,card_height,self.get_screen_num)
                else:
                    app_screen_shot = self.app_screenshot_widget_list[i]
                self.set_screenshot_signal.emit(app_screen_shot.screen_shot,i)
                # self.set_screenshot_signal.emit(app_screen_shot.screen_shot,i)
                if (self.get_screen_num > self.screen_h_num):
                    app_screen_shot.hide()
                    # self.move_and_resize()
                    app_screen_shot.screen_outer_frame.hide()
                    app_screen_shot.screen_shot.hide()
                    app_screen_shot.surface_color.hide()
                    QApplication.processEvents()
                    continue
                if (self.screen_h_num >= 1):
                    if(app_screen_shot.isHidden() == False):
                        app_screen_shot.show()
                        if(i == 0):
                            self.move_and_resize()
                        else:
                            self.resize_screen_shot()
                        # self.move_and_resize()
                        app_screen_shot.screen_outer_frame.show()
                        app_screen_shot.screen_shot.show()
                        app_screen_shot.surface_color.show()
                        QApplication.processEvents()
                    else:
                        app_screen_shot.show()
                        if (i == 0):
                            self.move_and_resize()
                        else:
                            self.resize_screen_shot()
                        # self.resize_screen_shot()
                        # self.move_and_resize()
                        app_screen_shot.screen_outer_frame.show()
                        app_screen_shot.screen_shot.show()
                        app_screen_shot.surface_color.show()
                    time.sleep(0.1)
            self.app_msg_change = False
            num = 0
            if(len(self.app_screenshot_widget_list) < self.screen_h_num):
                for i in range(self.screen_h_num-len(self.app_screenshot_widget_list)):
                    app_screen_shot = self.create_screenshot(card_width, card_height,len(self.app_screenshot_widget_list)+i)
            # for i in self.app_screenshot_widget_list:
            #     if(self.screen_h_num > get_screen_num and num >= get_screen_num and num < self.screen_h_num):
            #         i.show()
            #         i.screen_outer_frame.hide()
            #         i.screen_shot.hide()
            #         i.surface_color.hide()
            #         i.defualt_photo.show()
            #     if(num > self.screen_h_num):
            #         i.hide()
            #     num = num + 1
            for i in range(self.get_screen_num):
                # self.app_screenshot_widget_list[i].clicked.connect(lambda: self.show_screen_detail(i + 1))
                if(len(self.app_screenshot_widget_list) > i):
                   self.app_screenshot_widget_list[i].signal_connect()

            self.show_sheet_shot.shot_pic_change(self.app_msg.screenshots)

    #
    # 函数：展示截图的二级滑动查看界面
    #
    def show_screen_detail(self, picture_order):
        self.show_shot_detail.raise_()
        self.show_shot_detail.show()

        self.show_sheet_shot.shot_pic_click(picture_order)
        self.show_sheet_shot.resize(self.width(), self.height())
        self.show_sheet_shot.move(0, 0)
        self.show_sheet_shot.show()

    #
    # 函数：发送获取截图
    #
    def get_screen_shot(self, down_url, id, appname, update_time):
        self.get_screen_shot_signal.emit(down_url, id, appname, update_time)

    def slot_to_register(self):
        self.btn_login_and_register_register.setStyleSheet(
            "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
        url = Globals.REGISTER_ACCOUNT
        webbrowser_open_url(url)
        # webbrowser.open(url)

    #
    # 函数：获取截图结束后查看是否有截图可以显示
    #
    def check_screen_shot_file(self, appname, update_time):
        self.app_msg.screenshots.clear()
        if (appname == self.app_msg.pkgname):
            scre = self.app_msg.pkgname + "_thumbnail1_" + update_time
            filename = scre
            try:
                for file in os.listdir(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH):
                    if (scre in file):
                        filename = file
                        break
                set = os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + filename)
                file_list = filename.split(".")
                file_type = file_list[len(file_list) - 1]
                if set:
                    if ((UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + scre) not in self.app_msg.screenshots):
                        self.app_msg.screenshots.append(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + filename)
                    i = 2
                    while (1):
                        file_name = self.app_msg.pkgname + "_thumbnail" + str(i)
                        for file in os.listdir(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH):
                            if (file_name in file):
                                file_name = file
                                break
                        if (os.path.exists(UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name)):
                            if ((
                                    UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name) not in self.app_msg.screenshots):
                                self.app_msg.screenshots.append(
                                    UBUNTUKYLIN_CACHE_SETSCREENSHOTS_PATH + file_name)
                        else:
                            break
                        i = i + 1
            except:
                pass
            self.slot_get_screen_shot_over()

    #
    # 函数：处理获取截图结束
    #
    def slot_get_screen_shot_over(self):
        self.screen_loading.hide()
        self.show_screen_shot()
        self.move_and_resize()

    def check_review_words(self,text):
        has_sensitive_words = False
        if(os.path.exists("/usr/share/kylin-software-center/sensitive_words.txt")):
            fd = open("/usr/share/kylin-software-center/sensitive_words.txt","r")
            for i in fd.readlines():
                if(i.strip("\n").strip(" ") != "" and i.strip("\n") in text):
                    has_sensitive_words = True
                    break
        return has_sensitive_words

    #
    # 函数：发送我的评论
    #
    def submit_my_review(self):
        if (self.btn.text() == _("Open") or self.btn.text() == _("installed") or self.btn.text() == _("update")):
            if self.count == 30:
                self.timer_go.start()
                self.first_submit = True
                if (self.app_msg.from_ukscdb):
                    if (self.m_parent.mw.comment_pop_up.comment_edit_label.toPlainText().strip() != ""):
                        if(self.ratings_widget.MY_SCORE != 0):
                            te = self.m_parent.mw.comment_pop_up.comment_edit_label.toPlainText()
                            tet = te.replace("\n", " ")
                            # self.submit_btn.setEnabled(False)
                            has_sensitive_words = self.check_review_words(tet)
                            if(has_sensitive_words == True):
                                self.timer_go.stop()
                                self.count = 30
                                self.m_parent.mw.comment_pop_up.tip_label.setText(_("This comment contains sensitive words!"))
                                self.m_parent.mw.comment_pop_up.tip_label.adjustSize()
                                self.m_parent.mw.comment_pop_up.tip_label.show()
                            else:
                                if(type(self.app_msg) == Application):
                                    # self.submit_my_review_to_server.emit(self.app_msg.app_id, self.app_msg.pkgname, tet,self.app_msg.version)
                                    self.submit_my_review_to_server.emit(self.app_msg,tet)
                                else:
                                    self.submit_my_review_to_server_apk.emit(self.app_msg,tet)
                        else:
                            self.count = 0
                            self.m_parent.mw.comment_pop_up.tip_label.setText(_("Please rate before commenting"))
                            self.m_parent.mw.comment_pop_up.tip_label.show()
                        # self.editorial_comments_label.clear()
                    else:
                        self.count = 0
                        self.m_parent.mw.comment_pop_up.submit_btn.setEnabled(True)
                        self.m_parent.mw.show_tip_widget(_("Comment cannot be empty!"), True)
                        self.m_parent.mw.tip_widget.btn.clicked.connect(self.m_parent.mw.slot_close_tipwidget)
                else:
                    self.count = 0
                    self.m_parent.mw.comment_pop_up.submit_btn.setEnabled(True)
                    self.m_parent.mw.show_tip_widget(
                        _("Non database software, unable to score the software for the time being!"), True)
                    self.m_parent.mw.tip_widget.btn.clicked.connect(self.m_parent.mw.slot_close_tipwidget)
            else:

                self.first_submit = False
                self.submittime = False
        else:
            self.editorial_comments_label.clear()
            self.m_parent.mw.show_tip_widget(_("Please install the software before making comments!"),True)
            self.m_parent.mw.tip_widget.btn.clicked.connect(self.m_parent.mw.slot_close_tipwidget)


    #
    # 函数：处理发送我的评论结束后显示新的评论界面
    #
    def slot_submit_my_review_over(self, appname):
        if (appname == self.app_msg.pkgname):
            if self.result_review['code'] == 200:
                if(hasattr(self.m_parent.mw,"comment_pop_up")):
                    self.m_parent.mw.comment_pop_up.submit_btn.setEnabled(True)
                # self.editorial_comments_label.show()
                # self.Login_status_check()
                self.btn_login_and_register.hide()
                self.btn_login_and_register_line.hide()
                self.btn_login_and_register_register.hide()
                self.editorial_comments_label.setEnabled(True)
                # self.editorial_comments_widget.setStyleSheet("QFrame{border-radius: 12px;border: 2px solid #C0C4CC ;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
                self.is_first_time_reload_review = True
                self.reviewpage = 1
                self.reviews_num.hide()
                self.all_review_label.hide()
                self.loadinglabel.show()
                self.all_review_widget.hide()
                self.editorial_review_widget.hide()
                self.ratings_widget.btn_all_review.hide()
                self.reviewlist1.clear()
                if(self.app_msg.pkgname != Globals.KYDROID_PACKAGE_NAME):
                    if(type(self.app_msg) == Application):
                        self.get_app_reviews.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)
                    else:
                        self.get_app_reviews_apk.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)
            else:
                pass

    #
    # 函数：loading_label和按钮btn_all_review不能同时出现，分别在三个地方进行了调用：第一次点击，第n次点击卡片，初始化评论框
    #
    def deal_loading_and_btn_review(self):
        if self.loadinglabel.isHidden():
            if (self.num == 0):
                self.ratings_widget.btn_all_review.hide()
            elif (0 < self.num <= 2):
                self.ratings_widget.btn_all_review.hide()
            else:
                self.ratings_widget.btn_all_review.show()
        if not self.all_review_widget.isHidden():
            self.ratings_widget.btn_all_review.hide()

    #
    # 函数：初始化评论框，1.加载评论，展示评论框。2.界面相关设置
    #
    def get_reviewlist_to_init_review_ui(self, total, reviewlist, pkgname):
        if pkgname != self.app_msg.pkgname:
            return
        for i in reviewlist:
            self.reviewlist1.append(i)
        self.num = total
        self.loadinglabel.hide()
        if self.num == 0:
            self.ratings_widget.btn_all_review.hide()
            self.editorial_review_widget.setFixedHeight(0)
            self.btn_close_all_review.hide()
            # self.btn_retract.hide()
            self.all_review_widget.hide()
            self.move_and_resize()
            self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                               self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 30)
        else:
            # if len(self.reviewlist1) <= 10 and self.m ==1:
            if len(self.reviewlist1) <= 10 and self.is_first_time_reload_review == True:
                self.is_first_time_reload_review = False
                if self.all_review_state == False:

                    # self.reviews_num.setText(_("共"+str(len(self.reviewlist1))+"条评论"))
                    self.clearwidget(self.editorial_review_widget)
                    self.reviews_num.setText(_("There are ") + str(self.num) + _(" reviews"))
                    if self.local_lan == 'zh_CN':
                        self.reviews_num.setFixedSize(len(self.reviews_num.text()) * 14, 20)
                    else:
                        self.reviews_num.setFixedSize(len(self.reviews_num.text()) * 8, 20)
                    self.editorial_review_widget.hide()
                    self.editorial_review_widget.setFixedHeight(self.commend_height)
                    Globals.ADD_WIDGET.clear()
                    Globals.ALL_WIDGET_COMMENT.clear()
                    Globals.ALL_WIDGET_USERNAME.clear()
                    Globals.ALL_WIDGET.clear()
                    Globals.ADD_WIDGET_COMMENT.clear()
                    Globals.ADD_WIDGET_USERNAME.clear()
                    self.review_widget = Ahead_Review_Widget(self.reviewlist1, self, self.editorial_review_widget)
                    self.editorial_review_widget.show()
                    self.review_widget.show()

                    # self.clearwidget(self.all_review_widget)
                    self.all_review_widget.close()
                    self.all_review_widget = QFrame(self.show_scrollArea_widget)
                    # self.all_review_widget.setStyleSheet("QWidget{background:red;}")
                    self.all_review = All_Review_Widget(self.reviewlist1, self, self.all_review_widget)
                    self.all_review.set_show_review_height.connect(self.set_show_review_height)

                    self.all_review_widget.hide()
                    self.move_and_resize()
                else:
                    # self.clearwidget(self.all_review_widget)
                    self.reviews_num.setText(_("There are ") + str(self.num) + _(" reviews"))
                    self.reviews_num.setFixedSize(len(self.reviews_num.text()) * 14, 20)
                    self.reviews_num.show()
                    self.all_review_label.show()
                    self.editorial_review_widget.hide()
                    self.editorial_review_widget.setFixedHeight(self.commend_height)
                    self.review_widget = Ahead_Review_Widget(self.reviewlist1, self, self.editorial_review_widget)
                    self.editorial_review_widget.hide()
                    self.review_widget.hide()

                    # self.clearwidget(self.all_review_widget)
                    # self.all_review_widget.close()
                    self.all_review_widget = QFrame(self.show_scrollArea_widget)
                    # self.all_review_widget.setStyleSheet("QWidget{background:red;}")
                    self.all_review = All_Review_Widget(self.reviewlist1, self, self.all_review_widget)
                    self.all_review.set_show_review_height.connect(self.set_show_review_height)

                    self.all_review_widget.show()
                    self.move_and_resize()

            else:
                if (not hasattr(self, "all_review")):
                    self.all_review = All_Review_Widget(self.reviewlist1, self, self.all_review_widget)
                    self.all_review.set_show_review_height.connect(self.set_show_review_height)
                    self.all_review_widget.hide()
                # self.move_and_resize()
                self.all_review.get_review(self.reviewlist1)
            self.reviewpage += 1
        if not self.editorial_review_widget.isHidden():
            self.loadinglabel.hide()
            self.editorial_review_widget.show()
        Globals.IS_GETTING_REVIEW = False
        self.deal_loading_and_btn_review()

    def clearwidget(self, reviw_widget):
        widgets = reviw_widget.children()
        for wid in widgets:
            sip.delete(wid)

    #
    #函数：处理登录成功后详情界面需要修改的控件
    #
    def Login_status_check(self):
        # (^-^)
        if Globals.LOGIN_SUCCESS == False:
            # if self.Login_status == True:
            self.back_text.hide()
            self.submit_btn.hide()
            # self.editorial_comments_label.setPlaceholderText(" ")
            self.editorial_comments_label.clear()
            self.btn_login_and_register.show()
            self.btn_login_and_register_register.show()
            self.btn_login_and_register_line.show()
            self.editorial_comments_label.setEnabled(False)
            self.timer_go.stop()
            self.time_tip.hide()
            self.tip.hide()
            if (Globals.THEME_COLOR == 0):
                self.editorial_comments_widget.setStyleSheet(
                    "QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #C0C4CC;}")
            else:
                self.editorial_comments_widget.setStyleSheet(
                    "QFrame{border-radius: 12px;border: 2px solid #3D3D41;}QFrame:hover{border-radius: 12px;border: 2px solid #3D3D41;}")
            if(hasattr(self,"review_widget")):
                for i in self.review_widget.review_widget1.children():
                    i.check_account()
            if (hasattr(self, "all_review")):
                for i in self.all_review.widget.children():
                    i.check_account()

            # self.btn_login_and_register.setEnabled(True)
        else:
            # self.submit_btn.show()
            # self.editorial_comments_label.setPlaceholderText(_("Please enter your comments"))
            self.back_text.show()
            self.editorial_comments_label.hide()
            self.btn_login_and_register.hide()
            self.btn_login_and_register_line.hide()
            self.btn_login_and_register_register.hide()
            self.editorial_comments_label.setEnabled(True)
            if (Globals.THEME_COLOR == 0):
                self.editorial_comments_widget.setStyleSheet(
                    "QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
            else:
                self.editorial_comments_widget.setStyleSheet(
                    "QFrame{border-radius: 12px;border: 2px solid #3D3D41;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
            if (hasattr(self, "review_widget")):
                for i in self.review_widget.review_widget1.children():
                    i.check_account()
            if (hasattr(self, "all_review")):
                for i in self.all_review.widget.children():
                    i.check_account()

    # 在未登录状态时，点击 登录 | 注册 按钮时，弹出登录界面
    def slot_login_in(self):
        self.m_parent.mw.slot_login_account()

    def set_stylesheet(self):
        # self.m = 0
        if (self.app_msg.pkgname != self.appname_label.text() and self.app_msg.displayname_cn != self.appname_label.text()):
            Globals.IS_GETTING_REVIEW = False
        if (Globals.LOGIN_SUCCESS):
            self.back_text.show()
        else:
            self.back_text.hide()
        self.app_categories1.hide()
        self.app_categories2.hide()
        self.app_categories3.hide()
        self.is_first_time_reload_review = True
        self.ratings_widget.app_name = self.app_msg.pkgname
        self.ratings_widget.refresh_star()
        self.ratings_widget.change_star_state()
        Globals.GO_SHORT_REVIEWS = False
        self.reviewpage = 1
        iconpath = commontools.get_big_icon_path(self.app_msg.pkgname)
        # if(iconpath.split(".")[1] == "svg"):
        #     self.icon_label.setStyleSheet(
        #         "QLabel{border-radius: 0px;border-image: url('" + iconpath + "') 0 0 0 0 stretch strectch;background-color:transparent;border: 1px solid read;}")
        #     renderer = QSvgRenderer(iconpath)
        #     self.icon_label.resize(renderer.defaultSize())
        #     painter = QPainter(self.icon_label)
        #     painter.restore()
        #     renderer.render(painter)
        #     self.icon_label.show()
        # else:
        self.photo.shot_photo_change(iconpath)
        # photo.resize_image()
        self.photo.show()
        self.icon_label.setStyleSheet("QLabel{border-radius: 8px;background-color:transparent;}")
        # self.icon_label.setStyleSheet("QLabel{border-radius: 0px;border-image: url('" + iconpath + "') 0 0 0 0 stretch strectch;background-color:transparent;border: 1px solid read;}")
        # self.icon_label.setStyleSheet("QLabel{border-radius: 0px;border-image: url(./res/222.png) 0 0 0 0 stretch strectch;background-color:transparent;}")

        # self.kmre_platform_introduction_label.setMaximumWidth(self.width() - 50 * 2)
        self.kmre_platform_introduction_label.setFixedWidth(self.width() - 50 * 2)
        self.kmre_platform_introduction_label.setText(self.app_msg.summary_init)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_platform_introduction_label.setWordWrap(True)

        self.kmre_technical_features_label.setMaximumWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setFixedWidth(self.width() - 50 * 2)
        self.kmre_technical_features_label.setText(self.app_msg.description)
        # self.kmre_platform_introduction_label.adjustSize()
        self.kmre_technical_features_label.setWordWrap(True)
        if len(self.app_msg.screenshots) != '':
            self.show_sheet_shot.shot_pic_change(self.app_msg.screenshots)
        if (Globals.LAN == "zh_CN"):
            if self.app_msg.displayname_cn != '' and self.app_msg.displayname_cn is not None and self.app_msg.displayname_cn != 'None':
                self.appname_label.setText(self.app_msg.displayname_cn)
                self.appname_label.adjustSize()
            else:
                self.appname_label.setText(self.app_msg.pkgname)
                self.appname_label.adjustSize()
        else:
            self.appname_label.setText(self.app_msg.pkgname)
            self.appname_label.adjustSize()
        self.app_categories1.hide()
        self.app_categories2.hide()
        self.app_categories3.hide()
        #处理无截图时，左右滑动按钮处理
        self.screen_card_list_widget.resize(0, self.screen_card_list_widget.height())
        self.screen_right_btn.setEnabled(True)
        if (Globals.THEME_COLOR == 0):
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
        else:
            self.screen_right_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
        self.screen_left_btn.setEnabled(False)
        if (Globals.THEME_COLOR == 0):
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
        else:
            self.screen_left_btn.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
        # if(self.app_msg.softStyle != 2):
        if (self.app_msg.category_name_cn != "" and self.app_msg.category_name_cn != "None" and self.app_msg.category_name_cn != "None" and Globals.LAN == "zh_CN"):
            category_list = self.app_msg.category_name_cn.split("、")
            for i in range(len(category_list)):
                if (i == 0):
                    if (category_list[0] != ""):
                        self.app_categories1.setText(category_list[0])
                        self.change_category_width(self.app_categories1)
                        self.app_categories1.show()
                if (i == 1):
                    if (category_list[1] != ""):
                        self.app_categories2.setText(category_list[1])
                        self.change_category_width(self.app_categories2)
                        self.app_categories2.move(self.app_categories1.x() + self.app_categories1.width() + 4,
                                                  self.app_categories1.y())
                        self.app_categories2.show()
                if (i == 2):
                    if (category_list[2] != ""):
                        self.app_categories3.setText(category_list[2])
                        self.change_category_width(self.app_categories3)
                        self.app_categories3.move(self.app_categories2.x() + self.app_categories2.width() + 4,
                                                  self.app_categories1.y())
                        self.app_categories3.show()
                if (i >= 3):
                    break
        else:
            category_list = self.app_msg.category_name.split("、")
            for i in range(len(category_list)):
                if (i == 0):
                    if (category_list[0] != ""):
                        self.app_categories1.setText(category_list[0])
                        self.change_category_width(self.app_categories1)
                        self.app_categories1.show()
                if (i == 1):
                    if (category_list[1] != ""):
                        self.app_categories2.setText(category_list[1])
                        self.change_category_width(self.app_categories2)
                        self.app_categories2.move(self.app_categories1.x() + self.app_categories1.width() + 4,
                                                  self.app_categories1.y())
                        self.app_categories2.show()
                if (i == 2):
                    if (category_list[2] != ""):
                        self.app_categories3.setText(category_list[2])
                        self.change_category_width(self.app_categories3)
                        self.app_categories3.move(self.app_categories2.x() + self.app_categories2.width() + 4,
                                                  self.app_categories1.y())
                        self.app_categories3.show()
                if (i >= 3):
                    break
        # else:
        #     self.app_categories1.setText(self.app_msg.manufacturers)
        #     # self.app_categories1.resize(self.app_categories1.width()+20,self.app_categories1.height())
        #     self.change_category_width(self.app_categories1)
        #     self.app_categories1.show()
        #     self.app_categories2.hide()
        #     self.app_categories3.hide()
        #     # self.app_categories.setText(self.app_msg.category_name)
        count = self.app_msg.downloadcount
        if(int(count)/1000 <= 0):
            count = "<1k"
        else:
            if (int(int(count) / 1000) <= 0):
                count = "<1k"
            else:
                if (int(int(count) / 1000) >= 1 and int(int(count) / 1000) < 3):
                    count = "1k+"
                elif (int(int(count) / 1000) >= 3 and int(int(count) / 1000) < 5):
                    count = "3k+"
                elif (int(int(count) / 1000) >= 5 and int(int(count) / 1000) < 10):
                    count = "5k+"
                elif (int(int(count) / 1000) >= 10 and int(int(count) / 1000) < 11):
                    count = "10k+"
                elif (int(int(count) / 1000) >= 1000):
                    count = str(int(int(count) / 1000*1000)) + "M+"
                else:
                    count = str(int(int(count) / 1000)) + "k+"
        # if (int(count) < 1000):
        #     count = "<1k"
        # elif (int(count) >= 1000 and int(count) < 3000):
        #     count = "1k+"
        # elif (int(count) >= 3000 and int(count) < 5000):
        #     count = "3k+"
        # elif (int(count) >= 5000 and int(count) < 10000):
        #     count = "5k+"
        # else:
        #     count = "10k+"
        self.download_num_label.setText(_("amount of download:") + str(count))
        self.download_num_label.adjustSize()
        self.open_wibside_label.setText(_("Times of opening the official website:")+_("Getting"))
        self.open_wibside_label.adjustSize()
        self.download_size_label.setText(_("package size:") + self.get_size())
        if(self.app_msg.popularity != None):
            self.popularity_label.setText(_("popularity:") + str(self.app_msg.popularity))
        else:
            self.popularity_label.setText(_("popularity:") + "0")
        self.version_label.setText(_("version:") + self.app_msg.version)
        if(self.app_msg.screenshots_updatetime == "" or self.app_msg.screenshots_updatetime == None):
            self.update_time_label.setText(_("update time:getting"))
        else:
            self.update_time_label.setText(_("update time:") + self.app_msg.screenshots_updatetime.split(" ")[0].split("T")[0])
        if(self.app_msg.manufacturers == "" or self.app_msg.manufacturers == None):
            self.manufacturers_label.hide()
        else:
            self.manufacturers_label.setText(_("manufacturers:") + self.app_msg.manufacturers)
            self.setLongTextToElideFormat1(self.manufacturers_label,_("manufacturers:") , self.app_msg.manufacturers)
            self.manufacturers_label.adjustSize()
            # self.manufacturers_label.setText(_("manufacturers:") + self.app_msg.manufacturers)
        if(self.app_msg.website == "" or self.app_msg.website == None):
            self.website_label.hide()
        else:
            self.setLongTextToElideFormat1(self.website_label,_("website:") , self.app_msg.website)
            self.website_label.adjustSize()
            self.website_label.show()
            # self.manufacturers_label.setText(_("manufacturers:") + self.app_msg.manufacturers)
        app_name_text = _("package: ")
        # self.app_name.setText(app_name_text + self.app_msg.pkgname)
        self.app_name.setMaximumWidth(209)
        setLongTextToElideFormat(self.app_name, app_name_text + self.app_msg.pkgname)
        # self.app_name.setText("dgfjksdbfiusdfguseyfgsudifbsudfgsudfgsuydfgsudf")
        if ("…" in self.app_name.text()):
            self.app_name.setToolTip(self.app_msg.pkgname)
        else:
            self.app_name.setToolTip("")

        # self.setLongTextToElideFormat1(self.app_name,self.app_msg.pkgname)
        # self.setLongTextToElideFormat1(self.app_name,app_name_text, self.app_msg.pkgname)
        self.download_size_label.adjustSize()

        # self.app_sign.setText(_("Software signature"))
        # self.app_sign.adjustSize()
        # self.manual_detection.setText(_("Manual detection"))
        # self.manual_detection.adjustSize()

        self.app_sign_bottom.setText(_("Software signature"))
        self.app_sign_bottom.adjustSize()
        self.manual_detection_bottom.setText(_("Manual detection"))
        self.manual_detection_bottom.adjustSize()
        self.signature_unit_label.setText(_("Signature unit:"))
        self.signature_unit_label.adjustSize()

        self.version_label.adjustSize()
        self.update_time_label.adjustSize()
        self.manufacturers_label.adjustSize()
        self.app_name.adjustSize()
        # if(str(self.app_msg.softStyle) == "2"):
        #     self.manufacturer_label.show()
        #     self.manufacturer_name_label.setText(self.app_msg.manufacturers)
        #     self.manufacturer_name_label.adjustSize()
        #     self.manufacturer_name_label.show()
        # else:
        self.manufacturer_label.hide()
        self.manufacturer_name_label.hide()
        if(Globals.LAN == "zh_CN"):
            if (self.app_msg.summary_init != None  and str(self.app_msg.summary_init) != "None"):
                if( self.app_msg.description != None):
                    self.setLongTextToElideFormat(self.app_discription_label,
                                                  self.app_msg.description,str(self.app_msg.summary_init))
                else:
                    self.setLongTextToElideFormat(self.app_discription_label,self.app_msg.pkgname)
            else:
                if(self.app_msg.description != None):
                    self.setLongTextToElideFormat(self.app_discription_label, self.app_msg.description)
                else:
                    self.setLongTextToElideFormat(self.app_discription_label, self.app_msg.pkgname)
        else:
            try:
                package = self.ca[self.app_msg.pkgname]
            except:
                package = None
            if(package != None and package.candidate != None):
                if(package.candidate.description != "" and package.candidate.description != "\n"):
                    self.setLongTextToElideFormat(self.app_discription_label, package.candidate.description)
                else:
                    self.setLongTextToElideFormat(self.app_discription_label, package.name)
            else:
                self.setLongTextToElideFormat(self.app_discription_label, self.app_msg.pkgname)

        self.app_discription_label.setFixedHeight(130)

        # self.app_discription_label.show()
        # self.app_discription_label.setFixedHeight(100)
        self.app_discription_label.show()
        if(self.app_msg.from_ukscdb == True):
            self.download_num_label.show()
        else:
            self.download_num_label.hide()
        self.download_size_label.show()
        self.version_label.show()
        self.update_time_label.show()
        if(self.app_msg.manufacturers != None and self.app_msg.manufacturers != ""):
            self.manufacturers_label.show()
        else:
            self.manufacturers_label.hide()
        self.show_shot_detail.hide()

        self.review_text = ''
        self.count = 30
        self.timer_go.stop()
        self.submit_btn.setEnabled(True)

        self.app_name.show()
        if ("…" not in self.app_discription_label.toPlainText()):
            self.app_discription_label.setFixedHeight(115)
            self.show_more_text.hide()
        else:
            self.show_more_btn()
        self.line_label.show()
        self.software_rating_label.setText(_("Scoring and comments"))
        self.software_rating_label.adjustSize()
        self.software_rating_label.show()
        if(self.app_msg.ratings_total != "" and self.app_msg.ratings_total != None):
            self.ratings_widget.count_of_participation_rating.setText(str(self.app_msg.ratings_total) + _("people attend"))
        else:
            self.ratings_widget.count_of_participation_rating.setText("0" + _("people attend"))
        self.ratings_widget.count_of_participation_rating.adjustSize()
        self.ratings_widget.count_of_participation_rating.show()
        self.full_score_label.setAlignment(Qt.AlignCenter)
        self.full_score_label.setText(_("The full score is 5"))
        self.full_score_label.adjustSize()
        self.editorial_comments_widget.resize(self.show_scrollArea_widget.width() - self.software_rating_label.x() * 2,
                                              139)
        self.editorial_comments_widget.hide()
        self.editorial_comments_label.resize(self.editorial_comments_widget.width() - 32, 60)
        self.editorial_comments_label.hide()
        self.all_review_label.hide()
        self.editorial_comments_label.clear()
        self.time_tip.hide()
        self.tip.hide()
        # self.btn_all_review.hide()
        self.submit_btn.hide()

        # self.download_num_label.setText(_("amount of download:Getting"))
        # self.download_num_label.adjustSize()

        # self.app_sign.setText(_("Software signature：Getting"))
        # self.app_sign.adjustSize()
        # self.manual_detection.setText(_("Manual detection：Getting"))
        # self.manual_detection.adjustSize()

        self.app_sign_bottom.setText(_("Software signature") + " " + _("Getting"))
        self.app_sign_bottom.adjustSize()
        self.manual_detection_bottom.setText(_("Manual detection") + " " + _("Getting"))
        self.manual_detection_bottom.adjustSize()
        self.signature_unit_label.setText(_("Signature unit:") + _("Getting"))
        self.signature_unit_label.adjustSize()
        self.signature_unit_label.show()

        self.score_label.hide()
        self.score_label.setText(str(round(0.0, 1)))
        self.score_label.adjustSize()
        self.score_label.show()
        self.score_label_small.hide()
        self.score_label_small.setText(str(round(0.0, 1)))
        self.score_label_small.adjustSize()
        self.score_label_small.show()
        self.small_starwidget.hide()
        self.small_starwidget.changeGrade(0)
        self.small_starwidget.show()
        self.average_score_star.changeGrade(0)
        self.ratings_widget.clear_progress()
        self.btn_retract.hide()
        if(Globals.NEED_SHOW_KYLINID and self.app_msg.is_notice == False):
            if (Globals.LOGIN_SUCCESS):
                self.get_my_rating.emit(str(self.app_msg.app_id), self.app_msg.pkgname)
            self.need_get_download_count.emit(str(self.app_msg.app_id), self.app_msg.pkgname,str(self.app_msg.softStyle))
            # zzz
            # self.need_get_app_sign_state.emit(self.app_msg.app_id, self.app_msg.pkgname)
            # zzz
            self.need_get_new_rating.emit(str(self.app_msg.app_id), self.app_msg.pkgname,str(self.app_msg.softStyle))
        self.change_btn_state(self.app_msg.pkgname, self.app_msg.status)

        self.move_and_resize()
        self.show_scrollArea_widget.move(0, 0)
        self.hide_screen_shot()
        self.screen_loading.hide()
        self.slot_app_sign_state()
        if (self.app_msg.pkgname != Globals.KYDROID_PACKAGE_NAME):
            if(self.app_msg.init_screen_shot == False):
                self.get_screen_shot_msg_signal.emit(str(self.app_msg.app_id), self.app_msg.pkgname)
            else:
                mag = {"appId":self.app_msg.app_id,"updateTime": self.app_msg.screenshots_updatetime,"previewImage": self.app_msg.screenshots_url}
                self.get_screen_shot_msg(mag)
            # item = "get_screen_shot"
            # self.wt.mutex.acquire()
            # self.wt.work_list.append(item)
            # self.wt.mutex.release()

        # self.earn_crenshoots()
        if(Globals.NEED_SHOW_KYLINID):
            self.slot_change_shortreviewWindow()
            self.deal_loading_and_btn_review()
            self.loadinglabel.show()
        self.all_review_widget.hide()
        self.editorial_review_widget.hide()
        self.ratings_widget.btn_all_review.hide()
        self.btn_login_and_register_register.adjustSize()
        self.btn_login_and_register_line.adjustSize()
        self.btn_login_and_register.adjustSize()
        # self.reviewlist1.clear()
        # Globals.ALL_REVIEWLIST.clear()
        self.reviewlist1.clear()
        self.Login_status_check()
        # 将当前的app名字发送出去，用来获取当前app的评论内容
        # self.m_parent.mw.at.appmgr.get_application_reviews(self.app_msg.pkgname)
        if (self.app_msg.pkgname != Globals.KYDROID_PACKAGE_NAME and Globals.NEED_SHOW_KYLINID):
            if (type(self.app_msg) == Application):
                self.get_app_reviews.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)
            else:
                self.get_app_reviews_apk.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)

        if(self.app_msg.is_notice == True):
            self.get_previewnum.emit(str(self.app_msg.app_id))
        Globals.PKGNAME = self.app_msg.pkgname
        # Globals.ALL_REVIEWLIST.clear()
        # self.show_kmre_detail()
        if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
            self.down_progress.btn.move(int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                        int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
            self.cancel_btn.move(self.kmre_photo_label.x()+int((self.kmre_photo_label.width() - (self.down_progress.width() + self.cancel_btn.width() + 9))/2),
                                 self.kmre_btn.y() + int((self.kmre_btn.height() - self.cancel_btn.height())/2 ))
            self.down_progress.move(self.cancel_btn.x()+self.cancel_btn.width()+9,self.cancel_btn.y())

        else:
            self.down_progress.btn.move(int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                        int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
            self.down_progress.move(int(self.btn.x() + self.btn.width() - self.down_progress.width()),
                                    int(self.btn.y() + (int(self.btn.height() - self.down_progress.height()) / 2)))
            self.cancel_btn.move(int(self.down_progress.x() - self.cancel_btn.width() - 8),
                                 int(self.down_progress.y() - int((self.cancel_btn.height() - self.down_progress.height()) / 2)))

    def resize_screen_shot(self):
        self.download_num_label.move(int(self.icon_label.x() + self.icon_label.width() + 21),
                                     int(self.score_label_small.y() + self.score_label_small.height() + 8))
        for i in range(len(self.app_screenshot_widget_list)):
            if(i == 0):
                self.app_screen_shot_list_widget.move(10, int(self.download_num_label.y() + self.download_num_label.height() + 40))
                self.app_screenshot_widget_list[i].move(0, 0)
                self.app_screen_shot_list_widget.resize(self.width() - 10 * 2, self.app_screenshot_widget_list[i].height())
                self.show_app_screen_shot_widget.resize(self.app_screen_shot_list_widget.width()-self.screen_right_btn.width()*2-10*2,self.app_screen_shot_list_widget.height())
                self.show_app_screen_shot_widget.move(int(self.screen_left_btn.x()+self.screen_left_btn.width()+10),0)
            else:
                self.app_screenshot_widget_list[i].move(int(self.app_screenshot_widget_list[i-1].x() + self.app_screenshot_widget_list[i-1].width() + 20),
                                            int(self.app_screenshot_widget_list[i-1].y()))
            self.screen_card_list_widget.resize(self.app_screenshot_widget_list[i].x() + self.app_screenshot_widget_list[i].width(),self.app_screenshot_widget_list[i].height())
        self.screen_left_btn.move(0,int((self.app_screen_shot_list_widget.height() - self.screen_left_btn.height()) / 2))
        self.screen_right_btn.move(
            self.show_scrollArea_widget.width() - self.app_screen_shot_list_widget.x() - 10 - self.screen_right_btn.width(),
            int((self.app_screen_shot_list_widget.height() - self.screen_right_btn.height()) / 2))
        if(len(self.app_screenshot_widget_list) > 0):
            self.app_screen_shot_list_widget.show()
        else:
            self.screen_card_list_widget.resize(0,self.screen_card_list_widget.height())
            self.app_screen_shot_list_widget.hide()

    #
    # 函数：控件的位置和大小重置
    #
    def move_and_resize(self):
        self.score_label_small.move(self.app_categories1.x(),
                                    self.app_categories1.y() + self.app_categories1.height() + 12)
        self.download_num_label.move(self.icon_label.x() + self.icon_label.width() + 21,
                                     self.score_label_small.y() + self.score_label_small.height() + 8)
        self.open_wibside_label.move(self.icon_label.x() + self.icon_label.width() + 21,
                                     self.score_label_small.y() + self.score_label_small.height() + 8)
        self.popularity_label.move(self.download_num_label.x(),self.download_num_label.height() + self.download_num_label.y()+10)
        self.small_starwidget.move(self.score_label_small.x() + self.score_label_small.width() + 7,
                                   self.app_categories1.y() + self.app_categories1.height() + 14)
        if (self.app_msg.from_ukscdb == True):
            self.download_size_label.move(self.download_num_label.x() + self.download_num_label.width() + 20,
                                          self.download_num_label.y())
            self.app_name.move(self.download_size_label.x() + self.download_size_label.width() + 20,
                               self.download_size_label.y())
            self.version_label.move(self.app_name.x() + self.app_name.width() + 20, self.download_size_label.y())
            self.update_time_label.move(self.version_label.x()+self.version_label.width()+20,self.download_size_label.y())
            self.manufacturers_label.move(self.update_time_label.x()+self.update_time_label.width()+20,self.download_size_label.y())
            if(self.manufacturers_label.isHidden()):
                self.website_label.move(self.update_time_label.x() + self.update_time_label.width() + 20,
                                              self.download_size_label.y())
            else:
                self.website_label.move(self.manufacturers_label.x() + self.manufacturers_label.width() + 20,
                                              self.download_size_label.y())
        else:
            self.download_size_label.move(self.download_num_label.x(),
                                          self.download_num_label.y())
            self.app_name.move(self.download_size_label.x() + self.download_size_label.width() + 20,
                               self.download_size_label.y())
            self.version_label.move(self.app_name.x() + self.app_name.width() + 20, self.download_size_label.y())
            self.update_time_label.move(self.version_label.x() + self.version_label.width() + 20,
                                        self.download_size_label.y())
            self.manufacturers_label.move(self.update_time_label.x() + self.update_time_label.width() + 20,
                                          self.download_size_label.y())
            if (self.manufacturers_label.isHidden()):
                self.website_label.move(self.update_time_label.x() + self.update_time_label.width() + 20,
                                        self.download_size_label.y())
            else:
                self.website_label.move(self.manufacturers_label.x() + self.manufacturers_label.width() + 20,
                                        self.download_size_label.y())
        # self.app_sign.move(self.version_label.x() + self.version_label.width() + 20, 124)
        # self.manual_detection.move(self.app_sign.x() + self.app_sign.width() + 20,124)
        for i in range(len(self.app_screenshot_widget_list)):
            if(i == 0):
                self.app_screen_shot_list_widget.move(10, self.download_num_label.y() + self.download_num_label.height() + 40)
                self.app_screenshot_widget_list[i].move(0, 0)
                self.app_screen_shot_list_widget.resize(self.width() - 10 * 2, self.app_screenshot_widget_list[i].height())
                self.show_app_screen_shot_widget.resize(self.app_screen_shot_list_widget.width()-self.screen_right_btn.width()*2-10*2,self.app_screen_shot_list_widget.height())
                self.show_app_screen_shot_widget.move(self.screen_left_btn.x()+self.screen_left_btn.width()+10,0)
            else:
                self.app_screenshot_widget_list[i].move(self.app_screenshot_widget_list[i-1].x() + self.app_screenshot_widget_list[i-1].width() + 20,
                                            self.app_screenshot_widget_list[i-1].y())
            self.screen_card_list_widget.resize(self.app_screenshot_widget_list[i].x() + self.app_screenshot_widget_list[i].width(),self.app_screenshot_widget_list[i].height())
        self.screen_left_btn.move(0,int((self.app_screen_shot_list_widget.height() - self.screen_left_btn.height()) / 2))
        self.screen_right_btn.move(
            self.show_scrollArea_widget.width() - self.app_screen_shot_list_widget.x() - 10 - self.screen_right_btn.width(),
            int((self.app_screen_shot_list_widget.height() - self.screen_right_btn.height()) / 2))
        if(len(self.app_screenshot_widget_list) > 0):
            self.app_screen_shot_list_widget.show()
        else:
            self.screen_card_list_widget.resize(0,self.screen_card_list_widget.height())
            self.app_screen_shot_list_widget.hide()
        # if(len(self.app_screenshot_widget_list) == 0):
        self.screen_loading.move(int((self.show_scrollArea_widget.width() - self.screen_loading.width()) / 2),
                                 self.download_num_label.y() + self.download_num_label.height() + 20 )
        # else:
        #     self.screen_loading.move(int((self.show_scrollArea_widget.width() - self.screen_loading.width()) / 2),
        #                              self.app_screenshot_widget_list[0].y() + int(
        #                                  (self.app_screenshot_widget_list[0].height() - self.screen_loading.height()) / 2))
        no_screen = True
        for i in self.app_screenshot_widget_list:
            if(i.isHidden() == False ):
                no_screen = False
        if no_screen == False:
        # if not self.app_detail_photo1.isHidden():
        #     if(str(self.app_msg.softStyle) == "2"):
        #         self.manufacturer_label.move(50,self.app_screenshot_widget_list[0].y()+self.app_screenshot_widget_list[0].height()+30)
        #         self.manufacturer_name_label.move(self.manufacturer_label.x(),self.manufacturer_label.y()+self.manufacturer_label.height()+20)
        #         self.software_introduction.move(self.manufacturer_name_label.x(),
        #                                         self.manufacturer_name_label.y() + self.manufacturer_name_label.height() + 30)
        #     else:
            self.software_introduction.move(50,self.app_screen_shot_list_widget.y()+self.app_screen_shot_list_widget.height() + 40+30)
        else:
            # self.app_detail_photo1.setFixedHeight(0)
            # if(str(self.app_msg.softStyle) == "2"):
            #     self.manufacturer_label.move(50, self.download_num_label.y() + self.download_num_label.height() + 40)
            #     self.manufacturer_name_label.move(self.manufacturer_label.x(),
            #                                       self.manufacturer_label.y() + self.manufacturer_label.height() + 20)
            #     self.software_introduction.move(self.manufacturer_name_label.x(),
            #                                     self.manufacturer_name_label.y() + self.manufacturer_name_label.height() + 30)
            # else:
            self.software_introduction.move(50, self.download_num_label.y() + self.download_num_label.height() + 40)
        self.app_discription_label.move(self.software_introduction.x(),
                                        self.software_introduction.y() + self.software_introduction.height() + 20)
        # self.show_more_text.move(self.show_scrollArea_widget.width() - self.app_discription_label.x() - self.show_more_text.width(),
        #                          self.app_discription_label.y() + self.app_discription_label.height() - self.show_more_text.height())
        self.show_more_text.move(
            self.show_scrollArea_widget.width() - self.app_discription_label.x() - self.show_more_text.width(),
            self.app_discription_label.y() + self.app_discription_label.height() - self.show_more_text.height())
        # self.app_update_state.move(self.software_introduction.x(),self.app_discription_label.y() + self.app_discription_label.height()+ 40)
        # self.app_update_content.move(self.software_introduction.x(),self.app_update_state.y() + self.app_update_state.height() + 10)
        #
        # self.app_update_version.move(self.show_scrollArea_widget.width() -self.app_update_version.width()- 50,self.app_update_state.y() + self.app_update_state.height() + 10)
        # self.app_update_date.move(self.show_scrollArea_widget.width() -self.app_update_date.width()- 50, self.app_update_version.y() +self.app_update_version.height() + 5)

        self.app_safe_detection.move(self.software_introduction.x(),
                                     self.app_discription_label.y() + self.app_discription_label.height() + 40)
        self.app_sign_bottom.move(self.app_safe_detection.x(),
                                  self.app_safe_detection.y() + self.app_safe_detection.height() + 10)
        self.manual_detection_bottom.move(self.app_sign_bottom.x() + self.app_sign_bottom.width() + 38,
                                          self.app_sign_bottom.y())
        self.signature_unit_label.move(self.manual_detection_bottom.x() + self.manual_detection_bottom.width() + 38,
                                       self.app_sign_bottom.y())
        self.line_label.move(self.icon_label.x(),
                             self.manual_detection_bottom.y() + self.manual_detection_bottom.height() + 15)
        # self.line_label.move(self.app_detail_photo1.x(),self.app_discription_label.y()+self.app_discription_label.height()+20)
        # self.btn.move(self.line_label.x() + self.line_label.width() - self.btn.width(),35)
        self.software_rating_label.move(self.line_label.x(), self.line_label.y() + self.line_label.height() + 20)
        self.full_score_label.move(self.software_rating_label.x(),
                                                self.software_rating_label.y() + self.software_rating_label.height() + 16)
        self.count_of_participation_rating.move(self.full_score_label.x()+self.full_score_label.width()+50,
                                                self.full_score_label.y())
        # self.ratings_widget.move(self.line_label.x()+self.line_label.width()-self.ratings_widget.width(),self.count_of_participation_rating.y()+3)
        self.score_label.move(self.full_score_label.x(),
                              self.full_score_label.y() + self.full_score_label.height() + 12)
        self.average_score_star.move(self.score_label.x(),self.score_label.y()+self.score_label.height()+13)
        need_move_space = int((self.width() - self.average_score_star.width() - (
                    self.ratings_widget.process1.x() + self.ratings_widget.process1.width()) - (
                                           self.ratings_widget.star5.x() - self.ratings_widget.star1.x() + self.ratings_widget.star5.width()) - 50 * 2) / 2)
        self.ratings_widget.move(
            self.average_score_star.x() + self.average_score_star.width() + need_move_space,
            self.full_score_label.y() + 3)
        self.ratings_widget.re_move(need_move_space, self.proportion)
        # if (self.full_score_label.x() + self.full_score_label.width() - self.ratings_widget.x() + 20 > 0):
        #     self.ratings_widget.resize_progress(
        #         self.full_score_label.x() + self.full_score_label.width() - self.ratings_widget.x() + 20)
        #     self.ratings_widget.move(
        #         self.full_score_label.x() + self.full_score_label.width() + 20,
        #         self.ratings_widget.y())
        # else:
        #     self.ratings_widget.reload_progress()
        #     self.ratings_widget.move(
        #         self.full_score_label.x() + self.full_score_label.width() + need_move_space + 118,
        #         self.full_score_label.y() + 3)
        self.editorial_comments_widget.move(self.software_rating_label.x(),
                                            self.full_score_label.y() + self.full_score_label.height() + 42)
        self.btn_login_and_register.move(int(self.editorial_comments_widget.width() / 2) - 49, int(
            (self.editorial_comments_widget.height() - self.btn_login_and_register.height()) / 2))

        self.btn_login_and_register_line.move(int(self.editorial_comments_widget.width() / 2) - 49 + 37,
                                              int((
                                                              self.editorial_comments_widget.height() - self.btn_login_and_register_line.height()) / 2))
        self.btn_login_and_register_register.move(int(self.editorial_comments_widget.width() / 2) - 49 + 37 + 9 + 2,
                                                  int((
                                                                  self.editorial_comments_widget.height() - self.btn_login_and_register_register.height()) / 2))

        self.editorial_comments_label.move(16, 16)
        self.editorial_comments_widget.resize(self.show_scrollArea_widget.width() - self.software_rating_label.x() * 2,
                                              139)
        self.editorial_comments_label.setFixedSize(self.editorial_comments_widget.width() - 32, 60)

        self.tip.move(16, 103)
        self.time_tip.move(16,103)
        self.submit_btn.move(self.editorial_comments_widget.width() - 16 - self.submit_btn.width(),
                             self.editorial_comments_label.y() + self.editorial_comments_label.height() + 7)

        self.editorial_comments_label.hide()

        # self.btn_all_review.move(
        #     self.editorial_comments_widget.x() + self.editorial_comments_widget.width() - self.btn_all_review.width(),
        #     self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 21)
        self.all_review_label.move(50,
                                   self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 21)
        # self.reviews_num.move(self.editorial_comments_widget.x(),self.editorial_comments_widget.y()+self.editorial_comments_widget.height() +20)
        self.reviews_num.move(
            self.editorial_comments_widget.x() + self.editorial_comments_widget.width() - self.reviews_num.width(),
            self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 25)
        # 左右滑动review父窗口
        self.editorial_review_widget.resize(self.editorial_comments_widget.width() + 31 * 2 + 20 * 2, 260)
        # self.editorial_review_widget.move( int((self.show_scrollArea_widget.width()-self.editorial_review_widget.width())/2), self.editorial_comments_widget.y()+ self.editorial_comments_widget.height()+50)
        # self.editorial_review_widget.move( int((self.show_scrollArea_widget.width()-self.editorial_review_widget.width())/2), self.editorial_comments_widget.y()+ self.editorial_comments_widget.height()+50)
        self.editorial_review_widget.move(int((self.show_scrollArea_widget.width() - self.editorial_review_widget.width()) / 2),
            self.ratings_widget.y() + self.ratings_widget.height() + 24)

        self.loadinglabel.move(self.editorial_review_widget.x() + int(
            (self.editorial_review_widget.width() - self.loadinglabel.width()) / 2),
                               self.editorial_review_widget.y() + int(
                                   (self.editorial_review_widget.height() - self.loadinglabel.height()) / 2))
        # 展开的review父窗口
        self.all_review_widget.setFixedWidth(self.editorial_comments_widget.width())
        self.all_review_widget.move(int((self.show_scrollArea_widget.width() - self.all_review_widget.width()) / 2),
                                    self.ratings_widget.y() + self.ratings_widget.height() + 24)
        self.btn_close_all_review.move(
            self.editorial_comments_widget.x() + self.editorial_comments_widget.width() - self.btn_close_all_review.width(),
            self.all_review_widget.y() + self.all_review_widget.height())
        # self.btn_retract.move(1211,621)
        # self.show_scrollArea_widget.resize(self.width()-2*56,self.all_review_widget.y() + self.all_review_widget.height() + 10)
        if(Globals.NEED_SHOW_KYLINID):
            if(self.app_msg.is_notice == False):
                if (self.btn_close_all_review.isHidden()):
                    self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                                       self.editorial_review_widget.y() + self.editorial_review_widget.height() + 50)
                else:
                    self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                                       self.btn_close_all_review.y() + self.btn_close_all_review.height() + 10)
            else:
                self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                                   self.app_discription_label.y() + self.app_discription_label.height() + 30)
        else:
            self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                               self.line_label.y())
        self.show_scrollArea_widget.move(0, 0)
        # self.show_scrollArea_widget.resize(self.card_width*3+40,self.editorial_comments_widget.y() + self.editorial_comments_widget.height() + 10)
        # self.show_scrollArea_widget.move(int((self.width()-self.show_scrollArea_widget.width())/2), 0)
        self.show_scrollArea_widget.hide()
        self.show_scrollArea_widget.show()
        self.scrollArea.show()
        self.show_kmre_detail()

    # 函数：软件签名和人工检测
    # 参数：软件名，是否签名和过检
    # 返回：无
    def slot_app_sign_state(self):
        read_line = ""
        if(type(self.app_msg) != ApkInfo):
            read_line = os.popen("LC_ALL=en_US apt-cache show "+self.app_msg.pkgname+" |grep cert_subject_cn|awk '{print $2}'").readline().strip("\n").strip("\t")
        # if self.app_msg.certSubjectOu != None and self.app_msg.certSubjectOu != "":
        if read_line != None and read_line != "":
            # self.app_sign.setText(_("Software signature"))
            # # self.app_sign.setText("Software signature <img src = './res/correct_sign.png'>")
            # self.manual_detection.setText(_("Manual detection"))
            # self.app_sign.adjustSize()
            # self.manual_detection.adjustSize()
            self.app_sign_bottom.setText(_("Software signature") + " " + "✓️")
            self.manual_detection_bottom.setText(_("Manual detection") + " " + "✓")
            self.app_sign_bottom.adjustSize()
            self.manual_detection_bottom.adjustSize()
            self.signature_unit_label.setText(_("Signature unit:") + read_line)
            self.signature_unit_label.adjustSize()
            self.move_and_resize()

        else:
            # self.app_sign.setText(_("Software signature：x"))
            # self.manual_detection.setText(_("Manual detection：x"))
            # self.app_sign.adjustSize()
            # self.manual_detection.adjustSize()
            self.app_sign_bottom.setText(_("Software signature") + " " + "×")
            self.manual_detection_bottom.setText(_("Manual detection") + " " + "✓")
            self.app_sign_bottom.adjustSize()
            self.manual_detection_bottom.adjustSize()
            self.signature_unit_label.hide()
            self.signature_unit_label.setText(_("Signature unit:"))
            self.signature_unit_label.adjustSize()
            self.move_and_resize()
            self.signature_unit_label.hide()

    # 函数：提交评论后的提示
    # 参数：从服务器拿到的返回信息
    # 返回：无
    def get_submit_review_state(self, review_res):
        self.result_review = review_res
        if review_res == False:
            self.count = 0
            if self.first_submit == True:
                self.submittime = True
            else:
                self.submittime = False

        else:
            if review_res['code'] == 200:
                self.submittime = True
                if(hasattr(self.m_parent.mw,"comment_pop_up")):
                    self.m_parent.mw.comment_pop_up.comment_edit_label.clear()
                    self.m_parent.mw.comment_pop_up.close()

                pass
            else:
                self.count = 0
                self.submittime = False

    #
    # 函数：显示下载次数
    #
    def slot_change_download_count(self, appname, count , update_time):
        if (appname == self.app_msg.pkgname):
            self.app_msg.screenshots_updatetime = update_time
            if(update_time == ""):
                self.update_time_label.setText(_("update time:unknown"))
            else:
                self.update_time_label.setText(_("update time:") + self.app_msg.screenshots_updatetime.split(" ")[0])
            self.update_time_label.adjustSize()
            self.download_num_label.hide()
            if (count == 0 or count == "0"):
                pass
            else:
                if (int(int(count) / 1000) <= 0):
                    count = "<1k"
                else:
                    if (int(int(count) / 1000) >= 1 and int(int(count) / 1000) < 3):
                        count = "1k+"
                    elif (int(int(count) / 1000) >= 3 and int(int(count) / 1000) < 5):
                        count = "3k+"
                    elif (int(int(count) / 1000) >= 5 and int(int(count) / 1000) < 10):
                        count = "5k+"
                    elif (int(int(count) / 1000) >= 10 and int(int(count) / 1000) < 11):
                        count = "10k+"
                    elif (int(int(count) / 1000) >= 1000):
                        count = str(int(int(count) / 1000*1000)) + "M+"
                    else:
                        count = str(int(int(count) / 1000)) + "k+"
                # if(int(count) < 10000):
                #     count = "<1w"
                # elif(int(count) >= 10000 and int(count) < 30000):
                #     count = "1w+"
                # elif (int(count) >= 30000 and int(count) < 50000):
                #     count = "3w+"
                # elif (int(count) >= 50000 and int(count) < 100000):
                #     count = "5w+"
                # else:
                #     count = "10w+"
                self.download_num_label.setText(_("amount of download:") + str(count))
            self.download_num_label.adjustSize()
            if(self.app_msg.from_ukscdb == True):
                self.download_num_label.show()
            else:
                self.download_num_label.hide()
            self.move_and_resize()


    #
    # 函数：显示新的评分
    #
    def slot_change_rating(self, count, scoreDetail, appname, score):
        if (appname == self.app_msg.pkgname):
            if (score == -1):
                score = self.app_msg.ratings_average
            self.score_label_small.hide()
            self.score_label_small.setText(str(round(score, 1)))
            self.score_label_small.adjustSize()
            self.score_label_small.show()
            self.small_starwidget.hide()
            self.small_starwidget.changeGrade(score)
            self.small_starwidget.show()
            self.ratings_widget.count_of_participation_rating.hide()
            if (count == 0):
                self.ratings_widget.count_of_participation_rating.setText(str(self.app_msg.ratings_total) + _("people attend"))
            else:
                self.ratings_widget.count_of_participation_rating.setText(str(count) + _("people attend"))
            self.ratings_widget.count_of_participation_rating.adjustSize()
            self.ratings_widget.count_of_participation_rating.show()
            self.ratings_widget.clear_progress()
            for i in scoreDetail:
                value = int((i["count"] / count) * 100)
                if (value >= 1 and value <= 4):
                    value = 9
                if (i["score"] == 5.0):
                    self.ratings_widget.process1.hide()
                    self.ratings_widget.process1.setValue(value)
                    self.ratings_widget.process1.show()
                elif (i["score"] == 4.0):
                    self.ratings_widget.process2.hide()
                    self.ratings_widget.process2.setValue(value)
                    self.ratings_widget.process2.show()
                elif (i["score"] == 3.0):
                    self.ratings_widget.process3.hide()
                    self.ratings_widget.process3.setValue(value)
                    self.ratings_widget.process3.show()
                elif (i["score"] == 2.0):
                    self.ratings_widget.process4.hide()
                    self.ratings_widget.process4.setValue(value)
                    self.ratings_widget.process4.show()
                elif (i["score"] == 1.0):
                    self.ratings_widget.process5.hide()
                    self.ratings_widget.process5.setValue(value)
                    self.ratings_widget.process5.show()
            self.score_label.hide()
            self.score_label.setText(str(round(score, 1)))
            self.score_label.adjustSize()
            self.score_label.show()
            self.average_score_star.changeGrade(round(score, 1))
            self.move_and_resize()

    #
    # 函数：修改我的评分
    #
    def slot_change_my_rating(self, score, id, appname):
        if (appname == self.app_msg.pkgname):
            self.ratings_widget.slot_click_star(score, False)
            if(hasattr(self.m_parent.mw,"comment_pop_up")):
                self.m_parent.mw.comment_pop_up.slot_click_star(score, False)
            self.ratings_widget.change_star_state()
            pass

    #
    # 函数：处理文件大小的单位换算
    #
    def get_size(self):
        size_text = ""
        sizek = self.app_msg.packageSize / 1024
        if (sizek == 0):
            sizek = self.app_msg.installedSize / 1024
        if (sizek <= 1):
            size_text = "1 KB"
        elif (sizek > 1 and sizek < 1024):
            size_text = str('%.0f' % (sizek)) + " KB"
        elif (sizek >= 1024 and sizek < 1024 * 1024):
            if (len(str('%.2f' % (sizek / 1024.0)) + " MB") > 7):
                size_text = str('%.0f' % (sizek / 1024.0)) + " MB"
            else:
                size_text = str('%.2f' % (sizek / 1024.0)) + " MB"
        else:
            if (len(str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB") > 7):
                size_text = str('%.0f' % (sizek / (1024.0 * 1024.0))) + "GB"
            else:
                size_text = str('%.2f' % (sizek / (1024.0 * 1024.0))) + " GB"
        return size_text

    #
    # 函数：当展开的评论界面高度变化时，将此变化传到主界面，将相应的父界面高度也改变，参数：height：每次子界面改变的高度值
    def set_show_review_height(self, height):

        self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                           self.show_scrollArea_widget.height() + height)
        self.btn_close_all_review.hide()
        self.btn_close_all_review.move(self.btn_close_all_review.x(),
                                       self.show_scrollArea_widget.height() - self.btn_close_all_review.height() - 10)
        self.btn_close_all_review.show()
        # self.btn_retract.show()

    #
    # 函数：点击全部评论，切换评论界面，
    def slot_change_reviewWindow(self):
        Globals.GO_SHORT_REVIEWS = True
        self.all_review_state = True
        self.all_review_label.show()
        # self.editorial_comments_widget.setFixedHeight(0)
        # self.editorial_comments_widget.hide()
        self.editorial_review_widget.setFixedHeight(0)
        self.all_review.hide()
        self.clearwidget(self.all_review)
        self.all_review.show()
        self.all_review_widget.show()
        self.ratings_widget.btn_all_review.hide()
        self.btn_close_all_review.show()
        # self.btn_retract.show()
        self.reviews_num.show()
        self.move_and_resize()
        self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                           self.btn_close_all_review.y() + self.btn_close_all_review.height() + 10)

    #
    # 函数：点击收起评论按钮，切换评论界面，
    def slot_change_shortreviewWindow(self):

        Globals.GO_SHORT_REVIEWS = False
        self.all_review_state = False
        self.all_review_label.hide()
        self.reviews_num.hide()
        self.editorial_comments_widget.hide()
        self.editorial_review_widget.show()
        self.editorial_comments_widget.setFixedHeight(139)
        self.editorial_review_widget.setFixedHeight(self.commend_height)
        self.all_review_widget.hide()

        self.ratings_widget.btn_all_review.show()
        self.btn_close_all_review.hide()
        # self.btn_retract.hide()
        self.move_and_resize()
        self.show_scrollArea_widget.resize(self.show_scrollArea_widget.width(),
                                           self.editorial_review_widget.y() + self.editorial_review_widget.height() + 10)

    #
    # 函数，滑动到底部自动加载更多评论，调用get_review函数实现
    def slot_load_more_review(self, n):
        scrollArea = self.scrollArea
        scrollArea.setFocus(True)
        if not self.btn_close_all_review.isHidden():
            if (n in range(int(scrollArea.verticalScrollBar().maximum() / 4 * 3) - 100,
                           int(scrollArea.verticalScrollBar().maximum())) and Globals.IS_GETTING_REVIEW == False):
                Globals.IS_GETTING_REVIEW = True
                if (self.app_msg.pkgname != Globals.KYDROID_PACKAGE_NAME):
                    if (type(self.app_msg) == Application):
                        self.get_app_reviews.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)
                    else:
                        self.get_app_reviews_apk.emit(self.app_msg, self.app_msg.pkgname, self.reviewpage)
                    # self.all_review.get_review()
                    self.slot_dark_review_widget()
                    # self.all_review_widget.show()
        else:
            return

    #
    # 函数：设置深色模式
    #
    def slot_set_dark_mode(self):
        self.small_starwidget.changeGrade(self.small_starwidget.grade)
        self.ratings_widget.set_progressbar_color()
        if (Globals.THEME_COLOR == 0):
            self.setStyleSheet(
                "QFrame{background: rgba(255, 255, 255, 0.7);border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color: #ffffff;}")
            self.kmre_platform_introduction.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 26px;background-color:transparent;}")
            self.kmre_platform_introduction_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
            self.kmre_technical_features.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 26px;background-color:transparent;}")
            self.kmre_technical_features_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #303133;line-height: 20px;}")
            self.kmre_btn.setStyleSheet(
                "QPushButton{font-size: 20px;font-weight: 400;color: #262626;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #FFFFFF;}QPushButton:hover{background: #FFFFFF;border-radius: 20px;color: #3790FA;font-weight: 400;border:0px;}QPushButton:pressed{background: #FFFFFF;border-radius: 20px;color: #296CD9;font-weight: 400;border:0px;}")
            self.appname_label.setStyleSheet(
                "QLabel{font-size: 20px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.version_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.update_time_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.manufacturers_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.website_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.download_size_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.app_name.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.download_num_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.open_wibside_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.popularity_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            # self.app_sign.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            # self.manual_detection.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.app_categories1.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;background: rgba(55, 144, 250, 0.08);border-radius: 12px;}")
            self.app_categories2.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;background: rgba(55, 144, 250, 0.08);border-radius: 12px;}")
            self.app_categories3.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 16px;background: rgba(55, 144, 250, 0.08);border-radius: 12px;}")
            if (self.screen_left_btn.isEnabled()):
                self.screen_left_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}")
            else:
                self.screen_left_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
            if (self.screen_right_btn.isEnabled()):
                self.screen_right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
            else:
                self.screen_right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_btn.png);}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid #E5E7EE;background-color: #E5E7EE;}")
            self.software_introduction.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.manufacturer_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.btn_login_and_register.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.btn_login_and_register_register.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.btn_login_and_register_line.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
            self.cancel_btn.setStyleSheet(
                "QPushButton{background-image:url(./res/cancel2.png);border-radius:15px;}QPushButton:hover{background-image:url(./res/cancel2_hover.png);border-radius:15px;}")
            # self.app_update_state.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            # self.app_update_content.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 12px;}")
            # self.app_update_version.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            # self.app_update_date.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.app_safe_detection.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.app_safe_detection.adjustSize()
            self.app_sign_bottom.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.manual_detection_bottom.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.signature_unit_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 12px;}")
            self.app_discription_label.setStyleSheet(
                "QTextEdit{border:0px solid #fff000;font-weight: 400;color: #303133;line-height: 20px;}")
            self.manufacturer_name_label.setStyleSheet(
                "QLabel{border:0px solid #fff000;font-weight: 400;color: #303133;line-height: 20px;}")
            self.software_rating_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.count_of_participation_rating.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.full_score_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #C0C4CC;}")
            self.editorial_comments_label.setStyleSheet(
                "QTextEdit{border:0px;font-family: Source Han Sans CN;font-size: 14px;font-weight: 400;color: #606265;line-height: 20px;}")
            self.back_text.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#606265;font-size:14px;}")
            # self.app_detail_photo1.setStyleSheet("QPushButton{background-color: #F4F4F4;border-radius: 20px;}")
            # self.app_detail_photo2.setStyleSheet("QPushButton{background: #F4F4F4;border-radius: 20px;}")
            # self.app_detail_photo3.setStyleSheet("QPushButton{background: #F4F4F4;border-radius: 20px;}")
            # self.app_detail_photo4.setStyleSheet("QPushButton{background: #F4F4F4;border-radius: 20px;}")
            # self.surface_color1.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color2.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color3.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color4.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            self.all_review_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #303133;line-height: 20px;}")
            self.btn_retract.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:50px;image: url(./res/return_shortreview.png)}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked.png)}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked.png)}")
            self.screen_loading.setMovie(self.loadingmove)
            self.loadingmove.start()
            self.loadinglabel.setMovie(self.loadingmove)
            self.loadingmove.start()

            self.btn_close_all_review.setStyleSheet(
                "QPushButton{background: transparent;border:none;font-size: 18px;font-weight: 400;text-align: left;color: #3790FA;line-height: 12px;}")
            for review1 in Globals.ADD_WIDGET:
                review1.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            for review1.userName in Globals.ADD_WIDGET_USERNAME:
                review1.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
            for review1.comment in Globals.ADD_WIDGET_COMMENT:
                review1.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")

            for all_review in Globals.ALL_WIDGET:
                all_review.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            for all_review.userName in Globals.ALL_WIDGET_USERNAME:
                all_review.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
            for all_review.comment in Globals.ALL_WIDGET_COMMENT:
                all_review.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")

        else:
            self.setStyleSheet(
                "QFrame{border-bottom-left-radius:12px;border-bottom-right-radius:12px; background-color: #1F2022;}")
            self.kmre_platform_introduction.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 26px;background-color:transparent;}")
            self.kmre_platform_introduction_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.kmre_technical_features.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 26px;background-color:transparent;}")
            self.kmre_technical_features_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.kmre_btn.setStyleSheet(
                "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #454545;}QPushButton:hover{background: #454545;border-radius: 20px;color: #3790FA;font-weight: 400;border:0px;}QPushButton:pressed{background: #454545;border-radius: 20px;color: #296CD9;font-weight: 400;border:0px;}")

            self.appname_label.setStyleSheet(
                "QLabel{font-size: 20px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            self.version_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.update_time_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.manufacturers_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.website_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.download_size_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.app_name.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")

            self.download_num_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.open_wibside_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.popularity_label.setStyleSheet(
                "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            # self.app_sign.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            # self.manual_detection.setStyleSheet(
            #     "QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.app_categories1.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;text-align: center;background: #454545;border-radius: 12px;}")
            self.app_categories2.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;text-align: center;background: #454545;border-radius: 12px;}")
            self.app_categories3.setStyleSheet(
                "QLabel{border:0px;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;text-align: center;background: #454545;border-radius: 12px;}")

            if (self.screen_left_btn.isEnabled()):
                self.screen_left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}")
            else:
                self.screen_left_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
            if (self.screen_right_btn.isEnabled()):
                self.screen_right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
            else:
                self.screen_right_btn.setStyleSheet("QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_dark.png);}")
            self.line_label.setStyleSheet("QLabel{border: 1px solid #393939;background-color: #393939;}")
            self.software_introduction.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            self.manufacturer_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            self.btn_login_and_register.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.btn_login_and_register_line.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
            self.btn_login_and_register_register.setStyleSheet(
                "QPushButton{background:transparent;border:none;font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}QPushButton:hover{color: #3790FA;}QPushButton:pressed{color: #3790FA;}")
            self.cancel_btn.setStyleSheet(
                "QPushButton{background-image:url(./res/cancel_big_dark.png);border-radius:15px;}QPushButton:hover{background-image:url(./res/cancel_big_hover_dark.png);border-radius:15px;}")

            # self.app_update_state.setStyleSheet("QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            # self.app_update_content.setStyleSheet("QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            # self.app_update_version.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            # self.app_update_date.setStyleSheet("QLabel{font-size: 12px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.app_safe_detection.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #F9F9F9;line-height: 20px;}")
            self.app_safe_detection.adjustSize()
            self.app_sign_bottom.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.manual_detection_bottom.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.signature_unit_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 12px;}")
            self.app_discription_label.setStyleSheet(
                "QTextEdit{border:0px solid #fff000;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.manufacturer_name_label.setStyleSheet(
                "QLabel{border:0px solid #fff000;font-weight: 400;color: #C0C4CC;line-height: 20px;}")
            self.software_rating_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 500;color: #C0C4CC;line-height: 20px;}")
            self.count_of_participation_rating.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
            self.full_score_label.setStyleSheet(
                "QLabel{font-size: 14px;font-weight: 400;color: #8F9399;line-height: 20px;}")
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 12px;border: 2px solid #3D3D41;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
            self.editorial_comments_label.setStyleSheet(
                "QTextEdit{border:0px;font-family: Source Han Sans CN;font-size: 14px;font-weight: 400;color: #F9F9F9;line-height: 20px;}")
            self.back_text.setStyleSheet(
                "QLabel{background-color:transparent;border:0px;border-radius:2px;color:#C0C4CC;font-size:14px;}")
            # self.app_detail_photo1.setStyleSheet(
            #     "QPushButton{background-color: rgba(61,61,65,0.5);border-radius: 20px;}")
            # self.app_detail_photo2.setStyleSheet("QPushButton{background: rgba(61,61,65,0.5);border-radius: 20px;}")
            # self.app_detail_photo3.setStyleSheet("QPushButton{background: rgba(61,61,65,0.5);border-radius: 20px;}")
            # self.app_detail_photo4.setStyleSheet("QPushButton{background: rgba(61,61,65,0.5);border-radius: 20px;}")
            # self.surface_color1.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color2.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color3.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            # self.surface_color4.setStyleSheet(
            #     "QLabel{border-image: url(./res/surface_color_dark.png) 0 0 0 0 stretch strectch;border-radius: 0px;background-color:transparent;}")
            self.all_review_label.setStyleSheet(
                "QLabel{font-size: 18px;font-weight: 400;color: #F9F9F9;line-height: 20px;}")
            self.btn_retract.setStyleSheet(
                "QPushButton{background-color:transparent;border-radius:50px;image: url(./res/return_shortreview_dark.png)}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked_dark.png)}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/return_shortreview_clicked_dark.png)}")
            self.screen_loading.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()
            self.loadinglabel.setMovie(self.loadingmove_dark)
            self.loadingmove_dark.start()

            self.btn_close_all_review.setStyleSheet(
                "QPushButton{background: transparent;border:none;font-size: 18px;font-weight: 400;text-align: left;color: #12C188;line-height: 12px;}")

            for review1 in Globals.ADD_WIDGET:
                review1.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            for review1.userName in Globals.ADD_WIDGET_USERNAME:
                review1.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #F9F9F9;line-height: 22px;}")
            for review1.comment in Globals.ADD_WIDGET_COMMENT:
                review1.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 21px;}")

            for all_review in Globals.ALL_WIDGET:
                all_review.setStyleSheet(".QWidget{border:none;border-radius:12px;background-color:#393939}")
            for all_review.userName in Globals.ALL_WIDGET_USERNAME:
                all_review.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #F9F9F9;line-height: 22px;}")
            for all_review.comment in Globals.ALL_WIDGET_COMMENT:
                all_review.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 21px;}")

        for i in self.app_screenshot_widget_list:
            i.change_color()
        self.down_progress.reflash_color()
        if (hasattr(self, "review_widget")):
            self.review_widget.check_btn_styleSheet()

    #
    # 函数：为展开评论的widget单独设置dark模式
    #
    def slot_dark_review_widget(self):
        if (Globals.THEME_COLOR == 0):
            for all_review in Globals.ALL_WIDGET:
                all_review.setStyleSheet("QWidget{border:none;border-radius:12px;background-color:#F4F4F4}")
            for all_review.userName in Globals.ALL_WIDGET_USERNAME:
                all_review.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #303133;line-height: 22px;}")
            for all_review.comment in Globals.ALL_WIDGET_COMMENT:
                all_review.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #606265;line-height: 21px;}")
        else:
            for all_review in Globals.ALL_WIDGET:
                all_review.setStyleSheet("QWidget{border:none;border-radius:12px;background-color:#3D3D41}")
            for all_review.userName in Globals.ALL_WIDGET_USERNAME:
                all_review.userName.setStyleSheet(
                    "QLabel{font-size: 16px;font-weight: 500;color: #F9F9F9;line-height: 22px;}")
            for all_review.comment in Globals.ALL_WIDGET_COMMENT:
                all_review.comment.setStyleSheet(
                    "QLabel{font-size: 14px;font-weight: 400;color: #C0C4CC;line-height: 21px;}")

    #
    #
    #
    def setLongTextToElideFormat1(self, label, app_name_text, te):
        # if label.width() > 209:

        te = te.replace("\n", ",").replace("\r", "").replace("\t", "")
        metrics = QFontMetrics(label.font())
        elidedText = metrics.elidedText(app_name_text + te, Qt.ElideRight, 150)
        # label.document().adjustSize()
        label.setText(elidedText)
        # if ("…" in elidedText):
        label.setToolTip(te)

    #
    # 函数：显示更多描述
    #
    def slot_show_more_text(self):
        if (self.show_more_text.text() == _("more")):
            self.app_discription_label.clear()
            if(Globals.LAN == "zh_CN"):
                if (str(self.app_msg.summary_init) != "None"):
                    self.app_discription_label.setText(str(self.app_msg.summary_init) + "\n\n" + self.app_msg.description)
                else:
                    self.app_discription_label.setText(self.app_msg.description)
            else:
                try:
                    package = self.ca[self.app_msg.pkgname]
                except:
                    package = None
                if (package != None and package.candidate != None):
                    self.app_discription_label.setText(package.candidate.description)
                else:
                    self.app_discription_label.setText("")
            self.app_discription_label.setFixedHeight(self.app_discription_label.document().size().height())
            self.show_more_text.setText(_("hide"))
            self.show_more_text.setStyleSheet(
                "QPushButton{border: 0px;font-size: 18px;font-weight: 500;color: #3790FA;line-height: 20px;}")
            self.move_and_resize()
        else:
            self.app_discription_label.clear()
            if (Globals.LAN == "zh_CN"):
                if (str(self.app_msg.summary_init) != "None"):
                    self.setLongTextToElideFormat(self.app_discription_label,
                                                   self.app_msg.description,str(self.app_msg.summary_init) )
                else:
                    self.setLongTextToElideFormat(self.app_discription_label, self.app_msg.description)
            else:
                try:
                    package = self.ca[self.app_msg.pkgname]
                except:
                    package = None
                if (package != None and package.candidate != None):
                    self.setLongTextToElideFormat(self.app_discription_label, package.candidate.description)
                else:
                    self.setLongTextToElideFormat(self.app_discription_label, "")
            self.show_more_text.setText(_("more"))
            self.show_more_text.setStyleSheet(
                "QPushButton{border: 0px;font-size: 18px;font-weight: 500;color: #3790FA;line-height: 20px;}")
            self.move_and_resize()
        self.move_and_resize()
        self.show_scrollArea_widget.move(0, 0)
        # self.hide()
        # self.show()

    #
    # 函数：处理点击暂停
    #
    def slot_puase_download(self):
        # self.cancel_btn.move(self.btn.x()-self.cancel_btn.width()-10,self.btn.y())
        if (type(self.app_msg) != ApkInfo):
            self.puase_download.emit(self.app_msg)
        else:
            self.puase_apk_download.emit(self.app_msg)

    #
    # 函数：判断软件的desktop文件
    #
    def check_app_desktop(self, pkgname):
        # 对一些特殊软件单独处理
        fullcmd = ""
        desktopfile = "/usr/share/applications/" + pkgname + ".desktop"
        desktopfile1 = "/usr/share/applications/kde4/" + pkgname + ".desktop"

        if os.path.exists(desktopfile):
            return True
        elif os.path.exists(desktopfile1):
            return True
        else:
            if pkgname in Globals.PKG_NAME:
                pkgname = Globals.PKG_NAME[pkgname]

                desktopfile = "/usr/share/applications/" + pkgname + ".desktop"

                if os.path.exists(desktopfile):
                    return True
                else:
                    desktopfile = "/usr/share/applications/kde4/" + pkgname + ".desktop"
                    if (os.path.exists(desktopfile)):
                        return True
                    else:
                        return False
            else:
                return False

    def change_btn_width(self):
        s = self.m_font.boundingRect(self.btn.text()).size().width()
        if (s + 24 < 100):
            self.btn.setFixedSize(100, 40)
        else:
            self.btn.setFixedSize(s + 24, 40)
        # print("00000",self.btn.width())
        self.btn.move(self.scrollArea.width() - self.btn.width() - 50, 36)

    def change_kmre_btn_width(self):
        s = self.m_font.boundingRect(self.kmre_btn.text()).size().width()
        if (s + 24 < 100):
            self.kmre_btn.setFixedSize(140, 40)
        else:
            self.kmre_btn.setFixedSize(s + 24, 40)
        # print("00000",self.btn.width())
        # self.kmre_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width()-self.kmre_btn.width())/2),self.kmre_photo_label.y() + self.kmre_photo_label.height()-self.kmre_btn.height()-50)
        proportion = (self.width() - 50 * 2) / self.kmre_width
        self.kmre_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width()-self.kmre_btn.width())/2),self.kmre_photo_label.y() + self.kmre_photo_label.height()-self.kmre_btn.height()-int(50*proportion))

    def change_category_width(self, label):
        s = self.m_font2.boundingRect(label.text()).size().width()
        if (s+24 < 52):
            label.setFixedSize(52, 24)
        else:
            label.setFixedSize(s+24, 24)

    def change_kydroid_btn_statue(self):
        if(type(self.app_msg) == ApkInfo):
            for i in self.m_parent.mw.at.appmgr.apk_list:
                if(i.pkgname == self.app_msg.pkgname):
                    self.app_msg.is_installed = i.is_installed
                    self.app_msg.is_upgradable = i.is_upgradable
            self.check_btn_text()

    #
    # 函数：获取按钮的名字
    #
    def check_btn_text(self):
        self.btn.resize(100, 40)
        self.btn.move(self.scrollArea.width() - self.btn.width() - 50, 36)
        self.cancel_btn.hide()
        self.down_progress.hide()
        self.btn.setCheckable(True)
        self.btn.setEnabled(True)
        self.kmre_btn.setCheckable(True)
        self.kmre_btn.setEnabled(True)
        if (self.app_msg.status == PkgStates.REMOVING):
            self.btn.setStyleSheet(
                "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
            self.slot_uninstall(self.app_msg.pkgname, "uninstalling")
            return
        if (type(self.app_msg) != ApkInfo):
            try:
                self.package = self.ca[self.app_msg.pkgname]
            except:
                self.package = None
            if (self.package != None):
                if (self.package.is_installed == True and self.package.is_upgradable == True):
                    if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                        self.kmre_btn.setText(_("update"))
                        self.change_kmre_btn_width()
                    else:
                        self.btn.setText(_("update"))
                        self.change_btn_width()
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 20px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == False):
                    if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                        self.kmre_btn.setEnabled(True)
                        self.kmre_btn.setCheckable(True)
                        self.kmre_btn.setText(_("download experience"))
                        self.change_kmre_btn_width()
                        self.kmre_tip_label.hide()
                    else:
                        self.btn.setText(_("down"))
                        self.change_btn_width()
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                elif (self.package.is_installed == True and self.package.is_upgradable == False):
                    # self.btn.setText(_("run"))
                    if (self.app_msg.pkgname == "kylin-installer"):
                        has_desktop = False
                    else:
                        has_desktop = self.check_app_desktop(self.app_msg.pkgname)
                    if (has_desktop == True):
                        if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                            self.kmre_btn.setText(_("installed"))
                            self.kmre_btn.setEnabled(False)
                            self.change_kmre_btn_width()
                            self.kmre_tip_label.show()

                        else:
                            self.btn.setText(_("Open"))
                            self.change_btn_width()
                            self.btn.setStyleSheet(
                                "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 20px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                    else:
                        if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                            self.kmre_btn.setText(_("installed"))
                            self.kmre_btn.setEnabled(False)
                            self.change_kmre_btn_width()
                            self.kmre_tip_label.show()
                        else:
                            self.btn.setText(_("installed"))
                            self.change_btn_width()
                            self.btn.setEnabled(False)
                            self.btn.setStyleSheet(
                                "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 20px;}")
                    pass
            else:
                if(self.app_msg.is_notice == False):
                    self.btn.setText(_("uninstalled"))
                    self.change_btn_width()
                    self.btn.setStyleSheet("QPushButton{font-size: 20px;font-weight: 400;color: #F8F8F8;line-height: 12px;background: #C0C4CC;border-radius: 20px;}")
                else:
                    self.btn.setText(_("website"))
                    self.change_btn_width()
                    self.btn.setStyleSheet(
                        "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
        else:
            if (self.app_msg != None):
                if ((Globals.APK_EVNRUN == 0)):
                    self.btn.setText(_("start env"))
                    self.change_btn_width()
                    self.btn.setStyleSheet(
                        "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 20px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                else:
                    if (self.app_msg.is_installed == True and self.app_msg.is_upgradable == True):
                        self.btn.setText(_("update"))
                        self.change_btn_width()
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 20px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                    elif (self.app_msg.is_installed == False):
                        self.btn.setText(_("down"))
                        self.change_btn_width()
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                    elif (self.app_msg.is_installed == True and self.app_msg.is_upgradable == False):
                        self.btn.setText(_("Open"))
                        self.change_btn_width()
                        self.btn.setStyleSheet(
                            "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;border-radius: 20px;background: rgba(55, 144, 250, 0.12);}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                        pass
            else:
                self.btn.setText(_("down"))
                self.change_btn_width()
                self.btn.setStyleSheet("QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
        if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
            if(self.app_msg.status != PkgStates.DOWNLOADING and self.app_msg.status != PkgStates.INSTALLING):
                self.kmre_btn.show()
        else:
            self.btn.show()
            self.set_btn_stylesheet()
        self.kmre_btn.setEnabled(True)

    #
    # 函数：设置按钮的样式
    #
    def set_btn_stylesheet(self, is_uninstall=False):
        if (is_uninstall == True):
            self.btn.setStyleSheet(
                "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;background: #3790FA;border-radius: 20px;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;}")

    # 处理：卸载界面的显示
    def slot_uninstall(self, appname, state):
        if (appname == self.app_msg.pkgname):
            self.btn.resize(100, 40)
            self.btn.move(self.scrollArea.width() - self.btn.width() - 50, 36)
            if (state == "uninstalling"):
                self.down_progress.hide()
                self.btn.setStyleSheet(
                    "QPushButton{font-size: 20px;font-weight: 400;color: #FFFFFF;line-height: 12px;border-radius: 20px;border: 0px solid #3790FA;background: #3790FA;}QPushButton:hover{background: #40A9FB;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}QPushButton:pressed{background: #296CD9;border-radius: 20px;color: #ffffff;font-weight: 400;border:0px;}")
                self.btn.setText(_("uninstalling"))
                self.change_btn_width()
            else:
                self.down_progress.hide()
                self.btn.setText(_("uninstalled"))
                self.change_btn_width()
                self.btn.setStyleSheet(
                    "QPushButton{font-size: 20px;font-weight: 400;color: #F8F8F8;line-height: 12px;background: #C0C4CC;border-radius: 20px;}")

    #
    # 函数：重置按钮
    #
    def reload_btn(self, appname):
        if (appname == self.app_msg.pkgname):
            self.ca.close()
            self.ca.open()
            self.check_btn_text()

    #
    # 函数：处理安装过程中的界面
    #
    def slot_get_download_progress(self, appname, progress):
        if (appname == self.app_msg.pkgname):
            if (progress == 100):
                self.narrow_btn_time.stop()
                if(appname == Globals.KYDROID_PACKAGE_NAME):
                    self.kmre_btn.resize(140, 40)
                    self.cancel_btn.hide()
                    self.kmre_btn.setCheckable(False)
                    self.kmre_btn.setText(_("installing"))
                    self.change_kmre_btn_width()
                    self.kmre_btn.show()
                    self.down_progress.hide()
                else:
                    self.btn.resize(100, 40)
                    self.btn.move(self.scrollArea.width() - self.btn.width() - 50, 36)
                    self.cancel_btn.hide()
                    self.btn.setCheckable(False)
                    self.btn.setText(_("installing"))
                    self.change_btn_width()
                    self.btn.setStyleSheet(
                        "QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 20px;}")
                    self.btn.show()
                    self.down_progress.hide()
            else:
                if (self.app_msg.status != PkgStates.PUASE):
                    if (progress - self.pre_progress >= 2):
                        self.send_download_progress.emit(progress)
                        self.pre_progress = progress
                        self.app_msg.percent = progress
                    else:
                        self.send_download_progress.emit(self.pre_progress)

    def change_btn_state(self, appname, state):
        if (appname == self.app_msg.pkgname):
            if (state == PkgStates.NOTHING or state == PkgStates.REMOVED):
                self.app_msg.status = PkgStates.NOTHING
                self.check_btn_text()
                self.check_all_check_statue.emit()
            if (state == PkgStates.INSTALLING):
                self.slot_get_download_progress(appname, 100)
            if (state == PkgStates.REMOVING):
                self.slot_uninstall(appname, "uninstalling")
            if (state == PkgStates.INSTALLED):
                self.app_msg.is_installed = True
                self.app_msg.is_upgradable = False
                self.app_msg.status = PkgStates.NOTHING
                self.reload_btn(appname)
                self.check_all_check_statue.emit()
            if (state == PkgStates.DOWNLOADING):
                if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME and self.kmre_btn.isHidden() == False):
                    self.down_progress.init_down = True
                    self.down_progress.persent = 0
                if (self.btn.isHidden() == False and self.app_msg.pkgname != Globals.KYDROID_PACKAGE_NAME):
                    self.down_progress.init_down = True
                    self.down_progress.persent = 0
                if (self.app_msg.status != PkgStates.NOTHING):
                    if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                        if (self.kmre_btn.text() != "" and self.kmre_btn.text() != "installing"):
                            self.narrow_btn_time.start(1)
                        else:
                            self.kmre_btn.hide()
                            self.cancel_btn.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                            self.down_progress.show()
                    else:
                        if (self.btn.text() != ""):
                            self.narrow_btn_time.start(1)
                        else:
                            self.btn.hide()
                            self.cancel_btn.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                            self.down_progress.show()
                # self.show_downloading_widget(appname)
            if (state == PkgStates.PUASE):
                # self.show_downloading_widget(appname,False,True)
                self.app_msg.status = PkgStates.PUASE
                self.btn.hide()
                self.kmre_btn.hide()
                self.cancel_btn.show()
                self.down_progress.show()
                self.down_progress.puase_download(self.app_msg.percent)
                self.check_all_check_statue.emit()

    def move_btn(self):
        self.narrow_btn_time.stop()
        if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
            self.btn_text = self.kmre_btn.text()
            self.kmre_btn.setText("")
            w = self.kmre_btn.width()
            h = self.kmre_btn.height()
            x = self.kmre_btn.x()
            y = self.kmre_btn.y()
            if (self.app_msg.status != PkgStates.NOTHING):
                if (self.kmre_btn.width() > self.kmre_btn.height()):
                    self.kmre_btn.hide()
                    self.kmre_btn.setFixedSize(w - 1, h)
                    self.kmre_btn.move(x + 1, y)
                    self.kmre_btn.show()
                    self.narrow_btn_time.start(1)
                else:
                    # 返回列表倒数第一个是该界面自己
                    if (self.need_send_exposure_rate == True):
                        if (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                            "pagetype"] == PageStates.HOMEPAGE):
                            if (self.click_card_type == ClickCard.AD):
                                self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, 0)
                            elif (self.click_card_type == ClickCard.HOT):
                                self.m_parent.mw.save_down_hot_card(self.app_msg.app_id)
                            elif (self.click_card_type == ClickCard.NEWARRIVALS):
                                self.m_parent.mw.save_down_new_arrival_card(self.app_msg.app_id)
                        elif (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                                  "pagetype"] == PageStates.CAREGORYPAGE):
                            for i in Globals.ALL_CATEGORIES:
                                if (i["name"] == Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                                    "Type"]):
                                    self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, i["id"])
                                    break
                                elif (i["displayname"] ==
                                      Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2]["Type"]):
                                    self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, i["id"])
                                    break
                    self.kmre_btn.setText(self.btn_text)
                    self.down_progress.show()
                    self.show_downloading_widget(self.app_msg.pkgname)
        else:
            self.btn_text = self.btn.text()
            self.btn.setText("")
            w = self.btn.width()
            h = self.btn.height()
            x = self.btn.x()
            y = self.btn.y()
            if (self.app_msg.status != PkgStates.NOTHING):
                if (self.btn.width() > self.btn.height()):
                    self.btn.hide()
                    self.btn.setFixedSize(w - 1, h)
                    self.btn.move(x + 1, y)
                    self.btn.show()
                    self.narrow_btn_time.start(2)
                else:
                    # 返回列表倒数第一个是该界面自己
                    if (self.need_send_exposure_rate == True):
                        if (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                            "pagetype"] == PageStates.HOMEPAGE):
                            if(self.click_card_type == ClickCard.AD):
                                self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, 0)
                            elif(self.click_card_type == ClickCard.HOT):
                                self.m_parent.mw.save_down_hot_card(self.app_msg.app_id)
                            elif(self.click_card_type == ClickCard.NEWARRIVALS):
                                self.m_parent.mw.save_down_new_arrival_card(self.app_msg.app_id)
                        elif (Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                                  "pagetype"] == PageStates.CAREGORYPAGE):
                            for i in Globals.ALL_CATEGORIES:
                                if (i["name"] == Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2][
                                    "Type"]):
                                    self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, i["id"])
                                    break
                                elif (i["displayname"] ==
                                      Globals.RETURN_PATH_LIST[len(Globals.RETURN_PATH_LIST) - 2]["Type"]):
                                    self.m_parent.mw.save_down_ad_card(self.app_msg.app_id, i["id"])
                                    break

                    self.btn.setText(self.btn_text)
                    self.down_progress.show()
                    self.show_downloading_widget(self.app_msg.pkgname)
            else:
                self.btn.setText(self.btn_text)

    # 函数：处理软件介绍
    #
    def setLongTextToElideFormat(self, label, te,summary_init = ""):
        # te = te.replace("\n", "").replace("\r", "").replace("\t", "")
        te = summary_init+"\n\n"+te.replace("\n", "").replace("\r", "").replace("\t", "")
        metrics = QFontMetrics(label.font())
        elidedText = metrics.elidedText(te, Qt.ElideRight, label.width() * 3 - 160)
        # label.document().adjustSize()
        # if(summary_init != ""):
        #     label.setText(summary_init + "\n\n" + elidedText)
        #     label.setFixedHeight(label.document().size().height()+30)
        # else:
        label.setText(elidedText)
        label.setFixedHeight(int(label.document().size().height()))
        # label.resize(self.width(), label.document().size().height())

    # 函数：显示详情超过时更多按钮
    def show_more_btn(self):
        elidedText = self.app_discription_label.toPlainText()
        if ("…" in elidedText):
            self.show_more_text.setText(_("more"))
            self.show_more_text.setStyleSheet(
                "QPushButton{border: 0px;font-size: 18px;font-weight: 500;color: #3790FA;line-height: 20px;}")

            self.show_more_text.show()
        else:
            self.show_more_text.setText(_("hide"))
            self.show_more_text.setStyleSheet(
                "QPushButton{border: 0px;font-size: 18px;font-weight: 500;color: #606265;line-height: 20px;}")
            self.show_more_text.hide()
            # self.app_discription_label.setText((self.app_msg.description))
            self.app_discription_label.setFixedHeight(115)

    # 函数：显示下载接界面
    def show_downloading_widget(self, appname, is_cancel=False, is_puase=False):
        if (appname == self.app_msg.pkgname):
            if (self.app_msg.status != PkgStates.NOTHING):
                if (is_cancel == False):
                    if (is_puase == False):
                        if (self.app_msg.status != PkgStates.INSTALLING):
                            self.kmre_btn.hide()
                            self.btn.hide()
                            if (self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                                self.down_progress.btn.move(
                                    int((self.down_progress.width() - self.down_progress.btn.width()) / 2),
                                    int((self.down_progress.width() - self.down_progress.btn.width()) / 2))
                                self.cancel_btn.move(self.kmre_photo_label.x() + int((self.kmre_photo_label.width() - (self.down_progress.width() + self.cancel_btn.width() + 9)) / 2),
                                                     self.kmre_btn.y() + int((self.kmre_btn.height() - self.cancel_btn.height())/2 ))
                                self.down_progress.move(self.cancel_btn.x() + self.cancel_btn.width() + 9,
                                                        self.cancel_btn.y())
                            else:
                                self.cancel_btn.move(self.down_progress.x() - self.cancel_btn.width() - 8,
                                                     self.down_progress.y() - int(
                                                         (self.cancel_btn.height() - self.down_progress.height()) / 2))
                            self.cancel_btn.show()
                            self.down_progress.show()
                            self.down_progress.btn.show()
                            self.down_progress.cont_btn.hide()
                    else:
                        self.btn.setText(_("cont"))
                        self.btn.setStyleSheet("QPushButton{font-size: 20px;font-weight: 400;color: #3790FA;line-height: 12px;background: rgba(55, 144, 250, 0.12);border-radius: 20px;}QPushButton:hover{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;}QPushButton:pressed{background: #3790FA;border-radius: 20px;color: #ffffff;font-weight: 400;}")
                        self.cancel_btn.show()
                        # self.btn.show()
                        self.btn.hide()
                        self.kmre_btn.hide()
                        self.cancel_btn.show()
                        self.down_progress.show()
                        # self.scrollArea.verticalScrollBar().setValue(0)
                else:
                    if(self.app_msg.pkgname == Globals.KYDROID_PACKAGE_NAME):
                        # self.kmre_btn.move(self.kmre_photo_label.x() + int(
                        #     (self.kmre_photo_label.width() - self.kmre_btn.width()) / 2),
                        #                    self.kmre_photo_label.y() + self.kmre_photo_label.height() - self.kmre_btn.height() - 30)
                        proportion = (self.width() - 50 * 2) / self.kmre_width
                        self.kmre_btn.move(self.kmre_photo_label.x() + int(
                            (self.kmre_photo_label.width() - self.kmre_btn.width()) / 2),
                                           self.kmre_photo_label.y() + self.kmre_photo_label.height() - self.kmre_btn.height() - int(50*proportion))
                        self.reload_btn(appname)
                        self.cancel_btn.hide()
                        self.down_progress.hide()
                        self.kmre_btn.show()
                    else:
                        # self.btn.setText(self.btn_text)
                        self.btn.resize(100, 40)
                        self.app_msg.status = PkgStates.NOTHING
                        self.btn.move(self.scrollArea.width() - self.btn.width() - 50, 36)
                        self.reload_btn(appname)
                        self.cancel_btn.hide()
                        self.down_progress.hide()
                        self.btn.show()
            else:
                self.check_btn_text()

    def slot_edit_label_textchanged(self):
        maxLength = 200
        strText = (self.editorial_comments_label.toPlainText())
        length = self.editorial_comments_label.document().characterCount()
        if strText != "":
            self.back_text.hide()
            if length > maxLength:

                strText = strText[:-1]
                set_text = strText[0:200]
                self.editorial_comments_label.clear()
                self.editorial_comments_label.setText(set_text)
                tc = self.editorial_comments_label.textCursor()
                tc.movePosition(QTextCursor.End)
                self.editorial_comments_label.setTextCursor(tc)
                self.tip.setText(_("Input 200 words at most"))
                self.tip.show()

                # self.editorial_comments_widget.setStyleSheet(
                #     "QFrame{border-radius: 12px;border: 2px solid #F56C6C;}QFrame:hover{border-radius: 12px;border: 2px solid #F56C6C;}")
            else:
                if self.tip.text() == self.review_text:
                    pass
                else:
                    self.tip.hide()
                if (Globals.THEME_COLOR == 0):
                    self.editorial_comments_widget.setStyleSheet(
                        "QFrame{border-radius: 12px;border: 2px solid #3790FA;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
                else:
                    self.editorial_comments_widget.setStyleSheet(
                        "QFrame{border-radius: 12px;border: 2px solid #3D3D41;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
                # self.editorial_comments_widget.setStyleSheet("QFrame{border-radius: 12px;border: 2px solid #3790FA;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
                self.submit_btn.show()
        else:
            if (Globals.LOGIN_SUCCESS):
                self.back_text.show()
            else:
                self.back_text.hide()
            if self.tip.text() == self.review_text:
                pass
            else:
                self.tip.hide()
            self.editorial_comments_widget.setStyleSheet(
                "QFrame{border-radius: 12px;border: 2px solid #C0C4CC;}QFrame:hover{border-radius: 12px;border: 2px solid #3790FA;}")
            self.submit_btn.hide()
        # strText = (self.editorial_comments_label.toPlainText())


    # 函数：计时器30s一循环。

    def slot_time_Refresh(self):

        if self.count > 0:

            # self.string = (_("休息一下，"))
            self.string = (_("take a break,"))
            # self.string_b = (_("s后再试"))
            self.string_b = (_("s Try again"))
            self.m_parent.mw.comment_pop_up.tip_label.setText(self.string + str(self.count) + self.string_b)
            self.m_parent.mw.comment_pop_up.tip_label.adjustSize()
            self.review_text = self.string + str(self.count) + self.string_b

            if self.first_submit == True or self.submittime == True:
                self.time_tip.hide()

            else:
                self.m_parent.mw.comment_pop_up.tip_label.show()
                # self.tip.hide()
                # self.submit_btn.setEnabled(True)
            # self.ui.captcha.setStyleSheet("QPushButton{border:0px;font-size: 12px;color:#606265;background:transparent;}")
            self.count -= 1
        else:
            self.timer_go.stop()
            self.m_parent.mw.comment_pop_up.tip_label.hide()
            self.m_parent.mw.comment_pop_up.submit_btn.setEnabled(True)
            self.count = 30

    #
    # 函数：处理提交我的评分结束
    #
    def slot_submit_my_rating_over(self, score, appname):
        # (^-^)提交我的评分后获取新的平均分值和各分区评分人数占比，只需要刷新界面，不需要在重新获取评分了
        if (appname == self.app_msg.pkgname):
            if (int(score) == 0):
                # self.ratings_widget.MY_SCORE = 0
                self.ratings_widget.pre_app_name = ""
                self.ratings_widget.refresh_star()
            else:
                # self.ratings_widget.MY_SCORE = int(score)
                self.m_parent.mw.show_tip_widget(_("Submitted,thank you for your feedback!"))
                self.m_parent.mw.tip_widget.btn.clicked.connect(self.m_parent.mw.slot_close_tipwidget)
                self.need_get_new_rating.emit(str(self.app_msg.app_id), self.app_msg.pkgname,str(self.app_msg.softStyle))
            if(hasattr(self.m_parent.mw,"comment_pop_up") and self.m_parent.mw.comment_pop_up.isHidden() == False):
                # self.m_parent.mw.comment_pop_up.MY_SCORE = int(score)
                if(int(score) == 0):
                    self.m_parent.mw.comment_pop_up.pre_app_name = ""
                    self.m_parent.mw.comment_pop_up.refresh_star()
                else:
                    self.m_parent.mw.comment_pop_up.change_star_state()
                    self.ratings_widget.slot_click_star(self.m_parent.mw.comment_pop_up.MY_SCORE,False)
            self.ratings_widget.change_star_state()
            # self.need_get_new_rating.emit(self.app_msg.app_id,self.app_msg.pkgname)
        pass

    def slot_screen_right_time_timeout(self):
        self.screen_right_time.stop()

        if (self.screen_card_list_widget.x() == self.screen_widget_x or self.screen_card_list_widget.x() - 80 < self.screen_widget_x):
            self.screen_card_list_widget.move(self.screen_widget_x, 0)
            self.screen_left_btn.setEnabled(True)
            if (Globals.THEME_COLOR == 0):
                self.screen_left_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_clicked_btn.png);}")
            else:
                self.screen_left_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/left_true_click_dark.png);}")
            if (self.get_screen_num - self.screen_h_num - self.get_now_first_card_count == 0):
                self.screen_right_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.screen_right_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_btn.png);}")
                else:
                    self.screen_right_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_dark.png);}")
            for i in range(self.get_screen_num):
                if (i >= self.get_now_first_card_count and i < len(self.app_screenshot_widget_list)):
                    app_screen_shot = self.app_screenshot_widget_list[i]
                    app_screen_shot.show()
                    app_screen_shot.screen_outer_frame.show()
                    app_screen_shot.screen_shot.show()
                    app_screen_shot.surface_color.show()
            self.screen_right_btn.setEnabled(True)
        else:
            self.screen_card_list_widget.move(self.screen_card_list_widget.x() - 80, 0)
            self.screen_right_time.start(1)

    #
    #函数：处理点击截图右键
    #
    def slot_clicked_screen_right_btn(self):
        self.screen_left_btn.setEnabled(False)
        self.screen_right_btn.setEnabled(False)
        self.get_now_first_card_count = 0
        for i in range(self.get_screen_num):
            if(self.app_screenshot_widget_list[i].x() + self.screen_card_list_widget.x() == 0):
                self.get_now_first_card_count = i
                break
        if(self.get_screen_num - self.screen_h_num - self.get_now_first_card_count < self.screen_h_num):
            new_now_first_card_count = self.get_screen_num - self.screen_h_num
            self.get_now_first_card_count = new_now_first_card_count
            self.screen_widget_x =  - self.app_screenshot_widget_list[new_now_first_card_count].x()
            # self.screen_card_list_widget.move(self.screen_card_list_widget.x() - self.app_screenshot_widget_list[new_now_first_card_count].x(), 0)
            self.screen_right_time.start(1)
            # self.screen_card_list_widget.move(self.screen_card_list_widget.x()-self.app_screenshot_widget_list[self.screen_h_num + self.get_now_first_card_count].x(),0)
            # self.screen_right_time.start(1)
        else:
            if(self.get_screen_num-self.screen_h_num - self.get_now_first_card_count == 0):
                self.screen_right_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.screen_right_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_btn.png);}")
                else:
                    self.screen_right_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_false_dark.png);}")
            else:
                new_now_first_card_count = self.screen_h_num + self.get_now_first_card_count-abs((self.screen_h_num-(len(self.app_screenshot_widget_list)-1-self.screen_h_num - self.get_now_first_card_count)))
                self.get_now_first_card_count = new_now_first_card_count
                self.screen_widget_x = self.screen_card_list_widget.x() - self.app_screenshot_widget_list[new_now_first_card_count].x()
                # self.screen_card_list_widget.move(self.screen_card_list_widget.x() - self.app_screenshot_widget_list[new_now_first_card_count].x(), 0)
                self.screen_right_time.start(1)


    def slot_screen_left_time_timeout(self):
        self.screen_left_time.stop()

        if(self.screen_card_list_widget.x() == self.screen_widget_x or self.screen_card_list_widget.x()+80 > self.screen_widget_x):
            self.screen_card_list_widget.move(self.screen_widget_x,0)
            if(self.screen_card_list_widget.x() != 0):
                self.screen_left_btn.setEnabled(True)
            else:
                self.screen_left_btn.setEnabled(False)
            self.screen_right_btn.setEnabled(True)
            if (Globals.THEME_COLOR == 0):
                self.screen_right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_clicked_btn.png);}")
            else:
                self.screen_right_btn.setStyleSheet(
                    "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/right_true_btn_dark.png);}QPushButton:hover{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}QPushButton:pressed{background-color:transparent;border-radius:20px;image: url(./res/right_true_click_dark.png);}")
            if (self.screen_card_list_widget.x() == 0):
                self.screen_left_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.screen_left_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
                else:
                    self.screen_left_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
            for i in range(self.get_screen_num):
                if (i <self.screen_h_num and i < len(self.app_screenshot_widget_list)):
                    app_screen_shot = self.app_screenshot_widget_list[i]
                    app_screen_shot.show()
                    app_screen_shot.screen_outer_frame.show()
                    app_screen_shot.screen_shot.show()
                    app_screen_shot.surface_color.show()
        else:
            self.screen_card_list_widget.move(self.screen_card_list_widget.x()+80, 0)
            self.screen_left_time.start(1)


    def slot_clicked_screen_left_btn(self):
        self.screen_left_btn.setEnabled(False)
        self.screen_right_btn.setEnabled(False)
        get_now_first_card_count = 0
        for i in range(self.get_screen_num):
            if (self.app_screenshot_widget_list[i].x() + self.screen_card_list_widget.x() == 0):
                get_now_first_card_count = i
                break
        if (self.screen_h_num <= get_now_first_card_count):
            self.screen_left_btn.setEnabled(False)
            self.screen_right_btn.setEnabled(False)
            self.screen_widget_x = self.screen_card_list_widget.x() + self.app_screenshot_widget_list[self.screen_h_num].x()
            self.screen_left_time.start(1)
        else:
            if (self.screen_card_list_widget.x() == 0):
                self.screen_left_btn.setEnabled(False)
                if (Globals.THEME_COLOR == 0):
                    self.screen_left_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_btn.png);}")
                else:
                    self.screen_left_btn.setStyleSheet(
                        "QPushButton{background-color:transparent;border-radius:20px;image: url(./res/left_false_dark.png);}")
            else:
                self.screen_widget_x = 0
                self.screen_left_time.start(1)



    # 鼠标进入事件

    def enterEvent(self, event):
        self.status = BUTTON_ENTER
        self.scrollArea.verticalScrollBar().show()
        # self.scrollArea.show()

    #
    # 函数：鼠标离开事件
    #

    def leaveEvent(self, event):
        # if(self.status != BUTTON_PRESSED):
        self.status = BUTTON_LEAVE
        self.scrollArea.verticalScrollBar().hide()

    #
    # 函数：鼠标按下事件
    #
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.status = BUTTON_PRESSED
        else:
            event.ignore()

    def eventFilter(self, obj, event):
        if (event.type() == event.MouseButtonRelease):
            if (obj == self):
                self.m_parent.mw.hide_tip_widget()
        if((hasattr(self,"app_discription_label")) and obj == self.app_discription_label):
            if (event.type() == event.KeyPress):
                if (event.key() == Qt.Key_C):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
                if (event.key() == Qt.Key_V):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
                if (event.key() == Qt.Key_X):
                    if QApplication.keyboardModifiers() == Qt.ControlModifier:
                        return True
        if((hasattr(self,"app_screen_shot_list_widget") and event.type() == event.Enter and obj == self.app_screen_shot_list_widget) or (hasattr(self,"screen_card_list_widget") and event.type() == event.Enter and obj == self.screen_card_list_widget)):
            if(hasattr(self,"screen_right_btn") and hasattr(self,"screen_left_btn") and hasattr(self,"screen_card_list_widget") and self.show_scrollArea_widget.width() - 50*2 < self.screen_card_list_widget.width()-self.screen_left_btn.width()*2-10*2 and len(self.app_msg.screenshots) > self.screen_h_num and len(self.app_msg.screenshots) > 0):
                self.screen_right_btn.show()
                self.screen_left_btn.show()
        if (hasattr(self,"app_screen_shot_list_widget") and event.type() == event.Leave and obj == self.app_screen_shot_list_widget):
            if (hasattr(self,"screen_right_btn") and hasattr(self,"screen_left_btn")):
                self.screen_right_btn.hide()
                self.screen_left_btn.hide()
        # if(hasattr(self,"editorial_comments_label") and obj == self.editorial_comments_label):
        #     if (event.type() == QEvent.KeyPress):
        #         if (event.key() == Qt.Key_V):
        #             if QApplication.keyboardModifiers() == Qt.ControlModifier:
        #                 return True
        return False

    #
    #评论点赞
    #
    def slot_click_agree_with_btn(self,reviewlist,comment_widget):
        self.m_parent.mw.slot_click_agree_with_btn(reviewlist,str(self.app_msg.app_id),comment_widget)

    #
    #评论点踩
    #
    def slot_click_disagree_btn(self,reviewlist,comment_widget):
        self.m_parent.mw.slot_click_disagree_with_btn(reviewlist,str(self.app_msg.app_id),comment_widget)

    #
    #函数：删除用户评论
    #
    def slot_delete_comment(self,reviewlist,appname):
        # self.m_parent.mw.slot_delete_user_commemt(reviewlist)
        self.m_parent.mw.show_delete_user_comment_tip(reviewlist,appname)


    # 函数：展示回到顶部按钮
    def slot_show_btn_reacter(self):

        if (self.scrollArea.verticalScrollBar().value() > 0):
            self.btn_retract.show()
            self.btn_retract.move(self.width() - 80, self.height() - 100)
        elif (self.scrollArea.verticalScrollBar().value() == 0):
            self.btn_retract.hide()

    # 函数：回到顶部
    def slot_go_back_Top(self):
        self.scrollArea.verticalScrollBar().setValue(0)
        self.btn_retract.hide()

    def PixmapToRound(self,src,x,y):
        pixmapa = QPixmap(src)
        pixmap = QPixmap(x, y)
        pixmap.fill(Qt.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing, True)
        # painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        path = QPainterPath()
        rect = QRectF(0,0,x,y)
        path.addRoundedRect(rect, 12.0, 12.0)
        painter.setClipPath(path)
        painter.drawPixmap(0, 0, x, y, pixmapa)

        return pixmap

