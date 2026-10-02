#include "playwidget.h"
#include "ui_playwidget.h"
#include "QToolBar"
#include <QFileInfo>
#include <iostream>
#include <QKeyEvent>
#include <QProcess>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDBusInterface>
#include <QThread>

using namespace std;
WId playWidget::winID = 0;
#define MYHLAYOUT_SIZE_WIDTH 70
#define MYHLAYOUT_SIZE_HEIEGT 250
static GstBusSyncReply create_window(GstBus *bus, GstMessage *message, GstPipeline *pipeline)
{
    if (!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;

    qDebug()<<"Received bus message";

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), playWidget::winID);

    gst_message_unref(message);

    return GST_BUS_DROP;
}

playWidget::playWidget(QWidget *parent) : QWidget(parent),decodeFlag("hard"),
    ui(new Ui::playWidget)
{
    ui->setupUi(this);
    this->initUI();
    this->setProperty("useStyleWindowManager", false);
    this->init();

}


static int calc_delta(int oldval, int newval)
{
    int temp = newval-oldval;
    if(temp>=512||temp<-512)
    {
        return 0;
    }
    if(temp > 127)
    {
        temp = 127;
    }
    else if(temp < -127)
    {
        temp = -127;
    }
    if(temp < 0)
    {
        temp += 256;
    }
    return temp;
}
playWidget::~playWidget()
{

    gst_element_set_state (softPipeline, GST_STATE_NULL);
    gst_object_unref (softPipeline);
    gst_element_set_state (hardPipeline, GST_STATE_NULL);
    gst_object_unref (hardPipeline);
    delete ui;
}
void playWidget::init()
{
    playWidget::winID = ui->widget->winId();
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/src/kylin-projection.svg"), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(icon);
    this->left_time=6;
    QTimer *timer= new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(time_update()));


    timer->start(1000);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
//    QFont ft;
//    ft.setPointSize(11);
//    this->pushButton_changeDecode->setFont(ft);
}

void playWidget::initUI()
{
    pushButton_allscreen    = new QPushButton(this);
    pushButton_changeDecode = new QPushButton (this);
    pushButton_back         = new QPushButton (this);
    pushButton_fun          = new QPushButton (this);
    pushButton_home         = new QPushButton (this) ;

    pushButton_allscreen    ->setText("☆");
    pushButton_changeDecode ->setText(tr("SD"));
    pushButton_back         ->setText("◁");
    pushButton_fun          ->setText("□");
    pushButton_home         ->setText("○");

    pushButton_allscreen    ->setFocusPolicy(Qt::NoFocus);
    pushButton_changeDecode ->setFocusPolicy(Qt::NoFocus);
    pushButton_back         ->setFocusPolicy(Qt::NoFocus);
    pushButton_fun          ->setFocusPolicy(Qt::NoFocus);
    pushButton_home         ->setFocusPolicy(Qt::NoFocus);

    pushButton_allscreen->setStyleSheet("QPushButton{background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);}" );
    pushButton_changeDecode->setStyleSheet("QPushButton{border:0px;background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);}" );
    pushButton_back->setStyleSheet("QPushButton{background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);}" );
    pushButton_fun->setStyleSheet("QPushButton{background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);}" );
    pushButton_home->setStyleSheet("QPushButton{background-color: rgb(0, 0, 0);color:rgb(255, 255, 255);}" );

    pushButton_allscreen->setMaximumSize(QSize(70,50));
    pushButton_changeDecode->setMaximumSize(QSize(70,50));
    pushButton_back->setMaximumSize(QSize(70,50));
    pushButton_fun->setMaximumSize(QSize(70,50));
    pushButton_home->setMaximumSize(QSize(70,50));

    connect(this->pushButton_fun,SIGNAL(clicked()),this,SLOT(fun_clicked()));
    connect(this->pushButton_back,SIGNAL(clicked()),this,SLOT(back_clicked()));
    connect(this->pushButton_home,SIGNAL(clicked()),this,SLOT(home_clicked()));
    connect(this->pushButton_allscreen,SIGNAL(clicked()),this,SLOT(screen_clicked()));
    connect(this->pushButton_changeDecode,SIGNAL(clicked()),this,SLOT(on_pushButton_allscreen_2_clicked()));

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(pushButton_changeDecode);
    mLayout->addWidget(pushButton_allscreen);
    mLayout->addWidget(pushButton_back);
    mLayout->addWidget(pushButton_fun);
    mLayout->addWidget(pushButton_home);
    mLayout->setGeometry(QRect(this->width() - MYHLAYOUT_SIZE_WIDTH,(this->height() - MYHLAYOUT_SIZE_HEIEGT)/2,MYHLAYOUT_SIZE_WIDTH,MYHLAYOUT_SIZE_HEIEGT));
}
void playWidget::setWidgetSize(int x, int y)
{
    ui->widget->resize(x, y);
}
void playWidget::time_update()
{
    if(this->left_time > 0)
    {
        this->left_time--;
    }
    else
    {
        this->pushButton_allscreen->hide();
        this->pushButton_changeDecode->hide();
        this->pushButton_back->hide();
        this->pushButton_fun->hide();
        this->pushButton_home->hide();
        widget_show_flag=0;
        if(uibc==1 && hidc==1)
        {
            if(!this->kylinPc)
                ui->widget->setCursor(QCursor(Qt::BlankCursor));
        }
    }
}
void playWidget::init_mouse()
{
    window_width=this->size().width();
    window_height=this->size().height();
    widget_width=ui->widget->size().width();
    widget_height=ui->widget->size().height();
    if(this->uibc==1 && hidc==1)
    {
        if(!this->kylinPc)
            ui->widget->setCursor(QCursor(Qt::BlankCursor));
    }
}
void playWidget::init_keyboard()
{
}

