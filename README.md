# iris
Intermediate Representation Internal Structure - SSA-like IR developed as an educational project

<img src="images/logo.png" alt="logo" width="250">

### Overview

IRIS is an example of IR that includes simple operations, separated into different _dialects_ - _arith_, _builtin_ and _ctrlflow_.

Dialect _arith_:
- _ADD_, _SUB_, _MUL_, _DIV_ - basic arithmetical operations;
- _CONST_ - operation generating a constant SSA value;
- _CAST_ - operation, casting values between different data types;
- _CMP_ - comparison operation, which takes predicate (``EQ`` (equal), ``NEQ`` (not-equal), ``A`` (above), ``B`` (below), ``AE`` (above or equal), ``BE`` (below or equal));
- _AND_, _OR_, _XOR_, _NOT_ - bitwise operations;
- _SHL_, _SHR_, _SAL_, _SAR_ - arithmetical and logical bit-shifts

Dialect _builtin_:
- _PARAM_ - operation representing function's parameter;
- _COPY_ - operation that copies one SSA value into the new one.
    
Dialect _ctrlflow_:
- _CALL_ - routine call operation;
- _JUMP_ - unconditional jump;
- _JUMPC_ - conditional jump;
- _RETURN_ - return from the routine operation;
- _PHI_ - phi-function.

### IRIS structure & examples

Here's an example of IRIS IR for a programm that calculates factorial of a number:
```text
factorial:
  ^bb0 <start> <to bb1> :
      a0.ui builtin.param -> (v3,v5,v7)
      c1.ui arith.const (1) -> (v5,v8p)
      c2.ui arith.const (2) -> (v3)
  ^bb1 <from bb0> <to T:bb2 / F:bb3> :
      v3.b arith.cmp.b (a0 : ui, c2 : ui) -> (n4)
      n4 ctrlflow.jmpc (v3 : b)
  ^bb3 <from bb1> <to bb2> :
      v5.ui arith.sub (a0 : ui, c1 : ui) -> (v6)
      v6.ui ctrlflow.call @factorial (v5 : ui) -> (v7)
      v7.ui arith.mul (a0 : ui, v6 : ui) -> (v8p)
  ^bb2 <from bb3 bb1> <final> :
      v8p.ui ctrlflow.phi (c1 : ui, v7 : ui) -> (n9)
      n9 ctrlflow.return (v8p : ui)
```

Each **operation** is located in one of the **basic blocks**, and basic block is located in the **region**, which represent IR for a whole routine (function).

Function parameters are marked with literal ``a`` (``a0``, ``a1`` etc.).
Constant-generated SSA values are marked with literal ``c`` (``c1``, ``c2`` etc.).
Other SSA values are marked with literal ``v`` (``v3``, ``v4`` etc.).

Operations that do not produce material result, for example, _ctrlflow::jump_, are marked with literal ``n`` for 'none', as its return value type is ``DataType::NONE``.

Notice that different SSA values can have different literal preceeding its ID, but regardless of the literal **every value in the region has unique ID**.

Basic block properties:
- Basic block's ID is unique among the others in the same region;
- Each basic block has a list of **predecessors** (``<from bb3 bb1>``), or it is the starting basic block, in which case it is marked with ``<start>``;
- A basic block can have either zero (``<final>``), one (``<to bb2>``) or two **successors** (``<to T:bb2 / F:bb3>``);
- Basic block cannot have a **terminator** operation inside of it (it must be the last operation in the basic block);
- If operation have zero successors, its last operation must be _ctrlflow::return_ (TODO - add check);
- If operation have one successor, its last operation cannot be _ctrlflow::jumpc_;
- If operation have two successors, its last operation must be _ctrlflow::jumpc_.

Operations common properties:
- **Opcode** value, which is unique for every type of operation across all dialects;
- **SSA-value's ID**, which is unique across whole region;
- **Return data type**, which is ``DataType::NONE`` if operation does not produce any result;
- Optional list of **inputs**. Some operations can have variadic number of inputs (_ctrlflow::call_), other have fixed number of inputs (_arith::add_);
- List of **users**, who use this operation's result as their input/inputs.

