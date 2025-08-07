pragma ComponentBehavior: Bound
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
                required property int index
                required property var model

                Rectangle {
                    id: rect
                    anchors.fill: parent
                    color: root.currentIndex === tabItem.index ? Material.primary : "transparent"
                    radius: 6

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.currentIndex = tabItem.index;
                            root.tabClicked(tabItem.index);
                        }
                        cursorShape: Qt.PointingHandCursor
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
                            color: root.currentIndex === tabItem.index ? "white" : "#444"
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
