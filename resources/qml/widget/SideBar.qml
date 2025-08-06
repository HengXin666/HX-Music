import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

Item {
    id: root

    property int itemWidth;

    width: itemWidth
    height: parent.height

    property int currentIndex: 0
    
    signal tabClicked(int index);

    ColumnLayout {
        anchors.fill: parent
        spacing: 4
        Repeater {
            id: repeater
            model: ListModel {
                ListElement { icon: "🏠"; label: "主页" }
                ListElement { icon: "🔍"; label: "搜索" }
                ListElement { icon: "⚙️"; label: "设置" }
                ListElement { icon: "❓"; label: "关于" }
            }

            delegate: Item {
                id: tabItem
                required property var model
                width: parent.width
                height: 50

                Rectangle {
                    required property var root
                    required property int index
                    id: rect

                    anchors.fill: parent
                    color: root.currentIndex === index ? Material.primary : "transparent"
                    radius: 6

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            rect.root.currentIndex = rect.index
                            rect.root.tabClicked(rect.index)
                        }
                        cursorShape: Qt.PointingHandCursor

                        Rectangle {
                            anchors.fill: parent
                            color: hovered ? "#eeeeee33" : "transparent"
                            visible: !rect.root.currentIndex === rect.index
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Label {
                            text: tabItem.model.icon
                            font.pixelSize: 20
                        }

                        Label {
                            text: tabItem.model.label
                            font.pixelSize: 16
                            color: rect.root.currentIndex === rect.index ? "white" : "#444"
                        }
                    }
                }
            }
        }

        // 可选: 自动填充底部空白区域
        Rectangle {
            Layout.fillHeight: true
            color: "transparent"
        }
    }
}
