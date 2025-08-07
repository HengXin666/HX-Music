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
                width: parent.width
                height: 50

                Rectangle {
                    id: rect

                    anchors.fill: parent
                    color: root.currentIndex === index ? Material.primary : "transparent"
                    radius: 6

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.currentIndex = index
                            root.tabClicked(index)
                        }
                        cursorShape: Qt.PointingHandCursor

                        Rectangle {
                            anchors.fill: parent
                            color: hovered ? "#eeeeee33" : "transparent"
                            visible: !root.currentIndex === index
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Label {
                            text: model.icon
                            font.pixelSize: 20
                        }

                        Label {
                            text: model.label
                            font.pixelSize: 16
                            color: root.currentIndex === index ? "white" : "#444"
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
