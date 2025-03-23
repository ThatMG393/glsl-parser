#include "glsl-parser/converter.h"
#include "glsl-parser/ast.h"
#include "glsl-parser/lexer.h"
#include "glsl-parser/util.h"

#define EXPRC(type) astExpression::k##type##Constant
#define EXPRN(type) astExpression::k##type

namespace glsl {

inline indent_aware_stringbuilder astVariableToString(astVariable*, indent_aware_stringbuilder&, bool);
inline const char* astExpressionToString(astExpression*, indent_aware_stringbuilder&);
inline void astFunctionParameterToString(astFunctionParameter*, indent_aware_stringbuilder&);
inline void astStatementToString(astStatement*, indent_aware_stringbuilder&);
inline void astFunctionToString(astFunction*, indent_aware_stringbuilder&);

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

inline const char* precisionToString(int precision) {
    switch (precision) {
        case kLowp: return "lowp";
        case kMediump: return "mediump";
        case kHighp: return "highp";
        default: return "";
    }
}

inline const char* interpolationToString(int interpolation) {
    switch (interpolation) {
        case kSmooth: return "smooth";
        case kFlat: return "flat";
        case kNoPerspective: return "noperspective";
        default: return "";
    }
}

inline const char* operatorToString(int op) {
    static const char* operators[] = {
        "+", "-", "*", "/", "%", 
        "<<", ">>", "<", ">", "<=", ">=", 
        "==", "!=", "&", "^", "|", 
        "&&", "||", 
        "=", "+=", "-=", "*=", "/=", "%=", 
        "<<=", ">>=", "&=", "^=", "|="
    };
    return operators[op];
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
    } else {
        astStruct* structure = static_cast<astStruct*>(type);
        return structure->name;
    }
}

inline void printArraySize(const vector<astConstantExpression*>& arraySizes, indent_aware_stringbuilder& sb) {
    for (const auto& arraySize : arraySizes) {
        sb += "[";
        sb += astExpressionToString(arraySize, sb);
        sb += "]";
    }
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
    indent_aware_stringbuilder sb;
    if (memory & kCoherent) sb += "coherent ";
    if (memory & kVolatile) sb += "volatile ";
    if (memory & kRestrict) sb += "restrict ";
    if (memory & kReadOnly) sb += "readonly ";
    if (memory & kWriteOnly) sb += "writeonly ";
    return sb.toString();
}

inline indent_aware_stringbuilder astVariableToString(astVariable* var, indent_aware_stringbuilder& prevSb, bool nameOnly = false) {
    indent_aware_stringbuilder sb;
    sb.copyIndent(prevSb);

    if (var->isPrecise) sb += "precise ";
    if (nameOnly) {
        sb += var->name;
        return sb;
    }

    sb += typeToString(var->baseType);
    sb += " ";
    sb += var->name;

    if (var->isArray) {
        printArraySize(var->arraySizes, sb);
    }

    return sb;
}