void playWidget::setenv()
{
    QString LIST_VGA = "lspci";
    QString output="";

    QStringList sl={};
    QProcess proc ;
    proc.start(LIST_VGA);
    proc.waitForFinished();
    QString res=proc.readAllStandardOutput();
    if(res!=NULL&&res.contains("VGA"))
    {
        sl=res.split('\n');
        for(int i=0;i<sl.size();i++)
        {
            if(sl[i].contains("VGA"))
            {
                output+=sl[i]+'\n';
            }
        }
    }
    qDebug()<<"set env";
    qDebug()<<output;

    QString str = "Graphics";
    bool UHD = false;
    if(output.contains(str))
    {
        int pos = output.indexOf(str);
        QString str1 = output.mid(pos+9,1);
        if(str1 == "(")
        {
            UHD = true;
        }
    }


    if(output.contains("AMD"))
    {
        //remove hardsoft decode function on AMD Oland VGA
        if(output.contains("Oland"))
        {
            decodeFlag = "kylin";
        }
        else
        {
            qputenv("LIBVA_DRIVER_NAME", "radeonsi");
            qputenv("GST_VAAPI_ALL_DRIVERS", "1");
        }
    }
    else if(output.contains("Intel"))
    {
        if(UHD)
        {
            decodeFlag = "kylin";
        }else
        {
            qputenv("LIBVA_DRIVER_NAME", "i965");
        }
    }
    else
    {
        decodeFlag = "kylin";
    }
    qDebug()<<decodeFlag;
    qDebug()<<"lenove can not support"<<UHD;
}

