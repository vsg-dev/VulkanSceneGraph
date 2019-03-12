/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/MatrixTransform.h>

#include <iostream>

using namespace vsg;

MatrixTransform::MatrixTransform(Allocator* allocator) :
    Inherit(allocator)
{
    _matrix = new mat4Value;
    _pushConstant = vsg::PushConstants::create(VK_SHADER_STAGE_VERTEX_BIT, 128, _matrix);
}

MatrixTransform::MatrixTransform(const mat4& matrix, Allocator* allocator) :
    Inherit(allocator)
{
    _matrix = new mat4Value(matrix);
    _pushConstant = vsg::PushConstants::create(VK_SHADER_STAGE_VERTEX_BIT, 128, _matrix);
}

void MatrixTransform::read(Input& input)
{
    Group::read(input);

    input.read("Matrix", _matrix->value());
}

void MatrixTransform::write(Output& output) const
{
    Group::write(output);

    output.write("Matrix", _matrix->value());
}
