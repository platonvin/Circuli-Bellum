#pragma once
#ifndef __MACROS_HPP__
#define __MACROS_HPP__

#include <iostream>
#include <cassert>

//corrosion vibes
//no mut btw
#define let auto&

#define UNDEF_L_PL
// #define UNDEF_AL_APL

#ifndef UNDEF_L_PL
    #define l() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
    #define pl(x) std::cout << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
#else
    #define l()
    #define pl(x)
#endif

#ifndef UNDEF_L_PL
    #define apl(x) std::cout << __FILE__<<":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
    #define al() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
#else
    #define apl(x)
    #define al()
#endif

#define validate_bounds(index,size) \
    if(!(index < size)){\
        std::cout << "index " #index "=" << (index) << " is not in bounds of " #size "=" << (size) << "\n";\
        assert(index < size);\
    }\

#endif // __MACROS_HPP__