inline const char* astExpressionToString(astExpression* expression, indent_aware_stringbuilder& sb) {
    indent_aware_stringbuilder result;
    result.copyIndent(sb);

    switch (expression->type) {
        case EXPRC(Int): {
            return ntoa("%d", static_cast<astIntConstant*>(expression)->value);
        }
        case EXPRC(UInt): {
            return ntoa("%du", static_cast<astUIntConstant*>(expression)->value);
        }
        case EXPRC(Float): {
            float value = static_cast<astFloatConstant*>(expression)->value;
            char format[32];
            snprintf(format, sizeof(format), "%g", value);
            if (!strchr(format, '.'))
                return ntoa("%g.0", value);
            return ntoa("%g", value);
        }
        case EXPRC(Double): {
            return ntoa("%g", static_cast<astDoubleConstant*>(expression)->value);
        }
        case EXPRC(Bool): {
            return static_cast<astBoolConstant*>(expression)->value ? "true" : "false";
        }
        case EXPRN(VariableIdentifier): {
            astVariableIdentifier* varId = static_cast<astVariableIdentifier*>(expression);
            return astVariableToString(varId->variable, sb, true).toString();
        }
        case EXPRN(FieldOrSwizzle): {
            astFieldOrSwizzle* field = static_cast<astFieldOrSwizzle*>(expression);
            result += astExpressionToString(field->operand, sb);
            result += ".";
            result += field->name;
            return result.toString();
        }
        case EXPRN(ArraySubscript): {
            astArraySubscript* subscript = static_cast<astArraySubscript*>(expression);
            result += astExpressionToString(subscript->operand, sb);
            result += "[";
            result += astExpressionToString(subscript->index, sb);
            result += "]";
            return result.toString();
        }
        case EXPRN(FunctionCall): {
            astFunctionCall* call = static_cast<astFunctionCall*>(expression);
            result += call->name;
            result += "(";
            for (size_t i = 0; i < call->parameters.size(); i++) {
                result += astExpressionToString(call->parameters[i], sb);
                if (i != call->parameters.size() - 1)
                    result += ", ";
            }
            result += ")";
            return result.toString();
        }
        case EXPRN(ConstructorCall): {
            astConstructorCall* call = static_cast<astConstructorCall*>(expression);
            result += typeToString(call->type);
            result += "(";
            for (size_t i = 0; i < call->parameters.size(); i++) {
                result += astExpressionToString(call->parameters[i], sb);
                if (i != call->parameters.size() - 1)
                    result += ", ";
            }
            result += ")";
            return result.toString();
        }
        case EXPRN(PostIncrement): {
            astPostIncrementExpression* inc = static_cast<astPostIncrementExpression*>(expression);
            result += astExpressionToString(inc->operand, sb);
            result += "++";
            return result.toString();
        }
        case EXPRN(PostDecrement): {
            astPostDecrementExpression* dec = static_cast<astPostDecrementExpression*>(expression);
            result += astExpressionToString(dec->operand, sb);
            result += "--";
            return result.toString();
        }
        case EXPRN(UnaryMinus): {
            astUnaryMinusExpression* minus = static_cast<astUnaryMinusExpression*>(expression);
            result += "-";
            result += astExpressionToString(minus->operand, sb);
            return result.toString();
        }
        case EXPRN(UnaryPlus): {
            astUnaryPlusExpression* plus = static_cast<astUnaryPlusExpression*>(expression);
            result += "+";
            result += astExpressionToString(plus->operand, sb);
            return result.toString();
        }
        case EXPRN(BitNot): {
            astUnaryBitNotExpression* bitNot = static_cast<astUnaryBitNotExpression*>(expression);
            result += "~";
            result += astExpressionToString(bitNot->operand, sb);
            return result.toString();
        }
        case EXPRN(LogicalNot): {
            astUnaryLogicalNotExpression* logicalNot = static_cast<astUnaryLogicalNotExpression*>(expression);
            result += "!";
            result += astExpressionToString(logicalNot->operand, sb);
            return result.toString();
        }
        case EXPRN(PrefixIncrement): {
            astPrefixIncrementExpression* inc = static_cast<astPrefixIncrementExpression*>(expression);
            result += "++";
            result += astExpressionToString(inc->operand, sb);
            return result.toString();
        }
        case EXPRN(PrefixDecrement): {
            astPrefixDecrementExpression* dec = static_cast<astPrefixDecrementExpression*>(expression);
            result += "--";
            result += astExpressionToString(dec->operand, sb);
            return result.toString();
        }
        case EXPRN(Assign): {
            astAssignmentExpression* assign = static_cast<astAssignmentExpression*>(expression);
            result += "(";
            result += astExpressionToString(assign->operand1, sb);
            result += " ";
            result += operatorToString(assign->assignment);
            result += " ";
            result += astExpressionToString(assign->operand2, sb);
            result += ")";
            return result.toString();
        }
        case EXPRN(Sequence): {
            astSequenceExpression* seq = static_cast<astSequenceExpression*>(expression);
            result += "(";
            result += astExpressionToString(seq->operand1, sb);
            result += ", ";
            result += astExpressionToString(seq->operand2, sb);
            result += ")";
            return result.toString();
        }
        case EXPRN(Operation): {
            astOperationExpression* op = static_cast<astOperationExpression*>(expression);
            result += "(";
            result += astExpressionToString(op->operand1, sb);
            result += " ";
            result += operatorToString(op->operation);
            result += " ";
            result += astExpressionToString(op->operand2, sb);
            result += ")";
            return result.toString();
        }
        case EXPRN(Ternary): {
            astTernaryExpression* ternary = static_cast<astTernaryExpression*>(expression);
            result += "(";
            result += astExpressionToString(ternary->condition, sb);
            result += " ? ";
            result += astExpressionToString(ternary->onTrue, sb);
            result += " : ";
            result += astExpressionToString(ternary->onFalse, sb);
            result += ")";
            return result.toString();
        }
        default:
            return "";
    }
}

