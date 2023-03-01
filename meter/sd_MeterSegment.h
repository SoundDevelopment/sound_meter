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

#pragma once  // NOLINT

#include "sd_MeterHelpers.h"

#include <juce_graphics/juce_graphics.h>

namespace sd  // NOLINT
{

namespace SoundMeter
{


class Segment
{
public:
    /** @brief Construct a segment using the supplied options.*/
    explicit Segment (SegmentOptions options);

    /** @brief Set the level in decibels.*/
    void setLevel (float level_db);

    /** @brief Draw the segment.*/
    void draw (juce::Graphics& g);

    /** @brief Set the bounds of the total meter (all segments) */
    void setMeterBounds (juce::Rectangle<int> meterBounds);

    /** @brief Get the bounding box of this segment.*/
    [[nodiscard]] juce::Rectangle<int> getSegmentBounds() const noexcept { return m_segmentBounds; }

    /** @brief Set the tick marks to be drawn on the segment.*/
    void setTickMarks (const std::vector<float>& ticks_db);

    /** @brief Reset the peak hold.*/
    void resetPeakHold() noexcept;

    /** @brief Get the peak hold level.*/
    [[nodiscard]] float getPeakHold() const noexcept { return m_peakHoldLevel_db; }

    /** @brief Check if the segment needs to be re-drawn (dirty). */
    [[nodiscard]] bool isDirty() const noexcept { return m_isDirty; }

    /** @brief Set the segment's colour (or colours when using a gradient).*/
    void setColours (const juce::Colour& segmentColour, const juce::Colour& nextSegmentColour);

    /** @brief Enable or disable the use of a gradient colour fill. */
    void useGradients (bool gradientsUsed) noexcept { m_options.useGradients = gradientsUsed; }

    /** @brief Enable or disable the peak hold indicator. */
    void enablePeakHold (bool peakHoldEnabled) noexcept { m_options.enablePeakHold = peakHoldEnabled; }

    /** @brief Get segment options.*/
    const SegmentOptions& getOptions() const noexcept { return m_options; }

private:
    SegmentOptions       m_options {};
    std::vector<float>&  m_ticks_db {};
    juce::Rectangle<int> m_segmentBounds {};
    juce::Rectangle<int> m_drawnBounds {};
    juce::Rectangle<int> m_peakHoldBounds {};
    juce::Rectangle<int> m_drawnPeakHoldBounds {};

    float m_currentLevel_db  = Constants::kMinLevel_db;
    float m_peakHoldLevel_db = Constants::kMinLevel_db;
    bool  m_isDirty          = false;
    bool  m_isPeakHoldDirty  = false;

    juce::ColourGradient m_gradientFill {};

    void updateLevelBounds();
    void updatePeakHoldBounds();

    JUCE_LEAK_DETECTOR (Segment)
};

}  // namespace SoundMeter
}  // namespace sd
