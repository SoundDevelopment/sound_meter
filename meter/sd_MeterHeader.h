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
 * @brief The type of header info to display.
*/
enum class HeaderInfo
{
   channelName,      ///< The name of the channel (this can be anything the use assigns).
   fullChannelType,  ///< The full type of the channel ( left, right, centre, etc...).
   abbrChannelType,  ///< The abbreviated type of the channel ( L, R, C, etc...).
   channelIndex,     ///< The index of the channel in the full channel format ( L = 0, R = 1, etc...).
   none,             ///< There is no channel type, name or index set.
   referred,         ///< This meter follows the info option off another meter.
   notSet            ///< No header info option has been selected.
};

/**
 * @brief Class responsible for the meter's 'header' part.
 * The 'header' part, is the part above the meter displaying
 * the name. Also a button which can de-activate (mute) or 
 * activate a meter.
*/
class Header
{
public:
   // clang-format off

   Header() = default;
   Header (const juce::String& name, const juce::AudioChannelSet::ChannelType& type, int index) : m_name (name), m_type (type), m_channelIndex (index) { }

   void draw (juce::Graphics& g, bool meterActive, bool faderEnabled, const juce::Colour& mutedColour, const juce::Colour& mutedMouseOverColour,
              const juce::Colour& textColour, const juce::Colour& inactiveColour);

   /**
    * @brief Set the channel type.
    * 
    * For instance: left, right, centre, etc..
    * 
    * @param type The channel type assigned to the meter.
    * 
    * @see getType
   */
   void setType (const juce::AudioChannelSet::ChannelType& type); 

   [[nodiscard]] const juce::AudioChannelSet::ChannelType& getType() const noexcept;

   void                       setName (const juce::String name);
   [[nodiscard]] juce::String getName() const noexcept;
   void                       calculateInfoWidth();
   bool                       nameFits (const juce::String& name, int widthAvailable) const;

   /**
    * Get the width (in pixels) of the channel info in the 'header' part.
    *
    * @return The width (in pixels) taken by the channel info in the 'header' part.
    */
   [[nodiscard]] float getInfoWidth() const noexcept;

   
   [[nodiscard]] juce::String getInfo (HeaderInfo headerInfoType = HeaderInfo::notSet) const noexcept;

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

   void setIndex (int channelIndex) noexcept { m_channelIndex = channelIndex; }

   [[nodiscard]] bool isVisible() const noexcept { return m_visible; }
   void               setVisible (bool visible) noexcept { m_visible = visible; }

   [[nodiscard]] bool isMouseOver (const int y) noexcept
   {
      m_mouseOver = (y < m_bounds.getHeight());
      return m_mouseOver;
   }
   [[nodiscard]] bool isMouseOver() const noexcept { return m_mouseOver; }
   void               resetMouseOver() noexcept { m_mouseOver = false; }

private:
   HeaderInfo                         m_headerInfo           = HeaderInfo::notSet;
   juce::AudioChannelSet::ChannelType m_type                 = juce::AudioChannelSet::ChannelType::unknown;
   juce::String                       m_typeDescription      = "";
   juce::String                       m_typeAbbrDecscription = "";
   juce::String                       m_name                 = "";
   float                              m_infoWidth            = 0.0f;
   juce::Rectangle<int>               m_bounds {};
   juce::Font                         m_font;
   int                                m_channelIndex = 0;
   bool                               m_visible      = true;
   bool                               m_mouseOver    = false;

   // clang-format on
   JUCE_LEAK_DETECTOR (Header);
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_HEADER_H */