#ifndef COMMONDEFINITION_H
#define COMMONDEFINITION_H
#include <QSize>

const uint g_quitTimer = 60;
const uint g_defaultTimer = 6;

namespace MASTER {
    const uint  topWidgetWidth = 372;
    const uint  topWidgeShadowBlurRadius = 8; //阴影模糊半径
    const uint  topWidgeReservedHeight = 134; //距离屏幕底部的距离
    const uint  topWidgeMargin = 5;
    const uint  popupWidgetSpaceHeight = 5;
    const QSize popupWidgetSize(372, 134);
    const QSize popupWidgetBodyNoExsitSize(372, 108);
    const QSize popupWidgetBodyExsitSize(372, 134);
    const QSize popupWidgetActionNoExsitSize(372, 88);
    const QSize popupWidgetActionExsitSize(372, 134);
    const qreal  popupWidgetRadius = 12; //popWidget圆角半径
    const uint  iconWidgetWidth = 28;
    const QSize iconLabelSize(24, 24);
    const uint  operationWidgetHeight = 49;
    const uint  foldWidgetHeight = 36;
    const uint  closeBtnWidgetWidth = 34;
    const QSize closeBtnIconSize(16, 16);
    const QSize closeBtnSize(30, 30);
    const uint  summaryLabelWidth = 300;
    const uint  bodyLabelWidth = 300;
    const uint  actionBtnHeight = 36;
    const uint  actionBtnWidth =  96;
    const uint  foldLabelHeight = 28;
    const uint  foldLabelWidth = 300;
    const qreal foldLabelFontOpacity = 0.45;

    const uint  outAnimationDuration = 200;
    const uint  moveAnimationDuration = 300;

    //霸屏窗口UI参数
    const QSize centerIconWidgetSize(24, 24);
    const QSize centerIconLabelSize(24, 24);

    const QSize centerCloseBtnWidgetSize(424, 40);
    const QSize centerCloseBtnIconSize(11, 11);
    const QSize centerCloseBtnSize(30, 30);

    const QSize centerOnlySummarySize(424, 24);
    const QSize centerOnlySummaryLabelSize(344, 24);
    const qreal centerOnlySummaryFontOpacity = 1.0;

    const QSize centerOnlyBodyOneLineSize(424, 24);
    const QSize centerOnlyBodyOneLineLabelSize(344, 24);
    const QSize centerOnlyBodyMultilineSize(424, 85);
    const QSize centerOnlyBodyMultilineLabelSize(344, 85);
    const qreal centerOnlyBodyFontOpacity = 1.0;

    const QSize centerSummaryBodyWidgetSize(424, 89);
    const QSize centerSummaryLabelSize(344, 24);
    const QSize centerBodyLabelSize(344, 58);
    const qreal centerSummaryFontOpacity = 1.0;
    const qreal centerBodyFontOpacity = 0.35;


    const QSize centerButtonWidgetSize(424, 36);
    const QSize centerButtonSize(96, 36);


    const QSize centerWidgetSize(424, 156);
    const QSize centerWidgetOnlyBodyOneLineSize(424, 156);
    const QSize centerWidgetOnlyBodyMultilineSize(424, 198);
//    const QSize centerWidgetSize(424, 120);
    const qreal  centerWidgetRadius = 12;

}

namespace INTER {
    const uint  topWidgetWidth = 360;
    const uint  topWidgeShadowBlurRadius = 8;
    const uint  topWidgeReservedHeight = 250;
    const uint  topWidgeRightMargin = 8;
    const uint  topWidgeTopMargin = 5;
    const uint  popupWidgetSpaceHeight = 5;
    const QSize popupWidgetSize(360, 156);

    const QSize popupWidgetNoBodySize(360, 87);
    const QSize popupWidgetNormalSize(360, 109); //正常大小：提示、主题、一行正文
    const QSize popupWidgetTwoLineBodySize(360, 131); //提示、主题、两行正文
    const QSize popupWidgetOneLineBodyActionSize(360, 165); //提示、主题、一行正文、动作按钮
    const QSize popupWidgetTwoLineBodyActionSize(360, 187); //提示、主题、两行正文、动作按钮
    const qreal  popupWidgetRadius = 12; //popWidget圆角半径
    const uint  popupWidgetInfoAreaWidth = 300;
    const uint  iconWidgetWidth = 16;
    const QSize iconLabelSize(16, 16);
    const uint  closeBtnWidgetWidth = 16;
    const QSize closeBtnIconSize(12, 12);
    const QSize closeBtnSize(16, 16);
    const uint  appNameFontSize = 14;
    const qreal appNameFontOpacity = 0.45;
    const uint  appNameLabelHeight = 21;
    const uint  summaryLabelWidgetWidth = 300;
    const uint  summaryLabelWidth = 300;
    const uint  summaryLabelHeight = 24;
    const uint  summaryFontSize = 16;
    const qreal summaryFontOpacity = 0.85;
    const uint  bodyLabelWidgetWidth = 300;
    const uint  bodyLabelWidth = 300;
    const uint  bodyLabelHeight = 42;
    const uint  bodyFontSize = 14;
    const qreal bodyFontOpacity = 0.65;
    const uint  actionBtnHeight = 34;



    const uint  outAnimationDuration = 200;
    const uint  moveAnimationDuration = 300;

}


#endif // COMMONDEFINITION_H
