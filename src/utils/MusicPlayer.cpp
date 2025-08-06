#include <utils/MusicPlayer.h>

#include <config/MusicConfig.hpp>
#include <singleton/GlobalSingleton.hpp>
#include <cmd/MusicCommand.hpp>

HX::MusicPlayer::MusicPlayer() 
    : _player{}
{
    auto audioOutput = new QAudioOutput(&_player);
    _player.setAudioOutput(audioOutput);
    audioOutput->setVolume(1.f);

    // 播放状态变化
    connect(&_player, &QMediaPlayer::mediaStatusChanged, this, [this](
        QMediaPlayer::MediaStatus mediaState
    ){
        if (mediaState != QMediaPlayer::MediaStatus::EndOfMedia) {
            return;
        }
        // 播放完毕
        switch (GlobalSingleton::get().musicConfig.playMode) {
        case PlayMode::ListLoop:    // 列表循环
        case PlayMode::RandomPlay:  // 随机播放
            MusicCommand::nextMusic();
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

    // 播放位置变化
    connect(&_player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        SignalBusSingleton::get().musicPlayPosChanged(pos);
    });
}