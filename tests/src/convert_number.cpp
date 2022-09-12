#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comservatory/convert.hpp"

TEST(ConvertTest, Integer) {
    // Basic example.
    {
        std::string x = "12345";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 12345);
    }

    {
        std::string x = "+12345";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 12345);
    }

    {
        std::string x = "-12345";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), -12345);
    }

    // Works with zeros.
    {
        std::string x = "0";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 0);
    }

    {
        std::string x = "+0";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 0);
    }

    {
        std::string x = "-0";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 0);
    }

    // Works with leading zeros.
    {
        std::string x = "-01";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), -1);
    }

    {
        std::string x = "+01";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 1);
    }

    {
        std::string x = "01";
        EXPECT_EQ(comservatory::to_number(x.c_str(), x.size()), 1);
    }
}

TEST(ConvertTest, SimpleFloat) {
    {
        std::string x = "1.234";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 1.234);
    }

    {
        std::string x = "-101.234";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), -101.234);
    }

    {
        std::string x = "0.5678";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 0.5678);
    }

    {
        std::string x = "-0.0001";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), -0.0001);
    }

    {
        std::string x = "10.00";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 10);
    }

    // Leading zeros are fine.
    {
        std::string x = "-010.01";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), -10.01);
    }
}

TEST(ConvertTest, ScientificFloat) {
    {
        std::string x = "1.2e10";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 1.2e10);
    }
    
    {
        std::string x = "-3.45e+09";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), -3.45e9);
    }

    {
        std::string x = "1.999e-9";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 1.999e-9); 
    }

    {
        std::string x = "3.1e-00";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 3.1); 
    }

    {
        std::string x = "2.31e1";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 23.1); 
    }

    {
        std::string x = "2e2";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 200); 
    }

    {
        std::string x = "4e-3";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 0.004);
    }

    // Works with a big E.
    {
        std::string x = "4E-3";
        EXPECT_FLOAT_EQ(comservatory::to_number(x.c_str(), x.size()), 0.004);
    }
}

TEST(ConvertTest, Special) {
    {
        std::string x = "inf";
        auto val = comservatory::to_number(x.c_str(), x.size());
        EXPECT_TRUE(val > 0);
        EXPECT_TRUE(std::isinf(val));
    }

    {
        std::string x = "-Inf";
        auto val = comservatory::to_number(x.c_str(), x.size());
        EXPECT_TRUE(val < 0);
        EXPECT_TRUE(std::isinf(val));
    }

    {
        std::string x = "-nan";
        auto val = comservatory::to_number(x.c_str(), x.size());
        EXPECT_TRUE(std::isnan(val));
    }

    {
        std::string x = "Nan";
        auto val = comservatory::to_number(x.c_str(), x.size());
        EXPECT_TRUE(std::isnan(val));
    }
}

TEST(ConvertTest, IntegerFail) {
    {
        std::string x = "10.0";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple<true>(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("only integer values"));
                throw;
            }
        });
    }

    {
        std::string x = "10L";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple<true>(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("invalid character"));
                throw;
            }
        });
    }

    {
        std::string x = "";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple<true>(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("no numbers"));
                throw;
            }
        });
    }

    {
        std::string x = "+";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple<true>(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("no numbers"));
                throw;
            }
        });
    }
}

TEST(ConvertTest, SimpleFloatFail) {
    {
        std::string x = "10.";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("after the decimal point"));
                throw;
            }
        });
    }

    {
        std::string x = ".0";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("before the decimal point"));
                throw;
            }
        });
    }

    {
        std::string x = "0...0";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number_simple(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("multiple decimal points"));
                throw;
            }
        });
    }
}

TEST(ConvertTest, ScientificFloatFail) {
    {
        std::string x = "10e1";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("absolute value"));
                throw;
            }
        });
    }

    {
        std::string x = "2e1.1";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_number(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("exponent"));
                throw;
            }
        });
    }
}

