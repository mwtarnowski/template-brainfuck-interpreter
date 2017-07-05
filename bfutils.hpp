#ifndef BRAINFUCK_UTILS_HPP
#define BRAINFUCK_UTILS_HPP

#include <string>
#include "bfcore.hpp"

// This header provides some 'syntactic sugar' for the interpreter.
// It does not add functionality to the interpreter, but introduces
// tools and wrappers to allow a more pleasant look of programs.

namespace bf {

// Wrapper for the core brainfuck processor

template <typename Program, 
          typename Input = Null>
using Machine = Processor<Program, Input>;


// Lists constructing: creating a list from a sequence of characters

template <unsigned char... values>
struct CreateListImpl {
    using Result = Null;
};

template <unsigned char value, unsigned char... values>
struct CreateListImpl<value, values...> {
    using Result = List<Data<value>, typename CreateListImpl<values...>::Result>;
};

template <unsigned char... values>
using CreateList = typename CreateListImpl<values...>::Result;


// Lists constructing: creating a list from a string

constexpr unsigned StrLen(char const* string, unsigned length = 0) {
    return *string ? StrLen(string + 1, length + 1) : length;
}

template <typename Content,
          unsigned length = StrLen(Content::Str()), 
          typename Output = Null>
struct StrToListImpl 
: StrToListImpl<Content, length - 1, List<Data<static_cast<unsigned char>(Content::Str()[length - 1])>, Output>> 
{};

template <typename Content, typename Output>
struct StrToListImpl<Content, 0, Output> { 
    using Result = Output; 
};

template <typename Content>
using StrToList = typename StrToListImpl<Content>::Result;


// Converting a list to std::string
// That awkward moment when you have to do something in runtime...

template <typename InputList>
struct ListToStrImpl {
    static inline std::string Get() {
        return std::string();
    }
};

template <unsigned char value, typename Next>
struct ListToStrImpl<List<Data<value>, Next>> {
    static inline std::string Get() {
        return ListToStrImpl<Next>::Get().append(1, value);
    }
};

template <typename InputList>
inline std::string ListToStr() {
    return ListToStrImpl<InputList>::Get();
}

} //namespace bf

// Shortcuts: cut here if you have an aversion to macros.

#define BF_CREATE_LIST_FROM_CHARS(List, ...) \
using List = ::bf::CreateList<__VA_ARGS__>;

#define BF_CREATE_LIST_FROM_STRING(List, String) \
struct List##Content_ { constexpr static const char* Str() { return String; } }; \
using List = ::bf::StrToList<List##Content_>;

#endif //BRAINFUCK_UTILS_HPP
