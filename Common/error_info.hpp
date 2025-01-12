#pragma once
#include <source_location>
#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <ranges>
#include <filesystem>

class error_info {
    std::vector<std::source_location> traceback_;
    std::string message_;
public:
    error_info(const error_info& a) = default; 
    error_info& operator=(const error_info& a) = default;
    error_info(error_info&& a) noexcept = default;
    error_info& operator=(error_info& a) noexcept = default;
    ~error_info() = default;
    explicit error_info(std::string message, std::source_location sl = std::source_location::current()):
        message_(std::move(message)) {
        traceback_.emplace_back(std::move(sl));
    }
    error_info& propagate(std::source_location sl = std::source_location::current()) {
        traceback_.emplace_back(sl);
        return *this;
    }
    const std::vector<std::source_location>& traceback() const {
        return traceback_;
    }
    const std::string& message() const {
        return message_;
    }
    std::string formatted() const {
        namespace fs = std::filesystem;
        std::string ret =  std::format("{}:\n", message_);
        for (const auto& sl : traceback_ | std::views::reverse) {
            const std::string filename = fs::relative(fs::path(sl.file_name())).string();
            std::string_view fname = sl.function_name();
            const bool caused_here = fname == traceback_.front().function_name();
            const std::string_view symbol = caused_here ? "*" : "^";
#if defined (_MSC_VER)
            constexpr std::string_view cdecl_keyw = "__cdecl ";
            fname = fname.substr(fname.find(cdecl_keyw) + cdecl_keyw.size());
            fname = fname.substr(0, fname.find("("));
#endif
            ret += std::format(
                "  {} {}:{}: in function '{}'\n",
                symbol, filename, sl.line(), fname);
        }
        ret.pop_back();
        return ret;
    }
    friend std::ostream& operator<<(std::ostream& os, const error_info& error) {
        return os << error.formatted();
    }
};