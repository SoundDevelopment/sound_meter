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


namespace sd
{

namespace SoundMeter
{

//==============================================================================

void Level::drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const
{
    if (! m_valueVisible) return;
    if (m_valueBounds.isEmpty()) return;

    // Draw PEAK value...
    const auto peak = getPeakHoldLevel();
    if (m_valueBounds.getHeight() == Constants::kDefaultHeaderHeight && peak > kMin99Db)  // If active, present and enough space is available.
    {
        const float peak_db   = juce::Decibels::gainToDecibels (peak);
        const int   precision = peak_db <= -10.0f ? 1 : 2;  // Set precision depending on peak value. NOLINT

        g.setColour (textValueColour);
        g.drawFittedText (juce::String (peak_db, precision), m_valueBounds, juce::Justification::centred, 1);
    }
}
//==============================================================================

void Level::drawPeakHold (juce::Graphics& g, const juce::Colour& peakHoldColour) const
{
    using namespace Constants;

    if (! m_options.showPeakHoldIndicator) return;

    // Calculate peak hold y coordinate...
    const int y = m_meterBounds.getY() + static_cast<int> (m_meterBounds.getHeight() * (1.0f - m_peakHoldLevel));

    if (y + kPeakHoldHeight < m_meterBounds.getHeight())
    {
        g.setColour (peakHoldColour);
        g.fillRect (m_meterBounds.getX(), y, m_meterBounds.getWidth(), kPeakHoldHeight);
    }
}
//==============================================================================

void Level::drawMeter (juce::Graphics& g) const
{
    m_normalSegment.draw (g, m_options.useGradient);
    m_warningSegment.draw (g, m_options.useGradient);
    m_peakSegment.draw (g, m_options.useGradient);
}
//==============================================================================

void Level::drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const
{
    // Check if there is space enough to write the 'MUTE' text...
    if (m_meterBounds.getWidth() < (g.getCurrentFont().getHeight())) return;

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
    if (! m_tickMarksVisible || ! m_options.tickMarksEnabled) return;

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
    if (! m_tickMarksVisible) return;

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

void Level::setTickMarks (const std::vector<float>& ticks) noexcept
{
    m_tickMarks.clear();
    for (const auto& tick: ticks)
        m_tickMarks.emplace_back (Tick (tick));
}
//==============================================================================

[[nodiscard]] float Level::getInputLevel() noexcept
{
    m_inputLevelRead.store (true);
    return juce::jlimit (-m_maxLevel, m_maxLevel, m_inputLevel.load());
}
//==============================================================================

void Level::setInputLevel (float newLevel) noexcept
{
    m_inputLevel.store (m_inputLevelRead.load() ? newLevel : std::max (m_inputLevel.load(), newLevel));
    m_inputLevelRead.store (false);
}
//==============================================================================

void Level::setColours (const juce::Colour& normalColour, const juce::Colour& warningColour, const juce::Colour& peakColour) noexcept
{
    m_normalSegment.setColours (normalColour, warningColour);
    m_warningSegment.setColours (warningColour, peakColour);
    m_peakSegment.setColours (peakColour, peakColour.darker());
}
//==============================================================================

[[nodiscard]] juce::Rectangle<int> Level::calculateMeterLevel (float newLevel) noexcept
{
    m_meterLevel    = (newLevel > m_meterLevel ? newLevel : getDecayedLevel (newLevel));
    m_peakHoldLevel = std::max<float> (m_peakHoldLevel, newLevel);

    m_normalSegment.setLevel (m_meterLevel);
    m_warningSegment.setLevel (m_meterLevel);
    m_peakSegment.setLevel (m_meterLevel);

    m_dirtyRect = {};
    if (m_normalSegment.isDirty()) m_dirtyRect = m_normalSegment.getSegmentBounds();
    if (m_warningSegment.isDirty()) m_dirtyRect = m_dirtyRect.getUnion (m_warningSegment.getSegmentBounds());
    if (m_peakSegment.isDirty()) m_dirtyRect = m_dirtyRect.getUnion (m_peakSegment.getSegmentBounds());

    return m_dirtyRect;
}
//==============================================================================

void Level::setOptions (Options meterOptions)
{
    setDecay (meterOptions.decayTime_ms);
    defineSegments (meterOptions.warningSegment_db, meterOptions.peakSegment_db);
    setTickMarks (meterOptions.tickMarks);
    enableTickMarks (meterOptions.tickMarksEnabled);
    setDecay (meterOptions.decayTime_ms);
    useGradients (meterOptions.useGradient);
    setRefreshRate (meterOptions.refreshRate);
    showPeakHold (meterOptions.showPeakHoldIndicator);
    enableValue( meterOptions.valueEnabled);

    m_options = meterOptions;
}
//==============================================================================

void Level::setRefreshRate (float refreshRate_hz) noexcept
{
    if (refreshRate_hz <= 0.0f) return;

    m_options.refreshRate = refreshRate_hz;
    m_refreshPeriod_ms    = (1.0f / refreshRate_hz) * 1000.0f;
    calculateDecayCoeff();
}
//==============================================================================

void Level::setDecay (float decay_ms) noexcept
{
    m_options.decayTime_ms = juce::jlimit (Constants::kMinDecay_ms, Constants::kMaxDecay_ms, decay_ms);
    calculateDecayCoeff();
}
//==============================================================================

void Level::defineSegments (const float warningSegment_db, const float peakSegment_db)
{
    jassert (peakSegment_db > warningSegment_db);  // NOLINT
    if (peakSegment_db <= warningSegment_db) return;

    const auto warningSegmentLevel = juce::Decibels::decibelsToGain (warningSegment_db);
    const auto peakSegmentLevel    = juce::Decibels::decibelsToGain (peakSegment_db);

    m_normalSegment.setRange (0.0f, warningSegmentLevel);
    m_warningSegment.setRange (warningSegmentLevel, peakSegmentLevel);
    m_peakSegment.setRange (peakSegmentLevel, 1.0f);
}
//==============================================================================

void Level::reset() noexcept
{
    m_inputLevel.store (0.0f);
    m_meterLevel          = 0.0f;
    m_previousRefreshTime = 0;
}
//==============================================================================

[[nodiscard]] float Level::getDecayedLevel (const float callbackLevel)
{
    // Measure time passed...
    const auto currentTime = static_cast<int> (juce::Time::getMillisecondCounter());
    const auto timePassed  = static_cast<int> (juce::Time::getMillisecondCounter()) - m_previousRefreshTime;

    // A new frame is not needed yet, return the current value...
    if (timePassed < m_refreshPeriod_ms) return m_meterLevel;

    m_previousRefreshTime = currentTime;

    // More time has passed then the meter decay. The meter has fully decayed...
    if (timePassed > m_options.decayTime_ms) return callbackLevel;

    if (m_meterLevel == callbackLevel) return callbackLevel;

    // Convert that to refreshed frames...
    auto numberOfFramePassed = static_cast<int> (std::round ((timePassed * m_options.refreshRate) / 1000.0f));

    auto level = m_meterLevel;
    for (int frame = 0; frame < numberOfFramePassed; ++frame)
        level = callbackLevel + (m_decayCoeff * (level - callbackLevel));

    if (std::abs (level - callbackLevel) < kMinMeter_db) level = callbackLevel;

    return level;
}
//==============================================================================

void Level::setMeterBounds (juce::Rectangle<int> bounds) noexcept
{
    m_meterBounds = bounds;
    m_normalSegment.setMeterBounds (bounds);
    m_warningSegment.setMeterBounds (bounds);
    m_peakSegment.setMeterBounds (bounds);
}
//==============================================================================

void Level::calculateDecayCoeff() noexcept
{
    // Rises to 99% of in value over duration of time constant.
    m_decayCoeff = std::pow (0.01f, (1000.0f / (m_options.decayTime_ms * m_options.refreshRate)));  // NOLINT
}
//==============================================================================

bool Level::isMouseOverValue (const int y) noexcept
{
    m_mouseOverValue = (y >= m_valueBounds.getY() && ! m_valueBounds.isEmpty());
    return m_mouseOverValue;
}

}  // namespace SoundMeter
}  // namespace sd
