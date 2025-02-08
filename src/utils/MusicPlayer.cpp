#include <utils/MusicPlayer.h>

#include <singleton/GlobalSingleton.hpp>
#include <cmd/MusicCommand.hpp>

HX::MusicPlayer::MusicPlayer()
    : _player()
{
    auto audioOutput = new QAudioOutput(&_player);
    _player.setAudioOutput(audioOutput);
    audioOutput->setVolume(100);
    connect(&_player, &QMediaPlayer::mediaStatusChanged, this, [this](
        QMediaPlayer::MediaStatus mediaState
    ){
        if (mediaState != QMediaPlayer::MediaStatus::EndOfMedia) {
            return;
        }
        switch (GlobalSingleton::get().musicConfig.playMode) {
        case PlayMode::ListLoop:    // 列表循环
            // todo
            break;
        case PlayMode::RandomPlay:  // 随机播放
            // todo
            break;
        case PlayMode::SinglePlay:  // 单曲播放
            MusicCommand::pause();
            break;
        case PlayMode::SingleLoop:  // 单曲循环
            MusicCommand::resume();
            break;
        case PlayMode::PlayModeCnt: // !保留!
            break;
        }
    });
}