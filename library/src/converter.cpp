#include "glsl-parser/converter.h"
#include "glsl-parser/ast.h"
#include "glsl-parser/lexer.h"
#include "glsl-parser/util.h"
#include <cstring>

#define EXPRC(type) astExpression::k##type##Constant
#define EXPRN(type) astExpression::k##type
#define STATEMENT(type) astStatement::k##type

namespace glsl {

#undef KEYWORD
#define KEYWORD(X) #X,
static const char* builtinKeywordMap[] = {
    #include "glsl-parser/lexemes.h"
};
#undef KEYWORD
#define KEYWORD(...)

#undef OPERATOR
#define OPERATOR(N, S, P) S,
static const char* operatorMap[] = {
    #include "glsl-parser/lexemes.h"
};
#undef OPERATOR
#define OPERATOR(...)

enum {
    kSemicolon = 1 << 0,
    kNewLine = 1 << 1,
    kPopIndent = 1 << 2,
    kCompoundChain = 1 << 3,
    kDefault = kSemicolon | kNewLine | kPopIndent
};

inline void astExpressionToString(astExpression*, indent_aware_stringbuilder&);
inline void astVariableToString(astVariable*, indent_aware_stringbuilder&, bool);
inline void astStatementToString(astStatement*, indent_aware_stringbuilder&, int = kDefault);

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

inline const char* astTypeToString(astType* type) {
    if (type->builtin) {
        astBuiltin* builtin = static_cast<astBuiltin*>(type);
        return builtinKeywordMap[builtin->type];
    } else if (astStruct* structure = reinterpret_cast<astStruct*>(type)) {
        return structure->name;
    }

    return "unknown_type";
}

inline void expandParameters(vector<astExpression*> parameters, indent_aware_stringbuilder& sb) {
    sb += "(";
    if (!parameters.size()) {
        for (size_t i = 0; i < parameters.size(); ++i) {
            astExpression* parameterExpression = parameters[i];
            astExpressionToString(parameterExpression, sb);
            if (i != parameters.size() - 1)
                sb += ", ";
        }
    }
    sb += ")";
}

inline void incrementExpression(astUnaryExpression* expr, indent_aware_stringbuilder& sb, bool post) {
    if (!post) sb += operatorMap[5];
    astExpressionToString(expr->operand, sb);
    if (post) sb += operatorMap[5];
}


inline void decrementExpression(astUnaryExpression* expr, indent_aware_stringbuilder& sb, bool post) {
    if (!post) sb += operatorMap[6];
    astExpressionToString(expr->operand, sb);
    if (post) sb += operatorMap[6];
}

inline void astExpressionToString(astExpression* expression, indent_aware_stringbuilder& sb) {
    switch (expression->type) {
        case EXPRC(Int):
            sb += ntoa("%i", reinterpret_cast<astIntConstant*>(expression)->value);
        break;
        
        case EXPRC(UInt):
            sb += ntoa("%u", reinterpret_cast<astUIntConstant*>(expression)->value);
        break;
        
        case EXPRC(Float):
        {
            astFloatConstant* floatExpression = reinterpret_cast<astFloatConstant*>(expression);
            char format[1024];
            snprintf(format, sizeof format, "%g", floatExpression->value);
            if (!strchr(format, '.'))
                sb += ntoa("%g.0", floatExpression->value);
            else
                sb += ntoa("%f", format);
        }
        break;

        case EXPRC(Double):
            sb += ntoa("%g", reinterpret_cast<astDoubleConstant*>(expression)->value);
        break;

        case EXPRC(Bool):
            sb += reinterpret_cast<astBoolConstant*>(expression)->value ? "true" : "false";
        break;
        
        case EXPRN(VariableIdentifier):
            astVariableToString(reinterpret_cast<astVariableIdentifier*>(expression)->variable, sb, true);
        break;

        case EXPRN(FieldOrSwizzle):
        {
            astFieldOrSwizzle* fieldOrSwizzleExpression = reinterpret_cast<astFieldOrSwizzle*>(expression);
            astExpressionToString(fieldOrSwizzleExpression->operand, sb);
            sb += ".";
            sb += fieldOrSwizzleExpression->name;
        }
        break;

        case EXPRN(ArraySubscript):
        {
            astArraySubscript* arraySubscriptExpression = reinterpret_cast<astArraySubscript*>(expression);
            astExpressionToString(arraySubscriptExpression->operand, sb);
            sb += "[";
            astExpressionToString(arraySubscriptExpression->index, sb);
            sb += "]";
        }
        break;

        case EXPRN(FunctionCall):
        {
            astFunctionCall* functionCallExpression = reinterpret_cast<astFunctionCall*>(expression);
            sb += functionCallExpression->name;
            expandParameters(functionCallExpression->parameters, sb);
        }
        break;

        case EXPRN(ConstructorCall):
        {
            astConstructorCall* constructorCallExpression = reinterpret_cast<astConstructorCall*>(expression);
            sb += astTypeToString(constructorCallExpression->type);
            expandParameters(constructorCallExpression->parameters, sb);
        }
        break;

        case EXPRN(PostIncrement):
            incrementExpression(
                reinterpret_cast<astPostIncrementExpression*>(expression), 
                sb, true
            );
        break;

        case EXPRN(PostDecrement):
            decrementExpression(
                reinterpret_cast<astPostDecrementExpression*>(expression), 
                sb, true
            );
        break;

        case EXPRN(UnaryMinus):
            sb += operatorMap[13];
            astExpressionToString(reinterpret_cast<astUnaryMinusExpression*>(expression)->operand, sb);
        break;

        case EXPRN(UnaryPlus):
            sb += operatorMap[12];
            astExpressionToString(reinterpret_cast<astUnaryPlusExpression*>(expression)->operand, sb);
        break;

        case EXPRN(BitNot):
            sb += operatorMap[7];
            astExpressionToString(reinterpret_cast<astUnaryBitNotExpression*>(expression)->operand, sb);
        break;

        case EXPRN(LogicalNot):
            sb += operatorMap[8];
            astExpressionToString(reinterpret_cast<astUnaryLogicalNotExpression*>(expression)->operand, sb);
        break;

        case EXPRN(PrefixIncrement):
            incrementExpression(
                reinterpret_cast<astPrefixIncrementExpression*>(expression), 
                sb, false
            );
        break;

        case EXPRN(PrefixDecrement):
            decrementExpression(
                reinterpret_cast<astPrefixDecrementExpression*>(expression), 
                sb, false
            );
        break;

        case EXPRN(Assign):
        {
            astAssignmentExpression* assignmentExpression = reinterpret_cast<astAssignmentExpression*>(expression);
            astExpressionToString(assignmentExpression->operand1, sb);
            sb += " ";
            sb += operatorMap[assignmentExpression->assignment];
            sb += " ";
            astExpressionToString(assignmentExpression->operand2, sb);
        }
        break;

        case EXPRN(Sequence):
        {
            astSequenceExpression* sequenceExpression = reinterpret_cast<astSequenceExpression*>(expression);
            sb += "(";
            astExpressionToString(sequenceExpression->operand1, sb);
            sb += ", ";
            astExpressionToString(sequenceExpression->operand2, sb);
            sb += ")";
        }
        break;

        case EXPRN(Operation):
        {
        	astOperationExpression* operationExpression = reinterpret_cast<astOperationExpression*>(expression);
        	astExpressionToString(operationExpression->operand1, sb);
            sb += " ";
            sb += operatorMap[operationExpression->operation];
            sb += " ";
            astExpressionToString(operationExpression->operand2, sb);
        }
        break;

        case EXPRN(Ternary):
        {
            astTernaryExpression* ternaryExpression = reinterpret_cast<astTernaryExpression*>(expression);
            sb += "(";
            astExpressionToString(ternaryExpression->condition, sb);
            sb += " ? ";
            astExpressionToString(ternaryExpression->onTrue, sb);
            sb += " : ";
            astExpressionToString(ternaryExpression->onFalse, sb);
            sb += ")";
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

    sb += astTypeToString(var->baseType);
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

inline void astSwitchStatementToString(astSwitchStatement* switchStatement, indent_aware_stringbuilder& sb) {
    sb += "switch (";
    astExpressionToString(switchStatement->expression, sb);
    sb.appendLine(") {");
    sb.pushIndent();

    for (const auto& statement : switchStatement->statements) {
        astStatementToString(statement, sb, kSemicolon);
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

inline void astExpressionStatementToString(astExpressionStatement* exprStatement, indent_aware_stringbuilder& sb, int flags = kDefault) {
    astExpressionToString(exprStatement->expression, sb);
    
    if (flags & kSemicolon) sb += ";";
    if (flags & kNewLine) sb.appendLine();
}

inline void astWhileStatementToString(astWhileStatement* whileStatement, indent_aware_stringbuilder& sb) {
    sb += "while (";
    astStatementToString(whileStatement->condition, sb, false);
    sb += ") ";
    astStatementToString(whileStatement->body, sb);
}

inline void astDoStatementToString(astDoStatement* doStatement, indent_aware_stringbuilder& sb) {
    sb += "do ";
    astStatementToString(doStatement->body, sb, kSemicolon);
    if (doStatement->body->type != STATEMENT(Compound)) sb += " ";
    sb += "while (";
    astExpressionToString(doStatement->condition, sb);
    sb.appendLine(");");
}

inline void astForStatementToString(astForStatement* forStatement, indent_aware_stringbuilder& sb) {
    sb += "for (";

    if (forStatement->init) { // for (<statement>)
        astStatementToString(forStatement->init, sb, false);
    } else {
        sb += ";;"; // for (;;)
    }

    if (forStatement->condition) { // for (<statement>[;]<condition>)
        sb += "; ";
        astExpressionToString(forStatement->condition, sb);
    }

    if (forStatement->loop) { // for (<statement>[;]<condition>[;]<loop>)
        sb += "; ";
        astExpressionToString(forStatement->loop, sb);
    }
    sb += ") ";
    astStatementToString(forStatement->body, sb);
}

inline void astIfStatementToString(astIfStatement* ifStatement, indent_aware_stringbuilder& sb) {
	sb += "if (";
	astExpressionToString(ifStatement->condition, sb);
	sb += ") ";
	astStatementToString(ifStatement->thenStatement, sb, kSemicolon | kNewLine | kCompoundChain);
	if (ifStatement->thenStatement->type != STATEMENT(Compound)) sb.appendLine();

	if (ifStatement->elseStatement) {
		sb += " else ";
		if (ifStatement->elseStatement->type == STATEMENT(If)) sb += " ";
		astStatementToString(ifStatement->elseStatement, sb);
	}
}

inline void astStatementToString(astStatement* statement, indent_aware_stringbuilder& sb, int flags) {
    switch (statement->type) {
        case STATEMENT(Empty):
            sb += ";";
        break;
        case STATEMENT(Continue):
            sb += "continue;";
            if (flags & kPopIndent) sb.popIndent();
        break;
        case STATEMENT(Break):
            sb += "break;";
            if (flags & kPopIndent) sb.popIndent();
        break;
        case STATEMENT(Discard):
            sb += "discard;";
            if (flags & kPopIndent) sb.popIndent();
        break;

        case STATEMENT(Compound):
        {
            astCompoundStatement* compoundStatements = reinterpret_cast<astCompoundStatement*>(statement);

            if (!compoundStatements->statements.size()) {
                sb.append("{");
                sb.appendLine(" }");
            } else {
                sb.appendLine("{");
                sb.pushIndent();

                for (const auto& compoundStatement : compoundStatements->statements) {
                    astStatementToString(compoundStatement, sb, flags);
                }

                sb.popIndent();
                if (flags & kCompoundChain) sb += "}";
                else sb.appendLine("}");
            }
        }
        break;

        case STATEMENT(Declaration):
            for (const auto& variable : reinterpret_cast<astDeclarationStatement*>(statement)->variables) {
                astFunctionVariableToString(variable, sb, flags);
            }
        break;

        case STATEMENT(Expression):
            astExpressionStatementToString(reinterpret_cast<astExpressionStatement*>(statement), sb, flags);
        break;

        case STATEMENT(Switch):
            astSwitchStatementToString(reinterpret_cast<astSwitchStatement*>(statement), sb);
        break;

        case STATEMENT(CaseLabel):
            astCaseLabelStatementToString(reinterpret_cast<astCaseLabelStatement*>(statement), sb);
        break;

        case STATEMENT(While):
            astWhileStatementToString(reinterpret_cast<astWhileStatement*>(statement), sb);
        break;

        case STATEMENT(Do):
            astDoStatementToString(reinterpret_cast<astDoStatement*>(statement), sb);
        break;

        case STATEMENT(For):
            astForStatementToString(reinterpret_cast<astForStatement*>(statement), sb);
		break;

        case STATEMENT(If):
        	astIfStatementToString(reinterpret_cast<astIfStatement*>(statement), sb);
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
        stringBuffer += astTypeToString(function->returnType);
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
        }

        stringBuffer.popIndent();
        stringBuffer.appendLine("}");
        stringBuffer.appendLine();
    }
}

}
