import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import HX.Music
import "./internal"

Item {
    id: root
    signal playModeChanged(int newMode)

    width: 32
    height: 32

    MusicActionButton {
        id: btn
        anchors.fill: parent
        defaultColor: Theme.paratextColor
        pressedColor: Theme.textColor
        hoveredColor: Theme.highlightingColor
        property int currentMode: MusicController.playMode

        // 播放模式数量(最后那个 PlayModeCnt 不算实际模式)
        readonly property int modeCount: PlayMode.PlayModeCnt

        // 播放模式对应图标路径
        readonly property var playModeIcons: [
            "qrc:/icons/playmode_list_loop.svg",
            "qrc:/icons/playmode_random_play.svg",
            "qrc:/icons/playmode_single_loop.svg"
        ]

        // 当前图标路径
        property string iconSource: playModeIcons[currentMode]

        onClicked: {
            currentMode = (currentMode + 1) % modeCount;
            iconSource = playModeIcons[currentMode];
            root.playModeChanged(currentMode);
            MusicController.setPlayMode(currentMode);
        }

        url: btn.iconSource

        hoverEnabled: true
        ToolTip.visible: hovered
        ToolTip.text: {
            switch (currentMode) {
                case 0: return "列表循环"
                case 1: return "随机播放"
                case 2: return "单曲循环"
                default: return ""
            }
        }
    }
}