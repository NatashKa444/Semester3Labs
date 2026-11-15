#pragma once

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "Exceptions.hpp"

template <class T>
struct ValueCodec {
    static std::string ToText(const T& value);
    static T FromText(const std::string& text);
};

template <>
struct ValueCodec<std::string> {
    static std::string ToText(const std::string& value) { return value; }
    static std::string FromText(const std::string& text) { return text; }
};

template <class T>
std::string ValueCodec<T>::ToText(const T& value) {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return out.str();
}

template <class T>
T ValueCodec<T>::FromText(const std::string& text) {
    std::istringstream in(text);
    T value{};
    in >> value;
    if (in.fail() || !(in >> std::ws).eof()) {
        throw TreeException("Не удалось прочитать значение: '" + text + "'");
    }
    return value;
}
