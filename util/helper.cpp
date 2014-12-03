#include <string>
#include "helper.h"

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
