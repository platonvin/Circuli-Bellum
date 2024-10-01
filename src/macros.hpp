#pragma once
#ifndef __MACROS_HPP__
#define __MACROS_HPP__

#include <iostream>
#include <cassert>

//corrosion vibes
//no mut btw
#define let auto&

// #define UNDEF_L_PL
// #define UNDEF_AL_APL

#ifndef UNDEF_L_PL
    #define l() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
    #define pl(x) std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
    #define npl(x) std::cout << " :" << __LINE__ << " " #x " "  << x << "\n";
#else
    #define l()
    #define pl(x)
#endif

#ifndef UNDEF_L_PL
    #define al() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
    #define apl(x) std::cout << __FILE__<<":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
#else
    #define al()
    #define apl(x)
#endif

//assert index < size with nice look
#define validate_bounds(index,size) \
    if(!(index < size)){\
        std::cout << "index " #index "=" << (index) << " is not in bounds of " #size "=" << (size) << "\n";\
        assert(index < size);\
    }\

#endif // __MACROS_HPP__
