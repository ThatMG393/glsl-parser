#ifndef CONVERTER_HDL
#define CONVERTER_HDL

#include "glsl-parser/ast.h"
#include "glsl-parser/util.h"

namespace glsl {

struct converter {
    converter();

    const char* convertTU(astTU*);

private:
    indent_aware_stringbuilder stringBuffer;

    void visitPreprocessors(astTU*);
    void visitStructs(astTU*);
    void visitGlobals(astTU*);
};

}

#endif