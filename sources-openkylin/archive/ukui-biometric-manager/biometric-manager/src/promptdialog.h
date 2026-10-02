/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef PROMPTDIALOG_H
#define PROMPTDIALOG_H

#include "customtype.h"

#include <QDialog>

namespace Ui
{
class PromptDialog;
}

/**
 * @brief UpdateStauts调用返回的结果
 */
struct StatusReslut
{
    int result;
    int enable;
    int devNum;
    int devStatus;
    int opsStatus;
    int notifyMessageId;
};

/**
 * @brief 识别操作（Identify）的ops状态
 */
/* 定义操作类型 */
typedef enum
{
    OPS_TYPE_COMM = 0,
    OPS_TYPE_OPEN,
    OPS_TYPE_ENROLL,
    OPS_TYPE_VERIFY,
    OPS_TYPE_IDENTIFY,
    OPS_TYPE_CAPTURE,
    OPS_TYPE_SEARCH,
    OPS_TYPE_CLEAN,
    OPS_TYPE_GET_FLIST,
    OPS_TYPE_RENAME,
    OPS_TYPE_CLOSE,
} BioOpsType;

/*
 * 定义各种操作结果
 */
typedef enum
{
    OPS_COMM_SUCCESS = OPS_TYPE_COMM * 100, /** 空闲状态 **/
    OPS_COMM_FAIL,                          /** 操作失败 **/
    OPS_COMM_NO_MATCH = OPS_COMM_FAIL,      /** 不匹配 **/
    OPS_COMM_ERROR,                         /** 通用操作错误 **/
    OPS_COMM_STOP_BY_USER,                  /** 用户取消 **/
    OPS_COMM_TIMEOUT,                       /** 操作超时 **/
    OPS_COMM_OUT_OF_MEM,                    /** 无法分配内存 **/
    OPS_COMM_MAX,

    OPS_OPEN_SUCCESS = OPS_TYPE_OPEN * 100, /** 打开设备完成 **/
    OPS_OPEN_FAIL,                          /** 打开设备失败 **/
    OPS_OPEN_ERROR,                         /** 打开设备遇到错误 **/
    OPS_OPEN_MAX,

    OPS_ENROLL_SUCCESS = OPS_TYPE_ENROLL * 100, /** 录入信息成功 **/
    OPS_ENROLL_FAIL,                            /** 录入失败 **/
    OPS_ENROLL_ERROR,                           /** 录入过程中遇到错误 **/
    OPS_ENROLL_STOP_BY_USER,                    /** 录入被用户中断 **/
    OPS_ENROLL_TIMEOUT,                         /** 操作超时 **/
    OPS_ENROLL_MAX,

    OPS_VERIFY_MATCH = OPS_TYPE_VERIFY * 100, /** 认证匹配 **/
    OPS_VERIFY_NO_MATCH,                      /** 认证不匹配 **/
    OPS_VERIFY_ERROR,                         /** 认证过程中遇到错误 **/
    OPS_VERIFY_STOP_BY_USER,                  /** 认证被用户中断 **/
    OPS_VERIFY_TIMEOUT,                       /** 操作超时 **/
    OPS_VERIFY_MAX,

    OPS_IDENTIFY_MATCH = OPS_TYPE_IDENTIFY * 100, /** 识别到指定特征 **/
    OPS_IDENTIFY_NO_MATCH,                        /** 未识别出指定特征 **/
    OPS_IDENTIFY_ERROR,                           /** 识别过程中遇到错误 **/
    OPS_IDENTIFY_STOP_BY_USER,                    /** 识别被用户中断 **/
    OPS_IDENTIFY_TIMEOUT,                         /** 操作超时 **/
    OPS_IDENTIFY_MAX,

    OPS_CAPTURE_SUCCESS = OPS_TYPE_CAPTURE * 100, /** 捕获成功 **/
    OPS_CAPTURE_FAIL,                             /** 捕获失败 **/
    OPS_CAPTURE_ERROR,                            /** 捕获过程中遇到错误 **/
    OPS_CAPTURE_STOP_BY_USER,                     /** 捕获被用户中断 **/
    OPS_CAPTURE_TIMEOUT,                          /** 操作超时 **/
    OPS_CAPTURE_MAX,

    OPS_SEARCH_MATCH = OPS_TYPE_SEARCH * 100, /** 搜索到指定特征 **/
    OPS_SEARCH_NO_MATCH,                      /** 未搜索到指定特征 **/
    OPS_SEARCH_ERROR,                         /** 搜索过程中遇到错误 **/
    OPS_SEARCH_STOP_BY_USER,                  /** 搜索被用户中断 **/
    OPS_SEARCH_TIMEOUT,                       /** 操作超时 **/
    OPS_SEARCH_MAX,

    OPS_CLEAN_SUCCESS = OPS_TYPE_CLEAN * 100, /** 清理特征成功 **/
    OPS_CLEAN_FAIL,                           /** 清理失败 **/
    OPS_CLEAN_ERROR,                          /** 清理过程中遇到错误 **/
    OPS_CLEAN_STOP_BY_USER,                   /** 清理被用户中断 **/
    OPS_CLEAN_TIMEOUT,                        /** 操作超时 **/
    OPS_CLEAN_MAX,

    OPS_GET_FLIST_SUCCESS = OPS_TYPE_GET_FLIST * 100, /** 获取特征列表完成 **/
    OPS_GET_FLIST_FAIL,                               /** 获取特征列表失败 **/
    OPS_GET_FLIST_ERROR,                              /** 获取特征列表过程中遇到错误 **/
    OPS_GET_FLIST_STOP_BY_USER,                       /** 获取特征列表被用户中断 **/
    OPS_GET_FLIST_TIMEOUT,                            /** 获取特征列表超时 **/
    OPS_GET_FLIST_MAX,

    OPS_RENAME_SUCCESS = OPS_TYPE_RENAME * 100, /** 重命名特征完成 **/
    OPS_RENAME_FAIL,                            /** 重命名特征失败 **/
    OPS_RENAME_ERROR,                           /** 重命名特征过程中遇到错误 **/
    OPS_RENAME_STOP_BY_USER,                    /** 重命名特征被用户中断 **/
    OPS_RENAME_TIMEOUT,                         /** 重命名特征超时 **/
    OPS_RENAME_MAX,

    OPS_CLOSE_SUCCESS = OPS_TYPE_CLOSE * 100, /** 关闭设备完成 **/
    OPS_CLOSE_FAIL,                           /** 关闭设备失败 **/
    OPS_CLOSE_ERROR,                          /** 关闭设备过程中遇到错误 **/
    OPS_CLOSE_MAX,
} OpsResult;

class PromptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PromptDialog(QDBusInterface *service, int bioType, int deviceId, int uid, QWidget *parent = 0);
    ~PromptDialog();
    enum Result
    {
        SUCESS,
        ERROR,
        UNDEFINED
    };

public:
    void setTitle(int opsType);
    void setPrompt(const QString &text);
    void setProcessed(bool val);
    void setIsFace(bool val);

    int enroll(int drvId, int uid, int idx, const QString &idxName);
    int verify(int drvId, int uid, int idx);
    int search(int drvId, int uid, int idxStart, int idxEnd);
    Result getResult();

protected:
    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *editor, QEvent *event);

private:
    void setFailed();
    QString getGif(int type);
    QString getImage(int type);
    void getEnrollImageList(int type);
    void handleErrorResult(int error);
    void showClosePrompt(QString tips);
    void setSearchResult(bool isAdmin, const QList<SearchResult> &searchResultList);
    void chkSysState();
    QPixmap PixmapToRound(const QPixmap &src, int radius);
    void updatePixmap();
    void setTipPixmap(int type = UNDEFINED);
    QStringList getAllFeatureinfolist(int uid, int indexStart, int indexEnd, int driver_type, QList<int> &idxList);
    void resetUI();
    /**
     * @brief UpdateStatus  获取更新的设备状态
     * @param drvid 驱动id
     * @return  结果：<int result, int enable, int devNum,
     *                int devStatus, int opsStatus, notifyMessageId, ...>
     */
    StatusReslut UpdateStatus(int drvid);
    void updateEnrollImage();

Q_SIGNALS:
    void enrollFinished();

signals:
    void canceled(); /* 自定义信号，用于触发主窗口内的 slot */

private slots:
    void on_btnClose_clicked();
    void on_Ctbtn_clicked();
    void on_Fsbtn_clicked();
    void on_cancelBtn_clicked();
    void onStatusChanged(int, int);
    void onProcessChanged(int, QString, int, QString);
    void onFrameWritten(int);
    void onEnrollFrameWritten(int);
    void enrollCallBack(const QDBusMessage &);
    void verifyCallBack(const QDBusMessage &);
    void searchCallBack(const QDBusMessage &);
    void StopOpsCallBack(const QDBusMessage &);
    void errorCallBack(const QDBusError &);
    void onPrepareForSleep(bool sleep);
    void onSysLock();
    void onSysUnLock();
    void switchEnrollImage();

private:
    Ui::PromptDialog *ui;
    QDBusInterface *serviceInterface;
    QMovie *movie;
    int type;
    int deviceId;
    int uid;
    bool isEnrolling;
    enum OPS
    {
        IDLE,
        ENROLL,
        VERIFY,
        SEARCH
    } ops,
        m_hisOps;
    Result opsResult;
    bool isProcessed;
    bool isFace = false;
    int fd = -1;
    int dup_fd = -1;
    bool m_isUseEnrollFd = false;
    bool isEnroll = false;
    enum SysState_e
    {
        SYSSTATE_LOCK = 0x01,
        SYSSTATE_SLEEP = 0x02,
    };
    unsigned m_uSysState = 0;
    QList<QVariant> m_listEnrollArgs;
    QList<QVariant> m_listVerifyArgs;
    QList<QVariant> m_listSearchArgs;

    QPixmap m_waitingPixmap;
    QTimer *w_timer;

    //  指纹录入使用svg图片实现动图效果
    QStringList m_enrollImagePaths;
    int m_currentImage;
    QTimer *m_enrollTimer;
    bool m_enrollSort = true;
    bool m_enrollFinish = false;
};

#endif // PROMPTDIALOG_H
