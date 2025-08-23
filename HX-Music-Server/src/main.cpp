#include <HXLibs/log/Log.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // pybind11::scoped_interpreter

namespace py = pybind11;

auto hx_init = []{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    using namespace HX;
    try {
        auto cwd = std::filesystem::current_path();
        log::hxLog.debug("当前工作路径是:", cwd);
        std::filesystem::current_path("../../");
        log::hxLog.debug("切换到路径:", std::filesystem::current_path());
    } catch (const std::filesystem::filesystem_error& e) {
        log::hxLog.error("Error:", e.what());
    }
    return 0;
}();

int _main() {
    py::scoped_interpreter guard{};

    // 添加 Python 脚本目录到 sys.path
    py::module sys = py::module::import("sys");
    sys.attr("path").cast<py::list>().append("./pyTool");

    // 导入模块
    py::module myscript = py::module::import("main");

    myscript.attr("lddcAssToKaraokeAss")("./pyTool/test.ass", "./cpp.ass");

    return 0;
}

#include <db/SQLiteDB.hpp>

using namespace HX;

struct Man {
    int id;
    std::string name;
    double okane;
};

int main() {
    auto db = db::open("./test.db");
    db.createDatabase<Man>("man");
    Man t {
        1433223, "战士", 0.721
    };
    db.insert("man", t);
    auto res = db.queryAll<Man>("select * from man");
    log::hxLog.info("res:", res);
    return 0;
}
