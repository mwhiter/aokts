#include <string>
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