### Dominators analysis

``iris::Region`` class provides API:
- ``collectDomInfo()`` - collect dominators information from the graph;
- ``getDFS()`` and ``getRPO()`` - obtain corresponding traversal orders.

``iris::doms::DomInfo`` helper-class provides API:
- ``getIdom()`` and ``getIdomByID()`` - get immediate dominator
- ``getDominatedBlocks()`` and ``getDominatedBlocksByID()`` - get blocks, for which given one is immediate dominator;
- ``getDominatorsChain()`` and ``getDominatorsChainByID()`` - get chain of immediate dominators from the given block to the starting block.

Example graph:
```text
A → B → C → D
      ↓
      F → E → D
      ↓
      G → D
```

Example output:
```text
========== Dom Tree ===========
[Dominator Tree]
  BB#0
    BB#1
      BB#3
      BB#5
        BB#4
        BB#6
      BB#2
================================
```

### Loop analysis

``iris::Region`` class provides API:
- ``collectLoopInfo()`` - collect loops information from the graph.

``iris::loops::LoopInfo`` helper-class provides API:
- ``getRootLoop()`` - get root pseudo-loop, which is parent to top-level loops;
- ``getTopLevelLoops()`` - get top-level loops.

Class ``iris::loops::Loop`` provides methods to query:
- **Header** basic block;
- List of **latches**;
- **Reducibility** flag;
- **Exiting edges** and **blocks** contained inside for reducible loops;
- List of **nested** loops;
- Loop's **depth**;
- **Parent** loop (_outer_).

_Root_ loop is a special pseudo-loop, which:
- Has no parent loop, header, latches or exiting edges;
- Has depth 0;
- Contains all basic blocks, which are are not contained in any other loop in the region.

Example graph:
```text
A → B → C → D → E → F → G → I → K
    ↓       ↓       ↓   ↓
    J → C   C       E   H → B
```

Example output:
```text
========== Loop Tree ==========
[Root Loop]
  Depth: 0 | Reducible: true
  Blocks (3): 8 0 10
  Nested Loops:
  Loop Header: 1
    Depth: 1 | Reducible: true
    Latches: 7
    Blocks (2): 9 6
    Exits:
      6 -> 8
    Nested Loops:
    Loop Header: 4
      Depth: 2 | Reducible: true
      Latches: 5
      Blocks (0):
      Exits:
        5 -> 6
    Loop Header: 2
      Depth: 2 | Reducible: true
      Latches: 3
      Blocks (0):
      Exits:
        3 -> 4
================================
```

### IR Optimizations

IRIS provides a pass-based optimization framework: every optimization is a subclass of `iris::opt::Pass`, and a `iris::opt::PassManager` owns a pipeline of passes and runs them sequentially on a `iris::Region`. An example of an optimization pipeline:

```cpp
#include <iris.hpp>

void optimize(iris::Region& region) {
  iris::opt::PassManager pm;
  pm.addPass(std::make_unique<iris::opt::common::DCEPass>());
  pm.addPass(std::make_unique<iris::opt::arith::ArithConstFoldPass>());
  pm.addPass(std::make_unique<iris::opt::arith::ArithPeepHolePass>());
  pm.run(region);
}
```

Available passes:
- ``common::DCEPass`` – removes dead, side-effect-free, non-terminator operations with results that are never used (parameters are kept);
  - Example - ``examples/DCE``;
  - Tests - ``tests/opt/common/dce.cpp``;
- ``arith::ArithConstFoldPass`` – folds arithmetic, bitwise and comparison operations whose operands are compile-time constants into a single `arith.const`;
  - Example - ``examples/ConstFold``;
  - Tests - ``tests/opt/arith/constfold.cpp``;
- ``arith::ArithPeepHolePass`` – applies local algebraic/bitwise simplifications on small instruction patterns (peepholes) for `arith` operations;
  - Example - ``examples/PeepHole.cpp``;
  - Tests - ``tests/opt/arith/peephole.cpp``.

