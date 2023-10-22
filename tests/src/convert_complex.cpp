#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "comservatory/read.hpp"
#include "utils.h"

std::complex<double> convert_to_complex(std::string x) {
    x = "\"foo\"\n" + x + "\n";
    byteme::RawBufferReader reader(reinterpret_cast<const unsigned char*>(x.c_str()), x.size());

    auto output = comservatory::read(reader, comservatory::ReadOptions());
    EXPECT_EQ(output.fields.size(), 1);
    EXPECT_EQ(output.fields[0]->type(), comservatory::COMPLEX);

    const auto& v = static_cast<const comservatory::FilledComplexField*>(output.fields[0].get())->values;
    EXPECT_EQ(v.size(), 1);
    return v[0];
}

TEST(ConvertTest, Complex) {
    {
        auto out = convert_to_complex("1.2+1.3i");
        EXPECT_EQ(out.real(), 1.2);
        EXPECT_EQ(out.imag(), 1.3);
    }

    {
        auto out = convert_to_complex("1+1i");
        EXPECT_EQ(out.real(), 1);
        EXPECT_EQ(out.imag(), 1);
    }

    {
        auto out = convert_to_complex("1.2-2e4i");
        EXPECT_EQ(out.real(), 1.2);
        EXPECT_EQ(out.imag(), -2e4);
    }

    {
        auto out = convert_to_complex("0+0i");
        EXPECT_EQ(out.real(), 0);
        EXPECT_EQ(out.imag(), 0);
    }

    {
        auto out = convert_to_complex("-9.2e3-7.6i");
        EXPECT_EQ(out.real(), -9.2e3);
        EXPECT_EQ(out.imag(), -7.6);
    }
}

TEST(ConvertTest, ComplexFail) {
    simple_conversion_fail("1+", "truncated complex number");
    simple_conversion_fail("1+\n", "incorrectly formatted complex number");
    simple_conversion_fail("1+i", "incorrectly formatted complex number");
    simple_conversion_fail("1i", "incorrectly formatted number");
    simple_conversion_fail("21+i", "incorrectly formatted complex number");
    simple_conversion_fail("21+5a", "invalid number containing 'a'");
    simple_conversion_fail("21+5,", "incorrectly formatted complex number");
}
