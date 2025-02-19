#pragma once
#include <dluau.h>
#include <regex>
#include <expected>
#include <optional>
#include <chrono>
#include <span>
#include <set>
#include <boost/container/flat_map.hpp>
#include <ranges>
#include <bitset>
#include <filesystem>
#include <common.hpp>
#include <variant>
#include <memory>
namespace dluau {
using boost::container::flat_map;
using namespace std::string_literals;
using std::string, std::string_view;
using std::span, std::array;
using std::expected, std::unexpected;
using std::format_string, std::format;
namespace fs = std::filesystem;
extern lua_CompileOptions* compile_options;
static const array def_file_exts = {".luau"s, ".lua"s};
constexpr char arg_separator{','};
inline string_view args;
struct Preprocessed_file {
    fs::path normalized_path;
    string identifier;
    string source;
};
auto get_script_paths() -> const flat_map<lua_State*, string>&;
auto get_aliases() -> const flat_map<string, string>&;
auto resolve_require_path(lua_State* L, string name, span<const string> file_exts = def_file_exts) -> expected<string, string>;
auto resolve_path(string name, const fs::path& base, span<const string> = def_file_exts) -> expected<string, string>;
auto load_file(lua_State* L, string_view path) -> expected<lua_State*, string>;
auto load_file(lua_State* L, const Preprocessed_file& pf) -> expected<lua_State*, string>;
auto run_file(lua_State* L, const Preprocessed_file& pf) -> expected<void, string>;
auto run_file(lua_State* L, string_view script_path) -> expected<void, string>;
auto tasks_in_progress() -> bool;
auto task_step(lua_State* L) -> expected<void, string>;
auto has_permissions(lua_State* L) -> bool;
auto default_useratom(const char* key, size_t len) -> int16_t;
auto precompile(string& source) -> bool;
auto precompile(string& source, span<const std::pair<std::regex, string>> sv) -> bool;
namespace detail {
auto get_script_paths() -> flat_map<lua_State*, string>&;
}
inline auto get_precompiled_library_values(const fs::path& p) -> decltype(auto) {
    auto as_string_literal = [](const fs::path& path) {
        auto str = path.string();
        std::ranges::replace(str, '\\', '/');
        return format("(\"{}\")", str);
    };
    const auto arr = std::to_array<std::pair<std::regex, string>>({
        {std::regex(R"(\bscript.directory\b)"), as_string_literal(p.parent_path())},
        {std::regex(R"(\bscript.path\b)"), as_string_literal(p)},
        {std::regex(R"(\bscript.name\b)"), as_string_literal(fs::path(p).stem())},
    });
    return arr;
}
inline auto preprocess_source(const fs::path& path, std::set<std::string>* std_dep = nullptr) -> expected<Preprocessed_file, string> {
    auto source = common::read_file(path);
    if (not source) {
        return unexpected(format("couldn't read source '{}'.", path.string()));
    }
    if (std_dep) {
        std::regex std_pattern(R"(std\.(\w+))");
        std::smatch match;
        std::string src = *source;
        while(std::regex_search(src, match, std_pattern)) {
            std_dep->insert(match[1].str());
            src = match.suffix();
        }
    }
    Preprocessed_file data{};
    data.normalized_path = common::normalize_path(path);
    dluau::precompile(*source, get_precompiled_library_values(data.normalized_path));
    string identifier{fs::relative(path).string()};
    std::ranges::replace(identifier, '\\', '/');
    identifier = '=' + identifier;
    data.identifier = std::move(identifier);
    data.source = std::move(*source);
    return data;
}
}
