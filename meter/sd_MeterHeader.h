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


namespace sd::SoundMeter
{

/**
 * @brief Class responsible for the meter's 'header' part.
 * The 'header' part, is the part above the meter displaying
 * the name. Also a button which can de-activate (mute) or 
 * activate a meter.
*/
class Header
{
public:
   // clag-format off

   Header() = default;
   Header (const juce::String& name, const juce::AudioChannelSet::ChannelType& type) : m_name (name), m_type (type) { }

   void draw (juce::Graphics& g, bool meterActive, bool faderEnabled, const juce::Colour& mutedColour, const juce::Colour& mutedMouseOverColour,
              const juce::Colour& textColour, const juce::Colour& inactiveColour);

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

   [[nodiscard]] const juce::AudioChannelSet::ChannelType& getType() const noexcept;

   void                       setName (const juce::String name);
   [[nodiscard]] juce::String getName() const noexcept;

   bool infoFits (const juce::String& name, int widthAvailable) const;

   /**
    * Get the width (in pixels) of the channel name.
    *
    * @return The width (in pixels) taken by the channel name.
    */
   [[nodiscard]] float getNameWidth() const noexcept;

   /**
    * Get the width (in pixels) of the channel description.
    *
    * @return The width (in pixels) taken by the channel description.
    */
   [[nodiscard]] float getTypeWidth() const noexcept;

   [[nodiscard]] juce::String getInfo() const noexcept;

   void                               setBounds (const juce::Rectangle<int>& bounds) noexcept;
   [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept;

   [[nodiscard]] const juce::Font& getFont() const noexcept;

   /**
    * Set meter font. 
    *
    * Font to be used for the header, value and label strip.
    *
    * @param font the font to use.
    */
   void setFont (const juce::Font& font) noexcept;

   [[nodiscard]] bool isVisible() const noexcept { return m_visible; }
   void               setVisible (bool visible) noexcept { m_visible = visible; }

   [[nodiscard]] bool isMouseOver (const int y) noexcept
   {
      m_mouseOver = (y < m_bounds.getHeight());
      return m_mouseOver;
   }
   [[nodiscard]] bool isMouseOver() const noexcept { return m_mouseOver; }
   void               resetMouseOver() noexcept { m_mouseOver = false; }

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

private:
   juce::AudioChannelSet::ChannelType m_type = juce::AudioChannelSet::ChannelType::unknown;

   // Info
   juce::String m_name                 = "";
   juce::String m_typeDescription      = "";
   juce::String m_typeAbbrDecscription = "";


   float                m_nameWidth     = 0.0f;
   float                m_typeWidth     = 0.0f;
   float                m_referredWidth = 0.0f;
   juce::Rectangle<int> m_bounds {};
   juce::Font           m_font;
   bool                 m_visible   = true;
   bool                 m_mouseOver = false;

   /**
    * @brief Calculate the width (in pixels) the info would take up.
   */
   void calculateInfoWidth();

   // clang-format on
   JUCE_LEAK_DETECTOR (Header);
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_HEADER_H */