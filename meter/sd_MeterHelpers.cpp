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
juce::Rectangle<int> applyPadding (const juce::Rectangle<int>& rectToPad, Padding paddingToApply) noexcept
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

static constexpr bool containsUpTo (juce::Range<float> levelRange, float levelDb) noexcept
{
    return levelDb > levelRange.getStart() && levelDb <= levelRange.getEnd();
}

}  // namespace Helpers
//==============================================================================

std::vector<SegmentOptions> MeterScales::getDefaultScale()
{
    return getDefaultScale (juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getSmpteScale()
{
    return getSmpteScale (juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getSmpteOptions (Options options)
{
    options.tickMarks           = { 0.0f, -3.0f, -6.0f, -9.0f, -12.0f, -15.0f, -20.0f, -25.0f, -30.0f, -35.0f, -40.0f };
    options.decayTime_ms        = 2250.0f;
    options.defaultDecayTime_ms = 2250.0f;  // 20 dB/s.
    options.useGradient         = false;
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getEbuPpmScale()
{
    return getEbuPpmScale (juce::Colours::green, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getEbuPpmOptions (Options options)
{
    options.tickMarks           = { -12.0f, -16.0f, -20.0f, -24.0f, -28.0f, -32.0f, -36.0f };
    options.nominalLevel_db     = -24.0f;
    options.decayTime_ms        = 2100.0f;
    options.defaultDecayTime_ms = 2100.0f;  // 13.333 dB/s.
    options.useGradient         = false;
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getExtendedBottomScale()
{
    return getExtendedBottomScale (juce::Colours::navy, juce::Colours::white, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getExtendedBottomOptions (Options options)
{
    options.tickMarks           = { 0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -60.0f, -70.0f, -80.0f, -90.0f };
    options.decayTime_ms        = 4800.0f;
    options.defaultDecayTime_ms = 4800.0f;  // 20 dB/s.
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getExtendedTopScale()
{
    return getExtendedBottomScale (juce::Colours::navy, juce::Colours::white, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getExtendedTopOptions (Options options)
{
    options.tickMarks           = { 20.0f, 15.0f, 10.0f, 5.0f, 0.0f, -5.0f, -10.0f, -20.0f, -30.0f, -40.0f };
    options.decayTime_ms        = 4800.0f;
    options.defaultDecayTime_ms = 4800.0f;  // 20 dB/s.
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getFullRangeScale()
{
    return getFullRangeScale (juce::Colours::navy, juce::Colours::white, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getFullRangeOptions (Options options)
{
    options.tickMarks           = { 100.0f, 80.0f, 60.0f, 40.0f, 20.0f, 0.0f, -20.0f, -40.0f, -60.0f, -80.0f };
    options.decayTime_ms        = 4800.0f;
    options.defaultDecayTime_ms = 4800.0f;  // 20 dB/s.
    return options;
}
//==============================================================================

Options MeterScales::getK20Options (Options options)
{
    options.tickMarks           = { 0.0f, -4.0f, -8.0f, -12.0f, -16.0f, -20.0f, -24.0f, -28.0f, -32.0f, -36.0f, -40.0f, -44.0f };
    options.nominalLevel_db     = -20.0f;
    options.decayTime_ms        = 3666.7f;  // 12 dB/s.
    options.defaultDecayTime_ms = 3666.7f;
    options.useGradient         = false;
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getK20Scale()
{
    return getK20Scale (juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getK14Options (Options options)
{
    options.tickMarks           = { 0.0f, -6.0f, -10.0f, -14.0f, -18.0f, -22.0f, -26.0f, -30.0f, -34.0f, -38.0f };
    options.nominalLevel_db     = -14.0f;
    options.decayTime_ms        = 3166.7f;  // 12 dB/s.
    options.defaultDecayTime_ms = 3166.7f;
    options.useGradient         = false;
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getK14Scale()
{
    return getK14Scale (juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
}
//==============================================================================

Options MeterScales::getK12Options (Options options)
{
    options.tickMarks           = { 0.0f, -4.0f, -8.0f, -12.0f, -16.0f, -20.0f, -24.0f, -28.0f, -32.0f, -36.0f };
    options.nominalLevel_db     = -12.0f;
    options.decayTime_ms        = 3000.0f;  // 12 dB/s.
    options.defaultDecayTime_ms = 3000.0f;
    options.useGradient         = false;
    return options;
}
//==============================================================================

std::vector<SegmentOptions> MeterScales::getK12Scale()
{
    return getK12Scale (juce::Colours::green, juce::Colours::yellow, juce::Colours::red);
}
//==============================================================================

// namespace Helpers

}  // namespace SoundMeter
}  // namespace sd