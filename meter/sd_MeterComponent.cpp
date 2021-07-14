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


NewMeterComponent::NewMeterComponent (const juce::String& name, const std::vector<float>& ticks, MeterPadding padding, float meterDecy, bool headerVisible,
                                      bool valueVisible, bool isLabelStrip /*= false*/, SoundMeter::Fader::Listener* faderListener /*= nullptr*/,
                                      int channelIdx /*= 0*/, ChannelType channelType /*= ChannelType::unknown*/)
  : NewMeterComponent()
{
   setName (name);
   setTickMarks (ticks);
   setPadding (padding);
   setDecay (meterDecy);
   setChannelType (channelType);

   setHeaderVisible (headerVisible);
   setIsLabelStrip (isLabelStrip);
   m_level.setPeakValueVisible (valueVisible);

   m_header.setIndex (channelIdx);

   if (faderListener) addFaderListener (*faderListener);
}

//==============================================================================
void NewMeterComponent::reset()
{
   m_level.reset();
   refresh (true);
}

//==============================================================================
void NewMeterComponent::flashFader()
{
   m_fader.flash();
   refresh (true);
}

//==============================================================================
[[nodiscard]] juce::Colour NewMeterComponent::getColourFromLnf (int colourId, const juce::Colour& fallbackColour) const
{
   if (isColourSpecified (colourId)) return findColour (colourId);
   if (getLookAndFeel().isColourSpecified (colourId)) return getLookAndFeel().findColour (colourId);

   return fallbackColour;
}

//==============================================================================
void NewMeterComponent::setColours() noexcept
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
[[nodiscard]] bool NewMeterComponent::autoSetMinimalMode (int proposedWidth, int proposedHeight)
{
   bool minimalMode = ! nameFits ("Lfe", proposedWidth);
   if (proposedWidth < Constants::kMinModeWidthThreshold) minimalMode = true;
   if (proposedHeight < Constants::kMinModeHeightThreshold) minimalMode = true;

   setMinimalMode (minimalMode);

   return minimalMode;
}

//==============================================================================
void NewMeterComponent::setMinimalMode (bool minimalMode) noexcept
{
   if (minimalMode == m_minimalMode) return;

   m_minimalMode = minimalMode;
   showTickMarks (! m_minimalMode);                // ... show tick marks if it's not too narrow for ID and not in minimum mode.
   setHeaderVisible (! m_minimalMode);             // ... show channel ID if it's not too narrow for ID and not in minimum mode.
   showTickMarks (! m_minimalMode);                // ... show tick marks if it's not too narrow for ID and not in minimum mode.
   m_level.setPeakValueVisible (! m_minimalMode);  // ... show peak value if it's not too narrow for ID and not in minimum mode.
   setDirty();
}

//==============================================================================
void NewMeterComponent::lookAndFeelChanged()
{
   setColours();
   setDirty();
}

//==============================================================================
void NewMeterComponent::visibilityChanged()
{
   setColours();
   setDirty();
}

//==============================================================================
void NewMeterComponent::showPeakValue (bool showPeakValue /*= true*/)
{
   m_level.setPeakValueVisible (showPeakValue);
   setDirty();
}

//==============================================================================
void NewMeterComponent::showTickMarks (bool showTickMarks /*= true*/)
{
   m_level.setTickMarksVisible (showTickMarks);
   setDirty();
}

//==============================================================================
void NewMeterComponent::showPeakHold (bool showPeakHold /*= true*/)
{
   m_level.setPeakHoldVisible (showPeakHold);
   setDirty();
}

#pragma endregion


#pragma region Draw Methods