void playWidget::preparePipeline()
{
    QString pipe_str="udpsrc port=1991 buffer-size=622080 caps=\"application/x-rtp,clock-rate=90000, media=video\" ! rtpjitterbuffer latency=200  mode=4 ! rtpmp2tdepay ! tsdemux ";
    QString harde_pipe_str ="udpsrc port=1991 buffer-size=622080 caps=\"application/x-rtp,clock-rate=90000, media=video\" ! rtpjitterbuffer latency=200  mode=4 ! rtpmp2tdepay ! tsdemux ";
    if(audio==1 && kylinPc != 1)
    {
        pipe_str += "name=demuxer demuxer. ";
        harde_pipe_str += "name=demuxer demuxer. ";
    }

    pipe_str +=       "! queue name=video_ch max-size-buffers=0 max-size-time=0 ! h264parse ! queue name=dec0 max-size-buffers=0 max-size-time=0 ";
    harde_pipe_str += "! queue name=video_ch max-size-buffers=0 max-size-time=0 ! h264parse ! queue name=dec0 max-size-buffers=0 max-size-time=0 ";


    harde_pipe_str += "! vaapih264dec ";

    pipe_str += "! avdec_h264 max-threads=0 skip-frame=1 output-corrupt=false ";

    pipe_str +=       "! videoconvert n-threads=0 !" ;
    harde_pipe_str += "! videoconvert n-threads=0 !";

    g_autoptr(GstElementFactory) sinkfactor = NULL;
    sinkfactor = gst_element_factory_find("xvimagesink");
    if(sinkfactor && xvFlags)
    {
        pipe_str +=       " video/x-raw,format=YV12 ! xvimagesink force-aspect-ratio=false sync=false async=false ";
        harde_pipe_str += " video/x-raw,format=YV12 ! xvimagesink force-aspect-ratio=false sync=false async=false ";
    }else
    {
        if(scale == 1)
        {
            pipe_str += QString("videoscale method=1 ! video/x-raw,width=%1,height=%2 ! ").arg(arg_width).arg(arg_height);
            harde_pipe_str += QString("videoscale method=1 ! video/x-raw,width=%1,height=%2 ! ").arg(arg_width).arg(arg_height);
        }
        pipe_str +=       " ximagesink force-aspect-ratio=false sync=false async=false ";
        harde_pipe_str += " ximagesink force-aspect-ratio=false sync=false async=false ";
    }

    if(audio==1 && kylinPc != 1)
    {
        pipe_str +=       "demuxer. ! queue max-size-buffers=0 max-size-time=0 ! aacparse ! avdec_aac ! audioconvert ! audioresample ! autoaudiosink ";
        harde_pipe_str += "demuxer. ! queue max-size-buffers=0 max-size-time=0 ! aacparse ! avdec_aac ! audioconvert ! audioresample ! autoaudiosink ";
    }

    ba = pipe_str.toLatin1();
    softPip=ba.data();
    softPipeline = gst_parse_launch(softPip,nullptr);

    ba1 = harde_pipe_str.toLatin1();
    hardPip=ba1.data();
    hardPipeline = gst_parse_launch(hardPip,nullptr);
    gst_pipeline_set_latency(GST_PIPELINE(softPipeline),200 * GST_MSECOND);
    gst_pipeline_set_latency(GST_PIPELINE(softPipeline),200 * GST_MSECOND);

}

bool playWidget::setPlayerPipe(GstElement *pipe)
{
    playWidget::winID = ui->widget->winId();
    bus = gst_element_get_bus(pipe);
    currentPipe = pipe;
    gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window, pipe, NULL);
    gst_bus_add_signal_watch(bus);
    GstStateChangeReturn sret = gst_element_set_state (pipe, GST_STATE_PLAYING);

    GstState state;
    gst_element_get_state(pipe, &state, NULL, 0);
    qDebug() << "State : "<< state;
    if (sret == GST_STATE_CHANGE_FAILURE)
    {
        qDebug() << "set playing state error.";
        return false;
    }
    return true;
}

