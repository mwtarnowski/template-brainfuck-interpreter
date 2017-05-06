#ifndef BRAINFUCK_CORE_HPP
#define BRAINFUCK_CORE_HPP

#include <type_traits>

namespace bf {

// ************************************************
// Basic data types

struct Null {};

template <unsigned char value>
struct Data {
    using Inc = Data<static_cast<unsigned char>(value + 1)>;
    using Dec = Data<static_cast<unsigned char>(value - 1)>;
};

using Zero = Data<0>;


// ************************************************
// Brainfuck instructions

template <unsigned char opcode>
using Op = Data<opcode>;
using OpIncrement = Op<'+'>;  // Increment the memory cell
using OpDecrement = Op<'-'>;  // Decrement the memory cell
using OpMoveLeft  = Op<'<'>;  // Move the pointer to the left
using OpMoveRight = Op<'>'>;  // Move the pointer to the right
using OpLoopBegin = Op<'['>;  // Jump forward to the matching ] if the cell is zero
using OpLoopEnd   = Op<']'>;  // Jump back to the matching [ if the cell is nonzero
using OpReadChar  = Op<','>;  // Read a single input character
using OpWriteChar = Op<'.'>;  // Write the memory cell to output


// ************************************************
// Fundamental data structures

template <typename Value, 
          typename Next = Null>
struct List {
    using Head = Value;
    using Tail = Next;
};

template <typename Value = Zero, 
          typename Next  = Null>
struct CellList : List<Value, Next> {};

template <typename Value>
struct CellList<Value, Null> : CellList<Value, CellList<>> {};


// ************************************************
// Brainfuck memory model

template <typename Value = Zero, 
          typename Left  = CellList<>, 
          typename Right = CellList<>>
struct CellTape {
    using GetValue  = Value;
    template <typename NewValue>
    using SetValue  = CellTape<NewValue, Left, Right>;
    using MoveLeft  = CellTape<typename Left::Head, typename Left::Tail, CellList<Value, Right>>;
    using MoveRight = CellTape<typename Right::Head, CellList<Value, Left>, typename Right::Tail>;
};


// ************************************************
// Handling forward jumps

template <typename Code, unsigned Depth = 1>
struct OmitLoopImpl {
    static_assert(!std::is_same<Code, Null>::value, "ERROR: unmatched [");
};

template <unsigned char opcode, typename Code, unsigned Depth>
struct OmitLoopImpl<List<Op<opcode>, Code>, Depth>  : OmitLoopImpl<Code, Depth> {};

template <typename Code, unsigned Depth>
struct OmitLoopImpl<List<OpLoopBegin, Code>, Depth> : OmitLoopImpl<Code, Depth + 1> {};

template <typename Code, unsigned Depth>
struct OmitLoopImpl<List<OpLoopEnd, Code>, Depth>   : OmitLoopImpl<Code, Depth - 1> {};

template <typename Code>
struct OmitLoopImpl<List<OpLoopEnd, Code>, 1> {
    using Result = Code;
};

template <typename Code>
using OmitLoop = typename OmitLoopImpl<Code>::Result;


// ************************************************
// Instructions handling

template <typename Code, typename Input, typename Output, typename Memory, typename Stack>
struct Step;

template <typename Memory>
struct ConstMemoryStep { using NewMemory = Memory; };

template <typename Input>
struct ConstInputStep  { using NewInput  = Input;  };

template <typename Output>
struct ConstOutputStep { using NewOutput = Output; };

template <typename Input, typename Output>
struct ConstIOStep : ConstInputStep<Input>, ConstOutputStep<Output> {};

template <typename Code, typename Stack>
struct StandartFlow {
    using NewCode  = Code;
    using NewStack = Stack;
};

template <unsigned char opcode, typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<Op<opcode>, Code>, Input, Output, Stack, Memory> 
: ConstIOStep<Input, Output>, ConstMemoryStep<Memory>, StandartFlow<Code, Stack> {};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpIncrement, Code>, Input, Output, Stack, Memory> 
: ConstIOStep<Input, Output>, StandartFlow<Code, Stack> {
    using NewMemory = typename Memory::template SetValue<typename Memory::GetValue::Inc>;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpDecrement, Code>, Input, Output, Stack, Memory>
: ConstIOStep<Input, Output>, StandartFlow<Code, Stack> {
    using NewMemory = typename Memory::template SetValue<typename Memory::GetValue::Dec>;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpMoveLeft, Code>, Input, Output, Stack, Memory>
: ConstIOStep<Input, Output>, StandartFlow<Code, Stack> {
    using NewMemory = typename Memory::MoveLeft;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpMoveRight, Code>, Input, Output, Stack, Memory>
: ConstIOStep<Input, Output>, StandartFlow<Code, Stack> {
    using NewMemory = typename Memory::MoveRight;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpLoopBegin, Code>, Input, Output, Stack, Memory>
: ConstIOStep<Input, Output>, ConstMemoryStep<Memory> {
    static constexpr bool discard = std::is_same<typename Memory::GetValue, Zero>::value;
    using NewCode  = typename std::conditional<discard, OmitLoop<Code>, Code>::type;
    using NewStack = typename std::conditional<discard, Stack, List<Code, Stack>>::type;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpLoopEnd, Code>, Input, Output, Stack, Memory> 
: ConstIOStep<Input, Output>, ConstMemoryStep<Memory> {
    static_assert(!std::is_same<Stack, Null>::value, "ERROR: unmatched ]");
    static constexpr bool terminate = std::is_same<typename Memory::GetValue, Zero>::value;
    using NewCode  = typename std::conditional<terminate, Code, typename Stack::Head>::type;
    using NewStack = typename std::conditional<terminate, typename Stack::Tail, Stack>::type;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpWriteChar, Code>, Input, Output, Stack, Memory> 
: ConstInputStep<Input>, ConstMemoryStep<Memory>, StandartFlow<Code, Stack> {
    using NewOutput = List<typename Memory::GetValue, Output>;
};

template <typename Code, typename Input, typename Output, typename Stack, typename Memory>
struct Step<List<OpReadChar, Code>, Input, Output, Stack, Memory> 
: ConstOutputStep<Output>, StandartFlow<Code, Stack> {
    static_assert(!std::is_same<Input, Null>::value, "ERROR: input exhausted");
    using NewInput  = typename Input::Tail;
    using NewMemory = typename Memory::template SetValue<typename Input::Head>;
};


// ************************************************
// The core brainfuck processor

template <typename Code   = Null,
          typename Input  = Null,
          typename Output = Null,
          typename Stack  = Null,
          typename Memory = CellTape<>>
struct Processor
{
    using NextState = Step<Code, Input, Output, Stack, Memory>;

    using Apply = Processor<typename NextState::NewCode, 
                            typename NextState::NewInput, 
                            typename NextState::NewOutput, 
                            typename NextState::NewStack, 
                            typename NextState::NewMemory>;
    
    using Run = typename Apply::Run;

    using GetOutput = Output;
};

template <typename Input, typename Output, typename Stack, typename Memory>
struct Processor<Null, Input, Output, Stack, Memory>
{
    static_assert(std::is_same<Stack, Null>::value, "INTERNAL ERROR: non-empty stack");
    using Apply = Processor<Null, Input, Output, Stack, Memory>;
    using Run = Apply;
    using GetOutput = Output;
};

} //namespace bf

#endif //BRAINFUCK_CORE_HPP
