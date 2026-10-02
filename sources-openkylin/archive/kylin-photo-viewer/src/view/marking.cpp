/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "marking.h"
#include "global/interactiveqml.h"
Marking::Marking(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_mark");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_mark");
    this->setAccessibleDescription("mark widget");

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocusPolicy(Qt::NoFocus);

    //布局
    m_layoutWid = new QWidget(this);
    m_layoutWid->setObjectName("Kylin-photo-viewer_QWidget_layoutWid");
    m_layoutWid->setAccessibleName("Kylin-photo-viewer_QWidget_layoutWid");
    m_layoutWid->setAccessibleDescription("layout widget");
    m_layout = new QHBoxLayout(this);
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setObjectName("Kylin-photo-viewer_QButtonGroup_buttonGroup");


    //矩形
    m_rectangle = new QCheckBox(this);
    m_rectangle->setObjectName("Kylin-photo-viewer_QCheckBox_rectangle");
    m_rectangle->setAccessibleName("Kylin-photo-viewer_QCheckBox_rectangle");
    m_rectangle->setAccessibleDescription("rectangle checkbox");
    m_rectangle->setFocusPolicy(Qt::NoFocus);
    m_rectangle->setAttribute(Qt::WA_TranslucentBackground);
    m_list.append(m_rectangle);
    //    m_map.insert(m_rectangle,"rectangle");
    //椭圆形
    m_ellipse = new QCheckBox(this);
    m_ellipse->setObjectName("Kylin-photo-viewer_QCheckBox_ellipse");
    m_ellipse->setAccessibleName("Kylin-photo-viewer_QCheckBox_ellipse");
    m_ellipse->setAccessibleDescription("ellipse checkbox");
    m_ellipse->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_ellipse);
    //    m_map.insert(m_ellipse,"ellipse");
    //线段
    m_segment = new QCheckBox(this);
    m_segment->setObjectName("Kylin-photo-viewer_QCheckBox_segment");
    m_segment->setAccessibleName("Kylin-photo-viewer_QCheckBox_segment");
    m_segment->setAccessibleDescription("segment checkbox");
    m_segment->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_segment);
    //    m_map.insert(m_segment,"segment");
    //箭头
    m_arrow = new QCheckBox(this);
    m_arrow->setObjectName("Kylin-photo-viewer_QCheckBox_arrow");
    m_arrow->setAccessibleName("Kylin-photo-viewer_QCheckBox_arrow");
    m_arrow->setAccessibleDescription("arrow checkbox");
    m_arrow->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_arrow);
    //    m_map.insert(m_arrow,"arrow");
    //画笔
    m_paintBrush = new QCheckBox(this);
    m_paintBrush->setObjectName("Kylin-photo-viewer_QCheckBox_paintBrush");
    m_paintBrush->setAccessibleName("Kylin-photo-viewer_QCheckBox_paintBrush");
    m_paintBrush->setAccessibleDescription("paintBrush checkbox");
    m_paintBrush->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_paintBrush);
    //    m_map.insert(m_paintBrush,"paintBrush");
    // mark条
    m_markArticle = new QCheckBox(this);
    m_markArticle->setObjectName("Kylin-photo-viewer_QCheckBox_markArticle");
    m_markArticle->setAccessibleName("Kylin-photo-viewer_QCheckBox_markArticle");
    m_markArticle->setAccessibleDescription("markArticle checkbox");
    m_markArticle->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_markArticle);
    //    m_map.insert(m_markArticle,"markArticle");
    //文字
    m_text = new QCheckBox(this);
    m_text->setObjectName("Kylin-photo-viewer_QCheckBox_text");
    m_text->setAccessibleName("Kylin-photo-viewer_QCheckBox_text");
    m_text->setAccessibleDescription("text checkbox");
    m_text->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_text);
    //    m_map.insert(m_text,"text");
    //模糊
    m_blur = new QCheckBox(this);
    m_blur->setObjectName("Kylin-photo-viewer_QCheckBox_blur");
    m_blur->setAccessibleName("Kylin-photo-viewer_QCheckBox_blur");
    m_blur->setAccessibleDescription("blur checkbox");
    m_blur->setFocusPolicy(Qt::NoFocus);
    m_list.append(m_blur);
    //    m_map.insert(m_blur,"blur");
    //撤销
    m_undo = new QPushButton(this);
    m_undo->setObjectName("Kylin-photo-viewer_QPushButton_undo");
    m_undo->setAccessibleName("Kylin-photo-viewer_QPushButton_undo");
    m_undo->setAccessibleDescription("undo");
    m_undo->setFocusPolicy(Qt::NoFocus);
    //退出标注栏
    m_exit = new QPushButton(this);
    m_exit->setObjectName("Kylin-photo-viewer_QPushButton_exit");
    m_exit->setAccessibleName("Kylin-photo-viewer_QPushButton_exit");
    m_exit->setAccessibleDescription("exit");
    m_exit->setFocusPolicy(Qt::NoFocus);
    //完成
    m_finish = new QPushButton(this);
    m_finish->setObjectName("Kylin-photo-viewer_QPushButton_finish");
    m_finish->setAccessibleName("Kylin-photo-viewer_QPushButton_finish");
    m_finish->setAccessibleDescription("finish");
    m_finish->setFixedSize(62, 26);
    m_finish->setFocusPolicy(Qt::NoFocus);
    m_finish->setText("完成");
    //竖线
    m_line = new QLabel(this);
    m_line->setObjectName("Kylin-photo-viewer_QLabel_line");
    m_line->setAccessibleName("Kylin-photo-viewer_QLabel_line");
    m_line->setAccessibleDescription("line");
    m_line->setFixedSize(TOOL_LINE);
    //绘制阴影
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(TOOL_COLOR);       //设置阴影颜色，也可以setColor(QColor(220,220,220))
    effect->setBlurRadius(BLUR_RADIUS); //设定阴影的模糊半径，数值越大越模糊
    this->setGraphicsEffect(effect);
    this->initGsetting();
    this->resetMarkWidQss();
    //    this->buttonGroup();
    this->initConnect();
    //    this->toSetLayout();
}
//布局
void Marking::toSetLayout()
{
    int spaceSize = 20;
    int lineSpace = spaceSize / 4;
    if (platForm.contains(Variable::platForm)) {
        spaceSize = 16;
        lineSpace = 16;
        m_layout->setContentsMargins(10, 8, 8, 10);
    } else {
        spaceSize = 20;
        lineSpace = spaceSize / 4;
        m_layout->setContentsMargins(18, 12, 18, 18);
    }

    m_layout->addWidget(m_rectangle, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_ellipse, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_segment, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_arrow, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_paintBrush, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_markArticle, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_text, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_blur, 0, Qt::AlignCenter);
    m_layout->setSpacing(lineSpace);
    m_layout->addWidget(m_line, 0, Qt::AlignCenter);
    m_layout->setSpacing(lineSpace);
    m_layout->addWidget(m_undo, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_exit, 0, Qt::AlignCenter);
    m_layout->setSpacing(spaceSize);
    m_layout->addWidget(m_finish, 0, Qt::AlignCenter);

    m_layoutWid->setLayout(m_layout);
    m_layoutWid->resize(this->width(), this->height());
    m_layoutWid->move(0, 0);
}
//初始化信号槽
void Marking::initConnect()
{
    connect(m_rectangle, &QCheckBox::clicked, this, &Marking::rectangle);
    connect(m_ellipse, &QCheckBox::clicked, this, &Marking::ellipse);
    connect(m_segment, &QCheckBox::clicked, this, &Marking::segment);
    connect(m_arrow, &QCheckBox::clicked, this, &Marking::arrow);
    connect(m_paintBrush, &QCheckBox::clicked, this, &Marking::paintBrush);
    connect(m_markArticle, &QCheckBox::clicked, this, &Marking::markArticle);
    connect(m_text, &QCheckBox::clicked, this, &Marking::text);
    connect(m_blur, &QCheckBox::clicked, this, &Marking::blur);
    connect(m_undo, &QPushButton::clicked, this, &Marking::undo);
    connect(m_exit, &QPushButton::clicked, this, &Marking::exit);
    connect(m_finish, &QPushButton::clicked, this, &Marking::finish);
}
//点击某个按钮，另外的按钮清除选中状态
void Marking::buttonGroup(bool isChecked, const QCheckBox *checkbox)
{
    for (int i = 0; i < m_list.length(); i++) {
        if (checkbox != m_list[i]) {
            m_list[i]->setChecked(isChecked);
        }
    }
}
//保存选中的按钮
void Marking::recordButtonState(int type, bool buttonState)
{
    if (buttonState) {
        recordDate.type = type;
        InteractiveQml::getInstance()->setPainterType(type);
    } else {
        recordDate.type = -1;
        InteractiveQml::getInstance()->setPainterType(-1);
    }
}

