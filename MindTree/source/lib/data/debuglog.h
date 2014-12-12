#ifndef DEBUGLOG_H_D34UWMN6
#define DEBUGLOG_H_D34UWMN6

#include "iostream"

#define dbout(sstream) \
    std::cout << __PRETTY_FUNCTION__ << "(" << __LINE__ << "): " << sstream << std::endl;

#endif /* end of include guard: DEBUGLOG_H_D34UWMN6 */