void playWidget::mousePressEvent(QMouseEvent *e)
{
    QString str="("+QString::number(e->x())+","+QString::number(e->y())+")";
    int button=0;
    int type=0;


    if(hidc)
    {
        if(e->button()==Qt::LeftButton||
                e->button()==Qt::MidButton ||
                e->button()==Qt::RightButton )
        {
            type = cons_leftBtnPressed;
            if (e->button() == Qt::LeftButton)
            {
                button=cons_leftBtn;
            }
            else if (e->button() == Qt::MidButton)
            {
                button=cons_midBtn;
            }
            else if (e->button() == Qt::RightButton)
            {
                button=cons_rightBtn;
            }
        }
        else
        {
            type = cons_noBtnPressed;
        }
        int temp_uibc_x = e->pos().x() - min_hor_pos;
        int temp_uibc_y = e->pos().y() - min_ver_pos;
        int delta_x = 0, delta_y = 0;
        if (uibc_x >= 0 && uibc_y >= 0)
        {
            delta_x = calc_delta(uibc_x, temp_uibc_x);
            delta_y = calc_delta(uibc_y, temp_uibc_y);
        }
        uibc_x = temp_uibc_x;
        uibc_y = temp_uibc_y;
        if(this->kylinPc)
        {
            // qDebug()<<"鼠标点击current_width is "<<current_width<<", current_height is "<<current_height;
            double pr_scale = 1.77777778;
            double current_scale = (double)current_width/(double)current_height;
            // qDebug()<<"----"<<current_scale;
            if(pr_scale < current_scale)            //以高度为准
            {
                int scale_width = current_height*pr_scale;
                int c = (current_width - scale_width)/2;
                // qDebug()<<"以高度为准：";

                // if(e->x() >= c && e->x() <= (c + scale_width))
                // {
                if(button==cons_leftBtn){
                    QString a2 = QString("1,1,%1,%2,0,%3,%4").arg(e->x()- c ).arg(e->y()).arg(scale_width).arg(current_height);
                    cout<<a2.toLatin1().data() << endl;
                }
                else if(button==cons_rightBtn){
                    QString a2 = QString("1,4,%1,%2,0,%3,%4").arg(e->x()- c ).arg(e->y()).arg(scale_width).arg(current_height);
                    cout<<a2.toLatin1().data() << endl;
                }
                
                // }

            }else if(pr_scale > current_scale)      //以宽度为准
            {
                int scale_height = current_width/pr_scale;
                int c = (current_height - scale_height)/2;
                // if(e->y() >= c && e->y() <= (c + scale_height))
                // {
                if(button==cons_leftBtn){
                    QString a2 = QString("1,1,%1,%2,0,%3,%4").arg(e->x()).arg(e->y() - c ).arg(current_width).arg(scale_height);
                    cout <<a2.toLatin1().data() << endl;
                }
                else if(button==cons_rightBtn){
                   QString a2 = QString("1,4,%1,%2,0,%3,%4").arg(e->x()).arg(e->y() - c ).arg(current_width).arg(scale_height);
                    cout <<a2.toLatin1().data() << endl;
                }
                // QString a2 = QString("1,1,%1,%2,0,%3,%4").arg(e->x()).arg(e->y() - c ).arg(current_width).arg(scale_height);
                // cout <<a2.toLatin1().data() << endl;
                // }
            }else 
            {
                if(button==cons_leftBtn){
                    QString a2 = QString("1,1,%1,%2,0,%3,%4").arg(e->x()).arg(e->y()).arg(current_width).arg(current_height);
                    cout<<a2.toLatin1().data() << endl;
                }
                else if(button==cons_rightBtn){
                   QString a2 = QString("1,4,%1,%2,0,%3,%4").arg(e->x()).arg(e->y()).arg(current_width).arg(current_height);
                    cout<<a2.toLatin1().data() << endl;
                }
                // QString a2 = QString("1,1,%1,%2,0,%3,%4").arg(e->x()).arg(e->y()).arg(current_width).arg(current_height);
                // cout<<a2.toLatin1().data() << endl;
            }
        }else
        {
       
        cout << QString("1,%1,%2,%3,%4").arg(type).arg(button).arg(delta_x).arg(delta_y).toLatin1().data() << endl;
        }
    }
    else
    {
        // hidc不等于1的情况 过一阵子再写
        if(e->button()==Qt::LeftButton||
                e->button()==Qt::MidButton ||
                e->button()==Qt::RightButton )
        {
            type=cons_leftBtnPressed;
            if(e->button()==Qt::LeftButton)
            {
                button=cons_leftBtn;
            }
            else if(e->button()==Qt::MidButton)
            {
                button=cons_midBtn;
            }
            else if(e->button()==Qt::RightButton)
            {
                button=cons_rightBtn;
            }
        }
        else
        {
            type=cons_noBtnPressed;
        }
        cout<<QString("%1,1,0,%2,%3").arg(type).arg(e->x()).arg(e->y()).toLatin1().data()<<endl;
    }
}
void playWidget::mouseReleaseEvent(QMouseEvent *e)
{
    int type = cons_noBtnPressed;
    int button = 0;
    if (hidc)
    {
        if(e->button()==Qt::LeftButton||
                e->button()==Qt::MidButton ||
                e->button()==Qt::RightButton )
        {
            if (e->button() == Qt::LeftButton)
            {
                button = cons_leftBtn;
            }
            else if (e->button() == Qt::MidButton)
            {
                button = cons_midBtn;
            }
            else if (e->button() == Qt::RightButton)
            {
                button = cons_rightBtn;
            }
        }
        if(this->kylinPc)
        {
            if(button==cons_leftBtn){
                cout << "1,2,0,0,0,0,0"<< endl;
            }
            else if(button==cons_rightBtn){
                cout << "1,5,0,0,0,0,0"<< endl;
            }
            // cout << "1,2,0,0,0,0,0"<< endl;
        }else
        {
            cout<<QString("%1,%2,%3,0,0").arg(hid_mouse_event).arg(type).arg(button).toLatin1().data()<<endl;
        }
    }else{
        if(e->button()==Qt::LeftButton||
                e->button()==Qt::MidButton ||
                e->button()==Qt::RightButton )
        {
            if(e->button()==Qt::LeftButton)
            {
                button=cons_leftBtn;
            }
            else if(e->button()==Qt::MidButton)
            {
                button=cons_midBtn;
            }
            else if(e->button()==Qt::RightButton)
            {
                button=cons_rightBtn;
            }
        }
        if(this->kylinPc)
        {
            cout << "1,2,0,0,0"<< endl;

        }else
        {
            cout<<QString("%1,1,0,%2,%3").arg(type).arg(e->x()).arg(e->y()).toLatin1().data()<<endl;
        }
    }
}
void playWidget::circleMouse(QMouseEvent *e)
{
    if (this->kylinPc==1)
    {
        return;
    }
    if (this->uibc != 1 || widget_show_flag == 1)
    {
        return;
    }
    if (e->x() < 5) //左
    {
        QCursor::setPos(this->pos().x() + ui->widget->width() - 5, this->pos().y() + e->y());
        circle_flag = 1;
    }
    else if (e->y() < 5) //上
    {
        QCursor::setPos(this->pos().x() + e->x(), this->pos().y() + ui->widget->height() - 5);
        circle_flag = 1;
    }
    else if (e->x() > ui->widget->width() - 5) //右
    {
        QCursor::setPos(this->pos().x() + 5, this->pos().x() + e->y());
        circle_flag = 1;
    }
    else if (e->y() > ui->widget->height() - 5) //下
    {
        QCursor::setPos(this->pos().x() + e->x(), this->pos().y() + 5);
        circle_flag = 1;
    }
}

