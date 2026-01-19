#include <boost/ut.hpp>

#include <fmt/ostream.h>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Debugger"> debugger_suite = [] {
    using namespace ut;

    constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureVMDebugger;

    iterTestFiles(
        "DebuggerSuite",
        [](TestData&& data) {
            std::stringstream os;
            Ark::State state({ lib_path });

            state.loadFunction("prn", [&os](const std::vector<Ark::Value>& args, Ark::VM* vm) -> Ark::Value {
                for (const auto& value : args)
                    fmt::print(os, "{}", value.toString(*vm));
                fmt::println(os, "");
                return Ark::Nil;
            });

            should("compile without error for " + data.stem) = [&] {
                expect(mut(state).doFile(data.path, features));
            };

            should("launch the debugger and compute expressions for " + data.stem) = [&] {
                std::filesystem::path prompt_path(data.path);
                prompt_path.replace_extension("prompt");

                try
                {
                    Ark::VM vm(state);
                    vm.usePromptFileForDebugger(prompt_path.generic_string(), os);
                    vm.run(/* fail_with_exception= */ false);

                    const std::string output = sanitizeOutput(os.str());
                    expectOrDiff(data.expected, output);
                }
                catch (const std::exception&)
                {
                    expect(false);
                }
            };
        });
};
