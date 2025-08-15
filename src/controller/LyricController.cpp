// 傻逼QT, 定义你妈的 emit 宏, 冲突了, 草你妈的
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <controller/LyricController.h>

namespace HX {

AssParse LyricController::preprocessLyricBoundingBoxes(
    qint64 startTime, qint64 endTime, std::string_view data
) {
    struct _tb_lr {
        int topMin;
        int topMax;
        int btmMin;
        int btmMax;
    };
    constexpr int fps = 30;
    constexpr qint64 frameInterval = 1000 / fps;
    std::size_t n = (endTime - startTime) / frameInterval;
    int topMinY = INT_MAX, topMaxY = 0;
    int bottomMinY = INT_MAX, bottomMaxY = 0;
    int midLine = _assParse.getHeight() >> 1;

    _tb_lr ans;
    std::size_t maxConcurrency = std::thread::hardware_concurrency();
    tbb::task_arena task{static_cast<int>(maxConcurrency)};
    std::vector<AssParse> assArr(maxConcurrency);
    for (auto& assParse : assArr) {
        assParse.closeLog();
        assParse.setFrameSize(_assParse.getWidth(), _assParse.getHeight());
        assParse.readMemory(data.data());
    }
    task.execute([&] {
        ans = tbb::parallel_reduce(tbb::blocked_range<std::size_t>{0, n}, _tb_lr{}, 
        [&](tbb::blocked_range<std::size_t> const& r, _tb_lr const& rect) -> _tb_lr {
            int topMinY = INT_MAX, topMaxY = 0;
            int bottomMinY = INT_MAX, bottomMaxY = 0;
            AssParse& assParse = assArr[tbb::this_task_arena::current_thread_index()];
            for (auto it = r.begin(); it != r.end(); ++it) {
                int change;
                int t = it * frameInterval;
                auto* imgList = assParse.rendererFrame(t + _lyricConfig.lyricOffset, change);
                if (!imgList || !change) {
                    continue;
                }
                for (ASS_Image* img = imgList; img; img = img->next) {
                    int centerY = img->dst_y + (img->h >> 1);
                    if (centerY < midLine) {
                        topMinY = std::min(topMinY, img->dst_y);
                        topMaxY = std::max(topMaxY, img->dst_y + img->h);
                    } else {
                        bottomMinY = std::min(bottomMinY, img->dst_y);
                        bottomMaxY = std::max(bottomMaxY, img->dst_y + img->h);
                    }
                }
            }
            return {topMinY, topMaxY, bottomMinY, bottomMaxY};
        }, [&](_tb_lr const& res, _tb_lr const& mae) -> _tb_lr {
            auto& [ti, ta, bi, ba] = res;
            auto const& [_ti, _ta, _bi, _ba] = mae;
            return {
                std::min(ti, _ti),
                std::max(ta, _ta),
                std::min(bi, _bi),
                std::max(ba, _ba)
            };
        });
    });
    auto [ti, ta, bi, ba] = ans;
    _cachedTopYLR = QPoint{ti, ta};
    _cachedBottomYLR = QPoint{bi, ba};
    _hasCachedY = !(_cachedTopYLR.isNull() && _cachedBottomYLR.isNull());
    return std::move(assArr.front());
}

} // namespace HX