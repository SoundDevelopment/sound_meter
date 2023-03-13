/*
    ==============================================================================
    
    This file is part of the sound_meter JUCE module
    Copyright (c) 2019 - 2021 Sound Development - Marcel Huibers
    All rights reserved.

    ------------------------------------------------------------------------------

    sound_meter is provided under the terms of The MIT License (MIT):

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    ==============================================================================
*/

#include "sd_MeterHelpers.h"

namespace sd  // NOLINT
{
namespace SoundMeter
{
namespace Helpers
{

//==============================================================================
[[nodiscard]] juce::Rectangle<int> applyPadding (const juce::Rectangle<int>& rectToPad, Padding paddingToApply) noexcept
{
    juce::Rectangle<int> result { rectToPad };
    if (juce::isPositiveAndBelow (paddingToApply.left, result.getWidth()))
        result.setLeft (rectToPad.getX() + paddingToApply.left);
    if (juce::isPositiveAndBelow (paddingToApply.right, result.getWidth()))
        result.setWidth (rectToPad.getWidth() - paddingToApply.right);
    if (juce::isPositiveAndBelow (paddingToApply.bottom, result.getHeight()))
        result.setHeight (rectToPad.getHeight() - paddingToApply.bottom);

    return result;
}
//==============================================================================

[[nodiscard]] static constexpr bool containsUpTo (juce::Range<float> levelRange, float levelDb) noexcept
{
    return levelDb > levelRange.getStart() && levelDb <= levelRange.getEnd();
}

}  // namespace Helpers
}  // namespace SoundMeter
}  // namespace sd