### Dead Code Elimination (`DCEPass`)

IR before ``DCEPass``:
```text
dce:
  ^bb0 <start> <final> :
      a0.ui builtin.param -> (v6)
      a1.ui builtin.param -> (v6)
      a2.ui builtin.param -> (v8,v10)
      a3.ui builtin.param -> (v10)
      c4.ui arith.const (10) -> (v7)
      c5.ui arith.const (20) -> (v11)
      v6.ui arith.add (a0 : ui, a1 : ui) -> (v7)
      v7.ui arith.mul (v6 : ui, c4 : ui) -> (v8)
      v8.ui arith.sub (v7 : ui, a2 : ui) -> (v9)
      v9.ui builtin.copy (v8 : ui) -> (n18)
      v10.ui arith.mul (a2 : ui, a3 : ui) -> (v11)
      v11.ui arith.add (v10 : ui, c5 : ui) -> (v12,v12)
      v12.ui arith.mul (v11 : ui, v11 : ui)
      c13.ui arith.const (5) -> (v15)
      c14.ui arith.const (7) -> (v15,v17)
      v15.ui arith.add (c13 : ui, c14 : ui) -> (v16)
      v16.ui builtin.copy (v15 : ui) -> (v17)
      v17.ui arith.sub (v16 : ui, c14 : ui)
      n18 ctrlflow.return (v9 : ui)
```

IR after ``DCEPass``:
```text
dce:
  ^bb0 <start> <final> :
      a0.ui builtin.param -> (v6)
      a1.ui builtin.param -> (v6)
      a2.ui builtin.param -> (v8)
      a3.ui builtin.param
      c4.ui arith.const (10) -> (v7)
      v6.ui arith.add (a0 : ui, a1 : ui) -> (v7)
      v7.ui arith.mul (v6 : ui, c4 : ui) -> (v8)
      v8.ui arith.sub (v7 : ui, a2 : ui) -> (v9)
      v9.ui builtin.copy (v8 : ui) -> (n18)
      n18 ctrlflow.return (v9 : ui)
```

See corresponding example - ``examples/DCE``.
See corresponding tests - ``tests/opt/common/dce.cpp``.

### Constant folding patterns (`ArithConstFoldPass`)

- Addition: `add(c1, c2) -> const(c1 + c2)` for integer and floating-point types.
- Subtraction: `sub(c1, c2) -> const(c1 - c2)` for integer and floating-point types.
- Multiplication: `mul(c1, c2) -> const(c1 * c2)` for integer and floating-point types.
- Division:
  - Integer: `div(c1, c2) -> const(c1 / c2)` for signed/unsigned integers when `c2 != 0` (no folding on division by zero).
  - Floating-point: `div(c1, c2) -> const(c1 / c2)` using normal IEEE semantics.
- Bitwise AND: `and(c1, c2) -> const(c1 & c2)` for integer types.
- Bitwise OR: `or(c1, c2) -> const(c1 | c2)` for integer types.
- Bitwise XOR: `xor(c1, c2) -> const(c1 ^ c2)` for integer types.
- Shifts:
  - Signed left:  `sal(c, s) -> const(c << s)` for signed integers.
  - Signed right: `sar(c, s) -> const(c >> s)` for signed integers.
  - Unsigned left: `shl(c, s) -> const(c << s)` for unsigned integers.
  - Unsigned right:`shr(c, s) -> const(c >> s)` for unsigned integers.
- Bitwise NOT: `not(c) -> const(~c)` for signed and unsigned integers.
- Comparisons (result is a boolean constant) for integer, floating-point and boolean operands:
  - `cmp.eq(c1, c2)  -> const(c1 == c2)`
  - `cmp.neq(c1, c2) -> const(c1 != c2)`
  - `cmp.a(c1, c2)   -> const(c1 >  c2)`
  - `cmp.b(c1, c2)   -> const(c1 <  c2)`
  - `cmp.ae(c1, c2)  -> const(c1 >= c2)`
  - `cmp.be(c1, c2)  -> const(c1 <= c2)`

