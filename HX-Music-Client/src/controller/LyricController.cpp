// 傻逼QT, 定义你妈的 emit 宏, 冲突了, 草你妈的
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <controller/LyricController.h>

namespace HX {

AssParse LyricController::preprocessLyricBoundingBoxes(
    qint64 startTime, qint64 endTime, std::string_view data
) {
    constexpr int fps = 30;
    constexpr qint64 frameInterval = 1000 / fps;
    std::size_t n = static_cast<std::size_t>((endTime - startTime) / frameInterval) + 1;
    int midLine = _assParse.getHeight() >> 1;

    std::size_t maxConcurrency = std::thread::hardware_concurrency();
    tbb::task_arena task{static_cast<int>(maxConcurrency)};
    std::vector<AssParse> assArr(maxConcurrency);
    for (auto& assParse : assArr) {
        assParse.closeLog();
        assParse.setFrameSize(_assParse.getWidth(), _assParse.getHeight());
        assParse.readMemory(data);
    }
    TwoBlockBounds res;
    task.execute([&] {
        res = tbb::parallel_reduce(tbb::blocked_range<std::size_t>{0, n}, TwoBlockBounds{}, 
        [&](tbb::blocked_range<std::size_t> const& r, TwoBlockBounds const& rect) -> TwoBlockBounds {
            TwoBlockBounds res;
            AssParse& assParse = assArr[tbb::this_task_arena::current_thread_index()];
            for (auto i = r.begin(); i != r.end(); ++i) {
                int change;
                qint64 t = startTime + static_cast<qint64>(i) * frameInterval;
                auto* imgList = assParse.rendererFrame(t + _lyricConfig.lyricOffset, change);
                if (!imgList || !change) {
                    continue;
                }
                for (ASS_Image* img = imgList; img; img = img->next) {
                    int centerY = img->dst_y + (img->h >> 1);
                    if (centerY < midLine) {
                        res.topYMin = std::min(res.topYMin, img->dst_y);
                        res.topYMax = std::max(res.topYMax, img->dst_y + img->h);
                    } else {
                        res.btmYMin = std::min(res.btmYMin, img->dst_y);
                        res.btmYMax = std::max(res.btmYMax, img->dst_y + img->h);
                    }
                    res.left = std::min(res.left, img->dst_x);
                    res.right = std::max(res.right, img->dst_x + img->w);
                }
            }
            return res;
        }, [&](TwoBlockBounds const& res, TwoBlockBounds const& mae) noexcept -> TwoBlockBounds {
            return {
                std::min(res.topYMin, mae.topYMin),
                std::max(res.topYMax, mae.topYMax),
                std::min(res.btmYMin, mae.btmYMin),
                std::max(res.btmYMax, mae.btmYMax),
                std::min(res.left, mae.left),
                std::max(res.right, mae.right),
            };
        });
    });
    _hasCachedBlock = res.hasTop() || res.hasBtm();
    _twoBlockBounds = std::move(res);
    log::hxLog.info(_twoBlockBounds);
    return std::move(assArr.front());
}

/*
bug:
[INFO] {
    "topYMin": 360,
    "topYMax": 490,
    "btmYMin": 571,
    "btmYMax": 701,
    "left": 82,
    "right": 1082
} 

正常:
[INFO] {
    "topYMin": 360,
    "topYMax": 490,
    "btmYMin": 571,
    "btmYMax": 701,
    "left": 82,
    "right": 1920
}

*/

} // namespace HX