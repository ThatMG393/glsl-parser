#ifndef UTIL_H
#define UTIL_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h> // va_list
#include <vector>

namespace glsl {

// An implementation of std::find
template <typename I, typename T>
static inline I find(I first, I last, const T &value) {
    for (; first != last; ++first)
        if (*first == value)
            return first;
    return last;
}

// An implementation of itoa
// Remember to free the string!
static inline char* itoa(int x) {
    int length = snprintf(NULL, 0, "%d", x);
    char* str = reinterpret_cast<char*>(malloc(length + 1));
    snprintf(str, length + 1, "%d", x);
    return str;
}

// An implementation of vasprintf
int allocvfmt(char **str, const char *fmt, va_list vp);

// An implementation of vsprintf
int allocfmt(char **str, const char *fmt, ...);

// a tiny wrapper around std::vector so you can provide your own
template <typename T>
struct vector {
    size_t size() const { return m_data.size(); }
    bool empty() const { return m_data.empty(); }
    const T& operator[](size_t index) const { return m_data[index]; }
    T& operator[](size_t index) { return m_data[index]; }
    T* begin() { return &m_data[0]; }
    T* end() { return &m_data[size()]; }
    const T* begin() const { return &m_data[0]; }
    const T* end() const { return &m_data[size()]; }
    void insert(T *at, const T& value = T()) { m_data.insert(m_data.begin() + size_t(at - begin()), value); }
    void insert(T *at, const T *beg, const T *end) { m_data.insert(m_data.begin() + size_t(at - begin()), beg, end); }
    void push_back(const T &value) { m_data.push_back(value); }
    void reserve(size_t size) { m_data.reserve(size); }
    T* erase(T *position) { return &*m_data.erase(m_data.begin() + size_t(position - begin())); }
    T* erase(T *first, T *last) { return &*m_data.erase(m_data.begin() + size_t(first - begin()), m_data.begin() + size_t(last - begin())); }
    void pop_back() { m_data.pop_back(); }
    T &front() { return *begin(); }
    const T &front() const { return *begin(); }
    T &back() { return *(end() - 1); }
    const T& back() const { return *(end() - 1); }
    void resize(size_t size) { m_data.resize(size); }
private:
    std::vector<T> m_data;
};

struct indent_aware_stringbuilder {
    indent_aware_stringbuilder() : buffer(NULL), capacity(0), length(0), currentIndent(0), atLineStart(true) {
        resize(16);
    }
    
    ~indent_aware_stringbuilder() {
        delete[] buffer;
    }
    
    void pushIndent(int spaces = 4) {
        indentStack.push_back(spaces);
        currentIndent += spaces;
    }
    
    void popIndent() {
        if (!indentStack.empty()) {
            currentIndent -= indentStack.back();
            indentStack.pop_back();
        }
    }
    
    void append(const char* str) {
        if (!str) return;
        
        size_t strLen = strlen(str);
        if (strLen == 0) return;
        
        for (size_t i = 0; i < strLen; ++i) {
            if (atLineStart) {
                appendIndentation();
            }
            
            ensureCapacity(1);
            buffer[length++] = str[i];
            
            if (str[i] == '\n') {
                atLineStart = true;
            }
        }
    }
    
    void appendLine(const char* str = "") {
        append(str);
        append("\n");
    }
    
    const char* toString() const {
        // Ensure null termination
        if (length >= capacity) {
            const_cast<indent_aware_stringbuilder*>(this)->resize(length + 1);
        }
        buffer[length] = '\0';
        return buffer;
    }
    
    size_t getLength() const {
        return length;
    }
    
    void clear() {
        length = 0;
        currentIndent = 0;
        indentStack.clear();
        atLineStart = true;
    }

    indent_aware_stringbuilder& operator+=(const char* str) {
        append(str);
        return *this;
    }

private:
    char* buffer;
    size_t capacity;
    size_t length;
    
    std::vector<int> indentStack;
    int currentIndent;
    bool atLineStart;
    
    void resize(size_t newCapacity) {
        char* newBuffer = new char[newCapacity];
        if (buffer) {
            std::memcpy(newBuffer, buffer, length);
            delete[] buffer;
        }
        buffer = newBuffer;
        capacity = newCapacity;
    }
    
    void ensureCapacity(size_t additionalChars) {
        if (length + additionalChars >= capacity) {
            size_t newCapacity = (capacity == 0) ? 16 : capacity * 2;
            while (length + additionalChars >= newCapacity) {
                newCapacity *= 2;
            }
            resize(newCapacity);
        }
    }
    
    void appendIndentation() {
        if (atLineStart && currentIndent > 0) {
            ensureCapacity(currentIndent);
            for (int i = 0; i < currentIndent; ++i) {
                buffer[length++] = ' ';
            }
        }
        atLineStart = false;
    }
};

}

#endif