//矩形
void Marking::rectangle()
{
    buttonGroup(false, m_rectangle);
    recordButtonState(0, m_rectangle->isChecked());
    Q_EMIT showSetBar(m_rectangle->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX, 20);
}
//椭圆
void Marking::ellipse()
{
    buttonGroup(false, m_ellipse);
    recordButtonState(1, m_ellipse->isChecked());
    Q_EMIT showSetBar(m_ellipse->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth, 50 + m_positionCoefficient);
}
//线段
void Marking::segment()
{
    buttonGroup(false, m_segment);
    recordButtonState(2, m_segment->isChecked());
    Q_EMIT showSetBar(m_segment->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth * 2, 80 + m_positionCoefficient * 2);
}
//箭头
void Marking::arrow()
{
    buttonGroup(false, m_arrow);
    recordButtonState(3, m_arrow->isChecked());
    Q_EMIT showSetBar(m_arrow->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth * 3, 105 + m_positionCoefficient * 3);
}
//画笔
void Marking::paintBrush()
{
    buttonGroup(false, m_paintBrush);
    recordButtonState(4, m_paintBrush->isChecked());
    Q_EMIT showSetBar(m_paintBrush->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth * 4, 135 + m_positionCoefficient * 4);
}
// mark条
void Marking::markArticle()
{
    buttonGroup(false, m_markArticle);
    recordButtonState(5, m_markArticle->isChecked());
    Q_EMIT showSetBar(m_markArticle->isChecked(), false);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth * 5, 165 + m_positionCoefficient * 5);
}
//文字
void Marking::text()
{
    buttonGroup(false, m_text);
    recordButtonState(6, m_text->isChecked());
    Q_EMIT showSetBar(m_text->isChecked(), true);
    Q_EMIT showSetPos(m_triangleStartX + m_triangleWidth * 6, 195 + m_positionCoefficient * 6);
}
//模糊
void Marking::blur()
{
    buttonGroup(false, m_blur);
    recordButtonState(7, m_blur->isChecked());
    Q_EMIT showSetBar(false, false);
}
//撤销
void Marking::undo()
{
    //    InteractiveQml::getInstance()->setPainterType(8);
    Q_EMIT undoSignal();
}
//退出标注
void Marking::exit()
{
    //    InteractiveQml::getInstance()->setPainterType(9);
    Variable::g_needEnterSign = false;
    Q_EMIT exitSign();
}
//完成
void Marking::finish()
{
    Variable::g_needEnterSign = false;
    //    InteractiveQml::getInstance()->setPainterType(10);
    Q_EMIT finishSign();
}

