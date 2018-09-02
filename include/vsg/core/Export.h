#ifndef VSG_EXPORT
#define VSG_EXPORT

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( VSG_LIBRARY_STATIC )
    #    define VSG_EXPORT
    #  elif defined( VSG_LIBRARY )
    #    define VSG_EXPORT   __declspec(dllexport)
    #  else
    #    define VSG_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define VSG_EXPORT
#endif

#endif
