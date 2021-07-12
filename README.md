This project consists in an arbitrary precision integer header only library that uses `_ExtInt` as a backend.
It aims at providing a replacement to `ap_(u)int` to users of the SYCL HLS flow for programming Xilinx FPGA.

## Library design:

An Expression in the library context, consists in a recipe to compute a value.
It can be for instance:

+ A constant expression,
+ a sum expression (which when computed will return the sum of the evaluation of its operands),
+ a reduction (or reduction, xor reduction, ...) expression,
+ etc.

Expression have a defined result width and signedness.

Expression are built by composition of operations on other expressions, forming an expression tree.

Leaf Expression are scalar values (that can be constant or not).

Expression evaluation is lazy, and deferred until either explicitly asked by the user or affected to a Value.

`Value`s represent an integer of a fixed width, which is constructed from an expression.

A Value is itself an expression.

```c++
auto a = 0b101010_apv // Create a leaf expression of bit width 6 and no signedness
auto b = 36_apv // Create a leaf expression of bit width 6 and no signedness
auto sum = a + b // Create a sum expression of width 7 and no signedness
Value<7, false> sumVal { sum }; //sum will be evaluated when constructing sumVal
```

 When constructing a value from an expression, if the type and signedness of the expression result do not match the Value parameters, Value policies are used to determine what to do. 
 By default, Value is permissive and performs sign extension, truncation, and sign reinterpretation when needed.
 However, it can be parametrized to do something else or forbid those behaviors.
 
```c++
auto a = 0b101010_apv // Create a leaf expression of bit width 6 and no signedness
auto b = 36_apv // Create a leaf expression of bit width 6 and no signedness
auto sum = a + b // Create a sum expression of width 7 and no signedness

// sum will be evaluated, and left padded with zeros when constructing sumVal
Value<15, true> sumVal { sum };  

// This will result in a compile-time error as errorSumVal is forbidden to be constructed
// from expression having a bit width narrower or wider than 15 bits or a signedness 
// different from true  
Value<15, true, Forbid, Forbid, Forbid> errorSumVal { sum };  
```

Some aliases are defined to ease the library usage. These are :

+ `ap_int<W>`: signed integers of width `W` that uses Value default behavior (sign extension for too narrow expression, truncation for expression too wide, and sign reinterpretation for expression having a different signedness)
+ `ap_uint<W>`: same as `ap_int<W>` except it is unsigned
+ `checked_ap_int<W>`: signed integers of width `W` that can only be constructed from signed expression of width `W`
+ `checked_ap_uint<W>`: unsigned integers of width `W` that can only be constructed from unsigned expression of width `W`

## Library usage requirements

The library requires C++ 20 and compiler support for `_ExtInt()`, with support for `signed _ExtInt(1)` 

## Library API

### Creation of constant expression:

Constant Expression can be created either with the user defined literal `_apv`, or with the `toExpr()` method applied to a compile time integer constant. 
In the first case, the width of the resulting expression is such that its value most significant bit is one.
In the second case, the width of the resulting expression is the width of the type of the argument passed to `toExpr()`.

```c++
auto oneUDL = 1_apv; // Will result in an unsigned constant expression of width 1
auto oneInt = toExpr(std::int32_t{1}); // Will result in a signed constant expression of width 32
```

### Arithmetic operations

Usual arithmetic operation can be performed with usual operators between expressions.
The resulting expression has a width and signedness that is sufficient to store exactly the result of the operation (except for division and modulo by zero).

```c++
auto mult(ap_uint<3> const & lhs, ap_int<1> const & rhs) {
    return lhs * rhs; // Will return a signed product expression of width 4
}
```

Comparison are also supported.

### Bitwise logical operators

Usual operators allows bitwise logical operation such as `xor`, `and` or `or`.

Unary binary not is also supported.

### Logical reduction

Logical reductions are provided through functions taking an expression as input.
The returned expression has an unsigned result on one bit when evaluated, which is the result of 
the application of the associated logical operator between all the input expression results bits.

+ `orReduce(Expression const & E)` is true if at least one bit of the argument is set.
+ `xorReduce(Expression const &)`  is true if the number of bits set in the argument is odd.
+ `xnorReduce(Expression const &)` is true if the number of bits set in the argument is even.
+ `norReduce(Expression const &)` is true if no input bit is set.
+ `andReduce(Expression const &)` is true if all teh input bits are set.
+ `nandReduce(Expression const &)` is true if at least one bit of the argument is unset.

### Shift operations

Usual left and right shift operations are provided. 
On signed values, the fill bit is the sign bit on right shifts.

### Bit manipulation

The `concatenate()` functions returns a concatenation expression the evaluation of which gives the concatenation of all its inputs, the first input being the leftmost part of the result.

```c++
auto a = 0b1011_apv;
auto b = 0b0010_apv;
auto c = 0b1111_apv;
Value<12, false> concatenation = concatenate(a, b, c);
// concatenation value : 0b101100101111
```

At the opposite, a slice from an expression can be obtained with the `slice<High, Low>(Expression const &)` template function.
`High` and `Low` specifies the position in the input of the most significant and least significant bits of the slice.

```c++
auto a = 0b001101_apv;
auto b = slice<3, 1>(a);

// The evaluation of b gives 0b110
```

Finally, a single bit can be retrieved using the `getBit<Position>(Expression const &)` template function (which has the same semantics as `slice<Position, Position>()`).
