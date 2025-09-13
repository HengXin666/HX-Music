import QtQuick
import QtQuick.Dialogs

Item {
    id: root

    width: 50
    height: 50

    property color selectedColor: "white"
    signal accepted(color c)

    // 当前颜色按钮
    Rectangle {
        id: colorButton
        width: root.width
        height: root.height
        radius: 5
        color: root.selectedColor
        border.color: "black"
        border.width: 1
        anchors.centerIn: parent

        MouseArea {
            anchors.fill: parent
            onClicked: colorDialog.open();
        }
    }

    // 颜色选择弹窗
    ColorDialog {
        id: colorDialog
        selectedColor: root.selectedColor
        options: ColorDialog.ShowAlphaChannel
        onAccepted: {
            root.selectedColor = selectedColor;
            root.accepted(selectedColor);
        }
    }
}