inline void astFunctionVariableToString(astFunctionVariable* variable, indent_aware_stringbuilder& sb, bool semi = true, bool newLine = true) {
    sb.copyIndent(sb);
    
    if (variable->isConst)
        sb += "const ";
    
    sb.append(astVariableToString(static_cast<astVariable*>(variable), sb));
    
    if (variable->initialValue) {
        sb += " = ";
        sb += astExpressionToString(variable->initialValue, sb);
    }
    
    if (semi) sb += ";";
    if (newLine) sb.appendLine();
}

inline void astDeclarationStatementToString(astDeclarationStatement* statement, indent_aware_stringbuilder& sb, bool semi = true, bool newLine = true) {
    for (size_t i = 0; i < statement->variables.size(); i++) {
       astFunctionVariableToString(statement->variables[i], sb, semi, newLine);
    }
}

inline void astExpressionStatementToString(astExpressionStatement* statement, indent_aware_stringbuilder& sb, bool semi = true, bool newLine = true) {
    sb += astExpressionToString(statement->expression, sb);
    if (semi) sb += ";";
    if (newLine) sb.appendLine();
}

inline void astCompoundStatementToString(astCompoundStatement* statement, indent_aware_stringbuilder& sb) {
    sb.appendLine(" {");
    sb.pushIndent();
    
    for (size_t i = 0; i < statement->statements.size(); i++) {
        astStatementToString(statement->statements[i], sb);
    }
    
    sb.popIndent();
    sb.appendLine("}");
}

inline void astIfStatementToString(astIfStatement* statement, indent_aware_stringbuilder& sb) {
    sb += "if(";
    sb += astExpressionToString(statement->condition, sb);
    sb += ")";
    
    astStatementToString(statement->thenStatement, sb);
    
    if (statement->elseStatement) {
        sb += "else";
        if (statement->elseStatement->type == astStatement::kIf)
            sb += " ";
        astStatementToString(statement->elseStatement, sb);
    }
}

inline void astSwitchStatementToString(astSwitchStatement* statement, indent_aware_stringbuilder& sb) {
    sb += "switch(";
    sb += astExpressionToString(statement->expression, sb);
    sb.appendLine(") {");
    
    sb.pushIndent();
    for (size_t i = 0; i < statement->statements.size(); i++) {
        astStatementToString(statement->statements[i], sb);
    }
    sb.popIndent();
    
    sb.appendLine("}");
}

inline void astCaseLabelStatementToString(astCaseLabelStatement* statement, indent_aware_stringbuilder& sb) {
    if (statement->isDefault)
        sb += "default";
    else {
        sb += "case ";
        sb += astExpressionToString(statement->condition, sb);
    }
    sb.appendLine(":");
}

inline void astWhileStatementToString(astWhileStatement* statement, indent_aware_stringbuilder& sb) {
    sb += "while(";
    
    switch (statement->condition->type) {
        case astStatement::kDeclaration:
            astDeclarationStatementToString(static_cast<astDeclarationStatement*>(statement->condition), sb, false, false);
            break;
        case astStatement::kExpression:
            astExpressionStatementToString(static_cast<astExpressionStatement*>(statement->condition), sb, false, false);
            break;
    }
    
    sb += ")";
    astStatementToString(statement->body, sb);
}

inline void astDoStatementToString(astDoStatement* statement, indent_aware_stringbuilder& sb) {
    sb += "do";
    
    if (statement->body->type != astStatement::kCompound)
        sb += " ";
        
    astStatementToString(statement->body, sb);
    sb += "while(";
    sb += astExpressionToString(statement->condition, sb);
    sb.appendLine(");");
}

inline void astForStatementToString(astForStatement* statement, indent_aware_stringbuilder& sb) {
    sb += "for(";
    
    if (statement->init) {
        switch (statement->init->type) {
            case astStatement::kDeclaration:
                astDeclarationStatementToString(static_cast<astDeclarationStatement*>(statement->init), sb, true, false);
                break;
            case astStatement::kExpression:
                astExpressionStatementToString(static_cast<astExpressionStatement*>(statement->init), sb, true, false);
                break;
        }
    } else {
        sb += ";";
    }
    
    if (statement->condition) {
        sb += " ";
        sb += astExpressionToString(statement->condition, sb);
    }
    
    sb += ";";
    
    if (statement->loop) {
        sb += " ";
        sb += astExpressionToString(statement->loop, sb);
    }
    
    sb += ")";
    astStatementToString(statement->body, sb);
}

inline void astReturnStatementToString(astReturnStatement* statement, indent_aware_stringbuilder& sb) {
    if (statement->expression) {
        sb += "return ";
        sb += astExpressionToString(statement->expression, sb);
        sb.appendLine(";");
    } else {
        sb.appendLine("return;");
    }
}