IR before ``ArithConstFoldPass``:
```text
constfold:
  ^bb0 <start> <final> :
      a0.si builtin.param -> (v19)
      a1.ui builtin.param -> (v21)
      c2.si arith.const (10) -> (v10,v17)
      c3.si arith.const (3) -> (v10,v11,v17,v19)
      c4.ui arith.const (7) -> (v12,v13,v16)
      c5.ui arith.const (2) -> (v12,v16)
      c6.f arith.const (1.5) -> (v14)
      c7.f arith.const (0.5) -> (v14,v15)
      c8.b arith.const (true) -> (v18)
      c9.b arith.const (false) -> (v18)
      v10.si arith.add (c2 : si, c3 : si) -> (v11)
      v11.si arith.mul (v10 : si, c3 : si) -> (v20)
      v12.ui arith.and (c4 : ui, c5 : ui) -> (v13)
      v13.ui arith.xor (v12 : ui, c4 : ui) -> (v21)
      v14.f arith.add (c6 : f, c7 : f) -> (v15)
      v15.f arith.div (v14 : f, c7 : f) -> (v23)
      v16.ui arith.shl (c4 : ui, c5 : ui)
      v17.si arith.sar (c2 : si, c3 : si)
      v18.b arith.cmp.eq (c8 : b, c9 : b) -> (v22)
      v19.si arith.add (a0 : si, c3 : si) -> (v20)
      v20.si arith.add (v11 : si, v19 : si) -> (v24)
      v21.ui arith.and (v13 : ui, a1 : ui)
      v22.si arith.cast (v18 : b) -> (v24)
      v23.si arith.cast (v15 : f) -> (v25)
      v24.si arith.add (v20 : si, v22 : si) -> (v25)
      v25.si arith.add (v24 : si, v23 : si) -> (n26)
      n26 ctrlflow.return (v25 : si)
```

IR after ``ArithConstFoldPass`` + ``DCEPass``:
```text
constfold:
  ^bb0 <start> <final> :
      a0.si builtin.param -> (v19)
      a1.ui builtin.param
      c3.si arith.const (3) -> (v19)
      c11.si arith.const (39) -> (v20)
      c15.f arith.const (4) -> (v23)
      c18.b arith.const (false) -> (v22)
      v19.si arith.add (a0 : si, c3 : si) -> (v20)
      v20.si arith.add (c11 : si, v19 : si) -> (v24)
      v22.si arith.cast (c18 : b) -> (v24)
      v23.si arith.cast (c15 : f) -> (v25)
      v24.si arith.add (v20 : si, v22 : si) -> (v25)
      v25.si arith.add (v24 : si, v23 : si) -> (n26)
      n26 ctrlflow.return (v25 : si)
```

See corresponding example - ``examples/ConstFold``.
See corresponding tests - ``tests/opt/arith/constfold.cpp``.

### Peephole patterns (`ArithPeepHolePass`):

- ``add``:
  - ``add(x, 0)`` -> ``x``
  - ``add(0, x)`` -> ``x``
- ``sub``:
  - ``sub(x, 0)`` -> ``x``
  - ``sub(x, x)`` -> ``0``
- ``mul``:
  - ``mul(x, 1)`` -> ``x``
  - ``mul(1, x)`` -> ``x``
  - ``mul(x, 0)`` -> ``0``
  - ``mul(0, x)`` -> ``0``
- ``div``:
  - ``div(x, 1)`` -> ``x``
- ``and``:
  - ``and(x, 0)`` -> ``0``
  - ``and(0, x)`` -> ``0``
  - ``and(x, 11..1)`` -> ``x``
  - ``and(11..1, x)`` -> ``x``
  - ``and(x, x)`` -> ``x``
- ``or``:
  - ``or(x, 0)`` -> ``x``
  - ``or(0, x)`` -> ``x``
  - ``or(x, 11..1)`` -> ``11..1``
  - ``or(11..1, x)`` -> ``11..1``
  - ``or(x, x)`` -> ``x``
