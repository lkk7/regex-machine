# regex-machine

A small regex-like header-only lib for string matching.

**Works only on standard 8-bit `char`s, no UTF support.**
  
- `(expr1)(expr2)` – `expr1` immediately followed by `expr2`,
- `(expr1)|(expr2)` – `expr1` or `expr2`,
- `(expr)*` – zero or more of `expr`,
- `(expr)+` – one or more of `expr`,
- `(expr)?` – zero or one of `expr`,

Powered by NFAs (Nondeterministic Finite Automata), inspired by the article "Finite State Machines and Regular Expressions" by Eli Bendersky. Educational and not usable in a production scenario. 

# Usage

```cpp
// ...
#include "regex_machine.hpp"
// ...
const RM::Matcher regex{"ca(k|v)*e"};
regex.match("cae"); // returns true
regex.match("cake"); // true
regex.match("cavvvvve"); // true
regex.match("cape"); // false

const RM::Matcher wrong{"ca(k|ve"};
std::cout << wrong.err_msg; // "unbalanced parens"

RM::Matcher{"ca(k|v)\\*e"}.match("cav*e"); // true, '*' is escaped by the backslash 
```

# Local development

This is a header-only library, so the build process is about linting and tests.
There is an example file `CMakeUserPresets.example.json` with sane defaults.  
Copy it to `CMakeUserPresets.json` and modify anything if needed (it's ignored by git).
To build tests, use CMake with a given preset from that file, for example:
```sh
cmake --preset=dev-linux
cmake --build build/dev-linux
```
Then, to run tests:
```
cd build/dev-linux
ctest --output-on-failure
```
