import QtQuick 2.15
import QtQuick.Controls 2.0

Rectangle {
    id: light
    width: 18; height: 18
    color: "green"
    property color currentColor: color
    radius: width / 2
    antialiasing: true
    border.color: "black"

    // 呼吸灯动画
    SequentialAnimation {
        id: animation
        running: true
        loops: Animation.Infinite

        // 变亮动画
        PropertyAnimation {
            target: light
            properties: "color"
            from: currentColor
            to: Qt.lighter(currentColor, 1.5)
            duration: 1000
        }

        // 变暗动画
        PropertyAnimation {
            target: light
            properties: "color"
            from: Qt.lighter(currentColor, 1.5)
            to: currentColor
            duration: 1000
        }
    }


    function setColor(newColor) {
        if (animation.running) {
            animation.stop();
        }
        light.color = newColor;
        currentColor = newColor;
        animation.start();

        console.log("color set", newColor)
    }


    function stopBreathing(Boolean) {
        if (Boolean) {
            animation.pause();
            currentColor = light.color;
        }
        else{
            animation.start();
        }
    }

}
