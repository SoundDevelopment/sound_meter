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

#ifndef SD_SOUND_METER_SEGMENT_H
#define SD_SOUND_METER_SEGMENT_H


namespace sd
{
namespace SoundMeter
{

/**
 * @brief Individual meter segment.
*/
class Segment
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
     * @brief Set the level for the segment.
     * 
     * This also checks if the segment needs to be redrawn (is dirty).
     * 
     * @param level The meter level.
    */
    void setLevel (float level) noexcept;

    /**
     * @brief Set the range of the segment.
     * 
     * @param newStartLevel Start of the segment (in amp [0..1]).
     * @param newStopLevel  End of the segment (in amp [0..1]).
    */
    void setRange (float newStartLevel, float newStopLevel) noexcept;

    /**
     * @brief Set the bounds of the full level part (all segments).
     * 
     * @param bounds The bounds of the level part (all segments).
     * 
     * @see getSegmentBounds
    */
    void setMeterBounds (const juce::Rectangle<int>& bounds) noexcept;

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
    void setColours (const juce::Colour& segmentColour, const juce::Colour& nextColour) noexcept;

 private:
    float m_startLevel      = 0.0f;
    float m_stopLevel       = 1.0f;
    float m_currentLevel    = 0.0f;
    int   m_currentLevel_px = 0;
    float m_levelMultiplier = 0.0f;
    bool  m_dirty           = false;

    juce::Colour         m_segmentColour = juce::Colours::red;
    juce::Colour         m_nextColour    = m_segmentColour.brighter();
    juce::ColourGradient m_gradientFill;

    juce::Rectangle<int> m_meterBounds {};
    juce::Rectangle<int> m_segmentBounds {};

    void calculateSegment() noexcept;

    JUCE_LEAK_DETECTOR (Segment)
};

}  // namespace SoundMeter
}  // namespace sd

#endif /* SD_SOUND_METER_SEGMENT_H */