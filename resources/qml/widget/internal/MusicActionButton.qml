import QtQuick
import QtQuick.Controls

Button {
    id: btn
    property string url: ""
    property string defaultColor: "#a92285" // 默认颜色
    property string pressedColor: "#990099" // 点击时颜色
    property string hoveredColor: "gold" // 悬停颜色
    
    width: 64
    height: 64

    background: Rectangle { color: "transparent" }
    Image {
        id: img
        source: `image://svgColored/${btn.url}?color=${btn.pressed || btn.hovered 
                                                        ? (btn.pressed 
                                                            ? btn.pressedColor 
                                                            : btn.hoveredColor) 
                                                        : btn.defaultColor}`
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
    }
}