#include "glsl-parser/converter.h"
#include "glsl-parser/ast.h"
#include "glsl-parser/lexer.h"
#include "glsl-parser/util.h"

namespace glsl {

inline const char* getProfile(int type) {
    switch (type) {
        case kCore: return "core";
        case kCompatibility: return "compatibility";
        case kES: return "es";
        default: return "";
    }
}

inline const char* getExtensionBehavior(int behavior) {
    switch (behavior) {
        case kEnable: return "enable";
        case kRequire: return "require";
        case kWarn: return "warn";
        case kDisable: return "disable";
        default: return "";
    }
}

inline const char* storageToString(int storage) {
    switch (storage) {
        case kConst: return "const";
        case kIn: return "in";
        case kOut: return "out";
        case kAttribute: return "attribute";
        case kUniform: return "uniform";
        case kVarying: return "varying";
        case kBuffer: return "buffer";
        case kShared: return "shared";
        default: return "";
    }
}

inline const char* typeToString(astType* type) {
    if (type->builtin) {
            astBuiltin* builtin = static_cast<astBuiltin*>(type);
            switch (builtin->type) {
                case kKeyword_void: return "void";
                case kKeyword_bool: return "bool";
                case kKeyword_int: return "int";
                case kKeyword_uint: return "uint";
                case kKeyword_float: return "float";
                case kKeyword_double: return "double";
                case kKeyword_vec2: return "vec2";
                case kKeyword_vec3: return "vec3";
                case kKeyword_vec4: return "vec4";
                case kKeyword_ivec2: return "ivec2";
                case kKeyword_ivec3: return "ivec3";
                case kKeyword_ivec4: return "ivec4";
                case kKeyword_uvec2: return "uvec2";
                case kKeyword_uvec3: return "uvec3";
                case kKeyword_uvec4: return "uvec4";
                case kKeyword_bvec2: return "bvec2";
                case kKeyword_bvec3: return "bvec3";
                case kKeyword_bvec4: return "bvec4";
                case kKeyword_mat2: return "mat2";
                case kKeyword_mat3: return "mat3";
                case kKeyword_mat4: return "mat4";
                case kKeyword_mat2x3: return "mat2x3";
                case kKeyword_mat2x4: return "mat2x4";
                case kKeyword_mat3x2: return "mat3x2";
                case kKeyword_mat3x4: return "mat3x4";
                case kKeyword_mat4x2: return "mat4x2";
                case kKeyword_mat4x3: return "mat4x3";
                case kKeyword_sampler2D: return "sampler2D";
                case kKeyword_sampler3D: return "sampler3D";
                case kKeyword_samplerCube: return "samplerCube";
                case kKeyword_sampler2DShadow: return "sampler2DShadow";
                default: return "unknown_type";
            }
        } else if (astStruct* str = static_cast<astStruct*>(type)) {
            return str->name;
        } else if (astInterfaceBlock* block = static_cast<astInterfaceBlock*>(type)) {
            return block->name;
        }
        
    return "unknown_type";
}

inline const char* astExpressionToString(astExpression* expression) {
    switch (expression->type) {
        case astExpression::kIntConstant: return itoa(reinterpret_cast<astIntConstant*>(expression)->value);
        default: return "";
    }
}

inline const char* astVariableToString(astVariable* var, indent_aware_stringbuilder prevSb, bool nameOnly = false) {
    indent_aware_stringbuilder sb(prevSb);

    if (var->isPrecise) sb.append("precise ");
    if (nameOnly) {
        sb += var->name;
        goto end;
    }

    sb += typeToString(var->baseType);
    sb += var->name;

    if (var->isArray) {
        for (const auto& arraySize : var->arraySizes) {
            sb += "[";
            sb += astExpressionToString(arraySize);
            sb += "]";
        }
    }

    sb += ";";
    sb.appendLine();

    end:
    return sb.toString();
}

converter::converter() : stringBuffer(indent_aware_stringbuilder()) { }

const char* converter::convertTU(astTU* translationUnit) {
    visitPreprocessors(translationUnit);
    visitStructs(translationUnit);
    visitGlobals(translationUnit);

    return stringBuffer.toString();
}

void converter::visitPreprocessors(astTU* tu) {
    if (tu->versionDirective) {
        stringBuffer += "#version ";
        stringBuffer += itoa(tu->versionDirective->version);
        stringBuffer += " ";
        stringBuffer += getProfile(tu->versionDirective->type);
        stringBuffer.appendLine();
    }

    for (const auto& extension : tu->extensionDirectives) {
        stringBuffer += "#extension ";
        stringBuffer += extension->name;
        stringBuffer += " : ";
        stringBuffer += getExtensionBehavior(extension->behavior);
        stringBuffer.appendLine();
    }
}

void converter::visitStructs(astTU* tu) {
    for (const auto& structure : tu->structures) {
        stringBuffer += "struct ";
        stringBuffer += structure->name;
        stringBuffer.appendLine(" {");
        stringBuffer.pushIndent();

        for (const auto& field : structure->fields) {
            astVariableToString(field, stringBuffer);
        }

        stringBuffer.popIndent();
        stringBuffer.appendLine("};");
        stringBuffer.appendLine();
    }
}

void converter::visitInterfaceBlocks(astTU* tu) {
    for (const auto& interfaceBlock : tu->interfaceBlocks) {
        
    }
}

void converter::visitGlobals(astTU* tu) {
    for (const auto& global : tu->globals) {
        stringBuffer += storageToString(global->storage);
    }
}

}