- ``xor``:
  - ``xor(x, 0)`` -> ``x``
  - ``xor(0, x)`` -> ``x``
  - ``xor(x, x)`` -> ``0``
  - ``xor(x, 11..1)`` -> ``~x``
  - ``xor(11..1, x)`` -> ``~x``
- Shifts (arithmetical `sal`/`sar`, logical `shl`/`shr`):
  - ``sal(x, 0)`` -> ``x``, ``sal(0, x)`` -> ``0``
  - ``shl(x, 0)`` -> ``x``, ``shl(0, x)`` -> ``0``
  - ``sar(x, 0)`` -> ``x``, ``sar(0, x)`` -> ``0``
  - ``shr(x, 0)`` -> ``x``, ``shr(0, x)`` -> ``0``
- Double negation:
  - ``not(not(x))`` -> ``x``

IR before ``ArithPeepHolePass``:
```text
peephole:
  ^bb0 <start> <final> :
      a0.si builtin.param -> (v10,v11,v12,v14,v15,v16,v18,v34,v35,v36,v37)
      a1.si builtin.param -> (v13,v13,v17)
      a2.ui builtin.param -> (v19,v20,v21,v22,v24,v25,v26,v27,v29,v30,v32,v33,v38,v39,v40,v41,v42)
      a3.ui builtin.param -> (v23,v23,v28,v28,v31,v31)
      c4.si arith.const (0) -> (v10,v11,v12,v16,v17,v34,v35,v36,v37)
      c5.si arith.const (1) -> (v14,v15,v18)
      c6.ui arith.const (0) -> (v19,v20,v24,v25,v29,v30,v38,v39,v40,v41)
      c7.ui arith.const (1)
      c8.si arith.const (-1)
      c9.ui arith.const (18446744073709551615) -> (v21,v22,v26,v27,v32,v33)
      v10.si arith.add (a0 : si, c4 : si) -> (v44)
      v11.si arith.add (c4 : si, a0 : si)
      v12.si arith.sub (a0 : si, c4 : si) -> (v44)
      v13.si arith.sub (a1 : si, a1 : si)
      v14.si arith.mul (a0 : si, c5 : si) -> (v45)
      v15.si arith.mul (c5 : si, a0 : si)
      v16.si arith.mul (a0 : si, c4 : si)
      v17.si arith.mul (c4 : si, a1 : si)
      v18.si arith.div (a0 : si, c5 : si) -> (v45)
      v19.ui arith.and (a2 : ui, c6 : ui)
      v20.ui arith.and (c6 : ui, a2 : ui)
      v21.ui arith.and (a2 : ui, c9 : ui) -> (v46)
      v22.ui arith.and (c9 : ui, a2 : ui)
      v23.ui arith.and (a3 : ui, a3 : ui)
      v24.ui arith.or (a2 : ui, c6 : ui)
      v25.ui arith.or (c6 : ui, a2 : ui)
      v26.ui arith.or (a2 : ui, c9 : ui) -> (v46)
      v27.ui arith.or (c9 : ui, a2 : ui)
      v28.ui arith.or (a3 : ui, a3 : ui)
      v29.ui arith.xor (a2 : ui, c6 : ui)
      v30.ui arith.xor (c6 : ui, a2 : ui)
      v31.ui arith.xor (a3 : ui, a3 : ui)
      v32.ui arith.xor (a2 : ui, c9 : ui)
      v33.ui arith.xor (c9 : ui, a2 : ui)
      v34.si arith.sal (a0 : si, c4 : si)
      v35.si arith.sal (c4 : si, a0 : si)
      v36.si arith.sar (a0 : si, c4 : si)
      v37.si arith.sar (c4 : si, a0 : si)
      v38.ui arith.shl (a2 : ui, c6 : ui)
      v39.ui arith.shl (c6 : ui, a2 : ui)
      v40.ui arith.shr (a2 : ui, c6 : ui)
      v41.ui arith.shr (c6 : ui, a2 : ui)
      v42.ui arith.not (a2 : ui) -> (v43)
      v43.ui arith.not (v42 : ui) -> (n48)
      v44.si arith.add (v10 : si, v12 : si) -> (v47)
      v45.si arith.add (v14 : si, v18 : si) -> (v47)
      v46.ui arith.and (v21 : ui, v26 : ui)
      v47.si arith.add (v44 : si, v45 : si)
      n48 ctrlflow.return (v43 : ui)
```

