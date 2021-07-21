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

#ifndef SD_SOUND_METER_HEADER_H
#define SD_SOUND_METER_HEADER_H


namespace sd
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
class Header
{
public:
   /**
    * @brief Default constructor
   */
   Header() = default;

   /**
    * @brief Parameterized constructor.
    * 
    * @param name Channel name to display in the header.
    * @param type Channel type to display in the header.
   */
   Header (const juce::String& name, const juce::AudioChannelSet::ChannelType& type) : m_name (name), m_type (type) { }

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
   [[nodiscard]] const juce::AudioChannelSet::ChannelType& getType() const noexcept;

   /**
    * @brief Set the channel name.
    * 
    * Can be anything the user sets (mid, side, etc..).
    * 
    * @param name The channel name assigned to the meter.
    * 
    * @see getName
   */
   void setName (const juce::String name);

   /**
    * @brief Get the channel name.
    * 
    * Can be anything the user sets (mid, side, etc..).
    * 
    * @return The channel name assigned to the meter.
    * 
    * @see setName
   */
   [[nodiscard]] juce::String getName() const noexcept;

   /**
    * @brief Get the width (in pixels) of the channel name.
    *
    * @return The width (in pixels) taken by the channel name.
    * @see getTypeWidth, textFits
    */
   [[nodiscard]] float getNameWidth() const noexcept;

   /**
    * @brief Get the width (in pixels) of the channel description.
    *
    * @return The width (in pixels) taken by the channel description.
    * @see getNameWdith, textFits
    */
   [[nodiscard]] float getTypeWidth() const noexcept;

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
   bool textFits (const juce::String& text, int widthAvailable) const;

   /**
    * @brief Set the font used to display the info (and other text in the meter).
    *
    * Font to be used for the header, value and label strip.
    *
    * @param font The font to use.
    * @see getFont
    */
   void setFont (const juce::Font& font) noexcept;

   /**
    * @brief Get the font used to display the info.
    * 
    * @return The font used to display the info.
   */
   [[nodiscard]] const juce::Font& getFont() const noexcept;

   /**
    * @brief Set the bounds of the 'header' part of the meter.
    * 
    * @param bounds The bounds to use for the 'header' part of the meter.
    * @see getBounds
   */
   void setBounds (const juce::Rectangle<int>& bounds) noexcept;

   /**
    * @brief Get the bounds of the 'header' part of the meter.
    * 
    * @return The bounds of the 'header' part of the meter.
    * @see setBounds
   */
   [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept;

   /**
    * @brief Check if the 'header' part of the meter is visible.
    * 
    * @return True, when the 'header' part is visible.
    * @see setVisible, setEnabled
   */
   [[nodiscard]] bool isVisible() const noexcept { return m_visible && m_enabled; }

   /**
    * @brief Show the 'header' part of the meter.
    * 
    * @param visible When set to true, the 'header' part will be visible.
    * @see isVisible, setEnabled
   */
   void setVisible (bool visible) noexcept { m_visible = visible; }

   /**
    * @brief Enable the 'header' part of the meter.
    * 
    * @param enable When set to true, the header part will be enabled.
    * @see setVisible, isVisible
   */
   void setEnabled (bool enable) noexcept { m_enabled = enable; }

   /**
    * @brief Check if the mouse is over the 'header' part of the meter.
    * 
    * @param y The coordinate to use to check whether the mouse is over the 'header' part.
    * @return  True, when the mouse is over the 'header' part of the meter, using the supplied y coordinate.
    * @see resetMouseOver
   */
   [[nodiscard]] bool isMouseOver (const int y) noexcept;

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
    * @param[in,out] g           The juce graphics context to use.
    * @param meterActive         When true, the meter is un-muted (activated).
    * @param faderEnabled        When true, the fader overlay is enabled (so the header needs to display the muted button).
    * @param muteColour          The colour to use when the meter is muted (in-active).
    * @param muteMouseOverColour The colour to use when the mouse is over the meter and the meter is muted (in-active).
    * @param textColour          The colour of the text in the header.
    * @param inactiveColour      The colour to use when the meter is muted (in-active).
   */
   void draw (juce::Graphics& g, bool meterActive, bool faderEnabled, const juce::Colour& muteColour, const juce::Colour& muteMouseOverColour,
              const juce::Colour& textColour, const juce::Colour& inactiveColour);

private:
   bool m_visible = true;
   bool m_enabled = true;

   juce::Font m_font;

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
   JUCE_LEAK_DETECTOR (Header);
};

}  // namespace SoundMeter
}  // namespace sd

#endif /* SD_SOUND_METER_HEADER_H */