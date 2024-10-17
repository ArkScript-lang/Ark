#include <boost/ut.hpp>

#include <CLI/Formatter.hpp>

#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Formatter"> formatter_suite = [] {
    using namespace ut;

    iter_test_files(
        "FormatterSuite",
        [](TestData&& data) {
            std::string formatted_code;

            should("output a correctly formatted code for " + data.stem) = [&] {
                Formatter formatter(data.path, /* dry_run= */ true);
                expect(nothrow([&] {
                    mut(formatter).run();
                }));

                formatted_code = formatter.output();
                // data.expected is ltrim(rtrim(file content))
                // we want to ensure that a blank line has been added
                expect(that % formatted_code == (data.expected + "\n"));
            };

            should("not update an already correctly formatted code (" + data.stem + ")") = [&] {
                Formatter formatter(/* dry_run= */ true);
                expect(nothrow([&] {
                    mut(formatter).runWithString(formatted_code);
                }));

                const std::string code = formatter.output();
                expect(that % code == formatted_code);
            };
        });
};
