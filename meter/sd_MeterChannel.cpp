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


namespace sd::SoundMeter
{

#pragma region Misc Methods

MeterChannel::MeterChannel()
#if SDTK_ENABLE_FADER
  : m_fader (this)
#endif
{
   setPaintingIsUnclipped (true);
}
//==============================================================================

MeterChannel::MeterChannel (Options meterOptions, Padding padding, const juce::String& channelName, bool isLabelStrip /*= false*/,
                            ChannelType channelType /*= ChannelType::unknown*/, [[maybe_unused]] Fader::Listener* faderListener /*= nullptr*/)
  : MeterChannel()
{
   setName (channelName);
   setChannelType (channelType);

   setOptions (meterOptions);

   setIsLabelStrip (isLabelStrip);

   setPadding (padding);

#if SDTK_ENABLE_FADER
   if (faderListener) addFaderListener (*faderListener);
#endif
}
//==============================================================================

void MeterChannel::reset() noexcept
{
   m_level.reset();
   setDirty();
}


#if SDTK_ENABLE_FADER

//==============================================================================

void MeterChannel::flashFader() noexcept
{
   m_fader.flash();
   addDirty (m_fader.getBounds());
}

#endif /* SDTK_ENABLE_FADER */

//==============================================================================

[[nodiscard]] juce::Colour MeterChannel::getColourFromLnf (int colourId, const juce::Colour& fallbackColour) const
{
   if (isColourSpecified (colourId)) return findColour (colourId);
   if (getLookAndFeel().isColourSpecified (colourId)) return getLookAndFeel().findColour (colourId);

   return fallbackColour;
}
//==============================================================================

[[nodiscard]] bool MeterChannel::autoSetMinimalMode (int proposedWidth, int proposedHeight)
{
   bool minimalMode = ! nameFits ("Lfe", proposedWidth);
   if (proposedWidth < Constants::kMinModeWidthThreshold) minimalMode = true;
   if (proposedHeight < Constants::kMinModeHeightThreshold) minimalMode = true;

   setMinimalMode (minimalMode);

   return minimalMode;
}
//==============================================================================

[[nodiscard]] juce::Rectangle<int> MeterChannel::getLabelStripBounds() const noexcept
{
   return m_level.getMeterBounds().getUnion (m_header.getBounds());
}
//==============================================================================

void MeterChannel::setMinimalMode (bool minimalMode) noexcept
{
   if (minimalMode == m_minimalMode) return;

   m_minimalMode = minimalMode;
   showTickMarks (! m_minimalMode);            // ... show tick marks if it's not too narrow for ID and not in minimum mode.
   showHeader (! m_minimalMode);               // ... show channel ID if it's not too narrow for ID and not in minimum mode.
   showTickMarks (! m_minimalMode);            // ... show tick marks if it's not too narrow for ID and not in minimum mode.
   m_level.setValueVisible (! m_minimalMode);  // ... show peak value if it's not too narrow for ID and not in minimum mode.
   setDirty();
}
//==============================================================================

void MeterChannel::lookAndFeelChanged()
{
   setColours();
   setDirty();
}
//==============================================================================

void MeterChannel::visibilityChanged()
{
   setColours();
   setDirty();
}
//==============================================================================

void MeterChannel::setColours() noexcept
{
   m_backgroundColour     = getColourFromLnf (backgroundColourId, juce::Colours::black);
   m_inactiveColour       = getColourFromLnf (inactiveColourId, juce::Colours::red);
   m_textColour           = getColourFromLnf (textColourId, juce::Colours::white);
   m_textValueColour      = getColourFromLnf (textValueColourId, juce::Colours::white);
   m_mutedColour          = getColourFromLnf (mutedColourId, juce::Colours::red);
   m_mutedMouseOverColour = getColourFromLnf (mutedMouseOverColourId, juce::Colours::blue);
   m_faderColour          = getColourFromLnf (faderColourId, juce::Colours::yellow.withAlpha (Constants::kFaderAlphaMax));
   m_tickColour           = getColourFromLnf (tickMarkColourId, juce::Colours::white);
   m_peakColour           = getColourFromLnf (peakColourId, juce::Colours::red);
   m_warningColour        = getColourFromLnf (warningColourId, juce::Colours::yellow);
   m_normalColour         = getColourFromLnf (normalColourId, juce::Colours::green);
}
//==============================================================================

void MeterChannel::enableHeader (bool headerEnabled) noexcept
{
   m_header.setEnabled (headerEnabled);
   if (headerEnabled) m_header.setVisible (true);
   addDirty (m_header.getBounds());
}
//==============================================================================

void MeterChannel::showHeader (bool headerVisible) noexcept
{
   m_header.setVisible (headerVisible);
   addDirty (m_header.getBounds());
}
//==============================================================================

void MeterChannel::enableValue (bool valueEnabled /*= true*/) noexcept
{
   m_level.enableValue (valueEnabled);
   if (valueEnabled) m_level.setValueVisible (true);
   addDirty (m_level.getValueBounds());
}
//==============================================================================

void MeterChannel::showValue (bool showValue /*= true*/) noexcept
{
   m_level.setValueVisible (showValue);
   setDirty();
}
//==============================================================================

void MeterChannel::showTickMarks (bool showTickMarks /*= true*/) noexcept
{
   m_level.setTickMarksVisible (showTickMarks);
   setDirty();
}
//==============================================================================

void MeterChannel::showPeakHold (bool showPeakHold /*= true*/) noexcept
{
   m_level.setPeakHoldVisible (showPeakHold);
   setDirty();
}

#pragma endregion


#pragma region Draw Methods

//==============================================================================

void MeterChannel::resized()
{
   auto meterBounds = SoundMeter::Helpers::applyPadding (getLocalBounds(), m_padding);

   m_level.setValueBounds (m_level.getValueBounds().withHeight (0));  // Set value height to a default of 0.
   m_header.setBounds (m_header.getBounds().withHeight (0));          // Set header height to a default of 0.

   // channel IDs.
   if (m_header.isVisible()) m_header.setBounds (meterBounds.removeFromTop (Constants::kChannelNameHeight));

   // Resize channel name and value...
   if (! m_isLabelStrip)  // Label strips do not have channel names or peak values.
   {
      // Draw peak value.
      const bool wideEnoughForValue = m_header.getFont().getStringWidth ("-99.99") <= meterBounds.getWidth();
      if (m_level.isPeakValueVisible() && wideEnoughForValue) m_level.setValueBounds (meterBounds.removeFromBottom (Constants::kChannelNameHeight));
   }

#if SDTK_ENABLE_FADER
   m_fader.setBounds (meterBounds);
#endif
   m_level.setMeterBounds (meterBounds);
}
//==============================================================================

void MeterChannel::paint (juce::Graphics& g)
{
   if (getLocalBounds().isEmpty()) return;

   // Draw BACKGROUND ...
   g.setColour (m_backgroundColour);
   g.fillRect (getLocalBounds());

   g.setFont (m_header.getFont());

   // Draw channel HEADER...

   bool faderEnabled = false;

#if SDTK_ENABLE_FADER
   faderEnabled = m_fader.isEnabled();
#endif

   m_header.draw (g, isActive(), faderEnabled, m_mutedColour, m_mutedMouseOverColour, m_textColour, m_inactiveColour);

   // Draw the LABEL STRIP ...
   if (m_isLabelStrip)
   {
      // If not active, fill an inactive background.
      if (! m_active)
      {
         g.setColour (m_inactiveColour);
         g.fillRect (m_level.getMeterBounds());
      }

      m_level.drawLabels (g, m_textColour);
   }
   // ... otherwise draw the METER ...
   else
   {
      drawMeter (g);
   }

#if SDTK_ENABLE_FADER
   // Draw FADER....
   m_fader.draw (g, m_faderColour);
#endif

   setDirty (false);
}
//==============================================================================

void MeterChannel::drawMeter (juce::Graphics& g)
{
   using namespace SoundMeter::Constants;

   // Draw peak level VALUE...
   m_level.drawPeakValue (g, m_textValueColour);

   // Draw meter BACKGROUND...
   g.setColour (m_active ? m_backgroundColour : m_inactiveColour);
   g.fillRect (m_level.getMeterBounds());

   // Draw TICK-marks...
   m_level.drawTickMarks (g, m_tickColour);

   // Draw meter BAR SEGMENTS (normal, warning, peak)...
   m_active ? m_level.drawMeter (g, m_peakColour, m_warningColour, m_normalColour) : m_level.drawInactiveMeter (g, m_textColour.darker (0.7f));

   // Draw peak HOLD line...
   if (m_active) m_level.drawPeakHold (g, m_peakColour);
}
//==============================================================================

[[nodiscard]] bool MeterChannel::isDirty (const juce::Rectangle<int>& rectToCheck /*= {}*/) const noexcept
{
   if (rectToCheck.isEmpty()) return ! m_dirtyRect.isEmpty();
   return m_dirtyRect.intersects (rectToCheck);
}
//==============================================================================

void MeterChannel::setDirty (bool isDirty /*= true*/) noexcept
{
   m_dirtyRect = { 0, 0, 0, 0 };
   if (isDirty) m_dirtyRect = getLocalBounds();
}
//==============================================================================

void MeterChannel::refresh (const bool forceRefresh)
{
   if (m_active)
   {
      // Get input level...
      const auto callbackLevel = m_level.getInputLevel();
      const auto height        = static_cast<float> (m_level.getMeterBounds().getHeight());
      auto       level_px      = static_cast<int> (callbackLevel * height);

      // Check if the value part needs to be redrawn....
      if (callbackLevel > m_level.getPeakHoldLevel() && m_level.isPeakValueVisible()) addDirty (m_level.getValueBounds());

      // Only calculate level when it has changed (and not on label strips)...
      if (! m_isLabelStrip && level_px != m_level.getLevelDrawn())
      {
         m_level.setMeterLevel (callbackLevel);

         if (! isDirty (m_level.getMeterBounds()))
         {
            // Check if there is a different level then currently displayed...
            level_px = static_cast<int> (height * m_level.getMeterLevel());
            if (level_px != m_level.getLevelDrawn()) addDirty (m_level.getMeterBounds());  // ... if so, meter part is 'dirty' and needs to redrawn.
         }
      }
   }
#if SDTK_ENABLE_FADER
   // Repaint if the faders are being faded out...
   if (! isDirty (m_level.getMeterBounds()) && m_fader.isFading()) addDirty (m_level.getMeterBounds());
#endif


   // Redraw if dirty or forced to...
   if (forceRefresh)
      repaint();
   else if (isDirty())
      repaint (m_dirtyRect);
}

#pragma endregion


#pragma region Properties

//==============================================================================

void MeterChannel::setActive (bool isActive, [[maybe_unused]] NotificationOptions notify /*= NotificationOptions::dontNotify*/)
{
   if (m_active == isActive) return;
   reset();
   m_active = isActive;

#if SDTK_ENABLE_FADER
   if (notify == NotificationOptions::notify) m_fader.notify();
#endif

   setDirty();
}
//==============================================================================

void MeterChannel::resetMouseOvers() noexcept
{
   m_header.resetMouseOver();
   m_level.resetMouseOverValue();
}
//==============================================================================

void MeterChannel::setFont (const juce::Font& font) noexcept
{
   m_header.setFont (font.withHeight (Constants::kChannelNameFontHeight));
   setDirty();
}
//==============================================================================

void MeterChannel::resetPeakHold() noexcept
{
   m_level.resetPeakHoldLevel();
   setDirty();
}
//==============================================================================

void MeterChannel::setOptions (Options meterOptions)
{
   setVisible (meterOptions.enabled);
   setEnabled (meterOptions.enabled);
   setRegions (meterOptions.warningRegion_db, meterOptions.peakRegion_db);
   setTickMarks (meterOptions.tickMarks);
   showTickMarks (meterOptions.showTickMarks);
   setDecay (meterOptions.meterDecayTime_ms);
   useGradients (meterOptions.useGradient);
   enableHeader (meterOptions.headerEnabled);
   enableValue (meterOptions.valueEnabled);
   setRefreshRate (static_cast<float> (meterOptions.refreshRate));

   m_level.setPeakHoldVisible (meterOptions.showPeakHoldIndicator);

#if SDTK_ENABLE_FADER
   setFaderEnabled (meterOptions.faderEnabled);
#endif
}
//==============================================================================

void MeterChannel::setChannelName (const juce::String& channelName)
{
   m_header.setName (channelName);
   addDirty (m_header.getBounds());
}
//==============================================================================

void MeterChannel::setRegions (float warningRegion_db, float peakRegion_db)
{
   m_level.setRegions (warningRegion_db, peakRegion_db);
   refresh (true);
}
//==============================================================================

#if SDTK_ENABLE_FADER

void MeterChannel::setFaderActive (const bool faderActive /*= true */) noexcept
{
   m_fader.setActive (faderActive);

   // If slider needs to be DE-ACTIVATED...
   if (! faderActive || ! m_fader.isEnabled()) resetMouseOvers();

   addDirty (m_fader.getBounds());
}
//==============================================================================

void MeterChannel::setFaderValue (const float value, NotificationOptions notificationOption /*= NotificationOptions::DontNotify*/, const bool mustShowFader /*= true*/)
{
   if (m_fader.setValue (value, notificationOption))
   {
      if (mustShowFader && ! m_fader.isActive()) flashFader();

      addDirty (m_fader.getBounds());
   }
}
//==============================================================================

void MeterChannel::setFaderEnabled (bool faderEnabled /*= true*/) noexcept
{
   m_fader.setEnabled (faderEnabled);
   addDirty (m_fader.getBounds());
}

#endif /* SDTK_ENABLE_FADER */

#pragma endregion


#pragma region Mouse Methods


#if SDTK_ENABLE_FADER

//==============================================================================

void MeterChannel::mouseDown (const juce::MouseEvent& event)
{
   // Left mouse button down and fader is active...
   if (event.mods == juce::ModifierKeys::leftButtonModifier && m_fader.isEnabled())
   {
      // Clicked on the METER part...
      if (! m_header.isMouseOver (event.y) && ! m_level.isMouseOverValue (event.y) && m_fader.isActive())
      {
         if (! isActive()) setActive (true);  // Activate if it was de-activated.
         m_fader.setValueFromPos (event.y);   // Set the fader level at the value clicked.
         addDirty (m_fader.getBounds());
      }

      // Clicked on the HEADER part...
      if (m_header.isMouseOver (event.y))
      {
         setActive (! isActive(), NotificationOptions::notify);
      }
   }
}

#endif /* SDTK_ENABLE_FADER */
//==============================================================================

void MeterChannel::mouseMove (const juce::MouseEvent& event)
{
   // Check if the FADER is enabled...
   bool faderEnabled = false;
#if SDTK_ENABLE_FADER
   faderEnabled = m_fader.isEnabled();
#endif

   // Check if the mouse is over the header part...
   bool isMouseOverHeader      = m_header.isMouseOver();                                 // Get the previous mouse over flag, to check if it has changed.
   bool mouseOverHeaderChanged = (isMouseOverHeader != m_header.isMouseOver (event.y));  // Check if it has changed.
   if (m_header.isMouseOver() && mouseOverHeaderChanged && faderEnabled)  // If the mouse entered the 'header' part for the first time and the fader is enabled...
   {
      setMouseCursor (juce::MouseCursor::PointingHandCursor);
      setTooltip ("Mute or un-mute channel");
   }
   if (mouseOverHeaderChanged) addDirty (m_header.getBounds());  // Mouse over status has changed. Repaint.

   // Check if the mouse is over the value part...
   bool isMouseOverValue      = m_level.isMouseOverValue();
   bool mouseOverValueChanged = (isMouseOverValue != m_level.isMouseOverValue (event.y));
   if (m_level.isMouseOverValue() && mouseOverValueChanged)
   {
      setMouseCursor (juce::MouseCursor::PointingHandCursor);  // NOLINT
      setTooltip ("Double click to clear peak hold.");
   }
   if (mouseOverValueChanged) addDirty (m_level.getValueBounds());

   // Check if the mouse is over the meter part...
   if (! m_header.isMouseOver() && ! m_level.isMouseOverValue())
   {

#if SDTK_ENABLE_FADER
      if (m_fader.isActive())
      {
         setMouseCursor (juce::MouseCursor::PointingHandCursor);
         setTooltip (m_isLabelStrip ? "Drag to move master fader." : "Drag to move fader.");
      }
      else
#endif /* SDTK_ENABLE_FADER */

      {
         setMouseCursor (juce::MouseCursor::NormalCursor);
      }
   }
}
//==============================================================================

void MeterChannel::mouseExit (const juce::MouseEvent& /*event*/)
{
   if (m_header.isMouseOver())
      addDirty (m_header.getBounds());
   else if (m_level.isMouseOverValue())
      addDirty (m_level.getValueBounds());
   else
      addDirty (m_level.getMeterBounds());
   resetMouseOvers();
}
//==============================================================================

void MeterChannel::mouseDoubleClick (const juce::MouseEvent& event)
{
   if (event.mods == juce::ModifierKeys::leftButtonModifier)
   {
      if (! m_header.isMouseOver (event.y))
      {
         if (m_level.isMouseOverValue (event.y))  // Double clicking on VALUE resets peak hold...
            resetPeakHold();
#if SDTK_ENABLE_FADER
         else if (isActive())  // Double clicking on the FADER resets it to 0...
            setFaderValue (1.0f, NotificationOptions::notify);
#endif /* SDTK_ENABLE_FADER */
      }
   }
}
#if SDTK_ENABLE_FADER
//==============================================================================

void MeterChannel::mouseDrag (const juce::MouseEvent& event)
{
   // When left button down, the meter is active, the fader is active and the mouse is not over the 'info' area...
   if (event.mods == juce::ModifierKeys::leftButtonModifier && isActive() && m_fader.isActive() && ! m_header.isMouseOver (event.y)
       && ! m_level.isMouseOverValue (event.y))
   {
      m_fader.setValueFromPos (event.y);
      addDirty (m_fader.getBounds().getUnion (m_header.getBounds()));
   }
}
//==============================================================================

void MeterChannel::mouseWheelMove (const juce::MouseEvent& /*event*/, const juce::MouseWheelDetails& wheel)
{
   setFaderValue (std::clamp<float> (m_fader.getValue() + (wheel.deltaY / Constants::kFaderSensitivity), 0.0f, 1.0f), NotificationOptions::notify, false);
}

#endif /* SDTK_ENABLE_FADER */

#pragma endregion

}  // namespace sd::SoundMeter