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


#include "sd_MeterHeader.h"

namespace sd  // NOLINT
{

namespace SoundMeter
{

//==============================================================================

void Header::draw (juce::Graphics& g, bool meterActive, bool faderEnabled, const juce::Colour& muteColour, const juce::Colour& muteMouseOver,
                   const juce::Colour& textColour, const juce::Colour& inactiveColour)
{
    if (!isVisible())
        return;
    if (m_bounds.isEmpty())
        return;

    // Draw channel names...
    juce::String headerText = getInfo();

    // Draw 'button' for muting/de-activating channel...
    if (m_mouseOver && faderEnabled)
    {
        g.setColour (meterActive ? muteColour : muteMouseOver);
        g.fillRect (m_bounds);
        g.setColour (muteColour.contrasting (0.8f));  // NOLINT
    }
    else
    {
        if (meterActive)
        {
            g.setColour (textColour);
        }
        else
        {
            g.setColour (inactiveColour);
            g.fillRect (m_bounds);
            g.setColour (inactiveColour.contrasting (0.8f));  // NOLINT
        }
    }
    if (headerText.isNotEmpty())
        g.drawFittedText (headerText, m_bounds, juce::Justification::centred, 1);
}
//==============================================================================

void Header::setType (const juce::AudioChannelSet::ChannelType& type)
{
    m_type                 = type;
    m_typeDescription      = juce::AudioChannelSet::getChannelTypeName (type);
    m_typeAbbrDecscription = juce::AudioChannelSet::getAbbreviatedChannelTypeName (type);

    calculateInfoWidth();
}
//==============================================================================

const juce::AudioChannelSet::ChannelType& Header::getType() const noexcept
{
    return m_type;
}

//==============================================================================

void Header::setName (const juce::String& name)
{
    if (name.isEmpty())
        return;

    m_name = name;

    calculateInfoWidth();
}
//==============================================================================

void Header::calculateInfoWidth()
{
    m_nameWidth = m_font.getStringWidthFloat (m_name);
    m_typeWidth = m_font.getStringWidthFloat (m_typeDescription);
}
//==============================================================================

float Header::getNameWidth() const noexcept
{
    return m_nameWidth;
}
//==============================================================================

float Header::getTypeWidth() const noexcept
{
    return m_typeWidth;
}
//==============================================================================

juce::String Header::getName() const noexcept
{
    return m_name;
}
//==============================================================================

juce::String Header::getInfo() const
{
    // Check which type width to use. This meter's one or a referred meter...
    const auto typeWidthToCompare = (m_referredWidth > 0 ? m_referredWidth : m_typeWidth);

    // First check if the channel name fits and is not empty (preferred)...
    if (m_name.isNotEmpty() && m_nameWidth < m_bounds.getWidth())
        return m_name;
    else if (m_typeDescription.isNotEmpty() && typeWidthToCompare < m_bounds.getWidth())  // Check if there is room for the full channel description...
        return m_typeDescription;

    return m_typeAbbrDecscription;  // ... otherwise use the abbreviated one.
}
//==============================================================================

void Header::setBounds (const juce::Rectangle<int>& bounds) noexcept
{
    m_bounds = bounds;
}
//==============================================================================

juce::Rectangle<int> Header::getBounds() const noexcept
{
    return m_bounds;
}
//==============================================================================

void Header::setFont (juce::Font font)
{
    m_font = font;
    calculateInfoWidth();
}
//==============================================================================

bool Header::textFits (const juce::String& text, const int widthAvailable) const
{
    return m_font.getStringWidthFloat (text) <= static_cast<float> (widthAvailable);
}
//==============================================================================

bool Header::isMouseOver (const int y)
{
    m_mouseOver = (y < m_bounds.getHeight());
    return m_mouseOver;
}

}  // namespace SoundMeter
}  // namespace sd