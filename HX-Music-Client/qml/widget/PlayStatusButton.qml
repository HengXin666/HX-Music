pragma ComponentBehavior: Bound
import QtQuick 2.15
import QtQuick.Controls 2.15
import HX.Music

/*
    音乐状态按钮:
    没有选中 && 没有悬浮: [文本]
    没有选中 && 悬浮 && 没有暂停: [播放]
    选中 && 悬浮 || 选中 && 暂停: [暂停]
    选中 && 没有悬浮 && 没有暂停: [动画]
*/
Item {
    id: root
    property string text: ""        // 没有选中 && 没有悬浮 显示的文本内容
    property bool isSelected: false // 是否被选中
    property bool isHovered: false  // 是否悬浮 
    property string path            // 音乐路径

    width: 32
    height: 32

    // 文本
    Text {
        id: txt
        text: root.text
        color: Theme.paratextColor
        font.pixelSize: 16
        anchors.centerIn: parent
        // 没有选中 && 没有悬浮
        visible: !root.isSelected && !root.isHovered
    }

    Image {
        id: img
        // (没有选中 && 悬浮 && 没有暂停) || (选中 && (悬浮 || 暂停))
        // 也就是
        // !(没有选中 && 没有悬浮) && !(选中 && 没有悬浮 && 没有暂停)
        visible: !txt.visible && !audioVisualizerBars.visible
        anchors.fill: parent
        source: {
            if (root.isSelected) {
                return MusicController.isPlaying 
                    ? `image://svgColored/qrc:/icons/pause.svg?color=${"#990099"}`
                    : `image://svgColored/qrc:/icons/play.svg?color=${"#990099"}`;
            } else {
                return `image://svgColored/qrc:/icons/play.svg?color=${"#990099"}`;
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: false
            acceptedButtons: Qt.LeftButton

            onClicked: {
                if (root.isSelected) {
                    MusicController.togglePause();
                } else {
                    MusicController.playMusic(path);
                    root.switchThisMusic();
                }
            }
        }
    }

    AudioVisualizerBars {
        id: audioVisualizerBars
        anchors.fill: parent
        color: Theme.highlightingColor
        // 选中 && 没有悬浮 && 没有暂停
        visible: root.isSelected && !root.isHovered && MusicController.isPlaying
    }

    signal switchThisMusic();
}