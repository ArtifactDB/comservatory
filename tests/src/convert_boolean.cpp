#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comservatory/convert.hpp"

TEST(ConvertTest, Boolean) {
    {
        std::string x = "TRUE";
        EXPECT_TRUE(comservatory::to_boolean(x.c_str(), x.size()));
    }

    {
        std::string x = "true";
        EXPECT_TRUE(comservatory::to_boolean(x.c_str(), x.size()));
    }

    {
        std::string x = "True";
        EXPECT_TRUE(comservatory::to_boolean(x.c_str(), x.size()));
    }

    {
        std::string x = "FALSE";
        EXPECT_FALSE(comservatory::to_boolean(x.c_str(), x.size()));
    }

    {
        std::string x = "false";
        EXPECT_FALSE(comservatory::to_boolean(x.c_str(), x.size()));
    }

    {
        std::string x = "False";
        EXPECT_FALSE(comservatory::to_boolean(x.c_str(), x.size()));
    }
}

TEST(ConvertTest, BooleanFail) {
    // Wrong length.
    {
        std::string x = "falset";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_boolean(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("boolean"));
                throw;
            }
        });
    }

    // Bad words.
    {
        std::string x = "falsT";
        EXPECT_ANY_THROW({
            try {
                comservatory::to_boolean(x.c_str(), x.size());
            } catch (std::exception& e) {
                std::string msg(e.what());
                EXPECT_THAT(msg, ::testing::HasSubstr("boolean"));
                throw;
            }
        });
    }
}
