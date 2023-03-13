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

#include "sd_MeterFader.h"

namespace sd  // NOLINT
{
namespace SoundMeter
{
//==============================================================================

void Fader::flash() noexcept
{
    if (!m_enabled)
        return;

    m_fadeStart = static_cast<int> (juce::Time::getMillisecondCounter());
    m_isFading  = true;
}
//==============================================================================

void Fader::setVisible (bool visible /*= true*/) noexcept
{
    if (!m_enabled)
        return;

    // If fader needs to be HIDDEN...
    if (!visible)
    {
        // If it was visible, FADE it out...
        if (m_visible)
            m_fadeStart = static_cast<int> (juce::Time::getMillisecondCounter());

        // Hide fader...
        m_visible = false;
    }
    // ... if fader needs to be SHOWN...
    else
    {
        // Show fader...
        m_visible   = true;
        m_fadeStart = 0;
    }
}
//==============================================================================

void Fader::draw (juce::Graphics& g, const MeterColours& meterColours)
{
    m_isFading = false;

    if (!m_enabled)
        return;

    const auto timeSinceStartFade = getTimeSinceStartFade();

    // Return if the fader was already invisible and a new fade has not been started...
    if (!m_visible && timeSinceStartFade >= Constants::kFaderFadeTime_ms)
        return;

    auto alpha = Constants::kFaderAlphaMax;

    // If it's fading, calculate it's alpha...
    if (timeSinceStartFade < Constants::kFaderFadeTime_ms)
    {
        constexpr float fadePortion = 2.0f;
        alpha = juce::jlimit (0.0f, 1.0f, fadePortion - ((static_cast<float> (timeSinceStartFade) * fadePortion) / Constants::kFaderFadeTime_ms)) * Constants::kFaderAlphaMax;
        m_isFading = alpha > 0.0f;
    }

    // If the fader is not fully transparent, draw it...
    if (alpha > 0.0f)
    {
        g.setColour (meterColours.faderColour.withAlpha (alpha));
        auto faderRect    = m_bounds;
        m_drawnFaderValue = getValue();
        auto value_db     = juce::Decibels::gainToDecibels (m_drawnFaderValue);
        for (const auto& segment: m_segments)
        {
            if (Helpers::containsUpTo (segment.levelRange, value_db))
            {
                const auto valueInSegment = std::clamp ((value_db - segment.levelRange.getStart()) / segment.levelRange.getLength(), 0.0f, 1.0f);
                const auto convertedValue = juce::jmap (valueInSegment, segment.meterRange.getStart(), segment.meterRange.getEnd());
                g.fillRect (faderRect.removeFromBottom (m_bounds.proportionOfHeight (convertedValue)));
                break;
            }
        }
    }
}
//==============================================================================

void Fader::setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions)
{
    m_segments = segmentsOptions;
}
//==============================================================================

bool Fader::setValue (const float value, NotificationOptions notificationOption /*= NotificationOptions::Notify*/)  // NOLINT
{
    if (!m_enabled)
        return false;
    if (m_faderValue.load() == value)
        return false;
    m_faderValue.store (value);

#if SDTK_ENABLE_FADER
    if (notificationOption == NotificationOptions::notify && onFaderValueChanged)
        onFaderValueChanged();
#else
    juce::ignoreUnused (notificationOption);
#endif

    return true;
}
//==============================================================================

void Fader::setValueFromPos (const int position, NotificationOptions notificationOption /*= NotificationOptions::Notify*/)
{
    const auto height = static_cast<float> (m_bounds.getHeight());
    if (height <= 0.0f)
        return;

    auto value = 1.0f - std::clamp ((static_cast<float> (position) - m_bounds.getY()) / height, 0.0f, 1.0f);
    for (const auto& segment: m_segments)
    {
        if (Helpers::containsUpTo (segment.meterRange, value))
        {
            const auto valueInSegment = std::clamp ((value - segment.meterRange.getStart()) / segment.meterRange.getLength(), 0.0f, 1.0f);
            const auto value_db       = juce::jmap (valueInSegment, segment.levelRange.getStart(), segment.levelRange.getEnd());
            value                     = juce::Decibels::decibelsToGain (value_db);
            break;
        }
    }

    setValue (value, notificationOption);
}
//==============================================================================
}  // namespace SoundMeter
}  // namespace sd