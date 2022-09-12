#ifndef UTILS_H 
#define UTILS_H 

#include "comservatory/comservatory.hpp"

inline const unsigned char* raw_bytes(const std::string& x) {
    return reinterpret_cast<const unsigned char*>(x.c_str());
}

template<class Reader>
comservatory::Contents load_simple(Reader& reader) {
    return comservatory::ReadCsv().read(reader);
}

template<class Reader>
comservatory::Contents load_parallel(Reader& reader) {
    comservatory::ReadCsv foo;
    foo.parallel = true;
    return foo.read(reader);
}

inline comservatory::Contents load_path(std::string path) {
    return comservatory::ReadCsv().read(path);
}

template<class Reader>
comservatory::Contents load_subset(Reader& reader, std::vector<std::string> subset_names, std::vector<int> subset_indices) {
    comservatory::ReadCsv foo;
    foo.keep_subset = true;
    foo.keep_subset_names = std::move(subset_names);
    foo.keep_subset_indices = std::move(subset_indices);
    return foo.read(reader);
}

inline comservatory::Contents load_path_subset(std::string path, std::vector<std::string> subset_names, std::vector<int> subset_indices) {
    comservatory::ReadCsv foo;
    foo.keep_subset = true;
    foo.keep_subset_names = std::move(subset_names);
    foo.keep_subset_indices = std::move(subset_indices);
    return foo.read(path.c_str());
}

inline comservatory::Contents validate_path(std::string path) {
    comservatory::ReadCsv foo;
    foo.validate_only = true;
    return foo.read(path);
}

#endif


