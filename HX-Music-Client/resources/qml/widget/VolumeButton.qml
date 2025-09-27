import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls.Material
import HX.Music
import "qrc:/HX/Music/qml/widget/internal"

Item {
    id: root

    property real volumeLevel

    width: 32
    height: 32

    MusicActionButton {
        id: btn
        defaultColor: popup.visible ? Theme.highlightingColor : Theme.paratextColor
        pressedColor: Theme.textColor
        hoveredColor: Theme.highlightingColor
        anchors.fill: parent
        url: root.volumeLevel > 0.5 
            ? "qrc:/icons/volume_up.svg"
            : root.volumeLevel > 0
                ? "qrc:/icons/volume_low.svg"
                : "qrc:/icons/volume_off.svg"
    
        onClicked: {
            popup.open();
        }
    }

    Popup {
        id: popup
        modal: false
        focus: true
        x: btn.x + btn.width / 2 - width / 2
        y: btn.y - height - 10
        width: 48
        height: 135
        padding: 0
        background: Rectangle {
            color: "#333"
            radius: 6
            border.color: "#666"
        }

        contentItem: Column {
            width: parent.width
            spacing: 4
            padding: 6
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: percentText
                text: Math.round(root.volumeLevel * 100) + "%"
                color: "white"
                font.bold: true
                font.pointSize: 10
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Slider {
                id: volumeSlider
                orientation: Qt.Vertical
                from: 0.0
                to: 1.0
                value: root.volumeLevel
                stepSize: 0.01
                height: 100
                anchors.horizontalCenter: parent.horizontalCenter

                onValueChanged: {
                    root.volumeLevel = value;
                    percentText.text = Math.round(value * 100) + "%";
                }
            }
        }

        onClosed: focus = false
    }

    onVolumeLevelChanged: {
        MusicController.volume = volumeLevel;
    }

    Component.onCompleted: {
        volumeLevel = MusicController.volume;
    }
}
