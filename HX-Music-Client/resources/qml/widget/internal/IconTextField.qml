pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    width: 300
    height: 40

    // 暴露的属性
    property bool isPasswordMode: false
    property url iconSource
    property color iconDefaultColor: "gray"     // 图标默认颜色
    property color iconHighlightColor: "blue"   // 图标高亮颜色
    property color borderHighlightColor: "blue" // 边框高亮颜色
    property alias placeholderText: textInput.placeholderText

    // 内部属性
    property bool isFocused: false
    property string text: ""

    radius: 8
    border {
        width: 1
        color: isFocused ? borderHighlightColor : "lightgray"
    }
    color: Theme.backgroundColor

    RowLayout {
        anchors.fill: parent

        // SVG图标
        Image {
            id: icon
            Layout.leftMargin: 10
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignLeft
            source: `image://svgColored/${root.iconSource}?color=${root.isFocused ? root.iconHighlightColor : root.iconDefaultColor}`
            fillMode: Image.PreserveAspectFit
        }

        // 文本输入框
        TextField {
            id: textInput
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            echoMode: root.isPasswordMode ? TextInput.Password : TextInput.Normal
            selectByMouse: true
            clip: true
            text: root.text
            onTextChanged: {
                root.text = text;
            }

            onFocusChanged: root.isFocused = focus

            background: null
            placeholderTextColor: root.iconDefaultColor
            color: root.iconDefaultColor
            font {
                pixelSize: 16
                family: "Microsoft YaHei"
            }
        }
    }
}
