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

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>


namespace sd  // NOLINT
{
namespace SoundMeter
{

/**
 * @brief Class responsible for the meter's 'header' part.
 * 
 * The 'header' part, is the part above the meter displaying
 * the name (when supplied by the user), the type (left, right) or
 * the abbreviated type if the other two do not fit.
 * 
 * The 'header' also doubles as a button which can de-activate (mute) or 
 * activate the meter.
*/
class Header final
{
public:
    /**
    * @brief Constructor
    * @param font The font to use in the header.
    */
    explicit Header (juce::Font& font) noexcept : Header ({}, juce::AudioChannelSet::unknown, font) { }

    /**
    * @brief Constructor with channel identification.
    * 
    * @param name Channel name to display in the header.
    * @param type Channel type to display in the header.  
    * @param font The font to use in the header.
    */
    Header (juce::String name, const juce::AudioChannelSet::ChannelType& type, juce::Font& font) noexcept
      : m_font (font), m_name (std::move (name)), m_type (type)
    {
    }

    /**
    * @brief Set the channel type.
    * 
    * For instance: left, right, center, etc..
    * 
    * @param type The channel type assigned to the meter.
    * 
    * @see getType
    */
    void setType (const juce::AudioChannelSet::ChannelType& type);

    /**
     * @brief Get the channel type.
     * 
     * For instance: left, right, center, etc..
     * 
     * @return The channel type assigned to the meter.
     * 
     * @see setType
    */
    [[nodiscard]] const juce::AudioChannelSet::ChannelType& getType() const noexcept { return m_type; }

    /**
     * @brief Set the channel name.
     * 
     * Can be anything the user sets (mid, side, etc..).
     * 
     * @param name The channel name assigned to the meter.
     * 
     * @see getName
    */
    void setName (const juce::String& name);

    /**
     * @brief Get the channel name.
     * 
     * Can be anything the user sets (mid, side, etc..).
     * 
     * @return The channel name assigned to the meter.
     * 
     * @see setName
    */
    [[nodiscard]] juce::String getName() const noexcept { return m_name; }

    /**
    * @brief Get the width (in pixels) of the channel name.
    *
    * @return The width (in pixels) taken by the channel name.
    * @see getTypeWidth, textFits
    */
    [[nodiscard]] float getNameWidth() const noexcept { return m_nameWidth; }

    /**
     * @brief Get the width (in pixels) of the channel description.
     *
     * @return The width (in pixels) taken by the channel description.
     * @see getNameWdith, textFits
    */
    [[nodiscard]] float getTypeWidth() const noexcept { return m_typeWidth; }

    /**
     * @brief Get the info text displayed in the 'header'
     *
     * This can be either the channel name (when set by the user),
     * the channel type description (left, right, etc..) or the
     * abbreviated channel type description when the other two do not fit.
     * 
     * @return The info text displayed in the 'header'.
     * 
     * @see getName, getType
    */
    [[nodiscard]] juce::String getInfo() const noexcept;

    /**
     * @brief Check whether a certain text will fit the width available using the meter's specified font.
     * 
     * @param text           The info text to check the width of.
     * @param widthAvailable The width available in the 'header' part.
     * 
     * @see getInfo, getTypeWidth, getNameWidth
    */
    [[nodiscard]] bool textFits (const juce::String& text, int widthAvailable) const;

    /**
    * @brief Set the font used to display the info (and other text in the meter).
    *
    * Font to be used for the header, value and label strip.
    *
    * @param font The font to use.
    * @see getFont
    */
    void setFont (const juce::Font& font);

    /**
    * @brief Set the bounds of the 'header' part of the meter.
    * 
    * @param bounds The bounds to use for the 'header' part of the meter.
    * @see getBounds
   */
    void setBounds (const juce::Rectangle<int>& bounds) noexcept { m_bounds = bounds; }

    /**
     * @brief Get the bounds of the 'header' part of the meter.
     * 
     * @return The bounds of the 'header' part of the meter.
     * @see setBounds
    */
    [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept { return m_bounds; }

    /**
     * @brief Check if the mouse is over the 'header' part of the meter.
     * 
     * @param y The coordinate to use to check whether the mouse is over the 'header' part.
     * @return  True, when the mouse is over the 'header' part of the meter, using the supplied y coordinate.
     * @see resetMouseOver
    */
    [[nodiscard]] bool isMouseOver (int y) noexcept;

    /**
     * @brief Check if the mouse is over the 'header' part of the meter.
     * 
     * @return  True, when the mouse is over the 'header' part of the meter.
     * @see resetMouseOver
    */
    [[nodiscard]] bool isMouseOver() const noexcept { return m_mouseOver; }

    /**
     * @brief Reset 'mouse over' status of the 'header' part of the meter.
    */
    void resetMouseOver() noexcept { m_mouseOver = false; }

    /**
     * @brief Set the referred width (from other meters) used to decide what info to display.
     * 
     * When this is set to zero, each meter uses his own bounds to decide what to display.
     * When set to a non zero value (for instance from another meter) this meter will use that
     * value to decide what to display.
     * When there is not enough room (width) to display the full description or name, display
     * the abbreviated type description.
     * 
     * @param referredWidth The width (in pixels) to use when deciding what to display in the header.
    */
    void setReferredWidth (float referredWidth) noexcept { m_referredWidth = referredWidth; }

    /**
     * @brief Draw the 'header' part of the meter.
     * 
     * @param[in,out] g    The juce graphics context to use.
     * @param meterActive  True, when the meter is active (not muted).
     * @param faderEnabled True, when the fader overlay is enabled.
     * @param meterColours The colours to draw the header with.        
    */
    void draw (juce::Graphics& g, bool meterActive, bool faderEnabled, const MeterColours& meterColours);

private:
    juce::Font& m_font;

    // Info
    juce::String                       m_name                 = "";
    juce::AudioChannelSet::ChannelType m_type                 = juce::AudioChannelSet::ChannelType::unknown;
    juce::String                       m_typeDescription      = "";
    juce::String                       m_typeAbbrDecscription = "";

    // Bounds
    juce::Rectangle<int> m_bounds {};
    float                m_nameWidth     = 0.0f;
    float                m_typeWidth     = 0.0f;
    float                m_referredWidth = 0.0f;
    bool                 m_mouseOver     = false;

    void calculateInfoWidth();

    // clang-format on
    JUCE_LEAK_DETECTOR (Header)
};

}  // namespace SoundMeter
}  // namespace sd
