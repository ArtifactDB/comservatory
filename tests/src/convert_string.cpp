#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comservatory/convert.hpp"

TEST(ConvertTest, String) {
    {
        std::string x0 = "asdasdasd";
        std::string x = "\"" + x0 + "\"";
        EXPECT_EQ(comservatory::to_string(x.c_str(), x.size()), x0);
    }

    // Empty string works.
    {
        std::string x0 = "";
        std::string x = "\"" + x0 + "\"";
        EXPECT_EQ(comservatory::to_string(x.c_str(), x.size()), x0);
    }

    // Unusual characters, no problem. 
    {
        std::string x0 = "asdas\ndasd";
        std::string x = "\"" + x0 + "\"";
        EXPECT_EQ(comservatory::to_string(x.c_str(), x.size()), std::string("asdas\ndasd"));
    }

    {
        std::string x0 = "asdas,dasd";
        std::string x = "\"" + x0 + "\"";
        EXPECT_EQ(comservatory::to_string(x.c_str(), x.size()), x0);
    }

    // Unusual characters, no problem. 
    {
        std::string x0 = "asdas\"\"dasd";
        std::string x = "\"" + x0 + "\"";
        EXPECT_EQ(comservatory::to_string(x.c_str(), x.size()), "asdas\"dasd");
        EXPECT_EQ(comservatory::to_string<false>(x.c_str(), x.size()), x0);
    }
}

TEST(ConvertTest, StringFail) {
    // Must start and end with a double quote.
    {
        std::string x = "asdasd\"";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_string(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("double quote"));
                throw;
            }
        });
    }

    {
        std::string x = "\"asdasd";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_string(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("double quote"));
                throw;
            }
        });
    }
    
    {
        std::string x = "";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_string(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("double quote"));
                throw;
            }
        });
    }

    // Internal double quotes must be properly escaped.
    {
        std::string x = "\"asdas\"asdasd\"";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_string(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("should be escaped"));
                throw;
            }
        });
    }
    
    {
        std::string x = "\"asdasasdasd\"\"";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_string(x.c_str(), x.size());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("should be escaped"));
                throw;
            }
        });
    }
}

