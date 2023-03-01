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

//==============================================================================

void Level::drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const
{
    if (!m_valueVisible)
        return;
    if (m_valueBounds.isEmpty())
        return;

    // Draw PEAK value...
    const auto peak_db = getPeakHoldLevel();
    if (m_valueBounds.getHeight() == Constants::kDefaultHeaderHeight && peak_db > Constants::kMinLevel_db)  // If active, present and enough space is available.
    {
        const int precision = peak_db <= -10.0f ? 1 : 2;  // Set precision depending on peak value. NOLINT
        g.setColour (textValueColour);
        g.drawFittedText (juce::String (peak_db, precision), m_valueBounds, juce::Justification::centred, 1);
    }
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

void Level::drawMeter (juce::Graphics& g)
{
    for (auto& segment: m_segments)
        segment.draw (g);
}
//==============================================================================

void Level::drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const
{
    // Check if there is space enough to write the 'MUTE' text...
    if (m_meterBounds.getWidth() < (g.getCurrentFont().getHeight()))
        return;

    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::halfPi, static_cast<float> (m_meterBounds.getCentreX()),
                                                     static_cast<float> (m_meterBounds.getY() + (m_meterBounds.getWidth() / 2.0f))));  // NOLINT
    g.setColour (textColour);

    g.drawText (TRANS ("MUTE"), m_meterBounds.withWidth (m_meterBounds.getHeight()).withHeight (m_meterBounds.getWidth()), juce::Justification::centred);
    g.restoreState();
}
//==============================================================================

void Level::drawTickMarks (juce::Graphics& g, const juce::Colour& tickColour) const
{
    if (!m_tickMarksVisible || !m_options.tickMarksEnabled)
        return;

    g.setColour (tickColour);

    for (const auto& tick: m_tickMarks)  // Loop through all ticks.
    {
        g.fillRect (m_meterBounds.getX(),
                    m_meterBounds.getY() + static_cast<int> (round ((m_meterBounds.getHeight() * (1.0f - tick.gain)) - (Constants::kTickMarkHeight / 2.0f))),  // NOLINT
                    m_meterBounds.getWidth(), Constants::kTickMarkHeight);
    }
}
//==============================================================================

void Level::drawLabels (juce::Graphics& g, const juce::Colour& textColour) const
{
    if (!m_tickMarksVisible)
        return;

    g.setColour (textColour);
    const float fontsize = juce::jlimit (1.0f, 15.0f, m_meterBounds.getHeight() / 4.0f);  // Set font size proportionally. NOLINT
    g.setFont (fontsize);

    for (const auto& tick: m_tickMarks)
    {
        const juce::Rectangle<int> labelrect (m_meterBounds.getX(),
                                              m_meterBounds.getY() + static_cast<int> (round ((m_meterBounds.getHeight() * (1.0f - tick.gain)) - (fontsize / 2.0f))),  // NOLINT
                                              m_meterBounds.getWidth(), static_cast<int> (fontsize));

        g.drawFittedText (juce::String (std::abs (tick.decibels)), labelrect.reduced (Constants::kLabelStripTextPadding, 0), juce::Justification::topLeft, 1);
    }
}
//==============================================================================

void Level::setTickMarks (const std::vector<float>& ticks)
{
    m_tickMarks.clear();
    for (const auto& tick: ticks)
        m_tickMarks.emplace_back (Tick (tick));
}
//==============================================================================

void Level::useGradients (bool gradientsUsed) noexcept
{
    for (auto& segment: m_segments)
        segment.useGradients (gradientsUsed);
}
//==============================================================================

float Level::getInputLevel()
{
    m_inputLevelRead.store (true);
    return juce::jlimit (Constants::kMinLevel_db, Constants::kMaxLevel_db, juce::Decibels::gainToDecibels (m_inputLevel.load()));
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

void Level::setOptions (const Options& meterOptions)
{
    setDecay (meterOptions.decayTime_ms);
    setTickMarks (meterOptions.tickMarks);
    enableTickMarks (meterOptions.tickMarksEnabled);
    setDecay (meterOptions.decayTime_ms);
    setRefreshRate (meterOptions.refreshRate);
    enableValue (meterOptions.valueEnabled);
    defineSegments (meterOptions.segmentOptions);

    m_options = meterOptions;
}
//==============================================================================

void Level::setRefreshRate (float refreshRate_hz)
{
    if (refreshRate_hz <= 0.0f)
        return;

    m_options.refreshRate = refreshRate_hz;
    m_refreshPeriod_ms    = (1.0f / refreshRate_hz) * 1000.0f;
    calculateDecayCoeff();
}
//==============================================================================

void Level::setDecay (float decay_ms)
{
    m_options.decayTime_ms = juce::jlimit (Constants::kMinDecay_ms, Constants::kMaxDecay_ms, decay_ms);
    calculateDecayCoeff();
}
//==============================================================================

void Level::enablePeakHold (bool isVisible) noexcept
{
    for (auto& segment: m_segments)
        segment.enablePeakHold (isVisible);
}
//==============================================================================

void Level::defineSegments (const std::vector<SegmentOptions>& segmentsOptions)
{
    m_segments.clear();
    for (const auto& segmentOptions: segmentsOptions)
        m_segments.emplace_back (segmentOptions);
}
//==============================================================================

bool Level::isPeakHoldEnabled() const noexcept
{
    if (m_segments.empty())
        return false;
    return m_segments[0].getOptions().enablePeakHold;
}
//==============================================================================

void Level::reset()
{
    m_inputLevel.store (0.0f);
    m_meterLevel_db       = Constants::kMinLevel_db;
    m_previousRefreshTime = 0;
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
    if (timePassed > m_options.decayTime_ms)
        return newLevel_db;

    if (m_meterLevel_db == newLevel_db)
        return newLevel_db;

    // Convert that to refreshed frames...
    auto numberOfFramePassed = static_cast<int> (std::round ((timePassed * m_options.refreshRate) / 1000.0f));  // NOLINT

    auto level_db = m_meterLevel_db;
    for (int frame = 0; frame < numberOfFramePassed; ++frame)
        level_db = newLevel_db + (m_decayCoeff * (level_db - newLevel_db));

    if (std::abs (level_db - newLevel_db) < Constants::kMinLevel_db)
        level_db = newLevel_db;

    return level_db;
}
//==============================================================================

void Level::resetPeakHold()
{
    for (auto& segment: m_segments)
        segment.resetPeakHold();
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
    m_meterBounds = bounds;
    for (auto& segment: m_segments)
        segment.setMeterBounds (bounds);
}
//==============================================================================

void Level::calculateDecayCoeff()
{
    // Rises to 99% of in value over duration of time constant.
    m_decayCoeff = std::pow (0.01f, (1000.0f / (m_options.decayTime_ms * m_options.refreshRate)));  // NOLINT
}
//==============================================================================

bool Level::isMouseOverValue (const int y)
{
    m_mouseOverValue = (y >= m_valueBounds.getY() && !m_valueBounds.isEmpty());
    return m_mouseOverValue;
}

}  // namespace SoundMeter
}  // namespace sd
