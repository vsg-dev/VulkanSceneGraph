#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2026 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <fstream>

#include <vsg/vk/Device.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    /// Inputs to #pragma(tic) shader composition
    class VSG_DECLSPEC DefineMap
    {
    public:
        using Map = std::map<std::string, std::string>;
        using key_type = Map::key_type;
        using mapped_type = Map::mapped_type;
        using value_type = Map::value_type;
        using size_type = Map::size_type;
        using iterator = Map::iterator;
        using const_iterator = Map::const_iterator;

        DefineMap();
        DefineMap(const DefineMap& rhs);

        Map values;

        DefineMap& operator=(const DefineMap& rhs)
        {
            if (&rhs == this) return *this;
            values = rhs.values;
            return *this;
        }

        DefineMap& operator=(const std::set<std::string>& rhs)
        {
            values.clear();
            for (const auto& def : rhs) insert(def);
            return *this;
        }

        bool operator<(const DefineMap& rhs) const { return values < rhs.values; }
        bool operator==(const DefineMap& rhs) const { return values == rhs.values; }

        Map::size_type count(const std::string& def) const { return values.count(def); }

        void clear() { values.clear(); }
        bool empty() const { return values.empty(); }
        size_type size() const { return values.size(); }

        iterator begin() { return values.begin(); }
        iterator end() { return values.end(); }

        const_iterator begin() const { return values.begin(); }
        const_iterator end() const { return values.end(); }

        std::string& operator[](const std::string& def) { return values[def]; }

        void insert(const std::string& str) { values[str] = ""; }
        void insert(const std::string& def, const std::string& val) { values[def] = val; }
        void insert(const std::set<std::string>& defines)
        {
            for (const auto& def : defines) values[def] = "";
        }
        void insert(const DefineMap& defines)
        {
            for (auto& val : defines.values) values.insert(val);
        }

        iterator find(const std::string& str) { return values.find(str); }
        const_iterator find(const std::string& str) const { return values.find(str); }

        std::set<std::string> keys() const
        {
            std::set<std::string> key_values;
            for (auto itr = values.begin(); itr != values.end(); ++itr) key_values.insert(itr->first);
            return key_values;
        }
    };
    VSG_type_name(vsg::DefineMap);

} // namespace vsg
