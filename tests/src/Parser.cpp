#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "utils.h"
#include "compare_contents.h"
#include "byteme/RawBufferReader.hpp"

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

template<size_t chunksize>
class ChunkedBufferReader : public byteme::Reader {
public:
    ChunkedBufferReader(const unsigned char* buffer, size_t length) : buffer_(buffer), remaining(length), len_(chunksize) {}

    bool operator()() {
        if (!init) {
            buffer_ += chunksize;
            remaining -= chunksize;
        } else {
            init = false;
        }

        if (chunksize < remaining) {
            return true;
        } else {
            len_ = remaining;
            return false;
        }
    }

    const unsigned char* buffer() const {
        return buffer_;
    }

    size_t available() const {
        return len_;
    }

private:
    const unsigned char* buffer_;
    size_t remaining;
    size_t len_;
    bool init = true;
};

TEST(LoadTest, MultiChunk) {
    std::string x = "\"aaron\nlun\",\"britney,spears\",\"darth \"\"vader\"\"\"\n\"sdasd\",2e34,TRUE\n\"ccscs\",56.6,true\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());
    auto ref = load_simple(reader);

    ChunkedBufferReader<10> reader2(raw_bytes(x), x.size());
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
    std::vector<char> expected3{ 1, 1 };
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
    ChunkedBufferReader<10> reader3(raw_bytes(x), x.size());
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
    {
        std::string x = "\"aaron\",\"britney\",\"aaron\"\n1,2,3\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
            load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("duplicated header"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
            load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("fewer fields"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,3,4\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
            load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("more fields"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,3\nTRUE,3,4\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
            load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("previous and current types"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,3\n3\"asd,3,4\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
            load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("encountered quote"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,\"asdasd\"\n4,3,\"asdasd\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
                load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("not terminated by a double quote"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,\"asdasd\"\n\n4,3,\"asdasd\"";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
                load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("empty line"));
                throw;
            }
        });
    }

    {
        std::string x = "\"aaron\",\"britney\",\"foo\"\n1,2,\"asdasd\"\n4,3,\"asdasd\"";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());
        EXPECT_ANY_THROW({
            try {
                load_simple(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("terminated by a single"));
                throw;
            }
        });
    }
}

TEST(LoadTest, ParallelFailures) {
    {
        std::string x = "\"aaron\",\"britney\",\"aaron\"\n1,2,3\n";
        byteme::RawBufferReader reader(raw_bytes(x), x.size());

        comservatory::ReadCsv parser;
        parser.parallel = true;

        EXPECT_ANY_THROW({
            try {
                parser.read(reader);
            } catch (std::exception& e) {
                EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr("duplicated header"));
                throw;
            }
        });
    }
}

TEST(LoadTest, CustomCreator) {
    comservatory::DefaultFieldCreator<true> validator;
    comservatory::ReadCsv foo;
    foo.creator = &validator;

    std::string x = "\"aaron\",\"britney\",\"chuck\",\"darth\"\n123,4.5e3+2.1i,\"asdasd\",TRUE\n23.01,-1-4i,\"\",false\n";
    byteme::RawBufferReader reader(raw_bytes(x), x.size());

    auto output = foo.read(reader);
    EXPECT_EQ(output.names[0], "aaron");
    EXPECT_EQ(output.names[1], "britney");
    EXPECT_EQ(output.names[2], "chuck");
    EXPECT_EQ(output.names[3], "darth");

    EXPECT_FALSE(output.fields[0]->filled());
    EXPECT_FALSE(output.fields[1]->filled());
    EXPECT_FALSE(output.fields[2]->filled());
    EXPECT_FALSE(output.fields[3]->filled());
}
