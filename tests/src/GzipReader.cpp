#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>

#include "utils.h"
#include "temp_file_path.h"
#include "compare_contents.h"

#include "byteme/RawBufferReader.hpp"
#ifdef COMSERVATORY_USE_ZLIB
#include "byteme/GzipFileReader.hpp"
#include "zlib.h"

void write_csv_gzip(std::string path, std::string contents) {
    gzFile ohandle = gzopen(path.c_str(), "w");
    if (!ohandle) {
        throw std::runtime_error("failed to open file");
    }
    gzwrite(ohandle, contents.c_str(), contents.size());
    gzclose(ohandle);
    return;
}

TEST(GzipReaderTest, Simple) {
    std::string x = "\"jayaram\",\"needs\",\"to get off\",\"his ass\"\n\"and start\",1,2,true\n"; 
    byteme::RawBufferReader buf(raw_bytes(x), x.size());
    auto ref = load_simple(buf);

    auto path = temp_file_path("comservatory-test");
    write_csv_gzip(path, x);
    auto out = load_path(path);
    compare_contents(ref, out);

    // Works with subset.
    auto dummies = load_path_subset(path.c_str(), {}, {1, 2});
    EXPECT_EQ(dummies.fields.size(), 4);
    EXPECT_FALSE(dummies.fields[0]->filled());
    EXPECT_TRUE(dummies.fields[1]->filled());
    EXPECT_TRUE(dummies.fields[2]->filled());
    EXPECT_FALSE(dummies.fields[3]->filled());

    // Works with validation.
    auto val = validate_path(path);
    EXPECT_FALSE(val.fields[0]->filled());
    EXPECT_FALSE(val.fields[1]->filled());
    EXPECT_FALSE(val.fields[2]->filled());
    EXPECT_FALSE(val.fields[3]->filled());
}

TEST(GzipReaderTest, SmallerChunks) {
    std::string x = "\"jayaram\",\"needs\",\"to get off\",\"his ass\"\n\"and start\",1,2,true\n"; 
    byteme::RawBufferReader buf(raw_bytes(x), x.size());
    auto ref = load_simple(buf);
        
    auto path = temp_file_path("comservatory-chunk");
    write_csv_gzip(path, x);

    byteme::GzipFileReader opt;
    opt.buffer_size = 9;
    byteme::GzipFileReader txt(path.c_str(), opt);
    auto out = load_simple(txt);

    compare_contents(ref, out);
}
#endif
