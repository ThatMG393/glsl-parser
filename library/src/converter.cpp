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
        default: return "core";
    }
}

inline const char* getExtensionBehavior(int behavior) {
    switch (behavior) {
        case kEnable: return "enable";
        case kRequire: return "require";
        case kWarn: return "warn";
        case kDisable: return "disable";
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
    }
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
    
}

void converter::visitGlobals(astTU* tu) {
    for (const auto& global : tu->globals) {
        stringBuffer += storageToString(global->storage);
    }
}

}