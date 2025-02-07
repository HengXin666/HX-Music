#include <widget/PlayBar.h>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>
#include <cmd/MusicCommand.hpp>

PlayBar::PlayBar(QWidget* parent)
    : QWidget(parent)
{
    /*ui
    | ----------------------------- 播放条 ------------------------------> |
    | 图 歌曲信息(滚动) (歌曲名称 歌手名称 均可点击)     上  暂  下     播放 音 歌 |
    | 片 喜欢/评论/下载/分享   播放时长/总时长          首  停  首     序列 量 词 |
    */
    
    // 垂直布局
    QVBoxLayout* vBL = new QVBoxLayout(this);

    // 添加播放进度条
    vBL->addWidget(_barPlayProgress);

    // 歌曲信息
    _textMusicData->setText("歌曲信息(滚动) (歌曲名称 歌手名称 均可点击)");
    
    // 水平布局 (下方 总操作布局)
    QHBoxLayout* hMainMusicOp = new QHBoxLayout();

    // 图片
    QPixmap pixmap(":/icons/audio.svg");
    _imgMusic->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _imgMusic->setFixedSize(50, 50);
    hMainMusicOp->addWidget(_imgMusic);

    // 操作 水平布局
    QHBoxLayout* hLayoutActions = new QHBoxLayout();
    
    hLayoutActions->addWidget(_btnLike);
    hLayoutActions->addWidget(_btnComment);
    hLayoutActions->addWidget(_btnDownload);
    hLayoutActions->addWidget(_btnShare);

    // 播放时长/总时长 文本
    hLayoutActions->addWidget(_textTimeProgress);

    // 歌曲信息&操作垂直布局
    QVBoxLayout* vSongDataAndOpLayout = new QVBoxLayout();
    vSongDataAndOpLayout->addWidget(_textMusicData);
    vSongDataAndOpLayout->addLayout(hLayoutActions);

    hMainMusicOp->addLayout(vSongDataAndOpLayout);
    vBL->addLayout(hMainMusicOp);

    hMainMusicOp->addStretch();

    // 上一首/暂停/下一首
    QHBoxLayout* hPlayOpLayout = new QHBoxLayout();
    _btnPrevious->setIcon(QIcon(":/icons/previous.svg"));
    QIcon playIcon{":/icons/play.svg"};
    QIcon pauseIcon{":/icons/pause.svg"};
    _btnPlayPause->setIcon(playIcon);
    _btnNext->setIcon(QIcon(":/icons/next.svg"));
    hPlayOpLayout->addWidget(_btnPrevious);
    hPlayOpLayout->addWidget(_btnPlayPause);
    hPlayOpLayout->addWidget(_btnNext);
    hMainMusicOp->addLayout(hPlayOpLayout);

    hMainMusicOp->addStretch();

    // 播放序列/音量大小/歌词
    QHBoxLayout* hPlaySettingLayout = new QHBoxLayout();
    _cbxPlayMode->addItems({
        "列表循环",
        "随机播放",
        "单次播放",
        "单曲循环",
    });
    _cbxPlayMode->setCurrentIndex(static_cast<int>(GlobalSingleton::get().musicConfig.playMode));
    _btnLyric->setText("歌词");
    hPlaySettingLayout->addWidget(_cbxPlayMode);
    hPlaySettingLayout->addWidget(_volumeBar);
    hPlaySettingLayout->addWidget(_btnLyric);
    hMainMusicOp->addLayout(hPlaySettingLayout);

    // === 连接信号槽 (SignalBusSingleton) ===
    /* NewSongLoaded (加载新歌) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::newSongLoaded, this,
        [this](
            HX::MusicInfo const& info
        ) {
        // todo: 处理歌手
        _textMusicData->setText(info.getTitle());
        if (auto img = info.getAlbumArtAdvanced()) {
            _imgMusic->setPixmap(img->scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            _imgMusic->setPixmap(
                QPixmap{":/icons/audio.svg"}.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
        }
        GlobalSingleton::get().music.switchMusic(info.filePath()).play();
    });

    /* musicPaused (音乐暂停) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPaused, this,
        [this, playIcon = std::move(playIcon)]() {
        _btnPlayPause->setIcon(playIcon);
        _isPlay = false;
    });

    /* musicResumed (音乐继续) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicResumed, this,
        [this, pauseIcon]() {
        _btnPlayPause->setIcon(pauseIcon);
        _isPlay = true;
    });

    // 播放与继续按钮
    connect(_btnPlayPause, &QPushButton::clicked, this,
        [this, pauseIcon]() {
        if (_isPlay) {
            MusicCommand::pause();
        } else {
            MusicCommand::resume();
        }
    });

    /* playModeChanged (播放模式) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::playModeChanged, this,
        [this](PlayMode mode){
        _cbxPlayMode->setCurrentIndex(static_cast<int>(mode));
    });

    // 下拉框: 播放模式
    connect(_cbxPlayMode, &QComboBox::currentIndexChanged, this,
        [this](int idx) {
        MusicCommand::setPlayMode(PlayMode{idx});
    });
}