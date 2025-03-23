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

// An implementation of ntoa (number to ascii)
// Remember to free the string!

// ntoa("%d", 1); // integer
template <typename T>
static inline char* ntoa(const char* fmt, T x) {
    if (!strlen(fmt)) return nullptr;

    int length = snprintf(NULL, 0, fmt, x);
    char* str = reinterpret_cast<char*>(malloc(length + 1));
    snprintf(str, length + 1, fmt, x);
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
        // Start with a reasonable buffer size
        buffer = new char[64];
        capacity = 64;
        length = 0;
        buffer[0] = '\0';
    }
    
    ~indent_aware_stringbuilder() {
        if (buffer) {
            delete[] buffer;
            buffer = NULL;
        }
    }
    
    // Copy constructor
    indent_aware_stringbuilder(const indent_aware_stringbuilder& other) : 
        buffer(NULL), capacity(0), length(0), currentIndent(0), atLineStart(true) {
        *this = other;  // Use assignment operator
    }
    
    // Assignment operator
    indent_aware_stringbuilder& operator=(const indent_aware_stringbuilder& other) {
        if (this != &other) {
            // Allocate new buffer
            char* newBuffer = new char[other.capacity];
            
            // Copy the data
            memcpy(newBuffer, other.buffer, other.length);
            
            // Clean up old buffer
            if (buffer) {
                delete[] buffer;
            }
            
            // Update member variables
            buffer = newBuffer;
            capacity = other.capacity;
            length = other.length;
            currentIndent = other.currentIndent;
            indentStack = other.indentStack;
            atLineStart = other.atLineStart;
        }
        return *this;
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

    void copyIndent(indent_aware_stringbuilder& other) {
        currentIndent = other.currentIndent;
        indentStack = other.indentStack;
    }
    
    void append(const char* str) {
        if (!str) return;
        
        size_t strLen = strlen(str);
        if (strLen == 0) return;
        
        // Calculate worst-case required capacity (if every char is a newline)
        size_t maxNeeded = length + strLen + (strLen * currentIndent);
        if (maxNeeded >= capacity) {
            // Grow the buffer to at least double or what we need
            size_t newCapacity = capacity * 2;
            while (newCapacity <= maxNeeded) {
                newCapacity *= 2;
            }
            grow(newCapacity);
        }
        
        // Now append character by character, handling indentation
        for (size_t i = 0; i < strLen; ++i) {
            if (atLineStart) {
                // Add indentation at start of line
                for (int j = 0; j < currentIndent; ++j) {
                    buffer[length++] = ' ';
                }
                atLineStart = false;
            }
            
            // Add the character
            buffer[length++] = str[i];
            
            // Check for newline
            if (str[i] == '\n') {
                atLineStart = true;
            }
        }
        
        // Always null-terminate (even though we don't count it in length)
        buffer[length] = '\0';
    }

    void append(const indent_aware_stringbuilder& builder) {
        append(builder.toString());
    }
    
    void appendLine(const char* str = "") {
        append(str);
        append("\n");
    }
    
    const char* toString() const {
        if (!buffer) return "";
        buffer[length] = '\0';  // Ensure null termination
        return buffer;
    }
    
    size_t getLength() const {
        return length;
    }
    
    void clear() {
        length = 0;
        if (buffer) {
            buffer[0] = '\0';
        }
        atLineStart = true;
        // Don't reset indentation - that's the caller's job
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
    
    // Grow the buffer to the new capacity
    void grow(size_t newCapacity) {
        char* newBuffer = new char[newCapacity];
        
        // Copy existing content if we have any
        if (buffer && length > 0) {
            memcpy(newBuffer, buffer, length);
        }
        
        // Always null terminate
        newBuffer[length] = '\0';
        
        // Clean up old buffer
        if (buffer) {
            delete[] buffer;
        }
        
        // Update member variables
        buffer = newBuffer;
        capacity = newCapacity;
    }
};

}

#endif
