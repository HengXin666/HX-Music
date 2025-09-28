import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: textButton
    width: 120
    height: 40
    radius: 8                      // 默认圆角
    color: backgroundColor         // 固定背景颜色

    border.width: 1
    border.color: pressedColor

    // 属性定义
    property alias text: buttonText.text
    property color defaultColor: Theme.paratextColor        // 默认颜色
    property color hoverColor: Theme.textColor              // 悬浮文本颜色
    property color pressedColor: Theme.highlightingColor    // 按下文本颜色
    property color backgroundColor: Theme.backgroundColor   // 背景颜色

    signal clicked()  // 点击信号

    Text {
        id: buttonText
        anchors.centerIn: parent
        color: mouseArea.pressed ? textButton.pressedColor :
               mouseArea.containsMouse ? textButton.hoverColor :
               textButton.defaultColor
        font.pixelSize: 16
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            textButton.clicked()
        }
    }
}
