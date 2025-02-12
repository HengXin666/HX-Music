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
    _sliderPlayBar->setRange(0, 1'00'00); // 100.00% 百分比
    vBL->addWidget(_sliderPlayBar);

    // 歌曲信息
    _textMusicData->setText("歌曲信息(滚动) (歌曲名称 歌手名称 均可点击)");
    _textMusicData->setFixedWidth(300);
    _textMusicData->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _textMusicData->setPauseTime(1500); // 设置滚动暂停间隔

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

    hMainMusicOp->addStretch();

    // 上一首/暂停/下一首
    QHBoxLayout* hPlayOpLayout = new QHBoxLayout();
    _btnPrev->setIcon(QIcon(":/icons/previous.svg"));
    QIcon playIcon{":/icons/play.svg"};
    QIcon pauseIcon{":/icons/pause.svg"};
    _btnPlayPause->setIcon(playIcon);
    _btnNext->setIcon(QIcon(":/icons/next.svg"));
    hPlayOpLayout->addWidget(_btnPrev);
    hPlayOpLayout->addWidget(_btnPlayPause);
    hPlayOpLayout->addWidget(_btnNext);
    hMainMusicOp->addLayout(hPlayOpLayout);

    hMainMusicOp->addStretch();

    vBL->addLayout(hMainMusicOp);

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
    });

    /* musicPaused (音乐暂停) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPaused, this,
        [this, playIcon = std::move(playIcon)]() {
        _btnPlayPause->setIcon(playIcon);
    });

    /* musicResumed (音乐继续) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicResumed, this,
        [this, pauseIcon]() {
        _btnPlayPause->setIcon(pauseIcon);
    });

    // 播放与继续按钮
    connect(_btnPlayPause, &QPushButton::clicked, this,
        [this, pauseIcon]() {
        if (GlobalSingleton::get().musicConfig.isPlay) {
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

    // 上一首
    connect(_btnPrev, &QPushButton::clicked, this, [this]() {
        MusicCommand::prevMusic();
    });

    // 下一首
    connect(_btnNext, &QPushButton::clicked, this, [this]() {
        MusicCommand::nextMusic();
    });

    /* musicPlayPosChanged (播放毫秒改变) */
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPlayPosChanged, this,
        [this](qint64 pos) {
        if (_sliderPlayBar->isSliderDown()) // 如果是在拖动, 就先不更新位置了
            return;
        _sliderPlayBar->setSliderPosition(
            ((double)pos / GlobalSingleton::get().music.getLengthInMilliseconds()) * 1'00'00
        );
    });

    // 拖动条: 拖动位置释放 sliderReleased
    connect(_sliderPlayBar, &QSlider::sliderReleased, this, [this]() {
        MusicCommand::setMusicPos(
            ((double)_sliderPlayBar->sliderPosition() / 1'00'00) * GlobalSingleton::get().music.getLengthInMilliseconds()
        );
    });
}