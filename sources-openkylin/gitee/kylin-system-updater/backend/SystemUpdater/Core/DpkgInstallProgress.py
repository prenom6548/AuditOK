import os
from gettext import gettext as _
import apt
import logging
import fcntl
import apt_pkg


class LogInstallProgress(apt.progress.base.InstallProgress):
    """ Install progress that writes to self.progress_log
        (/var/run/unattended-upgrades.progress by default)
    """
    def __init__(self,file):
        # type: (str) -> None
        apt.progress.base.InstallProgress.__init__(self)
        self.output_logfd = None  # type: int
        self.filename=file
        self.error_pkg=""
        self.errormsg=""
        # raise Exception("for test!!!")

    def error(self,pkg, errormsg):
        logging.error(("Install mode - dpkg, Install error: %s"), errormsg)
        self.error_pkg=self.filename
        self.errormsg = errormsg

    def status_change(self, pkg, percent, status):
        # type: (str, float, str) -> None
        logging.info(("pkg:%s,percent:%s,status:%s"),pkg,percent,status)
        with open(self.progress_log, "w") as f:
            f.write(("%s")%percent)
            f.write(_("当前进度: %s ，正在安装：%s，当前状态：%s") % (percent, pkg,status))
            f.write(_("Progress: %s %% (%s)") % (percent, pkg))

    def _fixup_fds(self):
        # () -> None
        required_fds = [0, 1, 2,  # stdin, stdout, stderr
                        self.writefd,
                        self.write_stream.fileno(),
                        self.statusfd,
                        self.status_stream.fileno()
                        ]
        # ensure that our required fds close on exec
        for fd in required_fds[3:]:
            old_flags = fcntl.fcntl(fd, fcntl.F_GETFD)
            fcntl.fcntl(fd, fcntl.F_SETFD, old_flags | fcntl.FD_CLOEXEC)
        # close all fds
        try:
            # PEP-446 implemented in Python 3.4 made all descriptors
            # CLOEXEC, but we need to be able to pass writefd to dpkg
            # when we spawn it
            os.set_inheritable(self.writefd, True)
        except AttributeError:  # if we don't have os.set_inheritable()
            pass
        proc_fd = "/proc/self/fd"
        if os.path.exists(proc_fd):
            error_count = 0
            for fdname in os.listdir(proc_fd):
                try:
                    fd = int(fdname)
                except Exception:
                    print("ERROR: can not get fd for %s" % fdname)
                if fd in required_fds:
                    continue
                try:
                    os.close(fd)
                    # print("closed: ", fd)
                except OSError as e:
                    # there will be one fd that can not be closed
                    # as its the fd from pythons internal diropen()
                    # so its ok to ignore one close error
                    error_count += 1
                    if error_count > 1:
                        print("ERROR: os.close(%s): %s" % (fd, e))

    def _redirect_stdin(self):
        # type: () -> None
        REDIRECT_INPUT = os.devnull
        fd = os.open(REDIRECT_INPUT, os.O_RDWR)
        os.dup2(fd, 0)

    def _redirect_output(self):
        # type: () -> None
        # do not create log in dry-run mode, just output to stdout/stderr
        if not apt_pkg.config.find_b("Debug::pkgDPkgPM", False):
            logfd = self._get_logfile_dpkg_fd()
            os.dup2(logfd, 1)
            os.dup2(logfd, 2)

    def _get_logfile_dpkg_fd(self):
        # type: () -> int
        logfd = os.open(
            "/var/log/kylin-system-updater/kylin-system-updater.log.1", os.O_RDWR | os.O_APPEND | os.O_CREAT, 0o640)
        try:
            import grp
            adm_gid = grp.getgrnam("adm").gr_gid
            os.fchown(logfd, 0, adm_gid)
        except (KeyError, OSError):
            pass
        return logfd

    def update_interface(self):
        # type: () -> None
        # call super class first
        apt.progress.base.InstallProgress.update_interface(self)

    def _log_in_dpkg_log(self, msg):
        # type: (str) -> None
        logfd = self._get_logfile_dpkg_fd()
        os.write(logfd, msg.encode("utf-8"))
        os.close(logfd)

    def finish_update(self):
        # if error_status == 1:
        #     os._exit(1)
        pass

    def fork(self):
        pid = os.fork()
        if pid == 0:
            self._fixup_fds()
            self._redirect_stdin()
            self._redirect_output()
        return pid