#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Diagnostics"> diagnostics_suite = [] {
    using namespace ut;

    constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

    iterTestFiles(
        "DiagnosticsSuite/compileTime",
        [](TestData&& data) {
            Ark::State state({ lib_path });

            should("generate an error message at compile time for compileTime/" + data.stem) = [&] {
                try
                {
                    const bool ok = mut(state).doFile(data.path, features);
                    expect(!ok) << fatal;  // we shouldn't be here, the compilation has to fail
                }
                catch (const Ark::CodeError& e)
                {
                    std::string diag = sanitizeError(e, /* remove_in_file_line= */ true);
                    rtrim(diag);
                    expectOrDiff(data.expected, diag);
                }
            };
        });

    iterTestFiles(
        "DiagnosticsSuite/runtime",
        [](TestData&& data) {
            Ark::State state({ lib_path });

            should("compile without error runtime/" + data.stem) = [&] {
                expect(mut(state).doFile(data.path, features));
            };

            should("generate an error at runtime in " + data.stem) = [&] {
                try
                {
                    Ark::VM vm(state);
                    vm.run(/* fail_with_exception= */ true);
                    expect(0 == 1);  // we shouldn't be here, an error should be generated
                }
                catch (const std::exception& e)
                {
                    std::string diag = e.what();
                    // because of windows
                    diag.erase(std::ranges::remove(diag, '\r').begin(), diag.end());
                    // remove the directory prefix so that we are environment agnostic
                    while (diag.find(ARK_TESTS_ROOT) != std::string::npos)
                        diag.erase(diag.find(ARK_TESTS_ROOT), std::size(ARK_TESTS_ROOT) - 1);
                    ltrim(rtrim(diag));
                    // remove last line, At IP:.., PP:.., SP:..
                    diag.erase(diag.find_last_of('\n'), diag.size() - 1);
                    // we most likely have a blank line at the end now
                    rtrim(diag);
                    expectOrDiff(data.expected, diag);
                }
            };
        });
};
