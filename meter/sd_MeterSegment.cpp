#include "sd_MeterSegment.h"
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


Segment::Segment (SegmentOptions options) : m_options (options)
{
    // Check level range validity.
    jassert (options.levelRange.getLength() > 0.0f);  // NOLINT
    // Check meter range validity (0.0f - 1.0f).
    jassert (options.meterRange.getStart() >= 0.0f && options.meterRange.getEnd() <= 1.0f && options.meterRange.getLength() > 0.0f);  // NOLINT
}
//==============================================================================

void SoundMeter::Segment::draw (juce::Graphics& g)
{
    m_isDirty = false;
    if (!m_drawnBounds.isEmpty())
    {
        g.setColour (m_options.segmentColour);
        g.fillRect (m_drawnBounds);
    }

    if (m_options.enablePeakHold && !m_peakHoldBounds.isEmpty())
    {
        g.setColour (m_options.peakHoldColour);
        g.fillRect (m_peakHoldBounds);
        m_drawnPeakHoldBounds = m_peakHoldBounds;
    }
}
//==============================================================================

void Segment::setMeterBounds (juce::Rectangle<int> meterBounds)
{
    const auto segmentBounds = meterBounds.withY (meterBounds.getY() + meterBounds.proportionOfHeight (1.0f - m_options.meterRange.getEnd()))
                                 .withHeight (meterBounds.proportionOfHeight (m_options.meterRange.getLength()));
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

    const auto levelRatio  = std::clamp ((m_currentLevel_db - m_options.levelRange.getStart()) / m_options.levelRange.getLength(), 0.0f, 1.0f);
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
    if (m_peakHoldLevel_db > m_options.levelRange.getStart() && m_peakHoldLevel_db <= m_options.levelRange.getEnd())
    {
        const auto peakHoldRatio = std::clamp ((m_peakHoldLevel_db - m_options.levelRange.getStart()) / m_options.levelRange.getLength(), 0.0f, 1.0f);
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

void Segment::setColours (const juce::Colour& segmentColour, const juce::Colour& nextSegmentColour)
{
    m_options.segmentColour     = segmentColour;
    m_options.nextSegmentColour = nextSegmentColour;
}

//==============================================================================

}  // namespace SoundMeter
}  // namespace sd