//监听主题
void Marking::initGsetting()
{
    if (QGSettings::isSchemaInstalled(Variable::FITTHEMEWINDOW.toLocal8Bit())) {
        m_pGsettingThemeData = new QGSettings(Variable::FITTHEMEWINDOW.toLocal8Bit());
        connect(m_pGsettingThemeData, &QGSettings::changed, this, &Marking::dealSystemGsettingChange);
    }
}

void Marking::dealSystemGsettingChange(const QString key)
{
    if (key == "styleName") {
        changeStyle();
    }
}
void Marking::changeStyle()
{
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    if ("ukui-dark" == nowThemeStyle || "ukui-black" == nowThemeStyle) {
        m_layoutWid->setStyleSheet("QWidget{background-color:rgba(0,0,0,1);border-radius:" + widRadius + ";}");
        m_line->setStyleSheet("QLabel{border: 1px solid #393939;background-color: #393939;}");
        m_rectangle->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/1rectangle.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/rectangle_selected.png);}");
        m_ellipse->setStyleSheet("QCheckBox::indicator{" + checkBtnSize
                                 + "background-color:transparent;}"
                                   "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/1ellipse.png);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/ellipse_selected.png);}");
        m_segment->setStyleSheet("QCheckBox::indicator{" + checkBtnSize
                                 + "background-color:transparent;}"
                                   "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/1segment.png);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/segment_selected.png);}");
        m_arrow->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/1arrow.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image:"
              " url(:/res/res/mark/arrow_selected.png);}");
        m_paintBrush->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
              "transparent;image: url(:/res/res/mark/1paintBrush.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/paintBrush_selected.png);}");
        m_markArticle->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
              "transparent;image: url(:/res/res/mark/1markArticle.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/markArticle_selected.png);}");
        m_text->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/1text.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
              "url(:/res/res/mark/text_selected.png);}");
        m_blur->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/1blur.png);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
              "url(:/res/res/mark/blur_selected.png);}");
        m_undo->setStyleSheet(
            "QPushButton{border:0px;border-radius:4px;background:transparent;image: url(:/res/res/mark/1undo.png);}"
            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;image: "
            "url(:/res/res/mark/1undo.png);}");
        m_exit->setStyleSheet(
            "QPushButton{border:0px;border-radius:4px;background:transparent;image: url(:/res/res/mark/exit.png);}"
            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;image: "
            "url(:/res/res/mark/exit.png);}");
    } else {
        m_layoutWid->setStyleSheet("QWidget{background-color:rgba(255,255,255,1);border-radius:" + widRadius + ";}");
        m_line->setStyleSheet("QLabel{border: 1px solid #393939;background-color: #393939;}");
        m_rectangle->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/rectangle.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/rectangle_selected.svg);}");
        m_ellipse->setStyleSheet("QCheckBox::indicator{" + checkBtnSize
                                 + "background:transparent;}"
                                   "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/ellipse.svg);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/ellipse_selected.svg);}");
        m_segment->setStyleSheet("QCheckBox::indicator{" + checkBtnSize
                                 + "background-color:transparent;}"
                                   "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/segment.svg);}"
                                   "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
                                   "image: url(:/res/res/mark/segment_selected.svg);}");
        m_arrow->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/arrow.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image:"
              " url(:/res/res/mark/arrow_selected.svg);}");
        m_paintBrush->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
              "transparent;image: url(:/res/res/mark/paintBrush.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/paintBrush_selected.svg);}");
        m_markArticle->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:"
              "transparent;image: url(:/res/res/mark/markArticle.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/markArticle_selected.svg);}");
        m_text->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/text.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
              "url(:/res/res/mark/text_selected.svg);}");
        m_blur->setStyleSheet(
            "QCheckBox::indicator{" + checkBtnSize
            + "background-color:transparent;}"
              "QCheckBox::indicator:unchecked{border:0px;border-radius:4px;background:transparent;"
              "image: url(:/res/res/mark/blur.svg);}"
              "QCheckBox::indicator:checked{border:0px;border-radius:4px;background:transparent;image: "
              "url(:/res/res/mark/blur_selected.svg);}");
        m_undo->setStyleSheet(
            "QPushButton{border:0px;border-radius:4px;background:transparent;image: url(:/res/res/mark/undo.svg);}"
            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;image: "
            "url(:/res/res/mark/undo.svg);}");
        m_exit->setStyleSheet(
            "QPushButton{border:0px;border-radius:4px;background:transparent;image: url(:/res/res/mark/exit.svg);}"
            "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;image: "
            "url(:/res/res/mark/exit.svg);}");

    }
    m_finish->setStyleSheet("QPushButton{border:0px;border-radius:6px;background:#0062F0;color:white;}"
                            "QPushButton::pressed{border:0px;border-radius:6px;background:#0062F0;color:white;}");
}

