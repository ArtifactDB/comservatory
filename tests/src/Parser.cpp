#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "utils.h"
#include "compare_contents.h"
#include "byteme/byteme.hpp"

TEST(LoadTest, Basic) {
    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto output = load_simple(reader);

    std::vector<std::string> expected_names{ "aaron", "britney", "chuck", "darth" };
    EXPECT_EQ(expected_names, output.names);
    EXPECT_EQ(output.num_fields(), 4);
    EXPECT_EQ(output.num_records(), 2);

    EXPECT_EQ(output.fields[0]->type(), comservatory::NUMBER);
    EXPECT_EQ(output.fields[1]->type(), comservatory::COMPLEX);
    EXPECT_EQ(output.fields[2]->type(), comservatory::STRING);
    EXPECT_EQ(output.fields[3]->type(), comservatory::BOOLEAN);
    for (const auto& f : output.fields) {
        EXPECT_EQ(f->size(), 2);
        EXPECT_TRUE(f->filled());
    }

    comservatory::FilledNumberField* ptr1 = static_cast<comservatory::FilledNumberField*>(output.fields[0].get());
    EXPECT_EQ(ptr1->values[0], 123);
    EXPECT_EQ(ptr1->values[1], 23.01);

    comservatory::FilledComplexField* ptr2 = static_cast<comservatory::FilledComplexField*>(output.fields[1].get());
    EXPECT_EQ(ptr2->values[0], std::complex<double>(4500, 2.1));
    EXPECT_EQ(ptr2->values[1], std::complex<double>(-1, -4));

    comservatory::FilledStringField* ptr3 = static_cast<comservatory::FilledStringField*>(output.fields[2].get());
    EXPECT_EQ(ptr3->values[0], "asdasd");
    EXPECT_EQ(ptr3->values[1], "");

    comservatory::FilledBooleanField* ptr4 = static_cast<comservatory::FilledBooleanField*>(output.fields[3].get());
    EXPECT_TRUE(ptr4->values[0]);
    EXPECT_FALSE(ptr4->values[1]);
}

TEST(LoadTest, Missing) {
    std::string x = "\"aaron\",\"britney\",\"greg\"\nNA,NA,NA\nNA,1,NA\n\"foo\",NA,NA\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto output = load_simple(reader);

    std::vector<std::string> expected_names{ "aaron", "britney", "greg" };
    EXPECT_EQ(expected_names, output.names);

    EXPECT_EQ(output.fields[0]->type(), comservatory::STRING);
    EXPECT_EQ(output.fields[1]->type(), comservatory::NUMBER);
    EXPECT_EQ(output.fields[2]->type(), comservatory::UNKNOWN);
    for (const auto& f : output.fields) {
        EXPECT_EQ(f->size(), 3);
    }

    comservatory::FilledStringField * ptr1 = static_cast<comservatory::FilledStringField*>(output.fields[0].get());
    std::vector<size_t> expected1 { 0, 1 };
    EXPECT_EQ(ptr1->missing, expected1);
    EXPECT_EQ(ptr1->values[2], "foo");

    comservatory::FilledNumberField* ptr2 = static_cast<comservatory::FilledNumberField*>(output.fields[1].get());
    std::vector<size_t> expected2 { 0, 2 };
    EXPECT_EQ(ptr2->missing, expected2);
    EXPECT_EQ(ptr2->values[1], 1);
}

TEST(LoadTest, DifficultStrings) {
    std::string x = "\"aaron\nlun\",\"britney,spears\"\n\"foo\n40\",\"bar,50\"\n\"\"\"good\"\"\",\"gre\"\"\"\"at\"\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto output = load_simple(reader);

    std::vector<std::string> expected_names{ "aaron\nlun", "britney,spears" };
    EXPECT_EQ(expected_names, output.names);

    for (const auto& f : output.fields) {
        EXPECT_EQ(f->type(), comservatory::STRING);
        EXPECT_EQ(f->size(), 2);
    }

    comservatory::FilledStringField * ptr1 = static_cast<comservatory::FilledStringField*>(output.fields[0].get());
    std::vector<std::string> expected1 { "foo\n40", "\"good\"" };
    EXPECT_EQ(ptr1->values, expected1);

    comservatory::FilledStringField* ptr2 = static_cast<comservatory::FilledStringField*>(output.fields[1].get());
    std::vector<std::string> expected2 { "bar,50", "gre\"\"at" };
    EXPECT_EQ(ptr2->values, expected2);
}

