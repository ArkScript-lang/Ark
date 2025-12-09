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
                    std::string diag = sanitizeCodeError(e, /* remove_in_file_line= */ true);
                    Ark::Utils::rtrim(diag);
                    expectOrDiff(data.expected, diag);
                    if (shouldWriteNewDiffsTofile() && data.expected != diag)
                        updateExpectedFile(data, diag);
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
                    std::string diag = sanitizeRuntimeError(e);
                    expectOrDiff(data.expected, diag);
                    if (shouldWriteNewDiffsTofile() && data.expected != diag)
                        updateExpectedFile(data, diag);
                }
            };
        });

    iterTestFiles(
        "DiagnosticsSuite/typeChecking",
        [](TestData&& data) {
            Ark::State state({ lib_path });

            should("compile without error typeChecking/" + data.stem) = [&] {
                expect(mut(state).doFile(data.path, features));
            };

            should("generate an error at runtime (typeChecking) in " + data.stem) = [&] {
                try
                {
                    Ark::VM vm(state);
                    vm.run(/* fail_with_exception= */ true);
                    expect(0 == 1);  // we shouldn't be here, an error should be generated
                }
                catch (const std::exception& e)
                {
                    std::string diag = sanitizeRuntimeError(e);
                    expectOrDiff(data.expected, diag);
                    if (shouldWriteNewDiffsTofile() && data.expected != diag)
                        updateExpectedFile(data, diag);
                }
            };
        });
};
