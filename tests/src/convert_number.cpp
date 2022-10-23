#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "byteme/RawBufferReader.hpp"
#include "comservatory/ReadCsv.hpp"

#include "utils.h"

double convert_to_number(std::string x) {
    x = "\"foo\"\n" + x + "\n";
    byteme::RawBufferReader reader(reinterpret_cast<const unsigned char*>(x.c_str()), x.size());
    comservatory::ReadCsv parser;

    auto output = parser.read(reader);
    EXPECT_EQ(output.fields.size(), 1);
    EXPECT_EQ(output.fields[0]->type(), comservatory::NUMBER);

    const auto& v = static_cast<const comservatory::FilledNumberField*>(output.fields[0].get())->values;
    EXPECT_EQ(v.size(), 1);
    return v[0];
}

TEST(ConvertTest, Integer) {
    // Basic example.
    EXPECT_EQ(convert_to_number("12345"), 12345);
    EXPECT_EQ(convert_to_number("+12345"), 12345);
    EXPECT_EQ(convert_to_number("-12345"), -12345);

    // Works with zeros.
    EXPECT_EQ(convert_to_number("0"), 0);
    EXPECT_EQ(convert_to_number("+0"), 0);
    EXPECT_EQ(convert_to_number("-0"), 0);

    // Works with leading zeros.
    EXPECT_EQ(convert_to_number("-01"), -1);
    EXPECT_EQ(convert_to_number("+01"), 1);
    EXPECT_EQ(convert_to_number("01"), 1);
}

TEST(ConvertTest, SimpleFloat) {
    EXPECT_FLOAT_EQ(convert_to_number("1.234"), 1.234);
    EXPECT_FLOAT_EQ(convert_to_number("-101.234"), -101.234);
    EXPECT_FLOAT_EQ(convert_to_number("0.5678"), 0.5678);
    EXPECT_FLOAT_EQ(convert_to_number("-0.0001"), -0.0001);
    EXPECT_FLOAT_EQ(convert_to_number("10.00"), 10);

    // Leading zeros are fine.
    EXPECT_FLOAT_EQ(convert_to_number("-010.01"), -10.01);
}

TEST(ConvertTest, ScientificFloat) {
    EXPECT_FLOAT_EQ(convert_to_number("1.2e10"), 1.2e10);
    EXPECT_FLOAT_EQ(convert_to_number("-3.45e+09"), -3.45e9);
    EXPECT_FLOAT_EQ(convert_to_number("1.999e-9"), 1.999e-9); 
    EXPECT_FLOAT_EQ(convert_to_number("3.1e-00"), 3.1); 
    EXPECT_FLOAT_EQ(convert_to_number("2.31e1"), 23.1); 
    EXPECT_FLOAT_EQ(convert_to_number("2e2"), 200); 
    EXPECT_FLOAT_EQ(convert_to_number("4e-3"), 0.004);

    // Works with a big E.
    EXPECT_FLOAT_EQ(convert_to_number("4E-3"), 0.004);
}

TEST(ConvertTest, SpecialNumber) {
    for (auto x : std::vector<std::string>{"inf", "Inf", "INF"}) {
        auto val = convert_to_number(x);
        EXPECT_TRUE(val > 0);
        EXPECT_TRUE(std::isinf(val));

        auto nval = convert_to_number("-" + x);
        EXPECT_TRUE(nval < 0);
        EXPECT_TRUE(std::isinf(nval));
    }

    for (auto x : std::vector<std::string>{"nan", "NAN", "NaN"}) {
        auto val = convert_to_number(x);
        EXPECT_TRUE(std::isnan(val));
        auto nval = convert_to_number("-" + x);
        EXPECT_TRUE(std::isnan(nval));
    }
}

TEST(ConvertTest, NumberFail) {
    simple_conversion_fail("10", "should be terminated with a newline");
    simple_conversion_fail("10L", "invalid number containing 'L'");

    simple_conversion_fail("10.", "should be terminated with a newline");
    simple_conversion_fail("10.\n", "must be followed by at least one digit");
    simple_conversion_fail("10.a\n", "must be followed by at least one digit");
    simple_conversion_fail("10.0", "should be terminated with a newline");
    simple_conversion_fail("10.1a\n", "invalid fraction containing 'a'");
    simple_conversion_fail("10.0.0\n", "invalid fraction containing '.'");
    simple_conversion_fail(".0\n", "unknown type starting with '.'");

    simple_conversion_fail("10e1\n", "absolute value");
    simple_conversion_fail("1e", "should be terminated with a newline");
    simple_conversion_fail("1e\n", "should be followed by a sign or digit");
    simple_conversion_fail("1e+", "should be terminated with a newline");
    simple_conversion_fail("1e+\n", "must be followed by at least one digit");
    simple_conversion_fail("1e+a\n", "must be followed by at least one digit");

    simple_conversion_fail("1e+0", "should be terminated with a newline");
    simple_conversion_fail("1e+1a", "invalid exponent containing 'a'");
    simple_conversion_fail("2e1.1", "invalid exponent containing '.'");

    simple_conversion_fail("N", "truncated keyword");
    simple_conversion_fail("Nt", "unknown keyword");
    simple_conversion_fail("NA", "should terminate with a newline");
    simple_conversion_fail("Na", "truncated keyword");
    simple_conversion_fail("Na\n", "unknown keyword");
}
