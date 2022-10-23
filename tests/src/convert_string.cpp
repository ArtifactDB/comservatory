#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "comservatory/convert.hpp"

#include "utils.h"

std::string convert_to_string(std::string x) {
    x = "\"foo\"\n\"" + x + "\"\n";
    byteme::RawBufferReader reader(reinterpret_cast<const unsigned char*>(x.c_str()), x.size());
    comservatory::ReadCsv parser;

    auto output = parser.read(reader);
    EXPECT_EQ(output.fields.size(), 1);
    EXPECT_EQ(output.fields[0]->type(), comservatory::STRING);

    const auto& v = static_cast<const comservatory::FilledStringField*>(output.fields[0].get())->values;
    EXPECT_EQ(v.size(), 1);
    return v[0];
}

TEST(ConvertTest, String) {
    {
        std::string x = "asdasdasd";
        EXPECT_EQ(convert_to_string(x), x);
    }

    // Empty string works.
    {
        std::string x = "";
        EXPECT_EQ(convert_to_string(x), x);
    }

    // Unusual characters, no problem. 
    {
        std::string x = "asdas\ndasd";
        EXPECT_EQ(convert_to_string(x), x);
    }

    {
        std::string x = "asdas,dasd";
        EXPECT_EQ(convert_to_string(x), x);
    }

    // Internal quotes, no problem. 
    {
        std::string x = "asdas\"\"dasd";
        EXPECT_EQ(convert_to_string(x), "asdas\"dasd");
    }
}

TEST(ConvertTest, StringFail) {
    simple_conversion_fail("asdasd\"", "unknown type starting with 'a'");
    simple_conversion_fail("\"asdasd", "truncated string");
    simple_conversion_fail("\"asdasd\"", "terminated with a newline");
    simple_conversion_fail("\"asdasd\"asdasd\"\n", "trailing character 'a'");
    simple_conversion_fail("\"asdasdasdasd\"\"\n", "truncated string");
}
