# Strict CSV validation and loading

![Unit tests](https://github.com/ArtifactDB/comservatory/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/ArtifactDB/comservatory/actions/workflows/doxygenate.yaml/badge.svg)
[![codecov](https://codecov.io/gh/ArtifactDB/comservatory/branch/master/graph/badge.svg?token=J3dxS3MtT1)](https://codecov.io/gh/ArtifactDB/comservatory)

## Overview

The comma separated value (CSV) format uses a tabular layout where each record starts on a new line and fields are separated by commas.
Beyond this, though, there is little standardization on the specifics of how the fields are interpreted.
The **comservatory** repository defines an opinionated version of a "CSV standard" and provides a header-only C++ library to validate and load a CSV file.
It is primarily intended for use in data management applications that write CSV files and want to provide some structural guarantees for downstream users.

## Standard definition

### General

Each record in the file should have the same number and type of fields.
The following records are consistent:

```
1,2,"abc",5
2,3,"dce",7e-4
```

Whereas these are not:

```
1,2,"abc",5
2,"dce",3,"a"
```

Fields are inferred based on the types of the entries in the subsequent records, as described in the rest of this document.
This is usually unambiguous unless a field contains all-missing (`NA`) values, in which case it has indeterminate type.

Each record should start on a newline but may span multiple lines if a `String` field contains an entry with a newline.
For example, a record containing `[1, 2, "a\nb", 4]` would be formatted as:

``` 
1,2,"a
b",4
```

The first line(s) of the file should contain a header that defines the names of the columns.
Each name should follow the format described for `String`s and should be unique across all columns.
Again, the header line is allowed to span multiple lines if its entries contain newlines.
For example, a header of `["aasdas", "qwert,asdas", "voo\ndasdsd"]` would look like:

```
"aasdas","qwert,asdas","voo
dasdsd"
```

The last line of the file should be terminated with a newline.

Zero-column datasets are represented by an empty line in the header and one empty line per record.

There is no support for comment characters or lines.

The current version of the **comservatory** specification is 1.0.
(This should not be confused with the release versions of the **comservatory** C++ library.) 

### String

All strings should be enclosed in double quotes.
The only exception is that of missing strings, which are denoted as `NA` without quotes.
For example, the fourth record of the first record is missing below:

```
"a","b","c",NA
"d","e","f","g"
```

If the string itself contains a double quote, that quote should be escaped by another double quote.
For example, to represent the `x y "z"` , we would use:

```
...,"x y ""z""",...
```

Strings may contain any number of other characters, including commas and newlines.

Encoding is assumed to be UTF-8, so Unicode characters are allowed.

### Real number

#### Integer notation

Numeric entries can be written as simple integers, consisting of only `0-9` characters.
The numeric characters may be preceded by a single hyphen or plus sign to represent a negative or positive number, respectively.

```
...,100,...
...,+100,...
...,-13,...
```

Any number of leading zeros are permitted, though they are somewhat unusual.

```
...,0123,...
```

#### Decimal notation

Numeric entries may also contain a single intervening decimal point.
There must be numeric characters on both sides of the point.
Entries may be preceded by a single hyphen or plus sign for negative or positive numbers, respectively.
Leading zeros are still permitted.

```
...,1.123,...
...,-0.001,...
```

#### Scientific notation

Alternatively, numbers can be stored in scientific format.
This follows the format `XeY` where:

- The absolute value of `X` lies in `[1, 10)`.
- `X` is formatted as an integer or with a decimal point, as described above. 
- `Y` is formatted as an integer, as described above.

```
...,1.2e+08,...
...,9.9999e-8,...
```

It is also permitted to use a capital `E`:

```
...,-9.3E+07,...
```

#### Other comments

This standard does not make a distinction between the different types of notation for `Number` entries.
One record may store a number as an integer while another record uses decimal notation for the same field.

Missing values are denoted by `NA` entries.

Not-a-number values are represented by `nan`, `-nan` or any capitalization thereof, e.g., `NaN`.

Infinite values are represented by `inf`, `-inf` or any capitalization thereof, e.g., `Inf`.

### Boolean

Booleans should be stored as any capitalization of `TRUE` or `FALSE`:

```
...,TRUE,...
...,true,...
...,True,...
...,FALSE,...
...,false,...
...,False,...
```

Missing values are denoted by `NA` entries.

### Complex numbers

Complex numbers are represented by the format `A+Bi` where `A` and `B` are formatted as `Real number` fields.
Both `A` and `B` must be present, even if the complex number contains only a real or imaginary part.

```
...,0+1i,...
...,2+1.2i,...
...,1e8+1.2e7i,...
```

Missing values are denoted by `NA` entries.

## Implementation

### Quick start

A reference implementation of the validator is provided as a header-only C++ library in [`include/comservatory`](include/comservatory).
This is useful for portable deployment in different frameworks like R, Python, etc.
Given a path to a CSV file, we can load its contents using the `read_file()` function:

```{r}
#include "comservatory/comservatory.hpp"

auto contents = comservatory::read_file(path);
```

If we are only interested in a subset of fields, we can ask `read_file()` to only return that subset.
Note that all fields are still validated but only the contents of the requested fields are returned in memory - all other fields have placeholder entries.

```cpp
comservatory::ReadOptions opt;

// keep fields named 'field1', 'field2'
opt.keep_subset_names = std::vector<std::string>{ "field1", "field2" };

// also keep fields 3 and 4 (zero-indexed)
opt.keep_subset_indices = std::vector<int>{ 3, 4 };

auto contents = comservatory::read_file(path, opt);
```

If only validation is required, we can avoid storing contents in memory by setting `validate_only = true`.
This will parse the file and throw an error upon encountering an invalid format.

```{r}
comservatory::ReadOptions opt;
opt.validate_only = true;
auto dummy_contents = comservatory::read_file(path, opt);
```

See the [reference documentation](https://ltla.github.io/comservatory) for more details.

### Building projects 

#### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```
include(FetchContent)

FetchContent_Declare(
  libscran
  GIT_REPOSITORY https://github.com/ArtifactDB/comservatory
  GIT_TAG master # or any version of interest 
)

FetchContent_MakeAvailable(comservatory)
```

Then you can link to **comservatory** to make the headers available during compilation:

```
# For executables:
target_link_libraries(myexe comservatory)

# For libaries
target_link_libraries(mylib INTERFACE comservatory)
```

#### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DCOMSERVATORY_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(artifactdb_comservatory CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE artifactdb::comservatory)
```

#### Manual

If you're not using CMake, the simple approach is to just copy the files in the `include/` subdirectory - 
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
You will also need to link to [**byteme**](https://github.com/LTLA/byteme) directory, along with the Zlib library.

### Handling other inputs

Gzipped CSVs are automatically supported by `read_file()` once **comservatory** is compiled with Zlib support.

Other inputs are supported via the `read()` function for [`byteme::Reader`](https://github.com/LTLA/byteme) classes.
For example, we can parse a CSV file from an in-memory Zlib-compressed buffer:

```cpp
#include "byteme/ZlibBufferReader.hpp"

byteme::ZlibBufferReader reader(buffer, length);
auto contents = read(reader, comservatory::ReadOptions());
```

### Using known `Field`s

If the header names and/or field types are known in advance, we can specify them in a `Contents` object to be passed to `read_file()`.
This allows developers to strictly control the contents of each field while it is filled.

```cpp
Contents contents;
contents.names = std::vector<std::string>{ "loid", "yor", "anya" };
contents.fields.emplace_back(new comservatory::FilledBooleanField);
contents.fields.emplace_back(new comservatory::FilledStringField);
contents.fields.emplace_back(new comservatory::FilledNumberField);

read_file(path, contents, comservatory::ReadOptions());
```

If the data in the CSV does not match the supplied information, an error is immediately raised.
This is helpful for validation purposes, as opposed to reading the entire file into memory and then checking the contents.

### Customizing `Field` types

Developers may define their own `Field` subclasses to customize the in-memory representation of the data.
For example, the default `FilledField` uses a `std::vector` to store the data values.
If a `std::deque` is preferred instead, we could do so by modifying some code from [`include/comservatory/Field.hpp`](include/comservatory/Field.hpp):

```cpp
#include <deque>

template<typename T, Type tt>
struct DequeFilledField : public comservatory::TypedField<T, tt> {
    DequeFilledField(size_t n = 0) : missing(n), values(n) {
        if (n) {
            std::iota(missing.begin(), missing.end(), 0);
        }
    }

    std::deque<size_t> missing;
    std::deque<T> values;

    size_t size() const { 
        return values.size(); 
    }

    void push_back(T x) {
        values.emplace_back(std::move(x));
        return;
    }

    void add_missing() {
        size_t i = values.size();
        missing.push_back(i);
        values.resize(i + 1);
        return;
    }
};

typedef DequeFilledField<std::string, STRING> DequeFilledStringField;
typedef DequeFilledField<double, NUMBER> DequeFilledNumberField;
typedef DequeFilledField<char, BOOLEAN> DequeFilledBooleanField;
typedef DequeFilledField<std::complex<double>, COMPLEX> DequeFilledComplexField;
```

This requires an accompanying `FieldCreator` subclass to direct `ReadCsv` to use our newly defined `DequeFilledField` subclasses.
We'll recycle some code from [`include/comservatory/Creator.hpp`](include/comservatory/Creator.hpp) to demonstrate:

```cpp
struct DequeFieldCreator : public comservatory::FieldCreator {
    Field* create(Type observed, size_t n, bool dummy) const {
        Field* ptr;

        switch (observed) {
            case comservatory::STRING:
                if (dummy) {
                    ptr = new comservatory::DummyStringField(n);
                } else {
                    ptr = new DequeFilledStringField(n);
                }
                break;
            case comservatory::NUMBER:
                if (dummy) {
                    ptr = new comservatory::DummyNumberField(n);
                } else {
                    ptr = new DequeFilledNumberField(n);
                }
                break;
            case comservatory::BOOLEAN:
                if (dummy) {
                    ptr = new comservatory::DummyBooleanField(n);
                } else {
                    ptr = new DequeFilledBooleanField(n);
                }
                break;
            case comservatory::COMPLEX:
                if (dummy) {
                    ptr = new comservatory::DummyComplexField(n);
                } else {
                    ptr = new DequeFilledComplexField(n);
                }
                break;
            default:
                throw std::runtime_error("unrecognized type during field creation");
        }

        return ptr;
    }
};
```

And then we can direct `read_file()` to use this new `FieldCreator`:

```cpp
DequeFieldCreator custom;

comservatory::ReadCsv reader;
reader.creator = &custom;
auto deqcontents = reader.read(path);
```

## Links

If you see any bugs, report them in the [Issues](https://github.com/ArtifactDB/comservatory/issues).
Pull requests are also welcome.
