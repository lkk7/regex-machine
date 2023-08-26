# regex-machine

A small regex-like header-only lib for string matching.

**Works only on `char`s, no UTF support.**

- `(expr1)(expr2)` – `expr1` immediately followed by `expr2`,
- `(expr1)|(expr2)` – `expr1` or `expr2`,
- `(expr)*` – zero or more of `expr`,
- `(expr)+` – one or more of `expr`,
- `(expr)?` – zero or one of `expr`,

Powered by NFAs (Nondeterministic Finite Automata), inspired by article "Finite State Machines and Regular Expressions" by Eli Bendersky.

# Usage

```cpp
// ...
#include "regex_machine.hpp"
// ...
const RM::Matcher regex{"ca(k|v)e"};
bool cafe = regex.match("cake") // true
bool cake = regex.match("cave") // true
bool cape = regex.match("cape") // false

const RM::Matcher wrong{"ca(k|ve"};
std::cout << wrong.err_msg; // "unbalanced parens"
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

To run formatting checks as the CI does:
```sh
cmake --build build/dev-linux -t format-check spell-check
```
To fix all formatting errors if they do exist:
```sh
cmake --build build/dev-linux -t format-fix spell-fix
```