void Marking::clearButtonState()
{
    for (int i = 0; i < m_list.length(); i++) {
        m_list[i]->setChecked(false);
    }
}

void Marking::setWidAndBtnSize()
{
    QSize widSize = MARK_SIZE;
    QSize btnSize = MARK_BUTTON;
    if (platForm.contains(Variable::platForm)) {
        checkBtnSize = "width: 48px; height: 48px;";
        widRadius = "12px";
        widSize = MARK_SIZE_TM;
        btnSize = MARK_BUTTON_TM;
        m_triangleStartX = TRIANGLE_STARTX_TM;
        m_triangleWidth = TRIANGLE_WIDTH_TM;
        m_positionCoefficient = POSITION_COEFFICIENT_TM;
    } else {
        checkBtnSize = "width: 24px; height: 24px;";
        widRadius = "4px";
        widSize = MARK_SIZE;
        btnSize = MARK_BUTTON;
        m_triangleStartX = TRIANGLE_STARTX;
        m_triangleWidth = TRIANGLE_WIDTH;
        m_positionCoefficient = POSITION_COEFFICIENT;
    }
    this->setFixedSize(widSize);
    m_rectangle->setFixedSize(btnSize);
    m_ellipse->setFixedSize(btnSize);
    m_segment->setFixedSize(btnSize);
    m_arrow->setFixedSize(btnSize);
    m_paintBrush->setFixedSize(btnSize);
    m_markArticle->setFixedSize(btnSize);
    m_text->setFixedSize(btnSize);
    m_blur->setFixedSize(btnSize);
    m_undo->setFixedSize(btnSize);
    m_exit->setFixedSize(btnSize);
}

void Marking::resetMarkWidQss()
{
    this->setWidAndBtnSize();
    this->toSetLayout();
    this->changeStyle();
}

bool Marking::getTextState()
{
    return m_text->isChecked();
}
