#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <Ark/Compiler/Welder.hpp>

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
                expect(that % fmt::format("StateError: Compiler and VM versions don't match: got {0}.{2}.{3} while running {1}.{2}.{3}", 1, ARK_VERSION_MAJOR, ARK_VERSION_MINOR, ARK_VERSION_PATCH) == e.what());
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
};
