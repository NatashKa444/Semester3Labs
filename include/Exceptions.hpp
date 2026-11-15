#pragma once

#include <stdexcept>
#include <string>

class TreeException : public std::runtime_error {
public:
    TreeException(const std::string& message) : std::runtime_error(message) {}
};

class ElementNotFound : public TreeException {
public:
    ElementNotFound(const std::string& message) : TreeException(message) {}
};

class EmptyTree : public TreeException {
public:
    EmptyTree(const std::string& message) : TreeException(message) {}
};

class ParseError : public TreeException {
public:
    ParseError(const std::string& message, std::size_t position)
        : TreeException(message + " (позиция " + std::to_string(position) + ")") {}
};

class IndexOutOfRange : public std::out_of_range {
public:
    IndexOutOfRange(const std::string& message) : std::out_of_range(message) {}
};

class KeyNotFound : public ElementNotFound {
public:
    KeyNotFound(const std::string& message) : ElementNotFound(message) {}
};
