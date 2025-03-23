#include "glsl-parser/converter.h"
#include "glsl-parser/ast.h"
#include "glsl-parser/lexer.h"
#include "glsl-parser/util.h"
#include <cstring>

#define EXPRC(type) astExpression::k##type##Constant
#define EXPRN(type) astExpression::k##type
#define STATEMENT(type) astStatement::k##type

namespace glsl {

const std::vector<const char*> operatorMap[18] = {
    {}, // 0 (unused)
    {","}, // 1
    {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="}, // 2
    {"?", ":"}, // 3
    {"||"}, // 4
    {"^^"}, // 5
    {"&&"}, // 6
    {"|"}, // 7
    {"^"}, // 8
    {"&"}, // 9
    {"==", "!="}, // 10
    {"<", ">", "<=", ">="}, // 11
    {"<<", ">>"}, // 12
    {"+", "-"}, // 13 (Note: unary + and - are 15)
    {"*", "/", "%"}, // 14
    {"++", "--", "~", "!"}, // 15 (Note: post-fix ++ and -- are 16)
    {"[", "]", "."}, // 16
    {"(", ")"}, // 17
};

enum {
    kSemicolon = 1 << 0,
    kNewLine = 1 << 1,
    kDefault = kSemicolon | kNewLine
};

inline void astVariableToString(astVariable*, indent_aware_stringbuilder&, bool);
inline void astStatementToString(astStatement*, indent_aware_stringbuilder&);

inline const char* profileToString(int type) {
    switch (type) {
        case kCore: return "core";
        case kCompatibility: return "compatibility";
        case kES: return "es";
    }
    return "";
}

inline const char* extensionBehaviorToString(int behavior) {
    switch (behavior) {
        case kEnable: return "enable";
        case kRequire: return "require";
        case kWarn: return "warn";
        case kDisable: return "disable";
    }
    return "";
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
    }
    return "";
}


inline const char* auxiliaryToString(int auxiliary) {
    switch (auxiliary) {
        case kCentroid: return "centroid";
        case kSample: return "sample";
        case kPatch: return "patch";
    }
    return "";
}

inline const char* memoryToString(int memory) {
    if (memory & kCoherent) return "coherent";
    if (memory & kVolatile) return "volatile";
    if (memory & kRestrict) return "restrict";
    if (memory & kReadOnly) return "readonly";
    if (memory & kWriteOnly) return "writeonly";
    return "";
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
    } else if (astStruct* structure = reinterpret_cast<astStruct*>(type)) {
        return structure->name;
    }

    return "unknown_type";
}

inline void astExpressionToString(astExpression* expression, indent_aware_stringbuilder& sb) {
	printf("astExpression(%i)\n", expression->type);
    switch (expression->type) {
        case EXPRC(Int):
            sb += ntoa("%i", reinterpret_cast<astIntConstant*>(expression)->value);
        break;
        
        case EXPRC(UInt):
            sb += ntoa("%u", reinterpret_cast<astUIntConstant*>(expression)->value);
        break;
        
        case EXPRC(Float):
            sb += ntoa("%f", reinterpret_cast<astFloatConstant*>(expression)->value);
        break;

        case EXPRC(Double):
            sb += ntoa("%f", reinterpret_cast<astDoubleConstant*>(expression)->value);
        break;

        case EXPRC(Bool):
            sb += reinterpret_cast<astBoolConstant*>(expression)->value ? "true" : "false";
        break;
        
        case EXPRN(VariableIdentifier):
            astVariableToString(reinterpret_cast<astVariableIdentifier*>(expression)->variable, sb, true);
        break;

        case EXPRN(Assign):
        {
            astAssignmentExpression* assignmentExpression = reinterpret_cast<astAssignmentExpression*>(expression);
            astExpressionToString(assignmentExpression->operand1, sb);
            sb += " = ";
            astExpressionToString(assignmentExpression->operand2, sb);
        }
        break;

        case EXPRN(Operation):
        {
        	astOperationExpression* operationExpression = reinterpret_cast<astOperationExpression*>(expression);
        	astExpressionToString(operationExpression->operand1, sb);
            sb += " ";
            sb += operatorMap[operationExpression->operation][0];
            sb += " ";
            astExpressionToString(operationExpression->operand2, sb);
        }
        break;
    }
}

inline void astVariableToString(astVariable* var, indent_aware_stringbuilder& sb, bool nameOnly = false) {
    if (var->isPrecise) sb.append("precise ");
    if (nameOnly) {
        sb += var->name;
        return;
    }

    sb += typeToString(var->baseType);
    sb += " ";
    sb += var->name;

    if (var->isArray) {
        for (const auto& arraySize : var->arraySizes) {
            sb += "[";
            astExpressionToString(arraySize, sb);
            sb += "]";
        }
    }
}

