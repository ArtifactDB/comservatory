#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "byteme/RawBufferReader.hpp"
#include "byteme/RawFileReader.hpp"
#include "byteme/temp_file_path.hpp"

#include "utils.h"
#include "compare_contents.h"

#include <fstream>
#include <string>

void write_csv(std::string path, std::string contents) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("failed to open output file");
    }
    out << contents;
    out.close();
    return;
}

TEST(TextReaderTest, Simple) {
    std::string x = "\"jayaram\",\"needs\",\"to get off\",\"his ass\"\n\"and start\",1,2,true\n"; 
    byteme::RawBufferReader buf(raw_bytes(x), x.size());
    auto ref = load_simple(buf);

    auto path = byteme::temp_file_path("comservatory-test", ".csv");
    write_csv(path, x);
    auto out = load_path(path);
    compare_contents(ref, out);

    // Works with subset.
    auto dummies = load_path_subset(path.c_str(), {}, { 0, 3 });
    EXPECT_EQ(dummies.fields.size(), 4);
    EXPECT_TRUE(dummies.fields[0]->filled());
    EXPECT_FALSE(dummies.fields[1]->filled());
    EXPECT_FALSE(dummies.fields[2]->filled());
    EXPECT_TRUE(dummies.fields[3]->filled());

    // Works with validation.
    auto val = validate_path(path);
    EXPECT_FALSE(val.fields[0]->filled());
    EXPECT_FALSE(val.fields[1]->filled());
    EXPECT_FALSE(val.fields[2]->filled());
    EXPECT_FALSE(val.fields[3]->filled());
}

TEST(TextReaderTest, SmallerChunks) {
    std::string x = "\"jayaram\",\"needs\",\"to get off\",\"his ass\"\n\"and start\",1,2,true\n"; 
    byteme::RawBufferReader buf(raw_bytes(x), x.size());
    auto ref = load_simple(buf);
        
    auto path = byteme::temp_file_path("comservatory-chunk", ".csv");
    write_csv(path, x);
    byteme::RawFileReader txt(path.c_str(), 9);
    auto out = load_simple(txt);

    compare_contents(ref, out);
}
