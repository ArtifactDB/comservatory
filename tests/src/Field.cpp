#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comservatory/Field.hpp"
#include "comservatory/Creator.hpp"

TEST(FieldTest, Unknown) {
    comservatory::UnknownField x;
    EXPECT_EQ(x.type(), comservatory::UNKNOWN);

    x.add_missing();
    EXPECT_EQ(x.size(), 1);

    x.add_missing();
    EXPECT_EQ(x.size(), 2);
}

TEST(FieldTest, FilledString) {
    comservatory::FilledStringField x;

    x.push_back("asda");
    EXPECT_EQ(x.size(), 1);
    EXPECT_EQ(x.values[0], "asda");

    x.push_back("foo2");
    EXPECT_EQ(x.size(), 2);
    EXPECT_EQ(x.values[1], "foo2");
}

TEST(FieldTest, DummyString) {
    comservatory::FilledStringField x;

    x.push_back("asda");
    EXPECT_EQ(x.size(), 1);

    x.push_back("foo2");
    EXPECT_EQ(x.size(), 2);
}

TEST(FieldTest, FilledNumber) {
    comservatory::FilledNumberField x;

    x.push_back(1234);
    EXPECT_EQ(x.size(), 1);
    EXPECT_EQ(x.values[0], 1234);

    x.push_back(-98.76);
    EXPECT_EQ(x.size(), 2);
    EXPECT_EQ(x.values[1], -98.76);
}

TEST(FieldTest, DummyNumber) {
    comservatory::DummyNumberField x;

    x.push_back(1234);
    EXPECT_EQ(x.size(), 1);

    x.push_back(-98.76);
    EXPECT_EQ(x.size(), 2);
}

TEST(FieldTest, FilledBoolean) {
    comservatory::FilledBooleanField x;

    x.push_back(true);
    EXPECT_EQ(x.size(), 1);
    EXPECT_TRUE(x.values[0]);

    x.push_back(false);
    EXPECT_EQ(x.size(), 2);
    EXPECT_FALSE(x.values[1]);
}

TEST(FieldTest, DummyBoolean) {
    comservatory::DummyBooleanField x;

    x.push_back(true);
    EXPECT_EQ(x.size(), 1);

    x.push_back(false);
    EXPECT_EQ(x.size(), 2);
}

TEST(FieldTest, FilledComplex) {
    comservatory::FilledComplexField x;

    x.push_back(std::complex<double>(1,2));
    EXPECT_EQ(x.size(), 1);
    EXPECT_EQ(x.values[0].real(), 1);
    EXPECT_EQ(x.values[0].imag(), 2);

    x.push_back(std::complex<double>(3,4));
    EXPECT_EQ(x.size(), 2);
    EXPECT_EQ(x.values[1].real(), 3);
    EXPECT_EQ(x.values[1].imag(), 4);
}

TEST(FieldTest, DummyComplex) {
    comservatory::DummyComplexField x;

    x.push_back(std::complex<double>(1,2));
    EXPECT_EQ(x.size(), 1);

    x.push_back(std::complex<double>(3,4));
    EXPECT_EQ(x.size(), 2);
}

TEST(FieldTest, Missing) {
    std::shared_ptr<comservatory::Field> ptr(new comservatory::FilledStringField);
    comservatory::FilledStringField* copy = static_cast<comservatory::FilledStringField*>(ptr.get());

    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 1);
    EXPECT_EQ(copy->missing.size(), 1);
    EXPECT_EQ(copy->missing[0], 0);

    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 2);
    EXPECT_EQ(copy->missing.size(), 2);
    EXPECT_EQ(copy->missing[1], 1);

    // Increments correctly after bump.
    copy->push_back("yay");
    EXPECT_EQ(ptr->size(), 3);
    EXPECT_EQ(copy->missing.size(), 2);

    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 4);
    EXPECT_EQ(copy->values.back(), "");
    EXPECT_EQ(copy->missing.size(), 3);
    EXPECT_EQ(copy->missing[2], 3);
}

TEST(FieldTest, DummyMissing) {
    std::shared_ptr<comservatory::Field> ptr(new comservatory::DummyStringField);

    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 1);
    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 2);

    // Increments correctly after bump.
    static_cast<comservatory::StringField*>(ptr.get())->push_back("yay");
    EXPECT_EQ(ptr->size(), 3);

    ptr->add_missing();
    EXPECT_EQ(ptr->size(), 4);
}

TEST(FieldTest, Creator) {
    comservatory::DefaultFieldCreator<false> fun;

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::BOOLEAN, 4, false));
        EXPECT_EQ(ptr->type(), comservatory::BOOLEAN);
        EXPECT_EQ(ptr->size(), 4);
        EXPECT_TRUE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::STRING, 4, false));
        EXPECT_EQ(ptr->type(), comservatory::STRING);
        EXPECT_EQ(ptr->size(), 4);
        EXPECT_TRUE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::NUMBER, 4, false));
        EXPECT_EQ(ptr->type(), comservatory::NUMBER);
        EXPECT_EQ(ptr->size(), 4);
        EXPECT_TRUE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::COMPLEX, 4, false));
        EXPECT_EQ(ptr->type(), comservatory::COMPLEX);
        EXPECT_EQ(ptr->size(), 4);
        EXPECT_TRUE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::BOOLEAN, 3, true));
        EXPECT_EQ(ptr->type(), comservatory::BOOLEAN);
        EXPECT_EQ(ptr->size(), 3);
        EXPECT_FALSE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::STRING, 3, true));
        EXPECT_EQ(ptr->type(), comservatory::STRING);
        EXPECT_EQ(ptr->size(), 3);
        EXPECT_FALSE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::NUMBER, 3, true));
        EXPECT_EQ(ptr->type(), comservatory::NUMBER);
        EXPECT_EQ(ptr->size(), 3);
        EXPECT_FALSE(ptr->filled());
    }

    {
        std::unique_ptr<comservatory::Field> ptr(fun.create(comservatory::COMPLEX, 3, true));
        EXPECT_EQ(ptr->type(), comservatory::COMPLEX);
        EXPECT_EQ(ptr->size(), 3);
        EXPECT_FALSE(ptr->filled());
    }

    {
        std::string x = "";
        EXPECT_ANY_THROW({
            try {
                fun.create(comservatory::UNKNOWN, 0, false);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("unrecognized type"));
                throw;
            }
        });
    }
}

TEST(FieldTest, Names) {
    EXPECT_EQ(comservatory::type_to_name(comservatory::STRING), "STRING");
    EXPECT_EQ(comservatory::type_to_name(comservatory::BOOLEAN), "BOOLEAN");
    EXPECT_EQ(comservatory::type_to_name(comservatory::NUMBER), "NUMBER");
    EXPECT_EQ(comservatory::type_to_name(comservatory::COMPLEX), "COMPLEX");
    EXPECT_EQ(comservatory::type_to_name(comservatory::UNKNOWN), "UNKNOWN");
}
