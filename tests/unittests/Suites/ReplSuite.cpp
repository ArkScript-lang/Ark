#include <boost/ut.hpp>

#include <CLI/REPL/Utils.hpp>

#include <ranges>

using namespace boost;

ut::suite<"Repl"> repl_suite = [] {
    using namespace ut;

    const auto kws = Ark::internal::getAllKeywords();
    const auto colors = Ark::internal::getColorPerKeyword();
    const auto append_color = std::ranges::find_if(colors, [](const auto& pair) {
                                  return pair.first == "append";
                              })->second;

    "getters"_test = [&] {
        expect(that % kws.size() != 0);
        expect(that % kws.size() <= colors.size());
    };

    "hints"_test = [&] {
        int length = 5;
        const auto completions = Ark::internal::hookCompletion(kws, "appen", length);

        expect(that % completions.size() == 2);
        expect(std::ranges::find_if(completions, [](const replxx::Replxx::Completion& v) {
                   return v.text() == "append";
               }) != completions.end());
        expect(std::ranges::find_if(completions, [](const replxx::Replxx::Completion& v) {
                   return v.text() == "append!";
               }) != completions.end());

        replxx::Replxx::colors_t colored(6, replxx::Replxx::Color::DEFAULT);
        Ark::internal::hookColor(colors, "append", colored);
        expect(that % colored.size() == 6);
        expect(std::ranges::all_of(colored, [append_color](const replxx::Replxx::Color& c) {
            return c == append_color;
        }));

        length = 5;
        replxx::Replxx::Color color;
        const auto hints = Ark::internal::hookHint(kws, "toStr", length, color);

        expect(that % hints.size() == 1);
        expect(color == replxx::Replxx::Color::GREEN);
    };
};
