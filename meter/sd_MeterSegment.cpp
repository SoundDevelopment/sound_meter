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


namespace sd  // NOLINT
{
namespace SoundMeter
{

#include "sd_MeterSegment.h"


Segment::Segment (MeterOptions meterOptions, SegmentOptions segmentOptions)
{
    setSegmentOptions (segmentOptions);
    setMeterOptions (meterOptions);
}
//==============================================================================

void Segment::setMinimalMode (bool minimalMode)
{
    m_minimalModeActive = false;
    if (m_meterOptions.useMinimalMode && minimalMode)
        m_minimalModeActive = true;
    m_isDirty = true;
}
//==============================================================================

void Segment::setSegmentOptions (SegmentOptions segmentOptions) noexcept
{
    // Check level range validity.
    jassert (segmentOptions.levelRange.getLength() > 0.0f);  // NOLINT
    // Check meter range validity (0.0f - 1.0f).
    jassert (segmentOptions.meterRange.getStart() >= 0.0f && segmentOptions.meterRange.getEnd() <= 1.0f && segmentOptions.meterRange.getLength() > 0.0f);  // NOLINT

    m_segmentOptions = segmentOptions;

    if (!m_meterBounds.isEmpty())
        setMeterBounds (m_meterBounds);

    m_isDirty = true;
}
//==============================================================================

void Segment::draw (juce::Graphics& g)
{
    m_isDirty = false;

    if (!m_meterOptions.tickMarksOnTop)
        drawTickMarks (g);

    if (!m_drawnBounds.isEmpty())
    {
        g.setColour (m_segmentOptions.segmentColour);
        g.fillRect (m_drawnBounds);
    }

    if (m_meterOptions.enablePeakHold && !m_peakHoldBounds.isEmpty())
    {
        g.setColour (m_meterOptions.peakHoldColour);
        g.fillRect (m_peakHoldBounds);
        m_drawnPeakHoldBounds = m_peakHoldBounds;
    }

    if (m_meterOptions.tickMarksOnTop)
        drawTickMarks (g);
}
//==============================================================================

void SoundMeter::Segment::drawTickMarks (juce::Graphics& g)
{
    if (m_minimalModeActive)
        return;

    g.setColour (m_meterOptions.tickMarkColour);
    for (const auto& tickMark: m_tickMarks)
    {
        const auto tickMarkLevelRatio = (tickMark - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength();
        const auto tickMarkY          = m_segmentBounds.getY() + m_segmentBounds.getHeight() - (m_segmentBounds.getHeight() * tickMarkLevelRatio);
        const auto tickMarkBounds     = juce::Rectangle<int> (m_segmentBounds.getX(), juce::roundToInt (tickMarkY) - Constants::kTickMarkHeight,
                                                          m_segmentBounds.getWidth(), Constants::kTickMarkHeight);
        g.fillRect (tickMarkBounds);
    }
}
//==============================================================================

void Segment::setMeterBounds (juce::Rectangle<int> meterBounds)
{
    m_meterBounds            = meterBounds;
    const auto segmentBounds = meterBounds.withY (meterBounds.getY() + meterBounds.proportionOfHeight (1.0f - m_segmentOptions.meterRange.getEnd()))
                                 .withHeight (meterBounds.proportionOfHeight (m_segmentOptions.meterRange.getLength()));
    if (segmentBounds == m_segmentBounds)
        return;

    m_segmentBounds = segmentBounds;
    updateLevelBounds();
}
//==============================================================================

void Segment::setLevel (float level_db)
{
    if (level_db != m_currentLevel_db)
    {
        m_currentLevel_db = level_db;
        updateLevelBounds();
    }

    if (level_db > m_peakHoldLevel_db)
    {
        m_peakHoldLevel_db = level_db;
        updatePeakHoldBounds();
    }
}
//==============================================================================

void Segment::updateLevelBounds()
{
    if (m_segmentBounds.isEmpty())
        return;

    const auto levelRatio  = std::clamp ((m_currentLevel_db - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
    const auto levelBounds = m_segmentBounds.withTop (m_segmentBounds.getY() + m_segmentBounds.proportionOfHeight (1.0f - levelRatio));

    if (m_drawnBounds == levelBounds)
        return;

    m_drawnBounds = levelBounds;
    m_isDirty     = true;
}
//==============================================================================

void Segment::updatePeakHoldBounds()
{
    juce::Rectangle<int> peakHoldBounds {};
    if (m_peakHoldLevel_db > m_segmentOptions.levelRange.getStart() && m_peakHoldLevel_db <= m_segmentOptions.levelRange.getEnd())
    {
        const auto peakHoldRatio = std::clamp ((m_peakHoldLevel_db - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
        peakHoldBounds =
          m_segmentBounds.withTop (m_segmentBounds.getY() + m_segmentBounds.proportionOfHeight (1.0f - peakHoldRatio)).withHeight (Constants::kPeakHoldHeight);
    }

    if (peakHoldBounds == m_drawnPeakHoldBounds)
        return;

    m_peakHoldBounds = peakHoldBounds;
    m_isDirty        = true;
}
//==============================================================================

void Segment::resetPeakHold() noexcept
{
    m_peakHoldBounds.setHeight (0);
    m_peakHoldLevel_db    = Constants::kMinLevel_db;
    m_drawnPeakHoldBounds = m_peakHoldBounds;
    m_isDirty             = true;
}
//==============================================================================

void Segment::setMeterOptions (MeterOptions meterOptions) noexcept
{
    m_meterOptions = meterOptions;

    // Find all tickMark-marks in this segment's range...
    m_tickMarks.clear();
    for (const auto& tickMark: meterOptions.tickMarks)
    {
        if (tickMark > m_segmentOptions.levelRange.getStart() && tickMark <= m_segmentOptions.levelRange.getEnd())
            m_tickMarks.emplace_back (tickMark);
    }

    m_isDirty = true;
}
//==============================================================================

}  // namespace SoundMeter
}  // namespace sd
