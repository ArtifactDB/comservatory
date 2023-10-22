#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "comservatory/read.hpp"

#include "utils.h"

bool convert_to_boolean(std::string x) {
    x = "\"foo\"\n" + x + "\n";
    byteme::RawBufferReader reader(reinterpret_cast<const unsigned char*>(x.c_str()), x.size());

    auto output = comservatory::read(reader, comservatory::ReadOptions());
    EXPECT_EQ(output.fields.size(), 1);
    EXPECT_EQ(output.fields[0]->type(), comservatory::BOOLEAN);

    const auto& v = static_cast<const comservatory::FilledBooleanField*>(output.fields[0].get())->values;
    EXPECT_EQ(v.size(), 1);
    return v[0];
}

TEST(ConvertTest, Boolean) {
    EXPECT_TRUE(convert_to_boolean("TRUE"));
    EXPECT_TRUE(convert_to_boolean("true"));
    EXPECT_TRUE(convert_to_boolean("True"));
    EXPECT_FALSE(convert_to_boolean("FALSE"));
    EXPECT_FALSE(convert_to_boolean("false"));
    EXPECT_FALSE(convert_to_boolean("False"));
}

TEST(ConvertTest, BooleanFail) {
    simple_conversion_fail("Tru", "truncated keyword");
    simple_conversion_fail("Tank", "unknown keyword");
    simple_conversion_fail("True", "last line must be terminated");
    simple_conversion_fail("falsey", "trailing");
}
