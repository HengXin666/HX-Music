import QtQuick
import QtQuick.Controls
import HX.Music

/*
    音乐进度条, 可以拖动!
*/
Slider {
    id: root

    width: parent.width - 20

    // 应该初始化为 ms(毫秒) 单位, 以确保拖动时候传参是正确的
    stepSize: 1_000 // 1s (移动间隔)

    onValueChanged: {
        if (root.pressed) {
            musicController.setPosition(value); // 更新播放位置到指定值，实现手动控制进度条位置改变时同步更新播放位置
        }
    }

    Connections {
        id: songConn
        target: null
        // 绑定信号: 更新歌曲
        function onNewSongLoaded(song: MusicInfo) {
            root.to = song.getLengthInMilliseconds();
        }

        // 绑定信号: 播放位置变化
        function onMusicPlayPosChanged(pos: int) {
            root.value = pos;
        }
    }

    Component.onCompleted: {
        songConn.target = SignalBusSingleton;
    }
}