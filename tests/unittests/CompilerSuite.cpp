#include <boost/ut.hpp>

#include <Ark/Compiler/Word.hpp>

using namespace boost;

ut::suite<"Compiler"> compiler_suite = [] {
    using namespace ut;

    "Word construction"_test = [] {
        should("create a word with a single argument on 2 bytes") = [] {
            const auto word = Ark::internal::Word(12, 0x5678);
            expect(that % word.opcode == 12);
            expect(that % word.byte_1 == 0);
            expect(that % word.byte_2 == 0x56);
            expect(that % word.byte_3 == 0x78);
        };

        should("split arguments evenly between 3 bytes") = [] {
            const auto word = Ark::internal::Word(12, 0x0567, 0x089a);
            expect(that % word.opcode == 12);
            expect(that % word.byte_1 == 0x56);
            expect(that % word.byte_2 == 0x78);
            expect(that % word.byte_3 == 0x9a);
        };
    };
};
