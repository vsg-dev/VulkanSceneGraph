#ifndef OSG2VSG_EXPORT
#define OSG2VSG_EXPORT

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( VSG_LIBRARY_STATIC )
    #    define OSG2VSG_EXPORT
    #  elif defined( VSG_LIBRARY )
    #    define OSG2VSG_EXPORT   __declspec(dllexport)
    #  else
    #    define OSG2VSG_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define OSG2VSG_EXPORT
#endif

#endif
