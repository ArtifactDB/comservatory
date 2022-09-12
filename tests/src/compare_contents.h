#ifndef COMPARE_CONTENTS_H 
#define COMPARE_CONTENTS_H 

#include <gtest/gtest.h>
#include "comservatory/Parser.hpp"

inline void compare_contents(const comservatory::Contents& ref, const comservatory::Contents& out) {
    EXPECT_EQ(out.names, ref.names);

    for (size_t i = 0; i < out.names.size(); ++i) {
        EXPECT_EQ(out.fields[i]->size(), ref.fields[i]->size());
        EXPECT_TRUE(out.fields[i]->filled());
        EXPECT_TRUE(ref.fields[i]->filled());

        auto reftype = ref.fields[i]->type();
        EXPECT_EQ(out.fields[i]->type(), reftype);

        comservatory::Field* optr = out.fields[i].get();
        comservatory::Field* rptr = ref.fields[i].get();
        switch (reftype) {
            case comservatory::NUMBER:
                {
                    comservatory::FilledNumberField* left = static_cast<comservatory::FilledNumberField*>(optr);
                    comservatory::FilledNumberField* right = static_cast<comservatory::FilledNumberField*>(rptr);
                    EXPECT_EQ(left->values, right->values);
                    EXPECT_EQ(left->missing, right->missing);
                }
                break;
            case comservatory::STRING:
                {
                    comservatory::FilledStringField* left = static_cast<comservatory::FilledStringField*>(optr);
                    comservatory::FilledStringField* right = static_cast<comservatory::FilledStringField*>(rptr);
                    EXPECT_EQ(left->values, right->values);
                    EXPECT_EQ(left->missing, right->missing);
                }
                break;
            case comservatory::BOOLEAN:
                {
                    comservatory::FilledBooleanField* left = static_cast<comservatory::FilledBooleanField*>(optr);
                    comservatory::FilledBooleanField* right = static_cast<comservatory::FilledBooleanField*>(rptr);
                    EXPECT_EQ(left->values, right->values);
                    EXPECT_EQ(left->missing, right->missing);
                }
                break;
            case comservatory::COMPLEX:
                {
                    comservatory::FilledComplexField* left = static_cast<comservatory::FilledComplexField*>(optr);
                    comservatory::FilledComplexField* right = static_cast<comservatory::FilledComplexField*>(rptr);
                    EXPECT_EQ(left->values, right->values);
                    EXPECT_EQ(left->missing, right->missing);
                }
                break;
            case comservatory::UNKNOWN:
                break;
        }
    }
}

#endif
