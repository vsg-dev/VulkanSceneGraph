#pragma once

#include <atomic>

namespace vsg
{

    class Object
    {
    public:
        Object();

        void ref() const;

        void unref() const;

        void unref_nodelete() const;

    protected:
        virtual ~Object();

        mutable std::atomic_uint _referenceCount;
    };

}