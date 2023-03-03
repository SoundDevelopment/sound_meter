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


#include "sd_MeterLevel.h"

namespace sd  // NOLINT
{

namespace SoundMeter
{

Level::Level()
{
    setMeterSegments (m_segmentOptions);
}
//==============================================================================

void Level::drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const
{
    if (m_valueBounds.isEmpty())
        return;

    // Draw PEAK value...
    const auto peak_db = getPeakHoldLevel();
    if (peak_db > m_minLevel_db)  // If active, present and enough space is available.
    {
        const int precision = peak_db <= -10.0f ? 1 : 2;  // Set precision depending on peak value. NOLINT
        g.setColour (textValueColour);
        g.drawFittedText (juce::String (peak_db, precision), m_valueBounds, juce::Justification::centred, 1);
    }
}
//==============================================================================

void Level::drawMeter (juce::Graphics& g)
{
    for (auto& segment: m_segments)
        segment.draw (g);
}
//==============================================================================

void Level::drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const
{
    // Check if there is space enough to write the 'MUTE' text...
    if (static_cast<float> (m_meterBounds.getWidth()) < (g.getCurrentFont().getHeight()))
        return;

    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::halfPi, static_cast<float> (m_meterBounds.getCentreX()),
                                                     static_cast<float> (m_meterBounds.getY() + (m_meterBounds.getWidth() / 2.0f))));  // NOLINT
    g.setColour (textColour);

    g.drawText (TRANS ("MUTE"), m_meterBounds.withWidth (m_meterBounds.getHeight()).withHeight (m_meterBounds.getWidth()), juce::Justification::centred);
    g.restoreState();
}
//==============================================================================

float Level::getInputLevel()
{
    m_inputLevelRead.store (true);
    return juce::jlimit (m_minLevel_db, Constants::kMaxLevel_db, juce::Decibels::gainToDecibels (m_inputLevel.load()));
}
//==============================================================================

void Level::setInputLevel (float newLevel)
{
    m_inputLevel.store (m_inputLevelRead.load() ? newLevel : std::max (m_inputLevel.load(), newLevel));
    m_inputLevelRead.store (false);
}
//==============================================================================

void Level::refreshMeterLevel()
{
    const auto newLevel_db = getInputLevel();
    m_meterLevel_db        = (newLevel_db > m_meterLevel_db ? newLevel_db : getDecayedLevel (newLevel_db));

    if (m_meterLevel_db > getPeakHoldLevel())
        m_peakHoldDirty = true;

    for (auto& segment: m_segments)
        segment.setLevel (m_meterLevel_db);
}
//==============================================================================

void Level::setMeterOptions (const MeterOptions& meterOptions)
{
    m_meterOptions = meterOptions;

    calculateDecayCoeff (meterOptions);
    synchronizeMeterOptions();
}
//==============================================================================

void Level::synchronizeMeterOptions()
{
    setMinimalMode (m_minimalModeActive);
    for (auto& segment: m_segments)
    {
        segment.setMeterOptions (m_meterOptions);
        segment.setIsLabelStrip (m_isLabelStrip);
        segment.setMinimalMode (m_minimalModeActive);
    }
}

//==============================================================================

void Level::setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions)
{
    m_segments.clear();
    for (const auto& segmentOptions: segmentsOptions)
    {
        m_segments.emplace_back (m_meterOptions, segmentOptions);
        m_minLevel_db = std::min (m_minLevel_db, segmentOptions.levelRange.getStart());
    }

    synchronizeMeterOptions();
}
//==============================================================================

