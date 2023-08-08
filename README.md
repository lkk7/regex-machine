# regex-machine

A header-only library for regular expressions.

# Local development

This is a header-only library, so the build process is mostly about linting and tests.
There is an example file `CMakeUserPresets.example.json` with sane defaults.  
Copy it to `CMakeUserPresets.json` and modify anything if needed (it's ignored by git).
To build tests, use CMake with a given preset from that file, for example:
```sh
cmake --preset=dev-linux
cmake --build build/dev-linux
```
Then, to run tests:
```
cmake --build build/dev-linux -t test
```

To run formatting checks as the CI does:
```sh
cmake --build build/dev-linux -t format-check spell-check
```
To fix all formatting errors if they do exist:
```sh
cmake --build build/dev-linux -t format-fix spell-fix
```
