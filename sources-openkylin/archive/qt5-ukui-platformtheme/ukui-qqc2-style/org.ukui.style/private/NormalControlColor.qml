import QtQuick 2.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: root
//    StylePrivate.UKUIButton{
//        id: btn
//    }
//    StylePrivate.APPParameter{
//        id: appP
////        onFontChanged:{
////            console.log("onFontChanged.......", StylePrivate.APPParameter.font)
////        }
//    }
//    StylePrivate.ParseColorInterface{
//        id:pDTColor
//    }
//    StylePrivate.UKUIButton{
//        id:uBtn
//    }
    property color controlStartColor// : StylePrivate.UKUIButton.normalBC
    property color controlEndColor: controlStartColor
    property color borderColor

    ParseInterface{
        id: parseInterface
    }

    function changeColor(highlighted, enable, click, hover, focus){
        var closeBtn = false;
        var windowBtn = false;
        var importBtn = false;
        if(controlRoot.hasOwnProperty("buttonType")){
            closeBtn = (controlRoot.buttonType === "closeButton");
            windowBtn = (controlRoot.buttonType === "windowButton");
            importBtn = (controlRoot.buttonType === "importButton");
        }

        if(highlighted || importBtn){
            controlStartColor = !enable ? parseInterface.pStartColor(StylePrivate.UKUIButton.disableHBC) :
                                 click ? parseInterface.pStartColor(StylePrivate.UKUIButton.clickedHBC) :
                                 hover ? parseInterface.pStartColor(StylePrivate.UKUIButton.hoveredHBC) :
                                         parseInterface.pStartColor(StylePrivate.UKUIButton.normalHBC)
            controlEndColor = !enable ? parseInterface.pEndColor(StylePrivate.UKUIButton.disableHBC) :
                                 click ? parseInterface.pEndColor(StylePrivate.UKUIButton.clickedHBC) :
                                 hover ? parseInterface.pEndColor(StylePrivate.UKUIButton.hoveredHBC) :
                                                          parseInterface.pEndColor(StylePrivate.UKUIButton.normalHBC)

            borderColor = !enable ? parseInterface.pStartColor(StylePrivate.UKUIButton.disableBorderHColor) :
                                    focus ? parseInterface.pStartColor(StylePrivate.UKUIButton.focusBorderColor) :
                                    click ? parseInterface.pStartColor(StylePrivate.UKUIButton.clickBorderHColor) :
                                    hover ? parseInterface.pStartColor(StylePrivate.UKUIButton.hoverBorderHColor) :
                                            parseInterface.pStartColor(StylePrivate.UKUIButton.normalBorderHColor)
        }
        else{
            controlStartColor = !enable ? parseInterface.pStartColor(StylePrivate.UKUIButton.disableBC) :
                             click ? parseInterface.pStartColor(StylePrivate.UKUIButton.clickedBC) :
                             hover ? parseInterface.pStartColor(StylePrivate.UKUIButton.hoveredBC) :
                                     parseInterface.pStartColor(StylePrivate.UKUIButton.normalBC)

            controlEndColor = !enable ? parseInterface.pEndColor(StylePrivate.UKUIButton.disableBC) :
                             click ? parseInterface.pEndColor(StylePrivate.UKUIButton.clickedBC) :
                             hover ? parseInterface.pEndColor(StylePrivate.UKUIButton.hoveredBC) :
                                     parseInterface.pEndColor(StylePrivate.UKUIButton.normalBC)

            borderColor = !enable ? parseInterface.pStartColor(StylePrivate.UKUIButton.disableBorderColor) :
                            focus ? parseInterface.pStartColor(StylePrivate.UKUIButton.focusBorderColor) :
                            click ? parseInterface.pStartColor(StylePrivate.UKUIButton.clickBorderColor) :
                            hover ? parseInterface.pStartColor(StylePrivate.UKUIButton.hoverBorderColor) :
                                    parseInterface.pStartColor(StylePrivate.UKUIButton.normalBorderColor)
        }
    }
}
