#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comservatory/convert.hpp"

TEST(ConvertTest, Complex) {
    {
        std::string x = "1.2+1.3i";
        auto out = comservatory::to_complex(x.c_str(), x.size());
        EXPECT_EQ(out.real(), 1.2);
        EXPECT_EQ(out.imag(), 1.3);
    }

    {
        std::string x = "1.2-2e4i";
        auto out = comservatory::to_complex(x.c_str(), x.size());
        EXPECT_EQ(out.real(), 1.2);
        EXPECT_EQ(out.imag(), -2e4);
    }

    {
        std::string x = "-9.2e3-7.6i";
        auto out = comservatory::to_complex(x.c_str(), x.size());
        EXPECT_EQ(out.real(), -9.2e3);
        EXPECT_EQ(out.imag(), -7.6);
    }
}

TEST(ConvertTest, ComplexFail) {
    {
        std::string x = "1";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_complex(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("last character"));
                throw;
            }
        });
    }

    {
        std::string x = "1i";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_complex(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("could not find separator"));
                throw;
            }
        });
    }

    {
        std::string x = "++1i";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_complex(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("failed to parse the real part"));
                throw;
            }
        });
    }

    {
        std::string x = "1+i";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_complex(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("failed to parse the imaginary part"));
                throw;
            }
        });
    }
}