void playWidget::resizeEvent(QResizeEvent *e)
{
    // qDebug()<< e->size().width()<<"----"<<e->size().height();
    this->setMinimumHeight(270);
    this->setMinimumWidth(480);

    if(updateFlag)
    {
        if(ui->widget->updatesEnabled() == true)
            ui->widget->setUpdatesEnabled(false);
    }
    left_time=6;
    this->pushButton_allscreen->show();
    this->pushButton_changeDecode->show();
    this->pushButton_back->show();
    this->pushButton_fun->show();
    this->pushButton_home->show();
    
    this->current_width = e->size().width();
    this->current_height = e->size().height();
    mLayout->setGeometry(QRect(this->width() - MYHLAYOUT_SIZE_WIDTH,(this->height() - MYHLAYOUT_SIZE_HEIEGT)/2,MYHLAYOUT_SIZE_WIDTH,MYHLAYOUT_SIZE_HEIEGT));
    this->update();
}
void playWidget::mouseMoveEvent(QMouseEvent *e)
{
    static unsigned long s_filterCnt = 0;

    if ((++s_filterCnt % 4) != 1)
        return;

    QRect rect(this->pushButton_changeDecode->pos().x(), this->pushButton_changeDecode->pos().y(), this->pushButton_home->width(), this->pushButton_home->height()*5);
    if (rect.contains(e->pos(), false))
    {
        if (left_time > 0)
        {
            this->left_time=6;
            this->pushButton_allscreen->show();
            this->pushButton_changeDecode->show();
            this->pushButton_back->show();
            this->pushButton_fun->show();
            this->pushButton_home->show();
        }
    }

    if(uibc==1&&hidc==1)
    {
        circleMouse(e);
    }

    if (hidc)
    {
        int type = 0;
        int button = 0;
        int delta_x = 0, delta_y = 0;
        int temp_uibc_x, temp_uibc_y = 0;
        if (e->buttons() & Qt::LeftButton)
        {
            type = cons_leftBtnPressed;
            button = cons_leftBtn;
        }
        else
        {
            type = cons_noBtnPressed;
            button = cons_nullBtn;
        }
        temp_uibc_x = int(e->x() - min_hor_pos);
        temp_uibc_y = int(e->y() - min_ver_pos);
        if (this->uibc_x >= 0 && this->uibc_y >= 0)
        {
            delta_x = calc_delta(this->uibc_x, temp_uibc_x);
            delta_y = calc_delta(this->uibc_y, temp_uibc_y);
            if (delta_x == 0 || delta_y == 0)
            {
                if (circle_flag == 1)
                {
                    delta_x = 0;
                    delta_y = 0;
                    circle_flag = 0;
                }
            }

        }
        this->uibc_x = temp_uibc_x;
        this->uibc_y = temp_uibc_y;
        //
        if(this->kylinPc)
        {
             // qDebug()<<"鼠标点击current_width is "<<current_width<<", current_height is "<<current_height;
            double pr_scale = 1.77777778;
            double current_scale = (double)current_width/(double)current_height;
            // qDebug()<<"----"<<current_scale;
            if(pr_scale < current_scale)            //以高度为准
            {
                int scale_width = current_height*pr_scale;
                int c = (current_width - scale_width)/2;
                // qDebug()<<"以高度为准：";

                // if(e->x() >= c && e->x() <= (c + scale_width))
                // {
                QString a2 = QString("1,0,%1,%2,0,%3,%4").arg(e->x()- c ).arg(e->y()).arg(scale_width).arg(current_height);
                cout<<a2.toLatin1().data() << endl;
                // }

            }else if(pr_scale > current_scale)      //以宽度为准
            {
                int scale_height = current_width/pr_scale;
                int c = (current_height - scale_height)/2;
                // if(e->y() >= c && e->y() <= (c + scale_height))
                // {
                QString a2 = QString("1,0,%1,%2,0,%3,%4").arg(e->x()).arg(e->y() - c ).arg(current_width).arg(scale_height);
                cout <<a2.toLatin1().data() << endl;
                // }
            }else 
            {
                QString a2 = QString("1,0,%1,%2,0,%3,%4").arg(e->x()).arg(e->y()).arg(current_width).arg(current_height);
                cout  <<a2.toLatin1().data() << endl;
            }
            // qDebug()<<"鼠标移动e->x() is "<<e->x()<<", e->y() is "<<e->y();
            // QString a2 = QString("1,0,%1,%2,0,%3,%4").arg(e->x()).arg(e->y()).arg(screen_width).arg(screen_height);
            // cout << a2.toLatin1().data() << endl;
            g_usleep(15000);
        }else
        {
            QString a2 = QString("%1,%2,%3,%4,%5").arg(hid_mouse_event).arg(type).arg(button).arg(delta_x).arg(delta_y);
            cout << a2.toLatin1().data() << endl;
        }
    }

}
void playWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    return;
    //    if(e->button()==Qt::LeftButton)
    //    {
    //        left_time=6;
    //        if(uibc==1)
    //        {
    //            ui->widget->setCursor(QCursor(Qt::ArrowCursor));
    //        }
    //        ui->widget_2->show();
    //        widget_show_flag=1;
    //    }
}
void playWidget::wheelEvent(QWheelEvent *e)
{
    if (hidc)
    {
        int temp_scroll_step = 0;
        int kylin_pc_scroll = 0;
        if (e->delta() > 0)
        {
            temp_scroll_step = 1;
        }
        else if (e->delta() < 0)
        {
            temp_scroll_step = -1;
        }
        else
        {
            temp_scroll_step = 0;
        }
        if (temp_scroll_step != 0)
        {
            if(this->kylinPc)
            {
                if(temp_scroll_step == 1)
                    kylin_pc_scroll = 6;
                else if (temp_scroll_step == -1)
                    kylin_pc_scroll = 7;
                cout << QString("1,%1,0,0,0,0,0").arg(kylin_pc_scroll).toLatin1().data() << endl;
                // qDebug()<<"temp_scroll_step is "<< temp_scroll_step <<"kylin_pc_scroll  is "<<kylin_pc_scroll;

            }else
            {
                cout << QString("1,%1,4,0,0,%2").arg(cons_wheelRoll).arg(temp_scroll_step).toLatin1().data() << endl;

            }
        }
    }
}