void Level::setIsLabelStrip (bool isLabelStrip) noexcept
{
    m_isLabelStrip = isLabelStrip;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::reset()
{
    m_inputLevel.store (0.0f);
    m_meterLevel_db       = Constants::kMinLevel_db;
    m_previousRefreshTime = 0;
}
//==============================================================================

void Level::useGradients (bool useGradients)
{
    m_meterOptions.useGradients = useGradients;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::showTickMarks (bool tickMarksEnabled)
{
    m_meterOptions.tickMarksEnabled = tickMarksEnabled;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::showTickMarksOnTop (bool showTickMarksOnTop)
{
    m_meterOptions.tickMarksOnTop = showTickMarksOnTop;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::setTickMarks (const std::vector<float>& tickMarks)
{
    m_meterOptions.tickMarks = tickMarks;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::showValue (bool isVisible) noexcept
{
    m_meterOptions.valueEnabled = isVisible;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::setMinimalMode (bool minimalMode)
{
    if (m_minimalModeActive == minimalMode)
        return;

    m_minimalModeActive = minimalMode;

    setMeterBounds (m_meterBounds);

    synchronizeMeterOptions();
}
//==============================================================================

void Level::setRefreshRate (float refreshRate_hz)
{
    m_meterOptions.refreshRate = refreshRate_hz;
    calculateDecayCoeff (m_meterOptions);
    synchronizeMeterOptions();
}
//==============================================================================

void Level::setDecay (float decay_ms)
{
    m_meterOptions.decayTime_ms = decay_ms;
    calculateDecayCoeff (m_meterOptions);
    synchronizeMeterOptions();
}
//==============================================================================

float Level::getDecayedLevel (const float newLevel_db)
{
    const auto currentTime = static_cast<int> (juce::Time::getMillisecondCounter());
    const auto timePassed  = static_cast<float> (currentTime - static_cast<int> (m_previousRefreshTime));

    // A new frame is not needed yet, return the current value...
    if (timePassed < m_refreshPeriod_ms)
        return m_meterLevel_db;

    m_previousRefreshTime = currentTime;

    // More time has passed then the meter decay. The meter has fully decayed...
    if (timePassed > m_meterOptions.decayTime_ms)
        return newLevel_db;

    if (m_meterLevel_db == newLevel_db)
        return newLevel_db;

    // Convert that to refreshed frames...
    auto numberOfFramePassed = static_cast<int> (std::round ((timePassed * m_meterOptions.refreshRate) / 1000.0f));  // NOLINT

    auto level_db = m_meterLevel_db;
    for (int frame = 0; frame < numberOfFramePassed; ++frame)
        level_db = newLevel_db + (m_decayCoeff * (level_db - newLevel_db));

    if (std::abs (level_db - newLevel_db) < Constants::kMinLevel_db)
        level_db = newLevel_db;

    return level_db;
}
//==============================================================================

void Level::enablePeakHold (bool enablePeakHold)
{
    m_meterOptions.enablePeakHold = enablePeakHold;
    synchronizeMeterOptions();
}
//==============================================================================

void Level::resetPeakHold()
{
    for (auto& segment: m_segments)
        segment.resetPeakHold();
    m_peakHoldDirty = true;
}
//==============================================================================

float Level::getPeakHoldLevel() const noexcept
{
    if (m_segments.empty())
        return Constants::kMinLevel_db;

    return m_segments[0].getPeakHold();
}
//==============================================================================

void Level::setMeterBounds (const juce::Rectangle<int>& bounds)
{
    if (bounds == m_meterBounds)
        return;

    m_meterBounds = bounds;
    m_levelBounds = m_meterBounds;

    // If the meter is in minimal mode, the value is not displayed...
    if (m_meterOptions.valueEnabled && !m_minimalModeActive)
        m_valueBounds = m_levelBounds.removeFromBottom (Constants::kDefaultHeaderHeight);
    else
        m_valueBounds = juce::Rectangle<int>();

    for (auto& segment: m_segments)
        segment.setMeterBounds (m_levelBounds);

    m_peakHoldDirty = true;
}
//==============================================================================

juce::Rectangle<int> Level::getDirtyBounds()
{
    juce::Rectangle<int> dirtyBounds {};
    for (const auto& segment: m_segments)
    {
        if (segment.isDirty())
            dirtyBounds = dirtyBounds.getUnion (segment.getSegmentBounds());
    }

    if (m_peakHoldDirty)
    {
        dirtyBounds     = dirtyBounds.getUnion (m_valueBounds);
        m_peakHoldDirty = false;
    }

    return dirtyBounds;
}
//==============================================================================

void Level::calculateDecayCoeff (const MeterOptions& meterOptions)
{
    m_meterOptions.decayTime_ms = juce::jlimit (Constants::kMinDecay_ms, Constants::kMaxDecay_ms, meterOptions.decayTime_ms);
    m_meterOptions.refreshRate  = std::max (1.0f, meterOptions.refreshRate);
    m_refreshPeriod_ms          = (1.0f / m_meterOptions.refreshRate) * 1000.0f;  // NOLINT

    // Rises to 99% of in value over duration of time constant.
    m_decayCoeff = std::pow (0.01f, (1000.0f / (m_meterOptions.decayTime_ms * m_meterOptions.refreshRate)));  // NOLINT
}
//==============================================================================

bool Level::isMouseOverValue (const int y)
{
    m_mouseOverValue = (y >= m_valueBounds.getY() && !m_valueBounds.isEmpty());
    return m_mouseOverValue;
}

}  // namespace SoundMeter
}  // namespace sd
