/**
 *  Copyright (C) 2026 HJimmyK(Jericho Knox)
 *
 *  This file is part of LAMMP.
 *
 *  LAMMP is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License (LGPL) as published
 *  by the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed WITHOUT ANY WARRANTY.
 *
 *  See <https://www.gnu.org/licenses/>.
 */

/**
 * 新版 LAMMP 单元测试框架（C++17，无第三方依赖）。
 *
 * 设计思路：
 *   1. 显式注册：通过 TEST_CASE 宏在静态初始化阶段把测试函数注册到全局注册表。
 *   2. 非致命断言：失败后记录并继续运行，便于在一次运行中收集多个错误。
 *   3. 命令行过滤：支持 --filter 子串过滤（匹配 分类/名称），--list 列出用例。
 *   4. 输出风格统一：所有输出均为 ASCII 英文，避免 Windows 控制台代码页问题。
 */

#ifndef LAMMP_TEST_FRAMEWORK_HPP
#define LAMMP_TEST_FRAMEWORK_HPP

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace lammp_test {

struct TestCase {
    const char* category;
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct AutoRegister {
    AutoRegister(const char* category, const char* name, void (*fn)()) {
        registry().push_back({category, name, fn});
    }
};

struct Context {
    const char* category = "";
    const char* name = "";
    int checks = 0;
    int failures = 0;
    bool failed = false;
};

inline Context& ctx() {
    static Context c;
    return c;
}

inline void report_fail(const char* file, int line, const std::string& msg) {
    ++ctx().checks;
    ++ctx().failures;
    ctx().failed = true;
    std::printf("  [FAIL] %s:%d  %s\n", file, line, msg.c_str());
}

inline void report_ok() {
    ++ctx().checks;
}

inline int run_all(const std::string& filter) {
    int total = 0;
    int failed = 0;
    int checks = 0;

    for (const auto& tc : registry()) {
        std::string full = std::string(tc.category) + "/" + tc.name;
        if (!filter.empty() && full.find(filter) == std::string::npos)
            continue;

        ++total;
        ctx() = Context{tc.category, tc.name, 0, 0, false};
        std::printf("[ RUN    ] %s\n", full.c_str());

        tc.fn();

        checks += ctx().checks;
        if (ctx().failed) {
            ++failed;
            std::printf("[  FAILED] %s  (%d failure(s), %d check(s))\n",
                        full.c_str(), ctx().failures, ctx().checks);
        } else {
            std::printf("[  PASSED] %s  (%d check(s))\n", full.c_str(), ctx().checks);
        }
    }

    std::printf("========================================================\n");
    std::printf("TOTAL: %d test(s), %d failed, %d check(s)\n", total, failed, checks);
    return failed == 0 ? 0 : 1;
}

inline void list_all() {
    for (const auto& tc : registry()) {
        std::printf("%s/%s\n", tc.category, tc.name);
    }
}

}  // namespace lammp_test

#define TEST_CASE(category, name)                                            \
    static void test_fn_##name();                                            \
    static ::lammp_test::AutoRegister test_ar_##name(category, #name,       \
                                                       test_fn_##name);      \
    static void test_fn_##name()

#define TEST_CHECK(cond)                                                     \
    do {                                                                     \
        if (cond) {                                                          \
            ::lammp_test::report_ok();                                       \
        } else {                                                             \
            ::lammp_test::report_fail(__FILE__, __LINE__,                    \
                                      "CHECK failed: " #cond);               \
        }                                                                    \
    } while (0)

#define TEST_CHECK_MSG(cond, msg)                                            \
    do {                                                                     \
        if (cond) {                                                          \
            ::lammp_test::report_ok();                                       \
        } else {                                                             \
            ::lammp_test::report_fail(__FILE__, __LINE__,                    \
                                      std::string("CHECK failed: ") + #cond + \
                                          "  (" + std::string(msg) + ")");   \
        }                                                                    \
    } while (0)

#define TEST_CHECK_EQ(a, b)                                                  \
    do {                                                                     \
        auto _va = (a);                                                      \
        auto _vb = (b);                                                      \
        if (_va == _vb) {                                                    \
            ::lammp_test::report_ok();                                       \
        } else {                                                             \
            char _buf[256];                                                  \
            std::snprintf(_buf, sizeof(_buf),                                \
                          "CHECK_EQ failed: %s == %s", #a, #b);             \
            ::lammp_test::report_fail(__FILE__, __LINE__, _buf);             \
        }                                                                    \
    } while (0)

#endif  // LAMMP_TEST_FRAMEWORK_HPP