inline void astFunctionVariableToString(astFunctionVariable* var, indent_aware_stringbuilder& sb, int flags = kDefault) {
    if (var->isConst) sb += "const ";
    astVariableToString((astVariable*) var, sb);

    if (var->initialValue) {
        sb += " = ";
        astExpressionToString(var->initialValue, sb);
    }

    if (flags & kSemicolon) sb.append(";");
    if (flags & kNewLine) sb.appendLine();
}

inline void astDeclarationStatementToString(astDeclarationStatement* declStatement, indent_aware_stringbuilder& sb) {
    for (const auto& variable : declStatement->variables) {
        astFunctionVariableToString(variable, sb, kSemicolon);
    }
}

inline void astSwitchStatementToString(astSwitchStatement* switchStatement, indent_aware_stringbuilder& sb) {
    sb += "switch(";
    astExpressionToString(switchStatement->expression, sb);
    sb.appendLine(") {");
    sb.pushIndent();

    for (const auto& statement : switchStatement->statements) {
        astStatementToString(statement, sb);
        sb.appendLine();
    }

    sb.popIndent();
    sb.appendLine("}");
}

inline void astCaseLabelStatementToString(astCaseLabelStatement* caseLabelStatement, indent_aware_stringbuilder& sb) {
    if (caseLabelStatement->isDefault) {
        sb += "default";
    } else {
        sb += "case ";
        astExpressionToString(caseLabelStatement->condition, sb);
    }

    sb += (":");
    sb.pushIndent();
}

inline void astStatementToString(astStatement* statement, indent_aware_stringbuilder& sb) {
    switch (statement->type) {
        case STATEMENT(Empty):
            sb += ";";
        break;
        case STATEMENT(Continue):
            sb += "continue;";
            sb.popIndent();
        break;
        case STATEMENT(Break):
            sb += "break;";
            sb.popIndent();
        break;
        case STATEMENT(Discard):
            sb += "discard;";
            sb.popIndent();
        break;

        case STATEMENT(Declaration):
            astDeclarationStatementToString(reinterpret_cast<astDeclarationStatement*>(statement), sb);
        break;

        case STATEMENT(Expression):
            astExpressionToString(reinterpret_cast<astExpression*>(statement), sb);
        break;

        case STATEMENT(Switch):
            astSwitchStatementToString(reinterpret_cast<astSwitchStatement*>(statement), sb);
        break;

        case STATEMENT(CaseLabel):
            astCaseLabelStatementToString(reinterpret_cast<astCaseLabelStatement*>(statement), sb);
        break;
    }
}











converter::converter() : stringBuffer(indent_aware_stringbuilder()) { }

const char* converter::convertTU(astTU* translationUnit) {
    visitPreprocessors(translationUnit);
    visitStructures(translationUnit);
    visitInterfaceBlocks(translationUnit);
    visitGlobalVariables(translationUnit);
    visitFunctions(translationUnit);

    return stringBuffer.toString();
}

void converter::visitPreprocessors(astTU* tu) {
    if (tu->versionDirective) {
        stringBuffer += "#version ";
        stringBuffer += ntoa("%d", tu->versionDirective->version);
        stringBuffer += " ";
        stringBuffer += profileToString(tu->versionDirective->type);
        stringBuffer.appendLine();
    }

    for (const auto& extension : tu->extensionDirectives) {
        stringBuffer += "#extension ";
        stringBuffer += extension->name;
        stringBuffer += " : ";
        stringBuffer += extensionBehaviorToString(extension->behavior);
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
            astVariableToString(field, stringBuffer);
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
            astVariableToString(field, stringBuffer);
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
                    astExpressionToString(layoutQualifier->initialValue, stringBuffer);
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

        astVariableToString(reinterpret_cast<astVariable*>(global), stringBuffer);

        if (global->initialValue) {
            stringBuffer += " = ";
            astExpressionToString(global->initialValue, stringBuffer);
        }

        stringBuffer.appendLine(";");
    }
}

void converter::visitFunctions(astTU* tu) {
    for (const auto& function : tu->functions) {
        stringBuffer += typeToString(function->returnType);
        stringBuffer += " ";
        stringBuffer += function->name;

        stringBuffer += "(";
        if (!function->parameters.size()) {
            for (size_t i = 0; i < function->parameters.size(); ++i) {
                astFunctionParameter* parameter = function->parameters[i];
                astVariableToString(parameter, stringBuffer, true);
                if (i != function->parameters.size() - 1)
                    stringBuffer += ", ";
            }
        }
        stringBuffer += ")";

        if (function->isPrototype) {
            stringBuffer.appendLine(";");
            continue;
        }

        stringBuffer.appendLine(" {");
        stringBuffer.pushIndent();

        for (const auto& statement : function->statements) {
            astStatementToString(statement, stringBuffer);
            // stringBuffer.appendLine(statement->name());
            // stringBuffer.appendLine();
        }

        stringBuffer.popIndent();
        stringBuffer.appendLine("}");
        stringBuffer.appendLine();
    }
}

}
