# glsl-parser

**glsl-parser** is an offline GLSL parser which can be used to do many things with GLSL source code.

The straight-forward API allows you to parse GLSL into an abstact-syntax-tree in only a couple lines, for example
```cpp
#include <glsl-parser/converter.h>
#include <glsl-parser/parser.h>

glsl::parser parse(sourceCode);
glsl::astTU *translationUnit = parse.parse(astTU::kFragment);
if (translationUnit) {
    // Do something with the AST here

    // When done, convert back to GLSL
    // or implement your own AST to code
    // converter.
    const char* converted = converter().convertTU(translationUnit);
    printf("%s", converted);
} else {
    // A parse error occured
    fprintf(stderr, "%s\n", parse.error());
}
```

A test-suite and GLSL source-generator is included to get you started.

Check out the superior diagnostics [here](EXAMPLE_ERRORS.md)

### Known limitations
  * Does not support preprocessor directives
    * Does support `#version` and `#extension` though.
  * Does not handle new-line termination with the backslack character `\`
  * Not all of GLSL is supported, if you run into a missing feature open an issue.
  * None of the builtin functions or variables are provided, you must provide those yourself.

### Possible uses
  * Verify a shader without running it
  * Find hard-to-find syntax or semantic errors with the superior diagnostics
  * Extend GLSL
  * Transcompile GLSL to other languages
  * Quickly check or verify something
  * Optimize shaders
  * Generate introspection information

### Portable and embeddable
  * Written in portable C++11.
    * Uses *'std::vector'* from the standard library.
    * Uses *'for range'* loops.
    * Uses the *'auto'* type.
  * Exception free
  * Doesn't use virtual functions
  * Small (~200 KB)
  * Permissive (MIT)

### Building
Run:
```bash
mkdir build
cd build

cmake ..
cmake --build . -- -j$(nproc)
```

If you wish to install the built targets, run this:
```bash
sudo cmake --install .
```
