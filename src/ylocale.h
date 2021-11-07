/*
 *  IceWM - C++ wrapper for locale/unicode conversion
 *  Copyright (C) 2001 The Authors of IceWM
 *
 *  Release under terms of the GNU Library General Public License
 */

#ifndef YLOCALE_H
#define YLOCALE_H

#include <stddef.h>

class YLocale {
public:
    YLocale(char const* localeName = "");
    ~YLocale();

    static const char* getLocaleName();
    static int getRating(const char* localeStr);
    static bool RTL() { return instance->rightToLeft; }

#ifdef CONFIG_I18N
    static char* localeString(wchar_t const* uStr, size_t uLen, size_t& lLen);
    static wchar_t* unicodeString(char const* lStr, size_t lLen, size_t& uLen);
#endif

private:
    class YConverter* converter;
    bool rightToLeft;
    void getDirection();

    static YLocale* instance;
};

#endif

// vim: set sw=4 ts=4 et:
