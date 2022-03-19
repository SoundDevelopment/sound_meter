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

void Segment::draw (juce::Graphics& g, bool useGradient) const
{
    if (m_segmentBounds.isEmpty()) return;

    // Get the actual 'level' rectangle...
    const auto levelRect = m_segmentBounds.withTop (m_segmentBounds.getBottom() - m_currentLevel_px);

    // Set the fill of the meters (solid zones or gradients)...
    if (useGradient)
        g.setGradientFill (m_gradientFill);
    else
        g.setColour (m_segmentColour);

    // Actually draw the level...
    g.fillRect (levelRect);
}
//==============================================================================

void Segment::setLevel (float level)
{
    if (m_segmentBounds.isEmpty()) return;

    // Store previous level drawn (to check later if the segment needs to be redrawn).
    const auto previousLevel_px = m_currentLevel_px;

    // Set the new level and calculate the amount of pixels it corresponds to...
    m_currentLevel    = juce::jlimit (m_startLevel, m_stopLevel, level);
    m_currentLevel_px = static_cast<int> (std::round ((m_currentLevel - m_startLevel) * m_levelMultiplier));

    // If the level drawn needs to be update, make the segment 'dirty'.
    m_dirty = (m_currentLevel_px != previousLevel_px);
}
//==============================================================================

void Segment::setRange (float newStartLevel, float newStopLevel)
{
    const auto startLevel = juce::jlimit (0.0f, 1.0f, newStartLevel);
    const auto stopLevel  = juce::jlimit (0.0f, 1.0f, newStopLevel);

    if (startLevel >= stopLevel)
    {
        jassertfalse;  // NOLINT
        return;
    }

    m_startLevel = startLevel;
    m_stopLevel  = stopLevel;

    calculateSegment();
}
//==============================================================================

void Segment::setMeterBounds (const juce::Rectangle<int>& bounds)
{
    m_meterBounds = bounds;
    calculateSegment();
}
//==============================================================================

void Segment::calculateSegment()
{
    if (m_meterBounds.isEmpty()) return;

    // Calculate segment bounds...
    m_segmentBounds = m_meterBounds.toFloat().getProportion<float> ({ 0.0f, 1.0f - m_stopLevel, 1.0f, m_stopLevel - m_startLevel }).toNearestIntEdges();

    // Calculate level multiplier to optimize level drawing...
    m_levelMultiplier = m_segmentBounds.getHeight() / (m_stopLevel - m_startLevel);

    // Calculate gradient fill...
    const juce::Point<float> gradientPoint1 = { 0.0f, static_cast<float> (m_segmentBounds.getBottom()) };
    const juce::Point<float> gradientPoint2 = { 0.0f, static_cast<float> (m_segmentBounds.getY()) };
    m_gradientFill                          = juce::ColourGradient (m_segmentColour, gradientPoint1, m_nextColour, gradientPoint2, false);

    setLevel (m_currentLevel);
}
//==============================================================================

void Segment::setColours (const juce::Colour& segmentColour, const juce::Colour& nextColour)
{
    m_segmentColour = segmentColour;
    m_nextColour    = nextColour;
    calculateSegment();
}

}  // namespace SoundMeter
}  // namespace sd