inline void astStatementToString(astStatement* statement, indent_aware_stringbuilder& sb) {
    switch (statement->type) {
        case astStatement::kCompound:
            astCompoundStatementToString(static_cast<astCompoundStatement*>(statement), sb);
            break;
        case astStatement::kEmpty:
            sb.appendLine(";");
            break;
        case astStatement::kDeclaration:
            astDeclarationStatementToString(static_cast<astDeclarationStatement*>(statement), sb);
            break;
        case astStatement::kExpression:
            astExpressionStatementToString(static_cast<astExpressionStatement*>(statement), sb);
            break;
        case astStatement::kIf:
            astIfStatementToString(static_cast<astIfStatement*>(statement), sb);
            break;
        case astStatement::kSwitch:
            astSwitchStatementToString(static_cast<astSwitchStatement*>(statement), sb);
            break;
        case astStatement::kCaseLabel:
            astCaseLabelStatementToString(static_cast<astCaseLabelStatement*>(statement), sb);
            break;
        case astStatement::kWhile:
            astWhileStatementToString(static_cast<astWhileStatement*>(statement), sb);
            break;
        case astStatement::kDo:
            astDoStatementToString(static_cast<astDoStatement*>(statement), sb);
            break;
        case astStatement::kFor:
            astForStatementToString(static_cast<astForStatement*>(statement), sb);
            break;
        case astStatement::kContinue:
            sb.appendLine("continue;");
            break;
        case astStatement::kBreak:
            sb.appendLine("break;");
            break;
        case astStatement::kReturn:
            astReturnStatementToString(static_cast<astReturnStatement*>(statement), sb);
            break;
        case astStatement::kDiscard:
            sb.appendLine("discard;");
            break;
        default:
            sb.appendLine();
            break;
    }
}

inline void astFunctionParameterToString(astFunctionParameter* parameter, indent_aware_stringbuilder& sb) {
    if (parameter->storage != -1) {
        sb += storageToString(parameter->storage);
        sb += " ";
    }
    
    if (parameter->auxiliary != -1) {
        sb += auxiliaryToString(parameter->auxiliary);
        sb += " ";
    }
    
    if (parameter->memory != 0) {
        sb += memoryToString(parameter->memory);
    }
    
    if (parameter->precision != -1) {
        sb += precisionToString(parameter->precision);
        sb += " ";
    }
    
    sb += typeToString(parameter->baseType);
    
    if (parameter->name) {
        sb += " ";
        sb += parameter->name;
    }
    
    if (parameter->isArray) {
        printArraySize(parameter->arraySizes, sb);
    }
}

inline void astFunctionToString(astFunction* function, indent_aware_stringbuilder& sb) {
    sb += typeToString(function->returnType);
    sb += " ";
    sb += function->name;
    sb += "(";
    
    for (size_t i = 0; i < function->parameters.size(); i++) {
        astFunctionParameterToString(function->parameters[i], sb);
        if (i != function->parameters.size() - 1)
            sb += ", ";
    }
    
    sb += ")";
    
    if (function->isPrototype) {
        sb.appendLine(";");
        return;
    }
    
    sb.appendLine(" {");
    sb.pushIndent();
    
    for (size_t i = 0; i < function->statements.size(); i++) {
        astStatementToString(function->statements[i], sb);
    }
    
    sb.popIndent();
    sb.appendLine("}");
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
                    stringBuffer += astExpressionToString(layoutQualifier->initialValue, stringBuffer);
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

        if (global->memory) {
            stringBuffer += memoryToString(global->memory);
            stringBuffer += " ";
        }

        if (global->precision != -1) {
            stringBuffer += precisionToString(global->precision);
            stringBuffer += " ";
        }

        if (global->isInvariant) stringBuffer += "invariant ";
        if (global->interpolation != -1) {
            stringBuffer += interpolationToString(global->interpolation);
            stringBuffer += " ";
        }

        stringBuffer.append(astVariableToString(reinterpret_cast<astVariable*>(global), stringBuffer));

        if (global->initialValue) {
            stringBuffer += " = ";
            stringBuffer += astExpressionToString(global->initialValue, stringBuffer);
        }

        stringBuffer.appendLine(";");
    }
}

void converter::visitFunctions(astTU* tu) {
    for (const auto& function : tu->functions) {
        astFunctionToString(function,stringBuffer);
        stringBuffer.appendLine();
    }
}

}