//==============================================================================
void NewMeterComponent::resized()
{
   auto meterBounds = SoundMeter::Helpers::applyPadding (getLocalBounds(), m_padding);

   m_level.setValueBounds (m_level.getValueBounds().withHeight (0));  // Set value height to a default of 0.
   m_header.setBounds (m_header.getBounds().withHeight (0));          // Set header height to a default of 0.

   // Resize channel name and value...
   if (! m_isLabelStrip)  // Label strips do not have channel names or peak values.
   {
      // channel IDs.
      if (m_header.isVisible()) m_header.setBounds (meterBounds.removeFromTop (Constants::kChannelNameHeight));

      // Draw peak value.
      const bool wideEnoughForValue = m_header.getFont().getStringWidth ("-99.99") <= meterBounds.getWidth();
      if (m_level.isPeakValueVisible() && wideEnoughForValue) m_level.setValueBounds (meterBounds.removeFromBottom (Constants::kChannelNameHeight));
   }

   m_fader.setBounds (meterBounds);
   m_level.setMeterBounds (meterBounds);
}

//==============================================================================
void NewMeterComponent::paint (juce::Graphics& g)
{
   if (getLocalBounds().isEmpty()) return;

   // Draw BACKGROUND ...
   g.setColour (m_backgroundColour);
   g.fillRect (getLocalBounds());

   g.setFont (m_header.getFont());

   // Draw the METER, unless it is a LABEL strip. Then draw the level values...
   m_isLabelStrip ? m_level.drawLabels (g, m_textColour) : drawMeter (g);

   // Draw FADER....
   m_fader.draw (g, m_faderColour);
   setDirty (false);
}

//==============================================================================
void NewMeterComponent::drawMeter (juce::Graphics& g)
{
   using namespace SoundMeter::Constants;

   // Draw channel HEADER...
   m_header.draw (g, isActive(), m_fader.isEnabled(), m_mutedColour, m_mutedMouseOverColour, m_textColour, m_inactiveColour);

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
[[nodiscard]] bool NewMeterComponent::isDirty (const juce::Rectangle<int>& rectToCheck /*= {}*/) const noexcept
{
   if (rectToCheck.isEmpty()) return ! m_dirtyRect.isEmpty();
   return m_dirtyRect.intersects (rectToCheck);
}

//==============================================================================
void NewMeterComponent::refresh (const bool forceRefresh)
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

      // Repaint if the faders are being faded out...
      if (! isDirty (m_level.getMeterBounds()) && m_fader.isFading()) addDirty (m_level.getMeterBounds());
   }

   // Redraw if dirty or forced to...
   if (forceRefresh)
      repaint();
   else if (isDirty())
      repaint (m_dirtyRect);
}

#pragma endregion


#pragma region Properties

//==============================================================================
void NewMeterComponent::setActive (bool isActive, NotificationOptions notify /*= NotificationOptions::dontNotify*/)
{
   if (m_active == isActive) return;
   reset();
   m_active = isActive;
   if (notify == NotificationOptions::notify) m_fader.notify();
   setDirty();
}

//==============================================================================
void NewMeterComponent::setFaderActive (const bool faderActive /*= true */)
{
   m_fader.setActive (faderActive);

   // If slider needs to be DE-ACTIVATED...
   if (! faderActive || ! m_fader.isEnabled()) resetMouseOvers();

   addDirty (m_fader.getBounds());
}

void NewMeterComponent::resetMouseOvers() noexcept
{
   m_header.resetMouseOver();
   m_level.resetMouseOverValue();
}

//==============================================================================
void NewMeterComponent::setFaderValue (const float value, NotificationOptions notificationOption /*= NotificationOptions::DontNotify*/, const bool mustShowFader /*= true*/) noexcept
{
   if (m_fader.setValue (value, notificationOption))
   {
      if (mustShowFader && ! m_fader.isActive()) flashFader();

      addDirty (m_fader.getBounds());
   }
}

//==============================================================================
void NewMeterComponent::setFaderEnabled (bool faderEnabled /*= true*/)
{
   m_fader.setEnabled (faderEnabled);
   addDirty (m_fader.getBounds());
}

