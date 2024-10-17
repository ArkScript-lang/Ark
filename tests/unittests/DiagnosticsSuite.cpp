#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Diagnostics"> diagnostics_suite = [] {
    using namespace ut;

    constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

    iter_test_files(
        "DiagnosticsSuite/compileTime",
        [](TestData&& data) {
            Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

            should("generate an error message at compile time for compileTime/" + data.stem) = [&] {
                try
                {
                    mut(state).doFile(data.path, features);
                    expect(0 == 1);  // we shouldn't be here, the compilation has to fail
                }
                catch (const Ark::CodeError& e)
                {
                    std::string diag = sanitize_error(e, /* remove_in_file_line= */ true);
                    rtrim(diag);
                    expect(that % diag == data.expected);
                }
            };
        });

    iter_test_files(
        "DiagnosticsSuite/runtime",
        [](TestData&& data) {
            Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

            should("compile without error runtime/" + data.stem) = [&] {
                expect(mut(state).doFile(data.path, features));
            };

            should("generate an error at runtime in " + data.stem) = [&] {
                try
                {
                    Ark::VM vm(state);
                    vm.run(/* fail_with_exception= */ true);
                    expect(0 == 1);  // we shouldn't be here, an error should be generatedds
                }
                catch (const std::exception& e)
                {
                    std::string diag = e.what();
                    if (diag.find_first_of('\n') != std::string::npos)
                        diag.erase(diag.find_first_of('\n'), diag.size() - 1);
                    ltrim(rtrim(diag));
                    expect(that % diag == data.expected);
                }
            };
        });
};
