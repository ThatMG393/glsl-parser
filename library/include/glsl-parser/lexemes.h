// Types
TYPE(eof)
TYPE(whitespace)
TYPE(comment)
TYPE(keyword)
TYPE(identifier)
TYPE(constant_int)
TYPE(constant_uint)
TYPE(constant_float)
TYPE(constant_double)
TYPE(operator)
TYPE(semicolon)
TYPE(scope_begin)
TYPE(scope_end)
TYPE(directive)

// Keywords
KEYWORD(attribute)
KEYWORD(const)
KEYWORD(uniform)
KEYWORD(varying)
KEYWORD(buffer)
KEYWORD(shared)
KEYWORD(coherent)
KEYWORD(volatile)
KEYWORD(restrict)
KEYWORD(readonly)
KEYWORD(writeonly)
KEYWORD(atomic_uint)
KEYWORD(layout)
KEYWORD(centroid)
KEYWORD(flat)
KEYWORD(smooth)
KEYWORD(noperspective)
KEYWORD(patch)
KEYWORD(sample)
KEYWORD(break)
KEYWORD(continue)
KEYWORD(do)
KEYWORD(for)
KEYWORD(while)
KEYWORD(switch)
KEYWORD(case)
KEYWORD(default)
KEYWORD(if)
KEYWORD(else)
KEYWORD(subroutine)
KEYWORD(in)
KEYWORD(out)
KEYWORD(inout)

KEYWORD(float)
KEYWORD(double)
KEYWORD(int)
KEYWORD(void)
KEYWORD(bool)
KEYWORD(true)
KEYWORD(false)

KEYWORD(invariant)
KEYWORD(precise)

KEYWORD(discard)
KEYWORD(return)

KEYWORD(mat2)
KEYWORD(mat3)
KEYWORD(mat4)
KEYWORD(dmat2)
KEYWORD(dmat3)
KEYWORD(dmat4)

KEYWORD(mat2x2)
KEYWORD(mat2x3)
KEYWORD(mat2x4)
KEYWORD(dmat2x2)
KEYWORD(dmat2x3)
KEYWORD(dmat2x4)

KEYWORD(mat3x2)
KEYWORD(mat3x3)
KEYWORD(mat3x4)
KEYWORD(dmat3x2)
KEYWORD(dmat3x3)
KEYWORD(dmat3x4)

KEYWORD(mat4x2)
KEYWORD(mat4x3)
KEYWORD(mat4x4)
KEYWORD(dmat4x2)
KEYWORD(dmat4x3)
KEYWORD(dmat4x4)

KEYWORD(vec2)
KEYWORD(vec3)
KEYWORD(vec4)
KEYWORD(ivec2)
KEYWORD(ivec3)
KEYWORD(ivec4)
KEYWORD(bvec2)
KEYWORD(bvec3)
KEYWORD(bvec4)
KEYWORD(dvec2)
KEYWORD(dvec3)
KEYWORD(dvec4)

KEYWORD(uint)
KEYWORD(uvec2)
KEYWORD(uvec3)
KEYWORD(uvec4)

KEYWORD(lowp)
KEYWORD(mediump)
KEYWORD(highp)
KEYWORD(precision)

KEYWORD(sampler1D)
KEYWORD(sampler2D)
KEYWORD(sampler3D)
KEYWORD(samplerCube)

KEYWORD(sampler1DShadow)
KEYWORD(sampler2DShadow)
KEYWORD(samplerCubeShadow)

KEYWORD(sampler1DArray)
KEYWORD(sampler2DArray)

KEYWORD(sampler1DArrayShadow)
KEYWORD(sampler2DArrayShadow)

KEYWORD(isampler1D)
KEYWORD(isampler2D)
KEYWORD(isampler3D)
KEYWORD(isamplerCube)

KEYWORD(isampler1DArray)
KEYWORD(isampler2DArray)

KEYWORD(usampler1D)
KEYWORD(usampler2D)
KEYWORD(usampler3D)
KEYWORD(usamplerCube)

KEYWORD(usampler1DArray)
KEYWORD(usampler2DArray)

KEYWORD(sampler2DRect)
KEYWORD(sampler2DRectShadow)
KEYWORD(isampler2DRect)
KEYWORD(usampler2DRect)

KEYWORD(samplerBuffer)
KEYWORD(isamplerBuffer)
KEYWORD(usamplerBuffer)

KEYWORD(sampler2DMS)
KEYWORD(isampler2DMS)
KEYWORD(usampler2DMS)

KEYWORD(sampler2DMSArray)
KEYWORD(isampler2DMSArray)
KEYWORD(usampler2DMSArray)

KEYWORD(samplerCubeArray)
KEYWORD(samplerCubeArrayShadow)
KEYWORD(isamplerCubeArray)
KEYWORD(usamplerCubeArray)

