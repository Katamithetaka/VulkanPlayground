#ifndef MODULES_SHARED_H 
#define MODULES_SHARED_H

#ifdef _WIN32 
    #define MODULES_DLL __declspec(dllexport) extern "C"
#else
    #define MODULES_DLL extern "C"
#endif
#endif