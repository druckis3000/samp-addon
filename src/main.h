#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

// --------- main.h FUNCTION DECLARATIONS ----------

#endif // __MAIN_H__
