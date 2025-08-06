import QtQuick
import QtQuick.Controls
import HX.Music

/*
    音乐进度条, 可以拖动!
*/
Slider {
    id: root
    anchors.centerIn: parent

    width: parent.width - 20

    // 应该初始化为 ms(毫秒) 单位, 以确保拖动时候传参是正确的
    stepSize: 1_000 // 1s (移动间隔)

    MusicController {
        id: musicController
    }

    onValueChanged: {
        if (root.pressed) {
            musicController.setPosition(value); // 更新播放位置到指定值，实现手动控制进度条位置改变时同步更新播放位置
        }
    }

    Connections {
        target: SignalBusSingleton

        // 绑定信号: 更新歌曲
        function onNewSongLoaded(song) {
            console.log("新歌加载:", musicController.getLengthInMilliseconds());
            root.to = musicController.getLengthInMilliseconds();
            console.log("播放进度变化", pos, "但是:", root.from, " -> ", root.to, "当前: ", root.value);
        }

        // 绑定信号: 播放位置变化
        function onMusicPlayPosChanged(pos: int) {
            // { debug
            if (pos > root.to) /* [[unlikely]] */ {
                root.to = musicController.getLengthInMilliseconds();
            }
            // } debug
            root.value = pos;
        }
    }
}