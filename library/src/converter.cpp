#include "glsl-parser/converter.h"
#include "glsl-parser/ast.h"
#include "glsl-parser/lexer.h"
#include "glsl-parser/util.h"

#define EXPRC(type) astExpression::k##type##Constant
#define EXPRN(type) astExpression::k##type

namespace glsl {

inline indent_aware_stringbuilder astVariableToString(astVariable*, indent_aware_stringbuilder&, bool);

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
    }

    return "unknown_type";
}

inline const char* fieldOrSwizzleToString(astFieldOrSwizzle*, indent_aware_stringbuilder&);

inline const char* astExpressionToString(astExpression* expression, indent_aware_stringbuilder& sb) {
    switch (expression->type) {
        case EXPRC(Int): return ntoa("%i", reinterpret_cast<astIntConstant*>(expression)->value);
        case EXPRC(UInt): return ntoa("%u", reinterpret_cast<astUIntConstant*>(expression)->value);
        case EXPRC(Float): return ntoa("%f", reinterpret_cast<astFloatConstant*>(expression)->value);
        case EXPRC(Double): return ntoa("%f", reinterpret_cast<astDoubleConstant*>(expression)->value);
        case EXPRC(Bool): return reinterpret_cast<astBoolConstant*>(expression)->value ? "true" : "false";
        
        // potentially unsafe
        case EXPRN(VariableIdentifier): return astVariableToString(reinterpret_cast<astVariableIdentifier*>(expression)->variable, sb, true).toString();
        case EXPRN(FieldOrSwizzle): fieldOrSwizzleToString(reinterpret_cast<astFieldOrSwizzle*>(expression), sb);
        case EXPRN(ArraySubscript): break;
        default: return "";
    }
    return "";
}

inline const char* fieldOrSwizzleToString(astFieldOrSwizzle *expression, indent_aware_stringbuilder& sb) {
    astExpressionToString(expression->operand, sb);
    expression->name;
}

inline indent_aware_stringbuilder astVariableToString(astVariable* var, indent_aware_stringbuilder& prevSb, bool nameOnly = false) {
    indent_aware_stringbuilder sb;
    sb.copyIndent(prevSb);

    if (var->isPrecise) sb.append("precise ");
    if (nameOnly) {
        sb += var->name;
        goto end;
    }

    sb += typeToString(var->baseType);
    sb += " ";
    sb += var->name;

    if (var->isArray) {
        for (const auto& arraySize : var->arraySizes) {
            sb += "[";
            sb += astExpressionToString(arraySize, sb);
            sb += "]";
        }
    }

    end:
    return sb;
}

inline const char* auxiliaryToString(int auxiliary) {
    switch (auxiliary) {
        case kCentroid: return "centroid";
        case kSample: return "sample";
        case kPatch: return "patch";
        default: return "";
    }
}

inline const char* memoryToString(int memory) {
    if (memory & kCoherent) return "coherent";
    if (memory & kVolatile) return "volatile";
    if (memory & kRestrict) return "restrict";
    if (memory & kReadOnly) return "readonly";
    if (memory & kWriteOnly) return "writeonly";
    return "";
}

converter::converter() : stringBuffer(indent_aware_stringbuilder()) { }

const char* converter::convertTU(astTU* translationUnit) {
    visitPreprocessors(translationUnit);
    visitStructures(translationUnit);
    visitInterfaceBlocks(translationUnit);
    visitGlobalVariables(translationUnit);

    return stringBuffer.toString();
}

void converter::visitPreprocessors(astTU* tu) {
    if (tu->versionDirective) {
        stringBuffer += "#version ";
        stringBuffer += ntoa("%d", tu->versionDirective->version);
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

void converter::visitStructures(astTU* tu) {
    for (const auto& structure : tu->structures) {
        stringBuffer += "struct ";
        stringBuffer += structure->name;
        stringBuffer.appendLine(" {");
        stringBuffer.pushIndent();

        for (const auto& field : structure->fields) {
            stringBuffer.append(astVariableToString(field, stringBuffer));
            stringBuffer.appendLine(";");
        }

        stringBuffer.popIndent();
        stringBuffer.appendLine("};");
        stringBuffer.appendLine();
    }
}

void converter::visitInterfaceBlocks(astTU* tu) {
    for (const auto& interfaceBlock : tu->interfaceBlocks) {
        stringBuffer += storageToString(interfaceBlock->storage);
        stringBuffer += " ";
        stringBuffer += interfaceBlock->name;
        stringBuffer.appendLine(" {");
        stringBuffer.pushIndent();

        for (const auto& field : interfaceBlock->fields) {
            stringBuffer.append(astVariableToString(field, stringBuffer));
            stringBuffer.appendLine();
        }

        stringBuffer.popIndent();
        stringBuffer.appendLine("};");
        stringBuffer.appendLine();
    }
}

void converter::visitGlobalVariables(astTU* tu) {
    for (const auto& global : tu->globals) {
        if (global->layoutQualifiers.size()) {
            stringBuffer += "layout(";
            for (size_t i = 0; i < global->layoutQualifiers.size(); ++i) {
                astLayoutQualifier* layoutQualifier = global->layoutQualifiers[i];
                stringBuffer += layoutQualifier->name;
                if (layoutQualifier->initialValue) {
                    stringBuffer += " = ";
                    stringBuffer += astExpressionToString(layoutQualifier->initialValue);
                    if (i != global->layoutQualifiers.size() - 1)
                        stringBuffer += ", ";
                }
            }
            stringBuffer += ") ";
        }
 
        stringBuffer += storageToString(global->storage);
        stringBuffer += " ";

        if (global->auxiliary != -1) {
            stringBuffer += auxiliaryToString(global->auxiliary);
            stringBuffer += " ";
        }

        if (global->memory != 0) {
            stringBuffer += memoryToString(global->memory);
            stringBuffer += " ";
        }

        if (global->isInvariant) stringBuffer += "invariant ";

        stringBuffer.append(astVariableToString(reinterpret_cast<astVariable*>(global), stringBuffer));

        if (global->initialValue) {
            stringBuffer += " = ";
            stringBuffer += astExpressionToString(global->initialValue);
        }

        stringBuffer.appendLine(";");
    }
}

}