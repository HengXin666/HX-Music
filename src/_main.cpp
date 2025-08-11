#include <utils/AssParse.hpp>

#include <HXLibs/log/Log.hpp>

using namespace HX;

int _main(int argc, char* argv[]) {
    auto [a, b] = HX::internal::separateAssFile("/run/media/loli/アニメ専門/音乐/ass/いとうかなこ - ファティマ .ass");
    HX::log::hxLog.info(a, "\n\n", b);

    // log::hxLog.info(internal::containsVerticalEffect(R"(Dialogue: 1,0:01:04.90,0:01:04.91,OP JP 6,,0,0,0,fx,{\an5\blur1\clip(755,22,815,27)\pos(786,37)}と)"));
    return 0;
}