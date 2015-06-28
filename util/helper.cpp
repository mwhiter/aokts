#include <string>
#include <sstream>
#include "helper.h"

// for trim functions
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

bool replaced( std::string &s, const std::string &search, const std::string &replace ) {
    size_t pos = s.find( search);
    if (pos!=std::string::npos) {
        // Replace by erasing and inserting
        s.erase( pos, search.length() );
        s.insert( pos, replace );
        return true;
    }
    return false;
}

void replaceAll( std::string &s, const std::string &search, const std::string &replace ) {
   for( size_t pos = 0; ; pos += replace.length() ) {
       // Locate the substring to replace
       pos = s.find( search, pos );
       if( pos == std::string::npos ) break;
       // Replace by erasing and inserting
       s.erase( pos, search.length() );
       s.insert( pos, replace );
   }
}

// trim from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

std::string time_string(unsigned int seconds, bool shortened) {
    std::ostringstream convert;
    unsigned int mins = seconds / 60;
    unsigned int hours = mins / 60;
    bool input = false;

    seconds = seconds - mins * 60;
    mins = mins - hours * 60;

    if (seconds + mins + hours == 0) {
        convert << "no time";
    } else {
        if (hours > 0) {
            if (shortened) {
                convert << hours << "h";
            } else {
                convert << hours << " hr";
                if (hours > 1) {
                    convert << "s";
                }
            }
            input = true;
        }
        if (mins > 0) {
            if (shortened) {
                convert << mins << "m";
            } else {
                if (input) {
                    convert << " ";
                }
                convert << mins << " min";
            }
            input = true;
        }
        if (seconds > 0) {
            if (shortened) {
                convert << seconds << "s";
            } else {
                if (input) {
                    convert << " ";
                }
                convert << seconds << " sec";
            }
        }
    }
    return convert.str();
}

std::string wstringToString(std::wstring ws) {
    return std::string(ws.begin(), ws.end());
}

std::string wstringToString(const wchar_t * wcs) {
    if (wcs && wcslen(wcs)) {
        std::wstring ws(wcs);
        return std::string(ws.begin(), ws.end());
    } else {
        return std::string("");
    }
}

std::string playerPronoun(int p) {
    std::ostringstream convert;
    switch (p) {
    case -1:
        break;
    case 0:
        convert << "gaia";
        break;
    default:
        convert << "p" << p;
    }
    return convert.str();
}

std::string longToString(long l) {
    std::ostringstream convert;
    convert << l;
    return convert.str();
}
