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
#include <db/SQLiteMeta.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

using namespace HX;

struct Man {
    int id;
    std::string name;
    double okane;
    std::vector<std::string> arr;
};

namespace HX::db {

template <typename U>
struct SQLiteSqlType<std::vector<U>> {
    using T = std::vector<U>;
    static constexpr std::string bind(T const& t) noexcept {
        std::string res;
        reflection::toJson(t, res);
        return res;
    }

    static constexpr T columnType(std::string_view str) {
        T t{};
        reflection::fromJson(t, str);
        return t;
    }
};

} // namespace HX::db

int _tx_main() {
    auto db = db::open("./test.db");
    db.createDatabase<Man>();
    Man t {
        1433223, "战士", 0.721, {"1", "a", "#"}
    };
    db.insert(t);
    auto res = db.queryAll<Man>();
    log::hxLog.info("res:", res);

    db.deleteBy<Man>("where id = ?").bind(2233).exec();
    db.updateBy(Man{
        2233, "xxbb", 6.66, {"在", "あの場所"}
    }, "").exec();

    res = db.queryAll<Man>();
    log::hxLog.warning("res:", res);
    return 0;
}

#include <api/MusicApi.hpp>

int main() {
    net::HttpServer server{"0.0.0.0", "28205"};
    MusicApi {server};
    server.syncRun();
    return 0;
}