//==============================================================================
void NewMeterComponent::setFont (const juce::Font& font) noexcept
{
   m_header.setFont (font.withHeight (Constants::kChannelNameFontHeight));
   setDirty();
}

//==============================================================================
void NewMeterComponent::resetPeakHold() noexcept
{
   m_level.resetPeakHoldLevel();
   setDirty();
}

#pragma endregion


#pragma region Mouse Methods

//==============================================================================
void NewMeterComponent::mouseDown (const juce::MouseEvent& event)
{
   // Left mouse button down and fader is active...
   if (event.mods == juce::ModifierKeys::leftButtonModifier && m_fader.isActive())
   {
      // Clicked on the METER part...
      if (! m_header.isMouseOver (event.y) && ! m_level.isMouseOverValue (event.y))
      {
         if (! isActive()) setActive (true);  // Activate if it was de-activated.
         m_fader.setValueFromPos (event.y);   // Set the fader level at the value clicked.
         addDirty (m_fader.getBounds());
      }
      // Clicked on the HEADER part...
      else if (m_header.isMouseOver (event.y))
      {
         setActive (! isActive(), NotificationOptions::notify);
      }
   }
}

//==============================================================================
void NewMeterComponent::mouseMove (const juce::MouseEvent& event)
{
   setTooltip ("");

   // Check if the mouse is over the header part...
   bool isMouseOverHeader      = m_header.isMouseOver();                                 // Get the previous mouse over flag, to check if it has changed.
   bool mouseOverHeaderChanged = (isMouseOverHeader != m_header.isMouseOver (event.y));  // Check if it has changed.
   if (m_header.isMouseOver() && mouseOverHeaderChanged && m_fader.isEnabled())
   {
      setMouseCursor (juce::MouseCursor::PointingHandCursor);  // NOLINT
      setTooltip ("Mute or un-mute channel");                  // If it is over and changed set the tooltip.
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
      if (m_fader.isActive())
      {
         setMouseCursor (juce::MouseCursor::PointingHandCursor);
         setTooltip (m_isLabelStrip ? "Drag to move master fader." : "Drag to move fader.");
      }
      else
      {
         setMouseCursor (juce::MouseCursor::NormalCursor);
      }
   }
}

//==============================================================================
void NewMeterComponent::mouseExit (const juce::MouseEvent& /*event*/)
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
void NewMeterComponent::mouseDrag (const juce::MouseEvent& event)
{
   // When left button down, the meter is active, the fader is active and the mouse is not over the 'info' area...
   if (event.mods == juce::ModifierKeys::leftButtonModifier && isActive() && m_fader.isActive() && ! m_header.isMouseOver (event.y)
       && ! m_level.isMouseOverValue (event.y))
   {
      m_fader.setValueFromPos (event.y);
      addDirty (m_fader.getBounds());
   }
}

//==============================================================================
void NewMeterComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
   if (event.mods == juce::ModifierKeys::leftButtonModifier)
   {
      if (! m_header.isMouseOver (event.y))
      {
         if (m_level.isMouseOverValue (event.y))  // Double clicking on VALUE resets peak hold...
            resetPeakHold();
         else if (isActive())  // Double clicking on the FADER resets it to 0...
            setFaderValue (1.0f, NotificationOptions::notify);
      }
   }
}

//==============================================================================
void NewMeterComponent::mouseWheelMove (const juce::MouseEvent& /*event*/, const juce::MouseWheelDetails& wheel)
{
   setFaderValue (std::clamp<float> (m_fader.getValue() + (wheel.deltaY / Constants::kFaderSensitivity), 0.0f, 1.0f), NotificationOptions::notify, false);
}

//==============================================================================
void NewMeterComponent::setHeader (Header header) noexcept
{
   m_header = header;
   refresh (true);
}

//==============================================================================
void NewMeterComponent::setChannelName (const juce::String& channelName)
{
   m_header.setName (channelName);
   addDirty (m_header.getBounds());
}


#pragma endregion

}  // namespace sd::SoundMeter