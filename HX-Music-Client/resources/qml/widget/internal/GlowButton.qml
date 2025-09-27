import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    id: root
    width: 150
    height: 40
    
    // 公共属性 - 可自定义
    property string text: "科技按钮"
    property color baseColor: "#0a0e17"          // 基础颜色
    property color glowColor: "#00f3ff"          // 发光颜色
    property color accentColor: "#00c6ff"        // 强调色
    property int glowRadius: 20                  // 发光半径
    property int borderRadius: 8                 // 圆角半径
    property int fontSize: 16                    // 字体大小
    property bool enabled: true                  // 启用状态
    
    // 信号
    signal clicked()
    
    // 外部发光效果
    Glow {
        id: outerGlow
        anchors.fill: buttonBackground
        source: buttonBackground
        radius: enabled ? 0 : 0
        samples: 16
        color: root.glowColor
        spread: 0.2
        visible: root.enabled
        
        states: State {
            name: "hovered"
            when: mouseArea.containsMouse && root.enabled
            PropertyChanges { target: outerGlow; radius: root.glowRadius; spread: 0.4 }
        }
        
        transitions: Transition {
            NumberAnimation { properties: "radius, spread"; duration: 400; easing.type: Easing.OutCubic }
        }
    }
    
    // 按钮背景
    Rectangle {
        id: buttonBackground
        anchors.fill: parent
        color: root.baseColor
        radius: root.borderRadius
        border.color: Qt.lighter(root.baseColor, 2.5)
        border.width: 1
        
        // 内部渐变效果
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.baseColor, 1.8) }
            GradientStop { position: 0.5; color: root.baseColor }
            GradientStop { position: 1.0; color: Qt.darker(root.baseColor, 1.2) }
        }
        
        // 高光效果
        Rectangle {
            id: highlight
            width: parent.width
            height: parent.height / 2
            anchors.top: parent.top
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: root.borderRadius
            border.width: 0
        }
        
        // 内部发光效果
        Glow {
            id: innerGlow
            anchors.fill: buttonContent
            source: buttonContent
            radius: 8
            samples: 16
            color: root.glowColor
            spread: 0.1
            visible: root.enabled
        }
        
        // 按钮内容
        Item {
            id: buttonContent
            anchors.centerIn: parent
            width: textContent.width + 20
            height: textContent.height
            
            // 按钮文本
            Text {
                id: textContent
                anchors.centerIn: parent
                text: root.text
                color: root.enabled ? "white" : "#888"
                font.pointSize: root.fontSize
                font.bold: true
                font.family: "Arial"
                style: Text.Raised
                styleColor: Qt.rgba(0, 0, 0, 0.5)
            }
            
            // 科技感装饰元素
            Rectangle {
                width: 6
                height: width
                radius: width / 2
                color: root.accentColor
                anchors {
                    right: textContent.left
                    rightMargin: 8
                    verticalCenter: parent.verticalCenter
                }
                visible: root.enabled
                
                Glow {
                    anchors.fill: parent
                    source: parent
                    radius: 6
                    samples: 12
                    color: root.accentColor
                    spread: 0.4
                }
            }
        }
        
        // 鼠标区域
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            enabled: root.enabled
            cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: {
                root.clicked();
                clickEffect.start();
            }
        }
    }
    
    
    // 点击动画效果
    SequentialAnimation {
        id: clickEffect
        PropertyAnimation {
            target: buttonBackground
            property: "scale"
            to: 0.97
            duration: 80
        }
        PropertyAnimation {
            target: buttonBackground
            property: "scale"
            to: 1.0
            duration: 200
            easing.type: Easing.OutBack
        }
    }
    
    // 禁用状态效果
    states: State {
        name: "disabled"
        when: !root.enabled
        PropertyChanges { target: buttonBackground; opacity: 0.6 }
    }
}