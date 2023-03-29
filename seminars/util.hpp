#pragma once

#include "type_name.hpp"
#include <iostream>


#define type_of(x) type_name<decltype(x)>()

void print(auto name, const auto& item) {
    std::cout << name << ": " << item << '\n';
}

template<typename T>
void print(const T& item) {
    std::cout << item << '\n';
}

