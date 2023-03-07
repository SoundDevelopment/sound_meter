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

#include "sd_MeterSegment.h"

namespace sd  // NOLINT
{
namespace SoundMeter
{
Segment::Segment (const Options& meterOptions, const SegmentOptions& segmentOptions)
{
    setSegmentOptions (segmentOptions);
    setMeterOptions (meterOptions);
}
//==============================================================================

void Segment::setMinimalMode (bool minimalMode) noexcept
{
    if (minimalMode == m_minimalModeActive)
        return;

    m_minimalModeActive = false;
    if (m_meterOptions.useMinimalMode && minimalMode)
        m_minimalModeActive = true;
    m_isDirty = true;
}
//==============================================================================

void Segment::setSegmentOptions (SegmentOptions segmentOptions)
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

void Segment::draw (juce::Graphics& g, const MeterColours& meterColours)
{
    m_isDirty = false;

    if (m_isLabelStrip)
    {
        drawLabels (g, meterColours);
        return;
    }

    if (!m_meterOptions.tickMarksOnTop)
        drawTickMarks (g, meterColours);

    if (!m_drawnBounds.isEmpty())
    {
        if (m_meterOptions.useGradient)
            g.setGradientFill (m_gradientFill);
        else
            g.setColour (m_segmentOptions.segmentColour);

        g.fillRect (m_drawnBounds);
    }

    if (m_meterOptions.tickMarksOnTop)
        drawTickMarks (g, meterColours);

    if (m_meterOptions.showPeakHoldIndicator && !m_peakHoldBounds.isEmpty())
    {
        g.setColour (meterColours.peakHoldColour);
        g.fillRect (m_peakHoldBounds);
        m_drawnPeakHoldBounds = m_peakHoldBounds;
    }
}
//==============================================================================

void Segment::drawTickMarks (juce::Graphics& g, const MeterColours& meterColours)
{
    if (m_minimalModeActive)
        return;

    g.setColour (meterColours.tickMarkColour);
    for (const auto& tickMark: m_tickMarks)
    {
        if ((tickMark <= m_currentLevel_db) && !m_meterOptions.tickMarksOnTop)
            continue;

        const auto tickMarkLevelRatio = std::clamp ((tickMark - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
        const auto tickMarkY          = m_segmentBounds.getY() + juce::roundToInt (m_segmentBounds.toFloat().proportionOfHeight (1.0f - tickMarkLevelRatio));
        const auto tickMarkBounds     = juce::Rectangle<int> (m_segmentBounds.getX(), tickMarkY, m_segmentBounds.getWidth(), Constants::kTickMarkHeight);
        g.fillRect (tickMarkBounds);
    }
}
//==============================================================================

void Segment::drawLabels (juce::Graphics& g, const MeterColours& meterColours) const
{
    g.setColour (meterColours.textColour);
    const float fontsize = juce::jlimit (1.0f, 15.0f, m_meterBounds.getHeight() / 4.0f);  // Set font size proportionally. NOLINT
    g.setFont (fontsize);

    for (const auto& tickMark: m_tickMarks)
    {
        const auto tickMarkLevelRatio = std::clamp ((tickMark - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
        const auto tickMarkY          = m_segmentBounds.getY() + juce::roundToInt (m_segmentBounds.toFloat().proportionOfHeight (1.0f - tickMarkLevelRatio));
        const auto labelBounds =
          juce::Rectangle<int> (m_segmentBounds.getX(), tickMarkY - juce::roundToInt (fontsize / 2.0f), m_segmentBounds.getWidth(), static_cast<int> (fontsize));

        g.drawFittedText (juce::String (std::abs (tickMark)), labelBounds.reduced (Constants::kLabelStripTextPadding, 0), juce::Justification::topLeft, 1);
    }
}
//==============================================================================

constexpr bool Segment::containsUpTo (juce::Range<float> levelRange, float levelDb) noexcept
{
    return levelDb > levelRange.getStart() && levelDb <= levelRange.getEnd();
}
//==============================================================================

void Segment::setMeterBounds (juce::Rectangle<int> meterBounds)
{
    if (meterBounds == m_meterBounds)
        return;

    m_meterBounds = meterBounds;
    const auto segmentBounds =
      meterBounds.withY (meterBounds.getY() + juce::roundToInt (meterBounds.toFloat().proportionOfHeight (1.0f - m_segmentOptions.meterRange.getEnd())))
        .withHeight (juce::roundToInt (meterBounds.toFloat().proportionOfHeight (m_segmentOptions.meterRange.getLength())));
    m_segmentBounds = segmentBounds;
    updateLevelBounds();
    updatePeakHoldBounds();

    if (m_meterOptions.useGradient)
        m_gradientFill = juce::ColourGradient (m_segmentOptions.segmentColour, segmentBounds.toFloat().getBottomLeft(), m_segmentOptions.nextSegmentColour,
                                               segmentBounds.toFloat().getTopLeft(), false);

    m_isDirty = true;
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

    const auto levelRatio = std::clamp ((m_currentLevel_db - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
    const auto levelBounds = m_segmentBounds.withTop (m_segmentBounds.getY() + juce::roundToInt (m_segmentBounds.toFloat().proportionOfHeight (1.0f - levelRatio)));

    if (m_drawnBounds == levelBounds)
        return;

    m_drawnBounds = levelBounds;
    m_isDirty     = true;
}
//==============================================================================

void Segment::updatePeakHoldBounds()
{
    auto peakHoldBounds = juce::Rectangle<int>();

    if (Segment::containsUpTo (m_segmentOptions.levelRange, m_peakHoldLevel_db))
    {
        const auto peakHoldRatio = std::clamp ((m_peakHoldLevel_db - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
        if (peakHoldRatio == 0.0f)
            return;

        const auto peakHoldY = m_segmentBounds.getY() + juce::roundToInt (m_segmentBounds.toFloat().proportionOfHeight (1.0f - peakHoldRatio));
        peakHoldBounds       = m_segmentBounds.withTop (peakHoldY).withHeight (Constants::kPeakHoldHeight);
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

void Segment::setMeterOptions (const Options& meterOptions)
{
    m_meterOptions = meterOptions;

    // Find all tickMark-marks in this segment's range...
    m_tickMarks.clear();
    for (const auto& tickMark: meterOptions.tickMarks)
    {
        if (Segment::containsUpTo (m_segmentOptions.levelRange, tickMark))
            m_tickMarks.emplace_back (tickMark);
    }

    m_isDirty = true;
}
//==============================================================================
}  // namespace SoundMeter
}  // namespace sd