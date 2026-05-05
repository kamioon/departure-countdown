#pragma once

// Tell ArduinoJson to enable Arduino String / Stream support when this
// header is included before ArduinoJson.h (which it always is via ns_api.h).
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <string>

// Arduino-compatible String class backed by std::string
class String {
    std::string _s;
public:
    String() {}
    String(const char* s) : _s(s ? s : "") {}
    String(const std::string& s) : _s(s) {}
    String(int n)           { char b[32]; snprintf(b, sizeof(b), "%d",  n); _s = b; }
    String(long n)          { char b[32]; snprintf(b, sizeof(b), "%ld", n); _s = b; }
    String(unsigned long n) { char b[32]; snprintf(b, sizeof(b), "%lu", n); _s = b; }
    String(char c)          : _s(1, c) {}

    const char* c_str()   const { return _s.c_str(); }
    size_t      length()  const { return _s.length(); }
    bool        isEmpty() const { return _s.empty(); }

    int indexOf(char c, int from = 0) const {
        size_t pos = _s.find(c, (size_t)from);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int indexOf(const char* s, int from = 0) const {
        size_t pos = _s.find(s, (size_t)from);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int indexOf(const String& s, int from = 0) const {
        return indexOf(s.c_str(), from);
    }

    char charAt(int index) const {
        if (index < 0 || index >= (int)_s.size()) return 0;
        return _s[(size_t)index];
    }

    void trim() {
        size_t s = _s.find_first_not_of(" \t\r\n");
        size_t e = _s.find_last_not_of(" \t\r\n");
        _s = (s == std::string::npos) ? "" : _s.substr(s, e - s + 1);
    }

    bool startsWith(const char* prefix) const {
        return _s.rfind(prefix, 0) == 0;
    }

    bool startsWith(const String& prefix) const {
        return startsWith(prefix.c_str());
    }

    bool equalsIgnoreCase(const char* other) const {
        if (_s.size() != strlen(other)) return false;
        for (size_t i = 0; i < _s.size(); i++) {
            if (tolower((unsigned char)_s[i]) != tolower((unsigned char)other[i])) return false;
        }
        return true;
    }

    bool equalsIgnoreCase(const String& other) const {
        return equalsIgnoreCase(other.c_str());
    }

    void remove(int from, int count = 1) {
        if (from >= 0 && from < (int)_s.size())
            _s.erase((size_t)from, (size_t)count);
    }

    String substring(int from, int to = -1) const {
        if (from < 0) from = 0;
        int end = (to < 0 || to > (int)_s.size()) ? (int)_s.size() : to;
        if (from >= end) return String();
        return String(_s.substr((size_t)from, (size_t)(end - from)));
    }

    int toInt() const { return atoi(_s.c_str()); }

    // Used by ArduinoJson's ArduinoStringWriter when serialising JSON to a String
    bool concat(const char* s)   { if (s) _s += s; return true; }
    bool concat(const String& s) { _s += s._s; return true; }

    String& operator+=(const String& o)  { _s += o._s; return *this; }
    String& operator+=(const char* o)    { _s += o;    return *this; }
    String  operator+(const String& o)   const { return String(_s + o._s); }
    String  operator+(const char* o)     const { return String(_s + o); }
    bool    operator==(const String& o)  const { return _s == o._s; }
    bool    operator==(const char* o)    const { return _s == o; }
    bool    operator!=(const String& o)  const { return _s != o._s; }
    bool    operator!=(const char* o)    const { return _s != o; }
};

inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs) + rhs.c_str());
}

// Serial stub — writes to stdout so debug prints are visible during tests
class HardwareSerial {
public:
    void begin(long) {}
    void print(const char* s)    { fputs(s, stdout); fflush(stdout); }
    void print(const String& s)  { fputs(s.c_str(), stdout); fflush(stdout); }
    void print(int n)            { printf("%d",  n); fflush(stdout); }
    void print(long n)           { printf("%ld", n); fflush(stdout); }
    void print(unsigned long n)  { printf("%lu", n); fflush(stdout); }
    void println(const char* s)  { puts(s); }
    void println(const String& s){ puts(s.c_str()); }
    void println(int n)          { printf("%d\n",  n); }
    void println(long n)         { printf("%ld\n", n); }
    void println(unsigned long n){ printf("%lu\n", n); }
    void println()               { putchar('\n'); }
};

// static = internal linkage: each .cpp gets its own instance, fine for a no-state stub
static HardwareSerial Serial;

inline unsigned long millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long)(ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL);
}

inline void delay(unsigned long) {}

#define F(s)   (s)
#define PROGMEM
#define INPUT  0
#define OUTPUT 1
#define HIGH   1
#define LOW    0
