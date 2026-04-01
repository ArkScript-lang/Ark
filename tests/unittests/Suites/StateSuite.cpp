#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>

using namespace boost;

ut::suite<"State"> state_suite = [] {
    using namespace ut;

    "[load bytecode with bad version]"_test = [] {
        Ark::Welder welder(0, {}, Ark::DefaultFeatures);

        should("compile the string without any error") = [&] {
            expect(mut(welder).computeASTFromString("(let foo (fun () 4))"));
            expect(mut(welder).generateBytecode());
        };

        auto bytecode = welder.bytecode();

        should("throw a miss-matching compiler/vm version error") = [&] {
            // set major version to 1
            bytecode[5] = 1;

            Ark::State state;
            try
            {
                state.feed(bytecode, /* fail_with_exception= */ true);
                expect(false) << "state should throw an exception";
            }
            catch (const std::exception& e)
            {
                expect(that % fmt::format("StateError: Compiler and VM versions don't match: got {0}.{1}.{2} while running {3}", 1, ARK_VERSION_MINOR, ARK_VERSION_PATCH, ARK_VERSION) == e.what());
            }
        };
    };

    "[load bytecode with non matching checksum]"_test = [] {
        Ark::Welder welder(0, {}, Ark::DefaultFeatures);

        should("compile the string without any error") = [&] {
            expect(mut(welder).computeASTFromString("(let foo (fun () 4))"));
            expect(mut(welder).generateBytecode());
        };

        auto bytecode = welder.bytecode();

        should("throw a miss-matching checksum error") = [&] {
            // update a byte to 255 in the checksum,
            // so that the checksum will fail
            bytecode[Ark::internal::bytecode::HeaderSize + 2] = 255;

            Ark::State state;
            try
            {
                state.feed(bytecode, /* fail_with_exception= */ true);
                expect(false) << "state should throw an exception";
            }
            catch (const std::exception& e)
            {
                expect(that % std::string("StateError: Integrity check failed") == e.what());
            }
        };
    };

    "[compile code with known symbols and constants]"_test = [] {
        Ark::Welder welder(0, {}, Ark::DefaultFeatures);
        const std::vector<std::string> additional_symbols = { "bar", "egg", "a", "b", "c" };
        const std::vector<Ark::Value> additional_constants = { Ark::Value("bar"), Ark::Value("egg") };

        should("compile the string without any error") = [&] {
            expect(mut(welder).computeASTFromStringWithKnownSymbols("(let foo bar)", additional_symbols));
            expect(mut(welder).generateBytecodeUsingTables(additional_symbols, additional_constants, 0));
        };

        const Ark::bytecode_t bytecode = welder.bytecode();
        Ark::State state;

        should("accept the bytecode") = [&] {
            expect(nothrow([&] {
                mut(state).feed(bytecode, /* fail_with_exception= */ true);
            }));
        };

        Ark::BytecodeReader bcr;
        bcr.feed(bytecode);
        const auto symbols_block = bcr.symbols();
        const auto values_block = bcr.values(symbols_block);

        should("list all symbols") = [symbols_block, additional_symbols] {
            using namespace std::literals::string_literals;

            std::vector<std::string> expected_symbols = additional_symbols;
            expected_symbols.emplace_back("foo");

            const std::size_t symbols_bytes_count = std::accumulate(
                expected_symbols.begin(),
                expected_symbols.end(),
                expected_symbols.size(),
                [](const std::size_t acc, const std::string& sym) {
                    return acc + sym.size();
                });

            expect(that % symbols_block.symbols == expected_symbols);
            // 'ark\0' + version (2 bytes per number) + timestamp + sha -> first byte of the sym table
            expect(that % symbols_block.start == 4 + 6 + 8 + 32ull);
            // 50 = 4 + 6 + 8 + 32
            // + 1 for the header
            // + 2 because we need to count the size of the table (uint16)
            // + 3 because we need to count the \0
            expect(that % symbols_block.end == 50 + 1 + 2 + symbols_bytes_count);
        };

        should("list all values") = [symbols_block, values_block, additional_constants] {
            expect(that % values_block.values.size() == additional_constants.size());
            expect(that % values_block.start == symbols_block.end);
            expect(values_block.values[0] == additional_constants[0]);
            expect(values_block.values[1] == additional_constants[1]);
        };
    };

    "[extend bytecode]"_test = [] {
        Ark::Welder welder(0, {}, Ark::DefaultFeatures);
        should("compile the string without any error") = [&] {
            expect(mut(welder).computeASTFromString("(let foo 5) (let bar (fun (a b c) (+ a b c)))"));
            expect(mut(welder).generateBytecode());
        };

        const Ark::bytecode_t bytecode = welder.bytecode();
        Ark::State state;
        should("accept the bytecode") = [&] {
            expect(nothrow([&] {
                mut(state).feed(bytecode, /* fail_with_exception= */ true);
            }));
        };

        Ark::BytecodeReader bcr;
        bcr.feed(bytecode);
        const auto symbols_block = bcr.symbols();
        const auto values_block = bcr.values(symbols_block);

        {
            Ark::Welder welder2(0, {}, Ark::DefaultFeatures);
            should("compute additional code") = [&] {
                expect(mut(welder2).computeASTFromStringWithKnownSymbols("(let x foo) (let y 10) (print (bar foo foo foo))", symbols_block.symbols));
            };

            should("generate bytecode using existing bytecode tables") = [&] {
                // we have two existing pages in the original code
                expect(mut(welder2).generateBytecodeUsingTables(symbols_block.symbols, values_block.values, 2));
            };

            Ark::BytecodeReader bcr2;
            bcr2.feed(welder2.bytecode());
            const auto syms = bcr2.symbols();
            const auto vals = bcr2.values(syms);
            const auto files = bcr2.filenames(vals);
            const auto inst_locs = bcr2.instLocations(files);
            const auto [pages, _] = bcr2.code(inst_locs);

            should("have compiled a single additional page") = [&] {
                expect(that % pages.size() == 1u);
            };

            should("extend bytecode without errors") = [&] {
                expect(nothrow([&] {
                    mut(state).extendBytecode(pages, syms.symbols, vals.values);
                }));
            };
        }
    };
};