KEYWORD(image1D)
KEYWORD(iimage1D)
KEYWORD(uimage1D)

KEYWORD(image2D)
KEYWORD(iimage2D)
KEYWORD(uimage2D)

KEYWORD(image3D)
KEYWORD(iimage3D)
KEYWORD(uimage3D)

KEYWORD(image2DRect)
KEYWORD(iimage2DRect)
KEYWORD(uimage2DRect)

KEYWORD(imageCube)
KEYWORD(iimageCube)
KEYWORD(uimageCube)

KEYWORD(imageBuffer)
KEYWORD(iimageBuffer)
KEYWORD(uimageBuffer)

KEYWORD(image1DArray)
KEYWORD(iimage1DArray)
KEYWORD(uimage1DArray)

KEYWORD(image2DArray)
KEYWORD(iimage2DArray)
KEYWORD(uimage2DArray)

KEYWORD(imageCubeArray)
KEYWORD(iimageCubeArray)
KEYWORD(uimageCubeArray)

KEYWORD(image2DMS)
KEYWORD(iimage2DMS)
KEYWORD(uimage2DMS)

KEYWORD(image2DMSArray)
KEYWORD(iimage2DMSArray)
KEYWORD(uimage2DMSArray)

KEYWORD(struct)

// Reserved for future use
KEYWORD(common)
KEYWORD(partition)
KEYWORD(active)

KEYWORD(asm)

KEYWORD(class)
KEYWORD(union)
KEYWORD(enum)
KEYWORD(typedef)
KEYWORD(template)
KEYWORD(this)

KEYWORD(resource)

KEYWORD(goto)

KEYWORD(inline)
KEYWORD(noinline)
KEYWORD(public)
KEYWORD(static)
KEYWORD(extern)
KEYWORD(external)
KEYWORD(interface)

KEYWORD(long)
KEYWORD(short)
KEYWORD(half)
KEYWORD(fixed)
KEYWORD(unsigned)
KEYWORD(superp)

KEYWORD(input)
KEYWORD(output)

KEYWORD(hvec2)
KEYWORD(hvec3)
KEYWORD(hvec4)
KEYWORD(fvec2)
KEYWORD(fvec3)
KEYWORD(fvec4)

KEYWORD(sampler3DRect)

KEYWORD(filter)

KEYWORD(sizeof)
KEYWORD(cast)

KEYWORD(namespace)
KEYWORD(using)


// Operators
//       name,              sign, precedence, array index (n + 1)
OPERATOR(paranthesis_begin,  "(",       17) // 1
OPERATOR(paranthesis_end,    ")",       17) // 2
OPERATOR(bracket_begin,      "[",       16) // 3
OPERATOR(bracket_end,        "]",       16) // 4
OPERATOR(dot,                ".",       16) // 5
OPERATOR(increment,          "++",      15) // 6 // Note: 16 when used as post-fix
OPERATOR(decrement,          "--",      15) // 7 // Note: 16 when used as post-fix
OPERATOR(bit_not,            "~",       15) // 8
OPERATOR(logical_not,        "!",       15) // 9
OPERATOR(multiply,           "*",       14) // 10
OPERATOR(divide,             "/",       14) // 11
OPERATOR(modulus,            "%",       14) // 12
OPERATOR(plus,               "+",       13) // 13 // Note: 15 when unary
OPERATOR(minus,              "-",       13) // 14 // Note: 15 when unary
OPERATOR(shift_left,         "<<",      12) // 15
OPERATOR(shift_right,        ">>",      12) // 16
OPERATOR(less,               "<",       11) // 17
OPERATOR(greater,            ">",       11) // 18
OPERATOR(less_equal,         "<=",      11) // 19
OPERATOR(greater_equal,      ">=",      11) // 20
OPERATOR(equal,              "==",      10) // 21
OPERATOR(not_equal,          "!=",      10) // 22
OPERATOR(bit_and,            "&",       9)  // 23
OPERATOR(bit_xor,            "^",       8)  // 24
OPERATOR(bit_or,             "|",       7)  // 25
OPERATOR(logical_and,        "&&",      6)  // 26
OPERATOR(logical_xor,        "^^",      5)  // 27
OPERATOR(logical_or,         "||",      4)  // 28
OPERATOR(questionmark,       "?",       3)  // 29
OPERATOR(colon,              ":",       3)  // 30
OPERATOR(assign,             "=",       2)  // 31
OPERATOR(add_assign,         "+=",      2)  // 32
OPERATOR(sub_assign,         "-=",      2)  // 33
OPERATOR(multiply_assign,    "*=",      2)  // 34
OPERATOR(divide_assign,      "/=",      2)  // 35
OPERATOR(modulus_assign,     "%=",      2)  // 36
OPERATOR(shift_left_assign,  "<<=",     2)  // 37
OPERATOR(shift_right_assign, ">>=",     2)  // 38
OPERATOR(bit_and_assign,     "&=",      2)  // 39
OPERATOR(bit_xor_assign,     "^=",      2)  // 40
OPERATOR(bit_or_assign,      "|=",      2)  // 41
OPERATOR(comma,              ",",       1)  // 42

