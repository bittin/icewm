/*
 * IceWM
 *
 * Copyright (C) 2004,2005 Marko Macek
 */
#include "config.h"

#include "mstring.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <regex.h>
#include <new>
#include "base.h"
#include "ascii.h"

void MStringRef::alloc(size_t len) {
    fStr = new (malloc(sizeof(MStringData) + len + 1)) MStringData();
    fStr->fLen = unsigned(len);
}

void MStringRef::create(const char* str, size_t len) {
    if (len) {
        alloc(len);
        if (str) {
            strncpy(fStr->fStr, str, len);
            fStr->fStr[len] = 0;
        } else {
            memset(fStr->fStr, 0, len + 1);
        }
    } else {
        fStr = nullptr;
    }
}

mstring::mstring(const MStringRef& str, size_t offset, size_t count):
    fRef(str)
{
    if (fRef && (offset || count < length())) {
        fRef.create(&str[offset], count);
    }
    acquire();
}

mstring::mstring(const char* str1, size_t len1, const char* str2, size_t len2):
    fRef(len1 + len2)
{
    if (fRef) {
        if (len1)
            strncpy(fRef->fStr, str1, len1);
        if (len2)
            strncpy(fRef->fStr + len1, str2, len2);
        fRef[len1 + len2] = 0;
        fRef.acquire();
    }
}

mstring::mstring(const char* str):
    mstring(str, str ? strlen(str) : 0)
{
}

mstring::mstring(const char* str, size_t len):
    fRef(str, len)
{
    acquire();
}

mstring::mstring(const char *str1, const char *str2):
    mstring(str1, str1 ? strlen(str1) : 0, str2, str2 ? strlen(str2) : 0)
{
}

mstring::mstring(const char *str1, const char *str2, const char *str3) {
    size_t len1 = str1 ? strlen(str1) : 0;
    size_t len2 = str2 ? strlen(str2) : 0;
    size_t len3 = str3 ? strlen(str3) : 0;
    size_t count = len1 + len2 + len3;
    fRef.alloc(count);
    if (len1) memcpy(fRef->fStr, str1, len1);
    if (len2) memcpy(fRef->fStr + len1, str2, len2);
    if (len3) memcpy(fRef->fStr + len1 + len2, str3, len3);
    fRef[count] = 0;
    fRef.acquire();
}

mstring::mstring(const char *str1, const char *str2, const char *str3,
                 const char *str4, const char *str5, const char *str6)
{
    const int count = 6;
    const char* string[count] = {str1, str2, str3, str4, str5, str6};
    size_t length[count];
    size_t total = 0;
    for (int i = 0; i < count; ++i) {
        total += length[i] = (string[i] ? strlen(string[i]) : 0);
    }
    fRef.alloc(total);
    size_t build = 0;
    for (int i = 0; i < count; ++i) {
        if (length[i]) {
            memcpy(fRef->fStr + build, string[i], length[i]);
            build += length[i];
        }
    }
    fRef[total] = 0;
    fRef.acquire();
}

mstring::mstring(long n) {
    char buf[24];
    snprintf(buf, 24, "%ld", n);
    fRef.create(buf, strlen(buf));
    fRef.acquire();
}

mstring mstring::operator+(const mstring& rv) const {
    return rv.isEmpty() ? *this : isEmpty() ? rv :
        mstring(data(), length(), rv.data(), rv.length());
}

void mstring::operator+=(const mstring& rv) {
    *this = *this + rv;
}

mstring& mstring::operator=(const mstring& rv) {
    if (fRef != rv.fRef) {
        release();
        fRef = rv.fRef;
        acquire();
    }
    return *this;
}

mstring mstring::substring(size_t pos) const {
    return pos < length()
        ? mstring(fRef, pos, length() - pos)
        : null;
}

mstring mstring::substring(size_t pos, size_t len) const {
    return pos < length() && len
        ? mstring(fRef, pos, min(len, length() - pos))
        : null;
}

