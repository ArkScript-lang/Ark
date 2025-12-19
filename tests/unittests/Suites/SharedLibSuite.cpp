#include <boost/ut.hpp>

#include <TestsHelper.hpp>

#include <Ark/VM/SharedLibrary.hpp>

using namespace boost;

ut::suite<"SharedLib"> shared_lib_suite = [] {
    using namespace ut;

    "[create an empty shared lib and destroy it]"_test = [] {
        Ark::internal::SharedLibrary lib;
        expect(lib.path().empty());
        expect(nothrow([&] {
            lib.unload();
        }));
    };

    "[load bad shared library]"_test = [] {
        Ark::internal::SharedLibrary lib;

        try
        {
            lib.load(getResourcePath("LangSuite/vm-tests.ark"));
            expect(false) << "loading .ark file should fail";
        }
        catch (const std::exception& e)
        {
            expect(std::string(e.what()).starts_with("Couldn't load the library at"));
        }
    };
};