TYPENAME(void)
TYPENAME(bool)
TYPENAME(int)
TYPENAME(uint)
TYPENAME(float)
TYPENAME(double)
TYPENAME(vec2)
TYPENAME(vec3)
TYPENAME(vec4)
TYPENAME(dvec2)
TYPENAME(dvec3)
TYPENAME(dvec4)
TYPENAME(bvec2)
TYPENAME(bvec3)
TYPENAME(bvec4)
TYPENAME(ivec2)
TYPENAME(ivec3)
TYPENAME(ivec4)
TYPENAME(uvec2)
TYPENAME(uvec3)
TYPENAME(uvec4)
TYPENAME(mat2)
TYPENAME(mat3)
TYPENAME(mat4)
TYPENAME(mat2x2)
TYPENAME(mat2x3)
TYPENAME(mat2x4)
TYPENAME(mat3x2)
TYPENAME(mat3x3)
TYPENAME(mat3x4)
TYPENAME(mat4x2)
TYPENAME(mat4x3)
TYPENAME(mat4x4)
TYPENAME(dmat2)
TYPENAME(dmat3)
TYPENAME(dmat4)
TYPENAME(dmat2x2)
TYPENAME(dmat2x3)
TYPENAME(dmat2x4)
TYPENAME(dmat3x2)
TYPENAME(dmat3x3)
TYPENAME(dmat3x4)
TYPENAME(dmat4x2)
TYPENAME(dmat4x3)
TYPENAME(dmat4x4)
TYPENAME(sampler1D)
TYPENAME(image1D)
TYPENAME(sampler2D)
TYPENAME(image2D)
TYPENAME(sampler3D)
TYPENAME(image3D)
TYPENAME(samplerCube)
TYPENAME(imageCube)
TYPENAME(sampler2DRect)
TYPENAME(image2DRect)
TYPENAME(sampler1DArray)
TYPENAME(image1DArray)
TYPENAME(sampler2DArray)
TYPENAME(image2DArray)
TYPENAME(samplerBuffer)
TYPENAME(imageBuffer)
TYPENAME(sampler2DMS)
TYPENAME(image2DMS)
TYPENAME(sampler2DMSArray)
TYPENAME(image2DMSArray)
TYPENAME(samplerCubeArray)
TYPENAME(imageCubeArray)
TYPENAME(sampler1DShadow)
TYPENAME(sampler2DShadow)
TYPENAME(sampler2DRectShadow)
TYPENAME(sampler1DArrayShadow)
TYPENAME(sampler2DArrayShadow)
TYPENAME(samplerCubeShadow)
TYPENAME(samplerCubeArrayShadow)
TYPENAME(isampler1D)
TYPENAME(iimage1D)
TYPENAME(isampler2D)
TYPENAME(iimage2D)
TYPENAME(isampler3D)
TYPENAME(iimage3D)
TYPENAME(isamplerCube)
TYPENAME(iimageCube)
TYPENAME(isampler2DRect)
TYPENAME(iimage2DRect)
TYPENAME(isampler1DArray)
TYPENAME(iimage1DArray)
TYPENAME(isampler2DArray)
TYPENAME(iimage2DArray)
TYPENAME(isamplerBuffer)
TYPENAME(iimageBuffer)
TYPENAME(isampler2DMS)
TYPENAME(iimage2DMS)
TYPENAME(isampler2DMSArray)
TYPENAME(iimage2DMSArray)
TYPENAME(isamplerCubeArray)
TYPENAME(iimageCubeArray)
TYPENAME(atomic_uint)
TYPENAME(usampler1D)
TYPENAME(uimage1D)
TYPENAME(usampler2D)
TYPENAME(uimage2D)
TYPENAME(usampler3D)
TYPENAME(uimage3D)
TYPENAME(usamplerCube)
TYPENAME(uimageCube)
TYPENAME(usampler2DRect)
TYPENAME(uimage2DRect)
TYPENAME(usampler1DArray)
TYPENAME(uimage1DArray)
TYPENAME(usampler2DArray)
TYPENAME(uimage2DArray)
TYPENAME(usamplerBuffer)
TYPENAME(uimageBuffer)
TYPENAME(usampler2DMS)
TYPENAME(uimage2DMS)
TYPENAME(usampler2DMSArray)
TYPENAME(uimage2DMSArray)
TYPENAME(usamplerCubeArray)
TYPENAME(uimageCubeArray)