bool mstring::split(unsigned char token, mstring *left, mstring *remain) const {
    PRECONDITION(token < 128);
    int splitAt = indexOf(char(token));
    if (splitAt >= 0) {
        size_t i = size_t(splitAt);
        mstring l(substring(0, i));
        mstring r(substring(i + 1, length() - i - 1));
        *left = l;
        *remain = r;
        return true;
    }
    return false;
}

bool mstring::splitall(unsigned char token, mstring *left, mstring *remain) const {
    if (split(token, left, remain))
        return true;

    if (nonempty()) {
        *left = *this;
        *remain = null;
        return true;
    }
    return false;
}

int mstring::charAt(int pos) const {
    return size_t(pos) < length() ? data()[pos] : -1;
}

bool mstring::startsWith(const char* s) const {
    size_t slen = strlen(s);
    return slen == 0 || (slen <= length()
        && 0 == memcmp(data(), s, slen));
}

bool mstring::endsWith(const char* s) const {
    size_t slen = strlen(s);
    return slen == 0 || (slen <= length()
        && 0 == memcmp(data() + length() - slen, s, slen));
}

int mstring::find(const char* str) const {
    size_t len = strlen(str);
    const char* found = (len == 0 || isEmpty()) ? nullptr :
        static_cast<const char*>(memmem(
                data(), length(), str, len));
    return found ? int(found - data()) : (len ? -1 : 0);
}

int mstring::find(const mstring &str) const {
    const char* found = (str.isEmpty() || isEmpty()) ? nullptr :
        static_cast<const char*>(memmem(
                data(), length(), str.data(), str.length()));
    return found ? int(found - data()) : (str.isEmpty() - 1);
}

int mstring::indexOf(char ch) const {
    const char *str = isEmpty() ? nullptr :
        static_cast<const char *>(memchr(data(), ch, length()));
    return str ? int(str - data()) : -1;
}

int mstring::lastIndexOf(char ch) const {
    const char *str = isEmpty() ? nullptr :
        static_cast<const char *>(memrchr(data(), ch, length()));
    return str ? int(str - data()) : -1;
}

int mstring::count(char ch) const {
    int num = 0;
    for (const char* str = data(), *end = str + length(); str < end; ++str) {
        str = static_cast<const char *>(memchr(str, ch, end - str));
        if (str == nullptr)
            break;
        ++num;
    }
    return num;
}

bool mstring::equals(const char *str) const {
    return (str && *str) ? equals(str, strlen(str)) : isEmpty();
}

bool mstring::equals(const char *str, size_t len) const {
    return len == length() && 0 == memcmp(str, data(), len);
}

bool mstring::equals(const mstring &str) const {
    return equals(str.data(), str.length());
}

int mstring::collate(mstring s, bool ignoreCase) {
    if (!ignoreCase)
        return strcoll(*this, s);
    else
        return strcoll(this->lower(), s.lower());
}

int mstring::compareTo(const mstring &s) const {
    size_t len = min(s.length(), length());
    int cmp = len ? memcmp(data(), s.data(), len) : 0;
    return cmp ? cmp : int(length()) - int(s.length());
}

bool mstring::copyTo(char *dst, size_t len) const {
    if (len > 0) {
        size_t copy = min(len - 1, length());
        if (copy) memcpy(dst, data(), copy);
        dst[copy] = 0;
    }
    return length() < len;
}

mstring mstring::replace(int pos, int len, const mstring &insert) const {
    return substring(0, size_t(pos)) + insert + substring(size_t(pos + len));
}

mstring mstring::remove(int pos, int len) const {
    return substring(0, size_t(pos)) + substring(size_t(pos + len));
}

mstring mstring::insert(int pos, const mstring &str) const {
    return substring(0, size_t(pos)) + str + substring(size_t(pos));
}