void playWidget::keyPressEvent(QKeyEvent *e)
{

    if (e->key() == Qt::Key_Escape)
    {
        left_time=6;
        if(uibc==1&& hidc==1)
        {
            ui->widget->setCursor(QCursor(Qt::ArrowCursor));
        }
        this->pushButton_allscreen->show();
        this->pushButton_changeDecode->show();
        this->pushButton_back->show();
        this->pushButton_fun->show();
        this->pushButton_home->show();
        widget_show_flag = 1;
        updateFlag = true;
        return;
    }

    if (uibc && hidc)
    {
        if(this->kylinPc)
        {
            QString strI02 = QString::number(e->key());
            QString a1 = QString("0,0,%1").arg(strI02);
            cout << a1.toLatin1().data() << endl;
        }else
        {
            if (e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A)
            {
                if (e->nativeModifiers() == 0 || e->nativeModifiers() == 16 || e->nativeModifiers() == 3 || e->nativeModifiers() == 19)
                {
                    QString strI02 = QString::number(e->key() + 32, 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyPressed).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
                else if (e->nativeModifiers() == 2 || e->nativeModifiers() == 18 || e->nativeModifiers() == 1 || e->nativeModifiers() == 17)
                {
                    QString strI02 = QString::number(e->key(), 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyPressed).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
                else
                {
                    QString strI02 = QString::number(e->key(), 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyPressed).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
            }
            else
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1=QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyPressed).arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
        }
    }
    else if (uibc)
    {
        if(e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A)
        {
            if(e->nativeModifiers() == 0 || e->nativeModifiers() == 16 || e->nativeModifiers() == 3 || e->nativeModifiers() == 19)                //小写
            {
                QString strI02 = QString::number(e->key() + 32, 16);
                QString a1=QString("3,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
            else if(e->nativeModifiers() == 2 || e->nativeModifiers() == 18 || e->nativeModifiers() == 1 || e->nativeModifiers() == 17)
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1=QString("3,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
            else
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1=QString("3,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
        }
        else if(e->key()>=Qt::Key_Space && e->key() <= Qt::Key_AsciiTilde)
        {
            QString strI02 = QString::number(e->key(), 16);
            QString a1=QString("3,0x%1,0x0000").arg(strI02);
            cout<<a1.toLatin1().data()<<endl;
        }
        else if(e->key() == Qt::Key_Backspace)
        {
            QString a1=QString("3,0x08,0x0000");
            cout<<a1.toLatin1().data()<<endl;
        }
        else if(e->key() == Qt::Key_Delete)
        {
            QString a1=QString("3,0x7F,0x0000");
            cout<<a1.toLatin1().data()<<endl;
        }
    }
}

void playWidget::keyReleaseEvent(QKeyEvent *e)
{
    if(uibc&&hidc)
    {
        if(this->kylinPc)
        {
            QString strI02 = QString::number(e->key());
            QString a1 = QString("0,1,%1").arg(strI02);
            cout << a1.toLatin1().data() << endl;
        }else
        {
            if (e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A)
            {
                if (e->nativeModifiers() == 0 || e->nativeModifiers() == 16 || e->nativeModifiers() == 3 || e->nativeModifiers() == 19)
                {
                    QString strI02 = QString::number(e->key() + 32, 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyReleased).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
                else if (e->nativeModifiers() == 2 || e->nativeModifiers() == 18 || e->nativeModifiers() == 1 || e->nativeModifiers() == 17)
                {
                    QString strI02 = QString::number(e->key(), 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyReleased).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
                else
                {
                    QString strI02 = QString::number(e->key(), 16);
                    QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyReleased).arg(strI02);
                    cout << a1.toLatin1().data() << endl;
                }
            }
            else
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1 = QString("%1,%2,0x%3").arg(hid_key_event).arg(cons_keyReleased).arg(strI02);
                cout << a1.toLatin1().data() << endl;
            }
        }

    }
    else if (uibc)
    {
        if(e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A)
        {

            if(e->nativeModifiers() == 0 || e->nativeModifiers() == 16 || e->nativeModifiers() == 3 || e->nativeModifiers() == 19)                //小写
            {
                QString strI02 = QString::number(e->key() + 32, 16);
                QString a1=QString("4,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
            else if(e->nativeModifiers() == 2 || e->nativeModifiers() == 18 || e->nativeModifiers() == 1 || e->nativeModifiers() == 17)
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1=QString("4,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
            else
            {
                QString strI02 = QString::number(e->key(), 16);
                QString a1=QString("4,0x%1,0x0000").arg(strI02);
                cout<<a1.toLatin1().data()<<endl;
            }
        }
        else if(e->key()>=Qt::Key_Space && e->key() <= Qt::Key_AsciiTilde)
        {
            QString strI02 = QString::number(e->key(), 16);
            QString a1=QString("4,0x%1,0x0000").arg(strI02);
            cout<<a1.toLatin1().data()<<endl;
        }
        else if(e->key() == Qt::Key_Backspace)
        {
            QString a1=QString("4,0x08,0x0000");
            cout<<a1.toLatin1().data()<<endl;
        }
        else if(e->key() == Qt::Key_Delete)
        {
            QString a1=QString("4,0x7F,0x0000");
            cout<<a1.toLatin1().data()<<endl;
        }
    }
}
void playWidget::setargv(int audio, int scale, int uibc, int hidc, int resolution, int arg_width, int arg_height, int arg_port, QString decodeFlag, int kylinPc,bool xv)
{
    this->audio = audio;
    this->scale = scale;
    this->uibc = uibc;
    this->hidc = hidc;
    this->resolution = resolution;
    this->arg_width = arg_width;
    this->arg_height = arg_height;
    this->arg_port = arg_port;
    this->decodeFlag = decodeFlag;
    this->kylinPc = kylinPc;
    this->xvFlags = xv;
    qDebug()<<"current decode flag : "<<this->decodeFlag;

    preparePipeline();
    setPlayerPipe(softPipeline);
    qDebug()<<softPip;
}
void playWidget::back_clicked()
{
    left_time=6;
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_F3, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyPress);
    QKeyEvent keyRealease(QEvent::KeyRelease, Qt::Key_F3, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyRealease);
}
void playWidget::home_clicked()
{
    left_time=6;
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyPress);
    QKeyEvent keyRealease(QEvent::KeyRelease, Qt::Key_F1, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyRealease);
}

void playWidget::fun_clicked()
{
    left_time=6;
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyPress);
    QKeyEvent keyRealease(QEvent::KeyRelease, Qt::Key_F2, Qt::NoModifier, QString());
    QCoreApplication::sendEvent(this, &keyRealease);
}
void playWidget::screen_clicked()
{
    updateFlag = true;
    left_time=6;
    if(uibc==1&&hidc==1)
    {
        ui->widget->setCursor(QCursor(Qt::ArrowCursor));
    }
    if (this->isFullScreen())
    {
        this->setWindowFlags(Qt::Widget);
        this->showNormal();
        this->update();
    }
    else
    {
        this->setWindowFlags(Qt::Dialog);
        this->setWindowFlags(Qt::Window);
        this->showFullScreen();
        this->update();

    }
}

void playWidget::on_pushButton_allscreen_2_clicked()
{
    QLocale locale;
//    QFont ft;
//    ft.setPointSize(11);
//    this->pushButton_changeDecode->setFont(ft);
    if(this->pushButton_changeDecode->text() == tr("SD"))
    {
        if(decodeFlag == "hard")
        {
            gst_element_set_state (softPipeline, GST_STATE_NULL);
            gst_object_unref (softPipeline);
            softPipeline =  NULL;

            hardPipeline = gst_parse_launch(hardPip,nullptr);
            bool a = setPlayerPipe(hardPipeline);
            qDebug()<<hardPip;
            if(a)
                this->pushButton_changeDecode->setText(tr("HD"));
        }else
        {
            if (locale.language() == QLocale::Chinese){
                QMessageBox::warning(this,"提示","当前设备不支持硬解码","是");
            }else if (locale.language() == QLocale::English){
                QMessageBox::warning(this,"warning","The current device does not support hard decoding","Yes");
            }
            qDebug()<<"current device can not support hard decode";
        }

    }else{

        gst_element_set_state (hardPipeline, GST_STATE_NULL);
        gst_object_unref (hardPipeline);
        hardPipeline =  NULL;
        softPipeline = gst_parse_launch(softPip,nullptr);
        qDebug()<<softPip;
        bool a = setPlayerPipe(softPipeline);
        if(a)
            this->pushButton_changeDecode->setText(tr("SD"));
    }
}
void playWidget::closeEvent(QCloseEvent *event)
{
    cout<<"closed"<<endl;
    QThread::sleep(1);
    qDebug()<<"player quit"<<endl;

    QDBusInterface *Interface = new QDBusInterface("org.freedesktop.miracleagent",
                                             "/org/freedesktop/miracleagent",
                                             "org.freedesktop.miracleagent.op",
                                             QDBusConnection::sessionBus());
    Interface->call("disconnect");
}


