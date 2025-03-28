#include <string.h> // memcpy, strlen
#include <stdlib.h> // malloc, free
#include <limits.h> // INT_MAX, UINT_MAX

#include "glsl-parser/lexer.h"

namespace glsl {

// Lookup table of keywords
#undef KEYWORD
#define KEYWORD(X) { #X, kKeyword_##X },
static const keywordInfo kKeywords[] = {
    #include "glsl-parser/lexemes.h"
};
#undef KEYWORD
#define KEYWORD(...)

// Lookup table of operators
#undef OPERATOR
#define OPERATOR(X, S, PREC) { #X, S, PREC },
static const operatorInfo kOperators[] = {
    #include "glsl-parser/lexemes.h"
};
#undef OPERATOR
#define OPERATOR(...)

token::token()
    : m_type(0)
{
    asDouble = 0.0;
}

int token::precedence() const {
    if (m_type == kType_operator)
        return kOperators[asOperator].precedence;
    return -1;
}

/// location
location::location()
    : column(1)
    , line(1)
    , position(0)
{
}

void location::advanceColumn(size_t count) {
    column += count;
    position += count;
}

void location::advanceLine() {
    line++;
    position++;
    column = 1;
}

static inline bool isDigit(int ch) {
    return unsigned(ch) - '0' < 10;
}

static inline bool isChar(int ch) {
    return (unsigned(ch) | 32) - 'a' < 26;
}

static inline bool isOctal(int ch) {
    return unsigned(ch) - '0' < 8;
}

static inline bool isHex(int ch) {
    return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || isDigit(ch);
}

static inline bool isSpace(int ch) {
    return (ch >= '\t' && ch <= '\r') || ch == ' ';
}

lexer::lexer(const char *string)
    : m_data(string)
    , m_length(0)
    , m_error(0)
{
    if (m_data)
        m_length = strlen(m_data);
}

int lexer::at(int offset) const {
    if (position() + offset < m_length)
        return m_data[position() + offset];
    return 0;
}

void lexer::skipWhitespace(bool allowNewlines) {
    while (position() < m_length && isSpace(at())) {
        if (at() == '\n') {
            if (allowNewlines) {
                m_location.advanceLine();
            } else {
                break;
            }
        } else {
            m_location.advanceColumn();
        }
    }
}

void lexer::read(token &out) {
    // Any previous identifier must be freed
    if (out.m_type == kType_identifier)
        free(out.asIdentifier);
    else if (out.m_type == kType_directive && out.asDirective.type == directive::kExtension)
        free(out.asDirective.asExtension.name);

    // TODO: Line continuation (backslash `\'.)
    if (position() == m_length) {
        out.m_type = kType_eof;
        return;
    }

    int ch1 = at(1);
    int ch2 = at(2);

    // Lex numerics
    if (isDigit(at()) || (at() == '.' && isDigit(ch1)))
    {
        bool isFloat = false;
        bool isDouble = false;
        bool isUnsigned = false;
        bool isOctalish = false;
        bool isHexish = false;

        if (at() == '0') {
            if (ch1 && (ch1 == 'x' || ch1 == 'X')) {
                isHexish = true;
                m_location.advanceColumn(2);
            } else {
                isOctalish = true;
            }
        }

        vector<char> numeric = readNumeric(isOctalish, isHexish);
        if (position() != m_length && at() == '.') {
            isFloat = true;
            isOctalish = false;
            numeric.push_back('.');
            m_location.advanceColumn();
            vector<char> others = readNumeric(isOctalish, isHexish);
            numeric.reserve(numeric.size() + others.size());
            numeric.insert(numeric.end(), others.begin(), others.end());
        }

        if (position() != m_length && (at() == 'e' || at() == 'E')) {
            ch1 = at(1);
            ch2 = at(2);
            if ((ch1 == '+' || ch1 == '-') && (ch2 >= '0' && ch2 <= '9')) {
                numeric.push_back(ch1);
                numeric.push_back(ch2);
                m_location.advanceColumn(2);
                vector<char> others = readNumeric(isOctalish, isHexish);
                numeric.reserve(numeric.size() + others.size());
                numeric.insert(numeric.end(), others.begin(), others.end());
                isFloat = true;
                isOctalish = false;
            } else {
                m_error = "invalid numeric literal";
                return;
            }
        }

        if (position() != m_length && isChar(at())) {
            ch1 = at(1);
            if (at() == 'f' || at() == 'F') {
                isFloat = true;
                isOctalish = false;
            } else if ((at() == 'l' && ch1 == 'f') || (at() == 'L' && ch1 == 'F')) {
                isFloat = false;
                isDouble = true;
                isOctalish = false;
                m_location.advanceColumn();
            } else if (at() == 'u' || at() == 'U') {
                if (isFloat) {
                    m_error = "invalid use of suffix on literal";
                    return;
                }
                isUnsigned = true;
            } else {
                m_error = "invalid numeric literal";
                return;
            }
            m_location.advanceColumn();
        }

        if (isHexish && (isFloat || isDouble)) {
            m_error = "invalid numeric literal";
            return;
        }

        numeric.push_back('\0');
        int base = isHexish ? 16 : (isOctalish ? 8 : 10);
        char *error;
        if (isFloat) {
            out.m_type = kType_constant_float;
            out.asFloat = strtof(&numeric[0], &error);
            if (error == &numeric[0]) {
                m_error = "invalid numeric literal";
                return;
            }
        } else if (isDouble) {
            out.m_type = kType_constant_double;
            out.asDouble = strtod(&numeric[0], &error);
            if (error == &numeric[0]) {
                m_error = "invalid numeric literal";
                return;
            }
        } else if (isUnsigned) {
            out.m_type = kType_constant_uint;
            unsigned long long value = strtoull(&numeric[0], 0, base);
            if (value <= UINT_MAX) {
                out.asUnsigned = (unsigned int)value;
            } else {
                m_error = "literal needs more than 32-bits";
            }
        } else {
            out.m_type = kType_constant_int;
            long long value = strtoll(&numeric[0], 0, base);
            if (value <= INT_MAX) {
                out.asInt = (int)value;
            } else {
                m_error = "literal needs more than 32-bits";
            }
        }
    } else if (isChar(at()) || at() == '_') {
        // Identifiers
        out.m_type = kType_identifier;
        vector<char> identifier;
        while (position() != m_length && (isChar(at()) || isDigit(at()) || at() == '_')) {
            identifier.push_back(at());
            m_location.advanceColumn();
        }
        identifier.push_back('\0');
        out.asIdentifier = (char *)malloc(identifier.size());
        if (!out.asIdentifier) {
            m_error = "Out of memory";
            return;
        }
        memcpy(out.asIdentifier, &identifier[0], identifier.size());

        // Or is it a keyword?
        for (size_t i = 0; i < sizeof(kKeywords)/sizeof(kKeywords[0]); i++) {
            if (strcmp(kKeywords[i].name, out.asIdentifier))
                continue;
            free(out.asIdentifier);
            out.asIdentifier = 0;
            out.m_type = kType_keyword;
            out.asKeyword = int(i);
            break;
        }
    } else if (at() == '#') {
        m_location.advanceColumn(); // Skip '#'.

        vector<char> chars;
        while (isChar(at())) {
            chars.push_back(at());
            m_location.advanceColumn();
        }

        // Directive should immediately proceed the # token.
        if (chars.empty()) {
            m_error = "Expected directive";
            return;
        }

        if (!strcmp(&chars[0], "version")) {
            out.asDirective.type = directive::kVersion;

            // version [0-9]+ (core|compatibility|es)?
            skipWhitespace(false);

            vector<char> digits = readNumeric(false, false);
            if (digits.empty()) {
                m_error = "Expected version number in #version directive";
                return;
            }
            digits.push_back('\0');

            long long value = strtoll(&digits[0], 0, 10);
            out.asDirective.asVersion.version = value;
            out.asDirective.asVersion.type = kCore;

            skipWhitespace(false);

            vector<char> chars;
            while (isChar(at())) {
                chars.push_back(at());
                m_location.advanceColumn();
            }

            if (!chars.empty()) {
                chars.push_back('\0');
                if (!strcmp(&chars[0], "core")) {
                    // Do nothing, already core.
                } else if (!strcmp(&chars[0], "compatibility")) {
                    out.asDirective.asVersion.type = kCompatibility;
                } else if (!strcmp(&chars[0], "es")) {
                    out.asDirective.asVersion.type = kES;
                } else {
                    m_error = "Invalid profile in #version directive";
                    return;
                }
            }
        } else if (!strcmp(&chars[0], "extension")) {
            out.asDirective.type = directive::kExtension;

            // extension [a-zA-Z_]+ : (enable|require|warn|disable)
            skipWhitespace(false);

            vector<char> extension;
            while (isChar(at())) {
                extension.push_back(at());
                m_location.advanceColumn();
            }
            extension.push_back('\0');

            if (extension.empty()) {
                m_error = "Expected extension name in #extension directive";
                return;
            }

            skipWhitespace(false);

            if (at() != ':') {
                m_error = "Expected `:' in #extension directive";
                return;
            }

            m_location.advanceColumn(); // Skip ':'.

            skipWhitespace(false);

            vector<char> behavior;
            while (isChar(at())) {
                behavior.push_back(at());
                m_location.advanceColumn();
            }
            behavior.push_back('\0');

            if (behavior.empty()) {
                m_error = "Expected behavior in #extension directive";
                return;
            }

            if (!strcmp(&behavior[0], "enable")) {
                out.asDirective.asExtension.behavior = kEnable;
            } else if (!strcmp(&behavior[0], "require")) {
                out.asDirective.asExtension.behavior = kRequire;
            } else if (!strcmp(&behavior[0], "warn")) {
                out.asDirective.asExtension.behavior = kWarn;
            } else if (!strcmp(&behavior[0], "disable")) {
                out.asDirective.asExtension.behavior = kDisable;
            } else {
                m_error = "Unexpected behavior in #extension directive";
                return;
            }

            // Do this late when nothing can fail so we don't leak memory.
            size_t name_len = strlen(&extension[0]);
            char *name = (char*)malloc(name_len + 1);
            if (!name) {
                m_error = "Out of memory";
                return;
            }
            memcpy(name, &extension[0], name_len + 1);
            out.asDirective.asExtension.name = name;
        } else {
            m_error = "Unsupported directive";
            return;
        }

        out.m_type = kType_directive;
    } else {
        switch (at()) {
        // Non operators
        case '\n':
        case '\t':
        case '\f':
        case '\v':
        case '\r':
        case ' ':
            skipWhitespace(true);
            out.m_type = kType_whitespace; // Whitespace already skipped.
            break;
        case ';':
            out.m_type = kType_semicolon;
            m_location.advanceColumn();
            break;
        case '{':
            out.m_type = kType_scope_begin;
            m_location.advanceColumn();
            break;
        case '}':
            out.m_type = kType_scope_end;
            m_location.advanceColumn();
            break;
        // Operators
        case '.':
            out.m_type = kType_operator;
            out.asOperator = kOperator_dot;
            break;
        case '+':
            out.m_type = kType_operator;
            if (ch1 == '+')
                out.asOperator = kOperator_increment;
            else if (ch1 == '=')
                out.asOperator = kOperator_add_assign;
            else
                out.asOperator = kOperator_plus;
            break;
        case '-':
            out.m_type = kType_operator;
            if (ch1 == '-')
                out.asOperator = kOperator_decrement;
            else if (ch1 == '=')
                out.asOperator = kOperator_sub_assign;
            else
                out.asOperator = kOperator_minus;
            break;
        case '/':
            if (ch1 == '/') {
                // Skip line comments
                while (position() != m_length) {
                    if (at() == '\n') {
                        m_location.advanceLine();
                        break;
                    }
                    m_location.advanceColumn();
                }
                out.m_type = kType_comment;
            } else if (ch1 == '*') {
                // Skip block comments
                while (position() != m_length) {
                    if (at() == '\n') {
                        m_location.advanceLine();
                        continue;
                    }
                    if (at() == '*' && position() + 1 < m_length && m_data[position() + 1] == '/') {
                        m_location.advanceColumn(2);
                        break;
                    }
                    m_location.advanceColumn();
                }
                out.m_type = kType_comment;
            } else if (ch1 == '=') {
                out.m_type = kType_operator;
                out.asOperator = kOperator_divide_assign;
            } else {
                out.m_type = kType_operator;
                out.asOperator = kOperator_divide;
            }
            break;
        case '*':
            out.m_type = kType_operator;
            if (ch1 == '=')
                out.asOperator = kOperator_multiply_assign;
            else
                out.asOperator = kOperator_multiply;
            break;
        case '%':
            out.m_type = kType_operator;
            if (ch1 == '=')
                out.asOperator = kOperator_modulus_assign;
            else
                out.asOperator = kOperator_modulus;
            break;
        case '<':
            out.m_type = kType_operator;
            if (ch1 == '<' && ch2 == '=')
                out.asOperator = kOperator_shift_left_assign;
            else if (ch1 == '<')
                out.asOperator = kOperator_shift_left;
            else if (ch1 == '=')
                out.asOperator = kOperator_less_equal;
            else
                out.asOperator = kOperator_less;
            break;
        case '>':
            out.m_type = kType_operator;
            if (ch1 == '>' && ch2 == '=')
                out.asOperator = kOperator_shift_right_assign;
            else if (ch1 == '>')
                out.asOperator = kOperator_shift_right;
            else if (ch1 == '=')
                out.asOperator = kOperator_greater_equal;
            else
                out.asOperator = kOperator_greater;
            break;
        case '[':
            out.m_type = kType_operator;
            out.asOperator = kOperator_bracket_begin;
            break;
        case ']':
            out.m_type = kType_operator;
            out.asOperator = kOperator_bracket_end;
            break;
        case '(':
            out.m_type = kType_operator;
            out.asOperator = kOperator_paranthesis_begin;
            break;
        case ')':
            out.m_type = kType_operator;
            out.asOperator = kOperator_paranthesis_end;
            break;
        case '^':
            out.m_type = kType_operator;
            if (ch1 == '^')
                out.asOperator = kOperator_logical_xor;
            else if (ch1 == '=')
                out.asOperator = kOperator_bit_xor_assign;
            else
                out.asOperator = kOperator_bit_xor;
            break;
        case '|':
            out.m_type = kType_operator;
            if (ch1 == '|')
                out.asOperator = kOperator_logical_or;
            else if (ch1 == '=')
                out.asOperator = kOperator_bit_or_assign;
            else
                out.asOperator = kOperator_bit_or;
            break;
        case '&':
            out.m_type = kType_operator;
            if (ch1 == '&')
                out.asOperator = kOperator_logical_and;
            else if (ch1 == '=')
                out.asOperator = kOperator_bit_and_assign;
            else
                out.asOperator = kOperator_bit_and;
            break;
        case '~':
            out.m_type = kType_operator;
            out.asOperator = kOperator_bit_not;
            break;
        case '=':
            out.m_type = kType_operator;
            if (ch1 == '=')
                out.asOperator = kOperator_equal;
            else
                out.asOperator = kOperator_assign;
            break;
        case '!':
            out.m_type = kType_operator;
            if (ch1 == '=')
                out.asOperator = kOperator_not_equal;
            else
                out.asOperator = kOperator_logical_not;
            break;
        case ':':
            out.m_type = kType_operator;
            out.asOperator = kOperator_colon;
            break;
        case ',':
            out.m_type = kType_operator;
            out.asOperator = kOperator_comma;
            break;
        case '?':
            out.m_type = kType_operator;
            out.asOperator = kOperator_questionmark;
            break;
        default:
            m_error = "invalid character encountered";
            return;
        }
        // Skip whitespace for operator
        if (out.m_type == kType_operator)
            m_location.advanceColumn(strlen(kOperators[out.asOperator].string));
    }
}

vector<char> lexer::readNumeric(bool isOctalish, bool isHexish) {
    vector<char> digits;
    if (isOctalish) {
        while (position() < m_length && isOctal(at())) {
            digits.push_back(at());
            m_location.advanceColumn();
        }
    } else if (isHexish) {
        while (position() < m_length && isHex(at())) {
            digits.push_back(at());
            m_location.advanceColumn();
        }
    } else {
        while (position() < m_length && isDigit(at())) {
            digits.push_back(at());
            m_location.advanceColumn();
        }
    }
    return digits;
}

token lexer::peek() {
    token out;
    backup();
    read(out, true);
    restore();
    return out;
}

void lexer::read(token &out, bool) {
    do {
        read(out);
    } while ((out.m_type == kType_whitespace || out.m_type == kType_comment) && !m_error);
}

const char *lexer::error() const {
    return m_error;
}

void lexer::backup() {
    m_backup = m_location;
}

void lexer::restore() {
    m_location = m_backup;
}

}