mstring mstring::searchAndReplaceAll(const mstring& s, const mstring& r) const {
    mstring modified(*this);
    const int step = int(1 + r.length() - s.length());
    for (int offset = 0; size_t(offset) + s.length() <= modified.length(); ) {
        int found = offset + modified.substring(size_t(offset)).find(s);
        if (found < offset) {
            break;
        }
        modified = modified.replace(found, int(s.length()), r);
        offset = max(0, offset + step);
    }
    return modified;
}

mstring mstring::lower() const {
    const size_t len = length();
    mstring mstr(nullptr, len);
    for (size_t i = 0; i < len; ++i) {
        mstr.fRef[i] = tolower((unsigned char) data()[i]);
    }
    return mstr;
}

mstring mstring::upper() const {
    const size_t len = length();
    mstring mstr(nullptr, len);
    for (size_t i = 0; i < len; ++i) {
        mstr.fRef[i] = toupper((unsigned char) data()[i]);
    }
    return mstr;
}

mstring mstring::trim() const {
    size_t k = 0, n = length();
    while (k < n && ASCII::isWhiteSpace(data()[k])) {
        ++k;
    }
    while (k < n && ASCII::isWhiteSpace(data()[n - 1])) {
        --n;
    }
    return substring(k, n - k);
}

const char* mstring::c_str()
{
    if (isEmpty()) {
        return "";
    }
    else if (data()[length()]) {
        if (fRef->fRefCount == 1) {
            fRef[length()] = '\0';
        } else {
            const char* str = data();
            fRef.release();
            fRef.create(str, length());
            fRef.acquire();
        }
    }
    return data();
}

mstring mstring::match(const char* regex, const char* flags) const {
    int compFlags = REG_EXTENDED;
    int execFlags = 0;
    for (int i = 0; flags && flags[i]; ++i) {
        switch (flags[i]) {
            case 'i': compFlags |= REG_ICASE; break;
            case 'n': compFlags |= REG_NEWLINE; break;
            case 'B': execFlags |= REG_NOTBOL; break;
            case 'E': execFlags |= REG_NOTEOL; break;
        }
    }

    regex_t preg;
    int comp = regcomp(&preg, regex, compFlags);
    if (comp) {
        if (testOnce(regex, __LINE__)) {
            char rbuf[123] = "";
            regerror(comp, &preg, rbuf, sizeof rbuf);
            warn("match regcomp: %s", rbuf);
        }
        return null;
    }

    regmatch_t pos;
    int exec = regexec(&preg, data(), 1, &pos, execFlags);

    regfree(&preg);

    if (exec)
        return null;

    return mstring(data() + pos.rm_so, size_t(pos.rm_eo - pos.rm_so));
}

#include <errno.h>
#include <stdarg.h>

void mstring::fmt(const char* fmt, ...) {
    const int en = errno;
    va_list ap;
    va_start(ap, fmt);
    size_t len = strlen(fmt);
    for (const char* s = fmt; *s; ++s) {
        if (*s == '%' && s[1]) {
            s++;
            if (*s == 's') {
                len = len + strlen(va_arg(ap, const char *)) - 2;
            }
            if (*s == 'm') {
                len = len + strlen(strerror(en)) - 2;
            }
        }
    }
    va_end(ap);
    MStringRef ref(len);
    va_start(ap, fmt);
    int i = 0;
    for (const char* s = fmt; *s; ++s) {
        if (*s == '%' && s[1]) {
            s++;
            if (*s == 's') {
                const char* arg = va_arg(ap, const char *);
                for (int k = 0; arg[k]; ++k)
                    ref[i++] = arg[k];
            }
            if (*s == 'm') {
                const char* arg = strerror(en);
                for (int k = 0; arg[k]; ++k)
                    ref[i++] = arg[k];
            }
            if (*s == '%') {
                ref[i++] = '%';
            }
        } else {
            ref[i++] = *s;
        }
    }
    ref[i] = '\0';
    va_end(ap);
    release();
    fRef = ref;
    acquire();
}

// vim: set sw=4 ts=4 et:
