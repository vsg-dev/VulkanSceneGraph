#pragma once

#include <sstream>
#include <iostream>

namespace vsg
{

namespace CommandLine
{

    bool read(int& argc, char** argv, const std::string& match)
    {
        for(int i=1; i<argc; ++i)
        {
            if (match==argv[i])
            {
                // remove argument from argv
                for(; i<argc-1; ++i)
                {
                    argv[i] = argv[i+1];
                }
                --argc;
                return true;
            }
        }
        return false;
    }


    template<typename T>
    bool read(int& argc, char** argv, const std::string& match, T& value)
    {
        for(int i=1; i<argc; ++i)
        {
            if (match==argv[i])
            {
                if (i+1<argc)
                {
                    T local;
                    std::istringstream str(argv[i+1]);
                    str >> local;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+1]<<std::endl;
                        return false;
                    }

                    value = local;

                    // remove argument from argv
                    for(; i<argc-2; ++i)
                    {
                        argv[i] = argv[i+2];
                    }
                    argc -= 2;

                    return true;
                }
                else
                {
                    std::cout<<"Warning : Not enough parameters to match : "<<match<<" <value>"<<std::endl;
                }
            }
        }
        return false;
    }

    template<typename T1, typename T2>
    bool read(int& argc, char** argv, const std::string& match, T1& value1, T2& value2)
    {
        for(int i=1; i<argc; ++i)
        {
            if (match==argv[i])
            {
                if (i+2<argc)
                {
                    std::istringstream str(argv[i+1]);

                    T1 local1;
                    str >> local1;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+1]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+2]);

                    T2 local2;
                    str >> local2;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+2]<<std::endl;
                        return false;
                    }

                    value1 = local1;
                    value2 = local2;

                    // remove argument from argv
                    for(; i<argc-3; ++i)
                    {
                        argv[i] = argv[i+3];
                    }
                    argc -= 3;

                    return true;
                }
                else
                {
                    std::cout<<"Warning : Not enough parameters to match : "<<match<<" <value>"<<std::endl;
                }
            }
        }
        return false;
    }


    template<typename T1, typename T2, typename T3>
    bool read(int& argc, char** argv, const std::string& match, T1& value1, T2& value2, T3& value3)
    {
        for(int i=1; i<argc; ++i)
        {
            if (match==argv[i])
            {
                if (i+3<argc)
                {
                    std::istringstream str(argv[i+1]);

                    T1 local1;
                    str >> local1;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+1]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+2]);

                    T2 local2;
                    str >> local2;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+2]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+3]);

                    T3 local3;
                    str >> local3;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+3]<<std::endl;
                        return false;
                    }

                    value1 = local1;
                    value2 = local2;
                    value3 = local3;

                    // remove argument from argv
                    for(; i<argc-4; ++i)
                    {
                        argv[i] = argv[i+4];
                    }
                    argc -= 4;

                    return true;
                }
                else
                {
                    std::cout<<"Warning : Not enough parameters to match : "<<match<<" <value>"<<std::endl;
                }
            }
        }
        return false;
    }


    template<typename T1, typename T2, typename T3, typename T4>
    bool read(int& argc, char** argv, const std::string& match, T1& value1, T2& value2, T3& value3, T4& value4)
    {
        for(int i=1; i<argc; ++i)
        {
            if (match==argv[i])
            {
                if (i+4<argc)
                {
                    std::istringstream str(argv[i+1]);

                    T1 local1;
                    str >> local1;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+1]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+2]);

                    T2 local2;
                    str >> local2;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+2]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+3]);

                    T3 local3;
                    str >> local3;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+3]<<std::endl;
                        return false;
                    }

                    str.clear();
                    str.str(argv[i+4]);

                    T4 local4;
                    str >> local4;
                    if (!str)
                    {
                        std::cout<<"Warning : error reading command line parameter : "<<argv[i+4]<<std::endl;
                        return false;
                    }

                    value1 = local1;
                    value2 = local2;
                    value3 = local3;
                    value4 = local4;

                    // remove argument from argv
                    for(; i<argc-5; ++i)
                    {
                        argv[i] = argv[i+5];
                    }
                    argc -= 5;

                    return true;
                }
                else
                {
                    std::cout<<"Warning : Not enough parameters to match : "<<match<<" <value>"<<std::endl;
                }
            }
        }
        return false;
    }

    void print(int& argc, char** argv)
    {
        std::cout<<"Arguments argc="<<argc<<std::endl;
        for(int i=0; i<argc; ++i)
        {
            std::cout<<"  argc["<<i<<"] "<<argv[i]<<std::endl;
        }
    }

} // CommandLine


} // vsg