IR after ``ArithPeepHolePass``:
```text
peephole:
  ^bb0 <start> <final> :
      a0.si builtin.param -> (v44,v44,v45,v45)
      a1.si builtin.param
      a2.ui builtin.param -> (v32,v33,n48)
      a3.ui builtin.param
      c4.si arith.const (0)
      c5.si arith.const (1)
      c6.ui arith.const (0)
      c7.ui arith.const (1)
      c8.si arith.const (-1)
      c9.ui arith.const (18446744073709551615)
      c13.si arith.const (0)
      c16.si arith.const (0)
      c17.si arith.const (0)
      c19.ui arith.const (0)
      c20.ui arith.const (0)
      c31.ui arith.const (0)
      v32.ui arith.not (a2 : ui)
      v33.ui arith.not (a2 : ui)
      c35.si arith.const (0)
      c37.si arith.const (0)
      c39.ui arith.const (0)
      c41.ui arith.const (0)
      v44.si arith.add (a0 : si, a0 : si) -> (v47)
      v45.si arith.add (a0 : si, a0 : si) -> (v47)
      v47.si arith.add (v44 : si, v45 : si)
      n48 ctrlflow.return (a2 : ui)
```

See corresponding example - ``examples/PeepHole``.
See corresponding tests - ``tests/opt/arith/peephole.cpp``.

### Building examples

Requirements:
- _CMake_ >= 3.21
- _C++_ compiler supporting **C++23** standard

To build project, use following commads from the project's roor directory:
```text
cmake -B build
cmake --build build
```

If you want to build specific target, use following command:
```text
cmake --build build --target <target_name>
```

List of available targets:
- ``iris`` - IR-support static library;
- ``factorial`` - example of IR usage - building IR for a programm calculating factorial manually via ``iris::IRBuilder``;
- ``domExample01`` - ``domExample03`` - examples of running Dom analysis and building IDom tree;
- ``loopExample01`` - ``loopExample06`` - examples of running Loop analysis and building Loop tree;
- ``DCE`` - example of optimizing IR with use of DCE;
- ``ConstFold`` - example of optimizing IR with use of Constant Folding;
- ``PeepHole`` - example of optimizing IR with use of PeepHole optimizations.

Static library output files are located in ``build/lib/``, and executables - in ``build/bin``.

### Testing

Project includes tests, impemented using **GTest**.

To build and run tests, run following commands from project's root directory:
```text
cmake -B build
cmake --build build --target iris_tests
cd build
ctest
```

Tests include:
- **Dominator analysis** tests (``tests/doms.cpp``);
- **Loop analysis** tests (``tests/loops.cpp``).
- Separate test suites for each **dialect** and its **operations'** verifiers and construction pre-conditions (``tests/dialects/``);
- **Integration** tests (``tests/integration.cpp``);
- **Optimization** **passes** & **patterns** tests (``tests/opt/``):
    - Dead Code Elimination (``common/dce.cpp``);
    - Constant Folding for ``arith`` operations (``arith/constfold.cpp``);
    - PeepHole Optimizations for ``arith`` operations (``arith/peephole.cpp``).
- Tests of **verifiers** (ex-fails) and **API** tests for:
    - Generic operation ``iris::Operation``;
    - Basic block ``iris::BasicBlock``;
    - Region ``iris::Region``;
    - IR-builder ``iris::IRBuilder``;
    - Dominator analysis helper class ``iris::doms::DomInfo``;
    - Loop analysis helper class ``iris::doms::LoopInfo`` (TODO).

