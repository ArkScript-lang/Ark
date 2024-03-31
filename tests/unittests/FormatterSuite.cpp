#include <boost/ut.hpp>

#include <CLI/Formatter.hpp>

#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Formatter"> formatter_suite = [] {
    using namespace ut;

    iter_test_files(
        "FormatterSuite",
        [](TestData&& data) {
            Formatter formatter(data.path, /* dry_run= */ true);
            should("output a correctly formatted code for " + data.stem) = [&] {
                expect(nothrow([&] {
                    mut(formatter).run();
                }));
                std::string code = formatter.output();
                expect(that % code == data.expected);
            };
        });
};