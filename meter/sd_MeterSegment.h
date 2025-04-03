/*
    ==============================================================================
    
    This file is part of the sound_meter JUCE module
    Copyright (c) 2019 - 2025 Sound Development - Marcel Huibers
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

#pragma once

#include "sd_MeterHelpers.h"

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace sd  // NOLINT
{

namespace SoundMeter
{


class Segment final
{
public:
    /** @brief Construct a segment using the supplied options.*/
    Segment (const Options& meterOptions, const SegmentOptions& segmentOptions);

    /** @brief Set the level in decibels.*/
    void setLevel (float level_db);

    /** @brief Draw the segment.*/
    void draw (juce::Graphics& g, const MeterColours& meterColours);

    /** @brief Set the bounds of the total meter (all segments) */
    void setMeterBounds (juce::Rectangle<int> meterBounds);

    /** @brief Get the bounding box of this segment.*/
    [[nodiscard]] juce::Rectangle<float> getSegmentBounds() const noexcept { return m_segmentBounds; }

    /** @brief Reset the peak hold.*/
    void resetPeakHold() noexcept;

    /** @brief Get the peak hold level.*/
    [[nodiscard]] float getPeakHold() const noexcept { return m_peakHoldLevel_db; }

    /** @brief Check if the segment needs to be re-drawn (dirty). */
    [[nodiscard]] bool isDirty() const noexcept { return m_isDirty; }

    /**
     * @brief Set whether this meter is a label strip.
     *
     * A label strip only draws the value labels (at the tick-marks),
     * but does not display any level.
     *
     * @param isLabelStrip when set, this meter behave like a label strip.
    */
    void setIsLabelStrip (bool isLabelStrip = false) noexcept { m_isLabelStrip = isLabelStrip; }

    /**
     * @brief Set the meter in 'minimal' mode.
     *
     * In minimal mode, the meter is in it's cleanest state possible.
     * This means no header, no tick-marks, no value, no faders and no indicator.
     *
     * @param minimalMode When set to true, 'minimal' mode will be enabled.
     * @see isMinimalModeActive, autoSetMinimalMode
     */
    void setMinimalMode (bool minimalMode) noexcept;

    /** @brief Set the segment options, describing the range and colour of the segment. */
    void setSegmentOptions (SegmentOptions segmentOptions);

    /** @brief Get the segment options, describing the range and colour of the segment. */
    [[nodiscard]] SegmentOptions getSegmentOptions() const noexcept { return m_segmentOptions; }

    /** @brief Set meter options. */
    void setMeterOptions (const Options& meterOptions);

    /** @brief Get segment options.*/
    [[nodiscard]] Options getMeterOptions() const { return m_meterOptions; }

private:
    SegmentOptions         m_segmentOptions {};
    Options                m_meterOptions {};
    std::vector<float>     m_tickMarks;
    juce::Rectangle<int>   m_meterBounds;
    juce::Rectangle<float> m_segmentBounds;
    juce::Rectangle<float> m_drawnBounds;
    juce::Rectangle<float> m_peakHoldBounds;
    juce::Rectangle<float> m_drawnPeakHoldBounds;
    juce::ColourGradient   m_gradientFill;

    float m_currentLevel_db   = Constants::kMinLevel_db;
    float m_peakHoldLevel_db  = Constants::kMinLevel_db;
    bool  m_isDirty           = false;
    bool  m_minimalModeActive = false;
    bool  m_isLabelStrip      = false;

    void updateLevelBounds();
    void updatePeakHoldBounds();
    void drawTickMarks (juce::Graphics& g, const MeterColours& meterColours);
    void drawLabels (juce::Graphics& g, const MeterColours& meterColours) const;

    JUCE_LEAK_DETECTOR (Segment)
};

}  // namespace SoundMeter
}  // namespace sd