TEST(LoadTest, DifficultNumbers) {
    std::string x = "\"aaron lun\",\"britney,spears\"\n\"NaN\",NaN\n\"Inf\",Inf\n\"-nan\",-nan\n\"-inf\",-inf\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto output = load_simple(reader);

    EXPECT_EQ(output.fields[0]->type(), comservatory::STRING);
    EXPECT_EQ(output.fields[1]->type(), comservatory::NUMBER);

    comservatory::FilledStringField * ptr1 = static_cast<comservatory::FilledStringField*>(output.fields[0].get());
    std::vector<std::string> expected1 { "NaN", "Inf", "-nan", "-inf" }; 
    EXPECT_EQ(ptr1->values, expected1);

    comservatory::FilledNumberField* ptr2 = static_cast<comservatory::FilledNumberField*>(output.fields[1].get());
    EXPECT_TRUE(std::isnan(ptr2->values[0]));
    EXPECT_TRUE(std::isinf(ptr2->values[1]));
    EXPECT_TRUE(std::isnan(ptr2->values[2]));
    EXPECT_TRUE(std::isinf(ptr2->values[3]));
}

TEST(LoadTest, MultiChunk) {
    std::string x = "\"aaron\nlun\",\"britney,spears\",\"darth \"\"vader\"\"\"\n\"sdasd\",2e34,TRUE\n\"ccscs\",56.6,true\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto ref = load_simple(reader);

    byteme::ChunkedBufferReader reader2(raw_bytes(x), x.size(), 10);
    auto out = load_simple(reader2);
    compare_contents(ref, out);

    // Might as well check manually, while I'm here.
    comservatory::FilledStringField * ptr1 = static_cast<comservatory::FilledStringField*>(out.fields[0].get());
    std::vector<std::string> expected1{ "sdasd", "ccscs" };
    EXPECT_EQ(ptr1->values, expected1);

    comservatory::FilledNumberField * ptr2 = static_cast<comservatory::FilledNumberField*>(out.fields[1].get());
    std::vector<double> expected2{ 2e34, 56.6 };
    EXPECT_EQ(ptr2->values, expected2);

    comservatory::FilledBooleanField * ptr3 = static_cast<comservatory::FilledBooleanField*>(out.fields[2].get());
    std::vector<bool> expected3{ 1, 1 };
    EXPECT_EQ(ptr3->values, expected3);
}

TEST(LoadTest, Parallelized) {
    std::string x = "\"aaron\nlun\",\"britney,spears\",\"darth \"\"vader\"\"\"\n\"sdasd\",2e34,TRUE\n\"ccscs\",56.6,true\n";

    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto ref = load_simple(reader);

    byteme::RawBufferReader reader2(raw_bytes(x), x.size());
    auto par = load_parallel(reader2);
    compare_contents(ref, par);

    // Need small chunks for the parallelization to do something meaningful.
    byteme::ChunkedBufferReader reader3(raw_bytes(x), x.size(), 10);
    auto out = load_parallel(reader3);
    compare_contents(ref, out);
}

TEST(LoadTest, OneColumn) {
    std::string x = "\"aaron\"\n1\n2\n3\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto out = load_simple(reader);

    EXPECT_EQ(out.fields.size(), 1);
    EXPECT_EQ(out.fields.front()->type(), comservatory::NUMBER);

    comservatory::FilledNumberField * ptr = static_cast<comservatory::FilledNumberField*>(out.fields.front().get());
    std::vector<double> expected{1,2,3};
    EXPECT_EQ(ptr->values, expected);
}

TEST(LoadTest, NoColumns) {
    std::string x = "\n\n\n\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto out = load_simple(reader);

    EXPECT_EQ(out.fields.size(), 0);
    EXPECT_EQ(out.num_records(), 3);
}

TEST(LoadTest, HeaderOnly) {
    std::string x = "\"aaron\",\"britney\",\"darth\"\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto out = load_simple(reader);

    EXPECT_EQ(out.fields.size(), 3);
    EXPECT_EQ(out.num_records(), 0);
}

TEST(LoadTest, DummyByName) {
    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto ref = load_subset(reader, std::vector<std::string>{"britney", "darth"}, std::vector<int>{});

    EXPECT_FALSE(ref.fields[0]->filled());
    EXPECT_EQ(ref.fields[0]->type(), comservatory::NUMBER);

    EXPECT_TRUE(ref.fields[1]->filled());
    EXPECT_EQ(ref.fields[1]->type(), comservatory::COMPLEX);

    EXPECT_FALSE(ref.fields[2]->filled());
    EXPECT_EQ(ref.fields[2]->type(), comservatory::STRING);

    EXPECT_TRUE(ref.fields[3]->filled());
    EXPECT_EQ(ref.fields[3]->type(), comservatory::BOOLEAN);

    for (const auto& f : ref.fields) {
        EXPECT_EQ(f->size(), 2);
    }
}

TEST(LoadTest, DummyByIndex) {
    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto ref = load_subset(reader, std::vector<std::string>{}, std::vector<int>{1, 3});

    EXPECT_FALSE(ref.fields[0]->filled());
    EXPECT_TRUE(ref.fields[1]->filled());
    EXPECT_FALSE(ref.fields[2]->filled());
    EXPECT_TRUE(ref.fields[3]->filled());
}

