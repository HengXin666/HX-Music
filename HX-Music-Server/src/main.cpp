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
    // 明天写一个 反射获取 类名称的. 这样如果不指定表名称, 就使用结构体名称
    auto db = db::open("./test.db");
    db.createDatabase<Man>();
    Man t {
        1433223, "战士", 0.721
    };
    // db.insert(t);
    auto res = db.queryAll<Man>();
    log::hxLog.info("res:", res);

    // db.deleteBy<Man>().where("id = ? or name = ?");

    std::string sql = "UPDATE users SET score = ?, (level >= ?) WHERE +[-age_aA1 <= ?] AND x.name != ?";
    auto fields = db::internal::extractFields(sql);

    log::hxLog.info(fields);
    return 0;
}
