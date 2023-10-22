#ifndef UTILS_H 
#define UTILS_H 

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "byteme/RawBufferReader.hpp"
#include "comservatory/comservatory.hpp"

inline const unsigned char* raw_bytes(const std::string& x) {
    return reinterpret_cast<const unsigned char*>(x.c_str());
}

template<class Reader>
comservatory::Contents load_simple(Reader& reader) {
    return comservatory::read(reader, comservatory::ReadOptions());
}

template<class Reader>
comservatory::Contents load_parallel(Reader& reader) {
    comservatory::ReadOptions opt;
    opt.parallel = true;
    return comservatory::read(reader, opt);
}

inline comservatory::Contents load_path(std::string path) {
    return comservatory::read_file(path);
}

template<class Reader>
comservatory::Contents load_subset(Reader& reader, std::vector<std::string> subset_names, std::vector<int> subset_indices) {
    comservatory::ReadOptions opt;
    opt.keep_subset = true;
    opt.keep_subset_names = std::move(subset_names);
    opt.keep_subset_indices = std::move(subset_indices);
    return comservatory::read(reader, opt);
}

inline comservatory::Contents load_path_subset(const std::string& path, std::vector<std::string> subset_names, std::vector<int> subset_indices) {
    comservatory::ReadOptions opt;
    opt.keep_subset = true;
    opt.keep_subset_names = std::move(subset_names);
    opt.keep_subset_indices = std::move(subset_indices);

    // We could just return read_file directly, but we'll get some coverage on the overload...
    comservatory::Contents contents;
    comservatory::read_file(path.c_str(), contents, opt);
    return contents;
}

inline comservatory::Contents validate_path(const std::string& path) {
    comservatory::ReadOptions opt;
    opt.validate_only = true;
    return comservatory::read_file(path, opt);
}

inline void parse_fail(const std::string& x, const std::string& msg) {
    byteme::RawBufferReader reader(reinterpret_cast<const unsigned char*>(x.c_str()), x.size());

    EXPECT_ANY_THROW({
        try {
            comservatory::read(reader, comservatory::ReadOptions());
        } catch (std::exception& e) {
            EXPECT_THAT(std::string(e.what()), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

inline void simple_conversion_fail(std::string x, const std::string& msg) {
    x = "\"foo\"\n" + x;
    parse_fail(x, msg);
}

#endif