TEST(LoadTest, Failures) {
    parse_fail("12345,\"britney\",\"aaron\"\n1,2,3\n", "all headers should be quoted");
    parse_fail("\"britney\",\"aaron\"1234\n", "trailing character");
    parse_fail("\"aaron\",\"britney\",\"aaron\"\n1,2,3\n", "duplicated header");

    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2\n", "fewer fields");
    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,3,4\n", "more fields");

    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,3\nTRUE,3,4\n", "previous and current types");
    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,3\n3\"asd,3,4\n", "invalid number containing '\"'");
    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,\"asdasd\"\n4,3,\"asdasd\n", "truncated string");

    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,\"asdasd\"\n\n4,3,\"asdasd\"\n", "is empty");
    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,\n4,3,4\n", "is empty");
    parse_fail("\"aaron\",\"britney\",\"foo\"\n1,2,", "line 2 is truncated");

    parse_fail("", "CSV file is empty");
    parse_fail("\n1\n", "more fields on line 2");
}

TEST(LoadTest, ParallelFailures) {
    std::string x = "\"aaron\",\"britney\",\"aaron\"\n1,2,3\n";
    byteme::ChunkedBufferReader reader(raw_bytes(x), x.size(), 10);

    comservatory::ReadOptions opt;
    opt.parallel = true;

    EXPECT_ANY_THROW({
        try {
            comservatory::read(reader, opt);
        } catch (std::exception& e) {
            EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("duplicated header"));
            throw;
        }
    });
}

TEST(LoadTest, CustomCreator) {
    comservatory::DefaultFieldCreator<true> validator;
    comservatory::ReadOptions opt;
    opt.creator = &validator;

    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());

    auto output = comservatory::read(reader, opt);
    EXPECT_EQ(output.names[0], "aaron");
    EXPECT_EQ(output.names[1], "britney");
    EXPECT_EQ(output.names[2], "chuck");
    EXPECT_EQ(output.names[3], "darth");

    EXPECT_FALSE(output.fields[0]->filled());
    EXPECT_FALSE(output.fields[1]->filled());
    EXPECT_FALSE(output.fields[2]->filled());
    EXPECT_FALSE(output.fields[3]->filled());
}

TEST(LoadTest, PreloadedNames) {
    comservatory::Contents contents;
    contents.names = std::vector<std::string>{ "aaron", "britney", "chuck", "darth"};
    auto old_names = contents.names;

    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());

    comservatory::read(reader, contents, comservatory::ReadOptions());
    EXPECT_EQ(old_names, contents.names);
    EXPECT_EQ(old_names.size(), contents.num_fields());

    // What about a mismatch?
    {
        comservatory::Contents contents;
        contents.names = old_names;
        contents.names.pop_back();
        byteme::RawBufferReader reader(raw_bytes(x), x.size());

        EXPECT_ANY_THROW({
            try {
                comservatory::read(reader, contents, comservatory::ReadOptions());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("not equal to the number of header names"));
                throw;
            }
        });
    }

    {
        comservatory::Contents contents;
        contents.names = old_names;
        std::reverse(contents.names.begin(), contents.names.end());
        byteme::RawBufferReader reader(raw_bytes(x), x.size());

        EXPECT_ANY_THROW({
            try {
                comservatory::read(reader, contents, comservatory::ReadOptions());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("mismatch between provided"));
                throw;
            }
        });
    }
}

TEST(LoadTest, PreloadedFields) {
    comservatory::Contents contents;
    contents.fields.emplace_back(new comservatory::FilledNumberField);
    contents.fields.emplace_back(new comservatory::FilledComplexField);
    contents.fields.emplace_back(new comservatory::FilledStringField);
    contents.fields.emplace_back(new comservatory::FilledBooleanField);

    std::vector<uintptr_t> field_ptrs;
    for (const auto& cf : contents.fields) {
        field_ptrs.push_back(reinterpret_cast<uintptr_t>(cf.get()));
    }

    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());

    comservatory::read(reader, contents, comservatory::ReadOptions());
    std::vector<uintptr_t> observed_ptrs;
    for (const auto& cf : contents.fields) {
        observed_ptrs.push_back(reinterpret_cast<uintptr_t>(cf.get()));
    }
    EXPECT_EQ(field_ptrs, observed_ptrs);

    // What about a mismatch?
    {
        comservatory::Contents contents;
        contents.fields.emplace_back(new comservatory::FilledNumberField);
        byteme::RawBufferReader reader(raw_bytes(x), x.size());

        EXPECT_ANY_THROW({
            try {
                comservatory::read(reader, contents, comservatory::ReadOptions());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("not equal to the number of header names"));
                throw;
            }
        });
    }

    {
        comservatory::Contents contents;
        contents.fields.emplace_back(new comservatory::FilledBooleanField);
        contents.fields.emplace_back(new comservatory::FilledStringField);
        contents.fields.emplace_back(new comservatory::FilledComplexField);
        contents.fields.emplace_back(new comservatory::FilledNumberField);
        byteme::RawBufferReader reader(raw_bytes(x), x.size());

        EXPECT_ANY_THROW({
            try {
                comservatory::read(reader, contents, comservatory::ReadOptions());
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("previous and current types"));
                throw;
            }
        });
    }
}
