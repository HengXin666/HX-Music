#include <widget/TimeProgressText.h>

#include <singleton/SignalBusSingleton.h>

TimeProgressText::TimeProgressText(QWidget* parent)
    : QWidget(parent)
{
    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPlayPosChanged, this,
        [this](qint64 pos) {
        int sec = pos / 1000;
        updateNowTime(
            sec < 3600 
                ? QTime{0, 0}.addSecs(sec).toString("mm:ss")
                : QTime{0, 0}.addSecs(sec).toString("hh:mm:ss")
        );
    });

    connect(&SignalBusSingleton::get(), &SignalBusSingleton::newSongLoaded, this,
        [this](HX::MusicInfo const& info) {
        updateTotalTime(info.formatTimeLengthToHHMMSS());
    });
}