# regex-machine

A header-only library for regular expressions.

# Local 

To build tests, choose your platform (`linux`/`darwin`/`win64`) and use CMake, for example:
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
To fix them:
```sh
cmake --build build/dev-linux -t format-fix spell-fix
```
