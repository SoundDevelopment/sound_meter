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

#pragma once

#include "sd_MeterHelpers.h"

#include <juce_graphics/juce_graphics.h>

namespace sd
{

namespace SoundMeter
{

class DbSegment
{
public:
    DbSegment (SegmentOptions segmentOptions) : m_segmentOptions (segmentOptions)
    {
        // Check level range validity.
        jassert (segmentOptions.levelRange.getLength() > 0.0f);  // NOLINT
        // Check meter range validity (0.0f - 1.0f).
        jassert (segmentOptions.meterRange.getStart() >= 0.0f && segmentOptions.meterRange.getEnd() <= 1.0f && segmentOptions.meterRange.getLength() > 0.0f);  // NOLINT
    }

    void draw (juce::Graphics& g)
    {
        m_isDirty = false;
        if (!m_drawnBounds.isEmpty())
        {
            g.setColour (m_segmentOptions.segmentColour);
            g.fillRect (m_drawnBounds);
        }

        if (m_segmentOptions.showPeakHold && !m_peakHoldBounds.isEmpty())
        {
            g.setColour (m_segmentOptions.peakHoldColour);
            g.fillRect (m_peakHoldBounds);
        }
    }

    void setLevel (float level_db)
    {
        m_peakHoldLevel = std::max (m_peakHoldLevel, level_db);
        if (m_currentLevel == level_db)
            return;
        m_currentLevel = level_db;
        updateLevelBounds();
    }


    void setMeterBounds (juce::Rectangle<int> meterBounds)
    {
        const auto segmentBounds = meterBounds.withY (meterBounds.getY() + meterBounds.proportionOfHeight (1.0f - m_segmentOptions.meterRange.getEnd()))
                                     .withHeight (meterBounds.proportionOfHeight (m_segmentOptions.meterRange.getLength()));
        if (segmentBounds == m_segmentBounds)
            return;

        m_segmentBounds = segmentBounds;
        updateLevelBounds();
    }

    /** @brief Get the bounding box of this segment.*/
    [[nodiscard]] juce::Rectangle<int> getSegmentBounds() const noexcept { return m_segmentBounds; }

    void updateLevelBounds()
    {
        if (m_segmentBounds.isEmpty())
            return;

        const auto levelRatio  = std::clamp ((m_currentLevel - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
        const auto levelBounds = m_segmentBounds.withTop (m_segmentBounds.getY() + m_segmentBounds.proportionOfHeight (1.0f - levelRatio));

        if (m_drawnBounds == levelBounds)
            return;
        m_drawnBounds = levelBounds;

        // Create peak hold indicator...
        m_peakHoldBounds.setHeight (0);
        if (m_segmentOptions.levelRange.contains (m_peakHoldLevel) || m_peakHoldLevel == m_segmentOptions.levelRange.getEnd() )
        {
            const auto peakHoldRatio = std::clamp ((m_peakHoldLevel - m_segmentOptions.levelRange.getStart()) / m_segmentOptions.levelRange.getLength(), 0.0f, 1.0f);
            m_peakHoldBounds = m_segmentBounds.withTop (m_segmentBounds.getY() + m_segmentBounds.proportionOfHeight (1.0f - peakHoldRatio)).withHeight (2);
        }

        m_isDirty     = true;
    }

    [[nodiscard]] bool isDirty() const noexcept { return m_isDirty; }

    void setColours (const juce::Colour& segmentColour, const juce::Colour& nextColour)
    {
        m_segmentOptions.segmentColour     = segmentColour;
        m_segmentOptions.nextSegmentColour = nextColour;
    }

    void setUseGradients (bool useGradients) noexcept { m_segmentOptions.useGradients = useGradients; }


private:
    SegmentOptions       m_segmentOptions {};
    juce::Rectangle<int> m_segmentBounds {};
    juce::Rectangle<int> m_drawnBounds {};
    juce::Rectangle<int> m_peakHoldBounds {};

    float m_currentLevel  = Constants::kMinLevel_db;
    float m_peakHoldLevel = Constants::kMinLevel_db;
    bool  m_isDirty       = false;

    juce::ColourGradient m_gradientFill {};

    JUCE_LEAK_DETECTOR (DbSegment)
};


/**
 * @brief Individual meter segment.
*/
class Segment final
{
public:
    /**
     * @brief Draw the segment.
     * 
     * @param[in,out] g   The juce graphics context.
     * @param useGradient Set this to true when you want to use gradients for the meters.
    */
    void draw (juce::Graphics& g, bool useGradient) const;

    /**
     * @brief Set the input level.
     * 
     * This also checks if the segment needs to be redrawn (is dirty).
     * 
     * @param level The meter level in decibels.
    */
    void setLevel (float level_db);

    /**
     * @brief Set the range of the segment.
     * 
     * @param newStartLevel Start of the segment (in decibels).
     * @param newStopLevel  End of the segment (in decibels).
    */
    void setRange (float newStartLevel_db, float newStopLevel_db);

    /**
     * @brief Set the bounds of the full level part (all segments).
     * 
     * @param bounds The bounds of the level part (all segments).
     * 
     * @see getSegmentBounds, setSegmentBounds
    */
    void setMeterBounds (const juce::Rectangle<int>& bounds);

    /**
     * @brief Set the bounds of the segment.
     * 
     * @return The bounds of the segment.
     *
     * @see setMeterBounds,getSegmentBounds
    */
    void setSegmentBounds (juce::Rectangle<int> segmentBounds) noexcept { m_segmentBounds = segmentBounds; }

    /**
     * @brief Get the bounds of the segment.
     * 
     * @return The bounds of the segment.
     *
     * @see setMeterBounds
    */
    [[nodiscard]] juce::Rectangle<int> getSegmentBounds() const noexcept { return m_segmentBounds; }

    /**
     * @brief Check if the segment needs to be redrawn (is dirty).
     *
     * @return True, if the segment needs to be redrawn.
    */
    [[nodiscard]] bool isDirty() const noexcept { return m_dirty; }

    /**
     * @brief Set the segment colour (and next colour).
     * 
     * The 'next' colour is 2nd colour to use in the gradient.
     * 
     * @param segmentColour Segment colour.
     * @param nextColour    Next colour (used for gradient).
    */
    void setColours (const juce::Colour& segmentColour, const juce::Colour& nextColour);

private:
    float m_startLevel      = 0.0f;
    float m_stopLevel       = 1.0f;
    float m_currentLevel    = 0.0f;
    int   m_currentLevel_px = 0;
    float m_levelMultiplier = 0.0f;
    bool  m_dirty           = false;

    juce::Colour m_segmentColour = juce::Colours::red;
    juce::Colour m_nextColour    = m_segmentColour.brighter();

    juce::ColourGradient m_gradientFill {};
    juce::Rectangle<int> m_meterBounds {};
    juce::Rectangle<int> m_segmentBounds {};

    void calculateSegment();

    JUCE_LEAK_DETECTOR (Segment)
};

}  // namespace SoundMeter
}  // namespace sd
