#ifdef _MSC_VER
// warning C4503: decorated name length exceeded, name was truncated
#pragma warning(disable:4503)
#endif

#include <iostream>
#include "bfcore.hpp"
#include "bfutils.hpp"

// Brainfuck code can be provided in the command line:
// -DPROGRAM_CODE=\"...\" (for GCC, Clang)
// /DPROGRAM_CODE=\"...\" (for MSVC)
#ifndef PROGRAM_CODE
#define PROGRAM_CODE "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>."
#endif

// To pass the input for the program, use:
// -DPROGRAM_INPUT=\"...\" (for GCC, Clang)
// /DPROGRAM_INPUT=\"...\" (for MSVC)
#ifndef PROGRAM_INPUT
#define PROGRAM_INPUT ""
#endif

BF_CREATE_LIST_FROM_STRING(MyProgram, PROGRAM_CODE)
BF_CREATE_LIST_FROM_STRING(MyInput, PROGRAM_INPUT)

int main()
{
    using MyOutput = bf::Machine<MyProgram, MyInput>::Run::GetOutput;
    std::cout << bf::ListToStr<MyOutput>() << std::endl;
    return 0;
}
