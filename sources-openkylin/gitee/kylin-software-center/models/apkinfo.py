#!/usr/bin/python3
# -*- coding: utf-8 -*-

### BEGIN LICENSE

# Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd

# Author:
#     Shine Huang<shenghuang@ubuntukylin.com>
# Maintainer:
#     Shine Huang<shenghuang@ubuntukylin.com>

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

from utils import run
from models import enums
from models.application import BaseInfo
from models.globals import Globals
from utils.log_manager import LogManager
import gettext
gettext.textdomain("kylin-software-center")
_ = gettext.gettext

class ApkInfo(BaseInfo):

    def __init__(self, pkg_name, display_name, version, size, file_path, summary,is_notice = False):
        super().__init__(pkg_name)
        self.is_notice = is_notice
        self.app_id = 0
        self.pkgname = pkg_name
        self.displayname = display_name
        self.candidate_version = version
        self.category_name = ""
        self.category_name_cn = ""
        self.from_ukscdb = True
        self.version = version
        self.size = int(float(size))
        self.file_path = file_path
        self.summary_init = summary
        self.description_init = summary
        self.orig_name = pkg_name
        self.orig_summary = summary
        self.orig_description = summary
        self.thumbnail_url = None
        self.screenshot_url = None
        self.thumbnailfile = enums.UBUNTUKYLIN_RES_SCREENSHOT_PATH + pkg_name + "_thumbnail.png"
        self.screenshotfile = enums.UBUNTUKYLIN_RES_SCREENSHOT_PATH + pkg_name + "_screenshot.png"
        self.iconfile = enums.UBUNTUKYLIN_RES_ICON_PATH + pkg_name + ".png"
        self.screenshots = []
        self.screenshots_url = ""
        self.screenshots_updatetime = ""
        self.icons = []
        self.reviews = {}
        self.reivewpage = 0
        self.rnrStat = None
        self.ratings_average = 0
        self.ratings_total = 0
        self.review_total = 0
        self.downloadrank = 32767
        self.ratingrank = 32767
        self.pointoutrank = 32767
        self.recommendrank = 32767
        self.downloadcount = 0
        self.has_desktop = True
        self.arch = None
        self.system_name = None
        self.cpu_model = None
        self.no_cpu_model = None
        self.depends = None
        self.install_date = ''  # the date first install this app, get from server

        self.status = enums.PkgStates.NOTHING

        self.percent = 0

        self.installed_version = ''
        self.is_installed = False
        self.is_upgradable = False
        self.is_runnable = True
        self.is_removed = False

        self.kydroid_service = None

        self.package = None
        self.icon_path = ""
        self.down_cover_image_path = ""
        self.down_preview_image_path = ""
        self.initialsName = ""
        self.pinyinName = ""
        self.anotherName = ""
        self.certSubjectOu = ""
        self.softStyle = 3
        self.website = None
        self.manufacturers = ""
        self.init_screen_shot = False
        self.popularity = None

    @property
    def name(self):
        return self.pkgname

    @property
    def displayname_cn(self):
        return self.displayname

    @property
    def thumbnail(self):
        # self.thumbnail_url = SCREENSHOT_THUMB_URL % {
        #     'pkgname': self.pkgname,
        #     'version': self.version or 0,
        # }
        return self.thumbnail_url

    @property
    def screenshot(self):
        # self.screenshot_url = SCREENSHOT_LARGE_URL % {
        #     'pkgname': self.pkgname,
        #     'version': self.version or 0,
        # }
        return self.screenshot_url

    @property
    def description(self):
        return self.description_init

    @description.setter
    def description(self, description_set):
        self.description_init = description_set

    @property
    def summary(self):
        return self.summary_init

    @summary.setter
    def summary(self, summary_set):
        self.summary_init = summary_set

    @property
    def packageSize(self):
        return self.size

    @property
    def installedSize(self):
        return self.size

    @property
    def pkg_status(self):
        if (self.is_installed == False):
            return enums.PkgStates.UNINSTALLED
        else:
            if (self.is_upgradable == False):
                return enums.PkgStates.INSTALLED
            else:
                return enums.PkgStates.UPGRADABLE

    def run(self):
        if self.kydroid_service is not None:
            try:
                self.kydroid_service.launch_app(self.name)
            except:
                if (Globals.DEBUG_SWITCH):
                    #print("apk %s run error" % self.name)
                    Globals.LOG_ERROR.logger.error(_("apk %s run error" % self.name))

    def get_total_size(self):
        return self.size

    # get the reviews object list of this application
    def get_reviews(self, page):
        if page in self.reviews:
            return self.reviews[page]
        else:
            return None

    def add_reviews(self, page, reviewlist):
        if page not in self.reviews:
            self.reviews[page] = reviewlist

    def update_cache(self, apt_cache):
        pass


if __name__ == "__main__":
    pass
