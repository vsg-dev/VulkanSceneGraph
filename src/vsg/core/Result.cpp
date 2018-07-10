
#include <vsg/core/Result.h>

#include <iostream>

namespace vsg
{

    std::ostream& notice_stream() { return std::cout; }
    std::ostream& error_stream() { return std::cerr; }

}
