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


#ifndef SD_SOUND_METER_COMPONENT_H
#define SD_SOUND_METER_COMPONENT_H


namespace sd::SoundMeter
{

/**
 * @brief An individual meter component.
 * 
 * This represents a single meter.
 * Use the MetersPanel to create multiple meters matching
 * the specified channel format.
*/
class MeterComponent
  : public juce::Component
  , public juce::SettableTooltipClient
{
public:
   using ChannelType = juce::AudioChannelSet::ChannelType;

   /**
    * @brief Default constructor.
   */
   MeterComponent();

   /**
    * @brief Parameterized constructor.
    * 
    * @param meterOptions  Meter options to use (defining appearance and functionality).
    * @param padding       Padding to use (space between meter and the edge of the component).
    * @param channelName   The channel name (set by the user).
    * @param isLabelStrip  When set to true, this meter will function as a label strip (with markers for levels at the tick-marks).
    * @param channelType   The channel type (left, right, centre, etc...).
    * @param faderListener A listener to any changes in the fader.
   */
   MeterComponent (Options meterOptions, MeterPadding padding, const juce::String& channelName, bool isLabelStrip = false,
                   ChannelType channelType = ChannelType::unknown, [[maybe_unused]] Fader::Listener* faderListener = nullptr);

   /**
    * @brief Reset the meter (but not the peak hold).
    *
    * @see resetPeakHold
    */
   void reset();

   /**
    * @brief Set the input level from the audio engine. 
    *
    * Called from the audio thread!
    *
    * @param inputLevel New input level (in amp).
    */
   inline void setInputLevel (float inputLevel) noexcept { m_level.setInputLevel (inputLevel); }

   /**
    * @brief Show the fader briefly and fade out (unless overridden and shown longer).
    */
   void flashFader();

   /**
    * @brief Refresh the meter with the current input level.
    *
    * @param forceRefresh When set to true, the meter will be forced to repaint (even if not dirty).
    * @see setRefreshRate
    */
   void refresh (bool forceRefresh);

   //==============================================================================
   // Properties.

   /**
    * @brief Sets the meter's refresh rate. 
    *
    * Set this to optimize the meter's decay rate.
    * 
    * @param guiRefreshRate Refresh rate in Hz.
    * @see setDecay, getDecay
    */
   void setRefreshRate (float guiRefreshRate) noexcept { m_level.setRefreshRate (guiRefreshRate); }

   /**
    * @brief Set meter decay.
    *
    * @param decay_ms Meter decay in milliseconds.
    * @see getDecay, setRefreshRate
    */
   void setDecay (float decay_ms) noexcept { m_level.setDecay (decay_ms); }

   /**
    * @brief Get meter decay.
    *
    * @return Meter decay in milliseconds.
    * @see setDecay, setRefreshRate
    */
   [[nodiscard]] float getDecay() const noexcept { return m_level.getDecay(); }

   /**
    * @brief Set whether this meter is a label strip. 
    *
    * A label strip only draws the value labels (at the tick-marks),
    * but does display any level.
    *
    * @param isLabelStrip when set, this meter behave like a label strip.
    */
   void setIsLabelStrip (bool isLabelStrip = false) noexcept { m_isLabelStrip = isLabelStrip; }  // When enabled, just draw the labels, not the meter.

   /**
    * @brief Set the levels dividing the different regions of the meter. 
    *
    * The meter has 3 regions. Normal, warning and peak. 
    * The peak region level supplied need to be larger then the warning region level. 
    *
    * @param warningRegion_db sets the level (in db) dividing the normal and warning regions of the meter.
    * @param peakRegion_db sets the level (in db) dividing the warning and peak regions of the meter.
    */
   void setRegions (float warningRegion_db, float peakRegion_db);

   /**
    * @brief Set the padding of the meter. 
    *
    * The padding is the space between the meter and the edges 
    * of the component.
    *
    * @param padding Amount of padding to apply.
    */
   void setPadding (MeterPadding padding) noexcept { m_padding = padding; }

   /**
    * @brief Set the meter's options.
    *
    * The options determine the appearance and functionality of the meter.
    *
    * @param meterOptions Meter options to use.
    */
   void setOptions (Options meterOptions);

   /**
    * @brief Set the channel name.
    * 
    * Set's the channel name belonging to the track
    * feeding the meter.
    * @param channelName Name to assign to this meter. 
   */
   void setChannelName (const juce::String& channelName);

   /**
    * @brief Set the channel type.
    * 
    * For instance: left, right, center, etc..
    * 
    * @param type The channel type assigned to the meter.
    * 
    * @see getType
   */
   void setChannelType (ChannelType channelType) { m_header.setType (channelType); }

   /**
    * @brief Show the 'header' part of the meter.
    * 
    * The 'header' part is the part above the meter displaying
    * the channel name (when set) or the channel type.
    * It also doubles as a mute button for the specific channel.
    * 
    * @param headerVisible When set to true, the 'header' part will be visible.
    * @see enableHeader
   */
   void showHeader (bool headerVisible) noexcept;

   /**
    * @brief Enable the 'header' part of the meter.
    * 
    * The 'header' part is the part above the meter displaying
    * the channel name (when set) or the channel type.
    * It also doubles as a mute button for the specific channel.
    * 
    * @param headerEnabled When set to true, the header part will be enabled.
    * @see showHeader
   */
   void enableHeader (bool headerEnabled) noexcept;

   /**
    * @brief Check if a specified name will fit in a give width (in pixels).
    *
    * This can be a more detailed, multiline description of the function,
    * and it's containing logic.
    *
    * @param name           The name to check the width of.
    * @param widthAvailable The width (in pixels) available to fit the name in.
    * @return               True, if the name will fit in the given width (in pixels).
    *
    * @see getChannelNameWidth, setChannelNames
    */
   [[nodiscard]] bool nameFits (const juce::String& name, int widthAvailable) const { return m_header.infoFits (name, widthAvailable); }

   /**
    * @brief Get the width (in pixels) of the channel info in the 'header' part.
    *
    * @return The width (in pixels) taken by the channel info in the 'header' part.
    * 
    * @see getChannelTypeWidth, nameFits, setChannelName
    */
   [[nodiscard]] float getChannelNameWidth() const noexcept { return m_header.getNameWidth(); }

   /**
    * @brief Get the width (in pixels) of the full type description in the 'header' part.
    *
    * @return The width (in pixels) taken by the full type description in the 'header' part.
    * @see getChannelNameWidth, nameFits, setChannelType
    */
   [[nodiscard]] float getChannelTypeWidth() const noexcept { return m_header.getTypeWidth(); }

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
   void setReferredTypeWidth (float referredTypeWidth) { m_header.setReferredWidth (referredTypeWidth); }

   /**
    * @brief Activate or de-activate (mute) the meter.
    *
    * @param isActive When set to true, the meter is active.
    * @param notify   Determine whether to notify all listeners or not.
    * @see isActive
    */
   void               setActive (bool isActive, NotificationOptions notify = NotificationOptions::dontNotify);

   /**
    * @brief Check if the meter is active (un-muted).
    *
    * @return True, if the meter is active (un-muted).
    *
    * @see setActive
    */
   [[nodiscard]] bool isActive() const noexcept { return m_active; }

   /**
    * @brief Set the meter font. 
    *
    * Font to be used for the header, value and label strip.
    *
    * @param font The font to use.
    */
   void setFont (const juce::Font& font) noexcept;

   /**
    * @brief Reset the peak hold.
    *
    * Resets the peak hold indicator and value.
    *
    * @see showPeakHold
    */
   void resetPeakHold() noexcept;

   /**
    * @brief Show the peak hold indicator.
    *
    * @param showPeakHold When set true, the peak hold indicator will be shown.
    * @see showPeakValue, resetPeakHold
    */
   void showPeakHold (bool showPeakHold = true);

   /**
    * @brief Show the peak value.
    *
    * The peak value will be shown below the meter (in dB).
    * It's the same level as the peak hold bar.
    *
    * @param showPeakValue When set true, shows the value level (in dB) below the meter.
    * @see showPeakHold
    */
   void showValue (bool showValue = true);

   void enableValue (bool valueEnabled = true) noexcept;

   /**
    * @brief Show tick-marks (divider lines) on the meter.
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param showTickMarks When set true, shows the tick-marks. 
    * @see setTickMarks
    */
   void showTickMarks (bool showTickMarks = true);

   /**
    * @brief Set the level of the tick marks. 
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param ticks list of tick mark values (in amp).
    * @see showTickMarks
    */
   void setTickMarks (const std::vector<float>& ticks) { m_level.setTickMarks (ticks); }

   /**
    * @brief Use gradients in stead of hard region boundaries.
    * @param useGradients When set to true, uses smooth gradients. False gives hard region boundaries.=
   */
   void useGradients (bool useGradients) noexcept { m_level.useGradients (useGradients); }

   /**
    * Get the bounds of the 'meter' and 'header' parts combined.
    *
    * @return The bounds of the 'meter' and 'header' parts combined.
    */
   [[nodiscard]] juce::Rectangle<int> getLabelStripBounds() const noexcept;

   void               setMinimalMode (bool minimalMode) noexcept;
   [[nodiscard]] bool isMinimalModeActive() const noexcept { return m_minimalMode; }
   [[nodiscard]] bool autoSetMinimalMode (int proposedWidth, int proposedHeight);

#if SDTK_ENABLE_FADER
   /**
    * @brief Active (or de-activate) fader.
    * @param faderActive When set to true, will activate the meter's fader.
    * @see isFaderActive
   */
   void               setFaderActive (bool faderActive = true);
   [[nodiscard]] bool isFaderActive() const noexcept { return m_fader.isActive(); }

   void setFaderEnabled (bool faderEnabled = true);

   /**
    * Get the value of the meter fader.
    *
    * @return the current fader value [0..1].
    */
   [[nodiscard]] float getFaderValue() const noexcept { return m_fader.getValue(); }

   /**
    * @brief Set fader value.
    *
    * @param value              The value [0..1] the fader needs to be set to.
    * @param notificationOption Select whether to notify the listeners.
    * @param showFader          When set to true, the fader will briefly appear (when the value is changed).
    * @exceptsafe               Shall not throw exceptions.
    */
   void setFaderValue (float value, NotificationOptions notificationOption = NotificationOptions::dontNotify, bool showFader = true) noexcept;

   void addFaderListener (Fader::Listener& listener) { m_fader.addListener (listener); }
   void removeFaderListener (Fader::Listener& listener) { m_fader.removeListener (listener); }

#endif /* SDTK_ENABLE_FADER */


   //==============================================================================
   // Internal
   void paint (juce::Graphics& g) override;
   void resized() override;
   void lookAndFeelChanged() override;  // On change in lookandfeel or visibility. Cache the colours.
   void visibilityChanged() override;

   /**
    * @brief Colour IDs that can be used to customise the colours.
    * This can be done by overriding juce's LookAndFeel class.
   */
   enum ColourIds
   {
      backgroundColourId     = 0x1a03201,  ///< Background colour.
      peakColourId           = 0x1a03202,  ///< Peak area colour.
      warningColourId        = 0x1a03203,  ///< Warning area colour.
      normalColourId         = 0x1a03204,  ///< Normal colour.
      tickMarkColourId       = 0x1a03205,  ///< Tick-mark colour.
      textColourId           = 0x1a03206,  ///< Text colour.
      faderColourId          = 0x1a03207,  ///< Fader colour.
      textValueColourId      = 0x1a03208,  ///< Value text colour.
      textBackgroundColourId = 0x1a03209,  ///< Text background colour.
      mutedColourId          = 0x1a03210,  ///< Muted button colour.
      solodColourId          = 0x1a03211,  ///< Soloed button colour..
      mutedMouseOverColourId = 0x1a03212,  ///< Muted mouse over button colour.
      inactiveColourId       = 0x1a03213   ///< Inactive (muted) colour.
   };

private:
   //==============================================================================
   // Private members.
   Header m_header;  // Info relating to the meter (name, channel type, info rect, index in a sequence of multiple meters).

   Level m_level;

#if SDTK_ENABLE_FADER
   Fader m_fader;
#endif /* SDTK_ENABLE_FADER */

   // GUI
   MeterPadding m_padding { 0, 0, 0, 0 };  // Space between meter and component's edge.

   // Flags
   bool                 m_active       = true;
   bool                 m_isLabelStrip = false;
   bool                 m_minimalMode  = false;
   juce::Rectangle<int> m_dirtyRect {};

   // Cached colours...
   juce::Colour m_mutedColour          = juce::Colours::red;
   juce::Colour m_mutedMouseOverColour = juce::Colours::blue;
   juce::Colour m_textColour           = juce::Colours::white;
   juce::Colour m_textValueColour      = juce::Colours::white;
   juce::Colour m_inactiveColour       = juce::Colours::red;
   juce::Colour m_faderColour          = juce::Colours::yellow.withAlpha (Constants::kFaderAlphaMax);
   juce::Colour m_backgroundColour     = juce::Colours::black;
   juce::Colour m_tickColour           = juce::Colours::white;
   juce::Colour m_peakColour           = juce::Colours::red;
   juce::Colour m_warningColour        = juce::Colours::yellow;
   juce::Colour m_normalColour         = juce::Colours::green;

   //==============================================================================
   // Private methods.
   void setDirty (bool isDirty = true) noexcept
   {
      m_dirtyRect = { 0, 0, 0, 0 };
      if (isDirty) m_dirtyRect = getLocalBounds();
   }
   [[nodiscard]] bool isDirty (const juce::Rectangle<int>& rectToCheck = {}) const noexcept;
   void               addDirty (juce::Rectangle<int> dirtyRect) noexcept { m_dirtyRect = m_dirtyRect.getUnion (dirtyRect); }

   void drawMeter (juce::Graphics& g);

   [[nodiscard]] juce::Colour getColourFromLnf (int colourId, const juce::Colour& fallbackColour) const;

#if SDTK_ENABLE_FADER
   void mouseDrag (const juce::MouseEvent& event) override;
   void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
   void mouseDown (const juce::MouseEvent& event) override;
#endif /* SDTK_ENABLE_FADER */

   void mouseMove (const juce::MouseEvent& event) override;
   void mouseExit (const juce::MouseEvent& event) override;
   void mouseDoubleClick (const juce::MouseEvent& event) override;
   void resetMouseOvers() noexcept;
   void setColours() noexcept;

   // clang-format on
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_COMPONENT_H */