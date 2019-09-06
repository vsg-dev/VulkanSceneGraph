/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/DatabasePager.h>
#include <vsg/io/read.h>

#include <iostream>

using namespace vsg;

DatabasePager::DatabasePager()
{
}

DatabasePager::~DatabasePager()
{
}

void DatabasePager::request(ref_ptr<PagedLOD> plod)
{
    std::cout<<"DatabasePager::reqquest("<<plod.get()<<") "<<plod->filename<<", "<<plod->priority<<std::endl;

    auto subgraph = vsg::read_cast<vsg::Node>(plod->filename);

    if (subgraph)
    {
        // compiling subgarph
        if (compileTraversal)
        {
            subgraph->accept(*compileTraversal);
            compileTraversal->context.dispatchCommands();

        }

        //std::cout<<"   assigned subgraph to plod"<<std::endl;
        plod->getChild(0).node = subgraph;

    }
}

void DatabasePager::updateSceneGraph()
{
    //std::cout<<"DatabasePager::updateSceneGraph()"<<std::endl;
}

