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
    property url clickableIconSource: iconSource    // 可点击图标源, 默认为null表示不可点击
    property color iconDefaultColor: "gray"         // 图标默认颜色
    property color iconHighlightColor: "blue"       // 图标高亮颜色
    property color borderHighlightColor: "blue"     // 边框高亮颜色
    property alias placeholderText: textInput.placeholderText

    signal accepted() // 按下回车键时触发
    signal iconClicked(bool isDefaultState) // 图标点击信号, 传递当前状态

    // 内部属性
    property bool isFocused: false
    property string text: ""
    property bool isDefaultIconState: true // 图标状态, true表示默认状态, false表示切换后状态

    radius: 8
    border {
        width: 1
        color: isFocused ? borderHighlightColor : "lightgray"
    }
    color: Theme.backgroundColor

    RowLayout {
        anchors.fill: parent

        // SVG图标 - 修改为可点击
        Item {
            id: iconContainer
            Layout.leftMargin: 10
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignLeft

            // 只有设置了clickableIconSource时才可点击
            visible: root.iconSource !== ""

            Image {
                id: icon
                anchors.fill: parent
                source: {
                    if (root.clickableIconSource === "") {
                        // 不可点击模式: 使用默认图标
                        return `image://svgColored/${root.iconSource}?color=${root.isFocused ? root.iconHighlightColor : root.iconDefaultColor}`;
                    } else {
                        // 可点击模式: 根据状态选择图标
                        const currentIconSource = root.isDefaultIconState ? root.iconSource : root.clickableIconSource;
                        return `image://svgColored/${currentIconSource}?color=${root.isFocused ? root.iconHighlightColor : root.iconDefaultColor}`;
                    }
                }
                fillMode: Image.PreserveAspectFit
            }

            // 鼠标区域, 用于处理点击事件
            MouseArea {
                id: iconMouseArea
                anchors.fill: parent
                enabled: root.clickableIconSource !== null // 只有设置了可点击图标源时才启用
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: {
                    if (root.clickableIconSource !== null) {
                        // 切换状态
                        root.isDefaultIconState = !root.isDefaultIconState;
                        // 发出点击信号, 传递当前状态
                        root.iconClicked(root.isDefaultIconState);
                    }
                }

                // 悬停效果
                hoverEnabled: true
                onEntered: {
                    if (enabled) {
                        icon.scale = 1.1;
                    }
                }
                onExited: {
                    if (enabled) {
                        icon.scale = 1.0;
                    }
                }
            }
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
            onAccepted: {
                root.accepted();
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

    // 重置图标状态的方法
    function resetIconState() {
        root.isDefaultIconState = true;
    }

    // 设置图标状态的方法
    function setIconState(isDefault) {
        root.isDefaultIconState = isDefault;
    }
}