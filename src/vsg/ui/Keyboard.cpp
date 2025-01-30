/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/ui/Keyboard.h>

using namespace vsg;

void Keyboard::apply(KeyPressEvent& keyPress)
{
    //std::cout<<"Keyboard::apply(KeyPressEvent& keyPress)"<<std::endl;

    auto keyState_itr = keyState.find(keyPress.keyBase);
    if (keyState_itr != keyState.end())
    {
        auto& keyHistory = keyState_itr->second;
        keyHistory.handled = keyPress.handled;
        if (keyHistory.timeOfKeyRelease == keyPress.time || keyHistory.timeOfKeyRelease <= keyHistory.timeOfLastKeyPress)
        {
            keyHistory.timeOfKeyRelease = keyHistory.timeOfFirstKeyPress;
            keyHistory.timeOfLastKeyPress = keyPress.time;
            //std::cout<<"key repeated "<<std::chrono::duration<double, std::chrono::milliseconds::period>(keyPress.time - keyHistory.timeOfFirstKeyPress).count()<<"ms"<<std::endl;
        }
        else
        {
            keyHistory.timeOfFirstKeyPress = keyPress.time;
            keyHistory.timeOfLastKeyPress = keyPress.time;
            keyHistory.timeOfKeyRelease = keyPress.time;
            //std::cout<<"key reusing first press "<<std::chrono::duration<double, std::chrono::milliseconds::period>(keyPress.time - keyHistory.timeOfFirstKeyPress).count()<<"ms"<<std::endl;
        }
    }
    else
    {
        auto& keyHistory = keyState[keyPress.keyBase];
        keyHistory.handled = keyPress.handled;
        keyHistory.timeOfFirstKeyPress = keyPress.time;
        keyHistory.timeOfLastKeyPress = keyPress.time;
        keyHistory.timeOfKeyRelease = keyPress.time;
        //std::cout<<"key first press "<<std::chrono::duration<double, std::chrono::milliseconds::period>(keyPress.time - keyHistory.timeOfFirstKeyPress).count()<<"ms"<<std::endl;
    }
}

void Keyboard::apply(KeyReleaseEvent& keyRelease)
{
    //std::cout<<"Keyboard::apply(KeyReleaseEvent& keyRelease)"<<std::endl;

    auto keyState_itr = keyState.find(keyRelease.keyBase);
    if (keyState_itr != keyState.end())
    {
        auto& keyHistory = keyState_itr->second;
        keyHistory.handled = keyRelease.handled;
        keyHistory.timeOfKeyRelease = keyRelease.time;
        //std::cout<<"key provisionally released "<<std::chrono::duration<double, std::chrono::milliseconds::period>(keyRelease.time - keyHistory.timeOfFirstKeyPress).count()<<"ms"<<std::endl;
    }
}

void Keyboard::apply(FocusInEvent&)
{
}

void Keyboard::apply(FocusOutEvent& focusOut)
{
    focusOut.handled = true;
    keyState.clear();
}

bool Keyboard::pressed(KeySymbol key, bool ignore_handled_keys) const
{
    auto itr = keyState.find(key);
    if (itr == keyState.end()) return false;

    const auto& keyHistory = itr->second;
    if (keyHistory.timeOfKeyRelease != keyHistory.timeOfFirstKeyPress)
    {
        return false;
    }

    if (ignore_handled_keys && keyHistory.handled) return false;

    return true;
}

std::pair<double, double> Keyboard::times(KeySymbol key, bool ignore_handled_keys) const
{
    auto itr = keyState.find(key);
    if (itr == keyState.end()) return {-1.0, -1.0};

    auto currentTime = clock::now();

    const auto& keyHistory = itr->second;
    if (keyHistory.timeOfKeyRelease != keyHistory.timeOfFirstKeyPress)
    {
        return {std::chrono::duration<double, std::chrono::seconds::period>(currentTime - keyHistory.timeOfFirstKeyPress).count(),
                std::chrono::duration<double, std::chrono::seconds::period>(currentTime - keyHistory.timeOfLastKeyPress).count()};
    }

    if (ignore_handled_keys && keyHistory.handled) return {-1.0, -1.0};

    return {std::chrono::duration<double, std::chrono::seconds::period>(currentTime - keyHistory.timeOfFirstKeyPress).count(), 0.0};
}
