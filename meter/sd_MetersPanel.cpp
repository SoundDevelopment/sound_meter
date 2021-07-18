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

MetersPanel::MetersPanel() : MetersPanel (Options()) { }
//==============================================================================

MetersPanel::MetersPanel (Options meterOptions)
  : m_options (meterOptions),
    m_labelStrip (meterOptions, MeterPadding (kLabelStripLeftPadding, 0, 0, 0), Constants::kMetersPanelId, true, juce::AudioChannelSet::ChannelType::unknown,
#if SDTK_ENABLE_FADER
                  this)
#else
                  nullptr)
#endif
{
#if SDTK_ENABLE_FADER
   m_labelStrip.setFaderActive (m_options.faderEnabled);
   m_labelStrip.addMouseListener (this, true);
#endif

   addAndMakeVisible (m_labelStrip);

   setPaintingIsUnclipped (true);

   setName (Constants::kMetersPanelId);

   startTimerHz (m_options.refreshRate);
}
//==============================================================================

MetersPanel::MetersPanel (const juce::AudioChannelSet& channelFormat) : MetersPanel (Options(), channelFormat) {};
//==============================================================================

MetersPanel::MetersPanel (Options meterOptions, const juce::AudioChannelSet& channelFormat) : MetersPanel (meterOptions)
{
   m_channelFormat = channelFormat;

   createMeters (channelFormat, {});
}
//==============================================================================

MetersPanel::~MetersPanel()
{

#if SDTK_ENABLE_FADER
   m_labelStrip.removeFaderListener (*this);
   m_labelStrip.removeMouseListener (this);
#endif

   deleteMeters();
}
//==============================================================================

void MetersPanel::reset()
{
   deleteMeters();

#if SDTK_ENABLE_FADER
   m_faderGains.clear();
   m_faderGainsBuffer.clear();
   m_labelStrip.setFaderActive (false);
#endif

   m_labelStrip.showTickMarks (false);
   m_channelFormat   = juce::AudioChannelSet::stereo();

   refresh (true);
}
//==============================================================================

void MetersPanel::refresh (const bool forceRefresh /*= false*/)
{
   m_labelStrip.refresh (forceRefresh);
   for (auto* meter: m_meters)
      meter->refresh (forceRefresh);
}
//==============================================================================

void MetersPanel::setPanelRefreshRate (int refreshRate_hz) noexcept
{
   m_options.refreshRate = refreshRate_hz;

   m_labelStrip.setRefreshRate (static_cast<float> (refreshRate_hz));
   for (auto* meter: m_meters)
      meter->setRefreshRate (static_cast<float> (refreshRate_hz));

   if (m_useInternalTimer)
   {
      stopTimer();
      startTimerHz (refreshRate_hz);
   }
}
//==============================================================================

void MetersPanel::useInternalTiming (bool useInternalTiming) noexcept
{
   m_useInternalTimer = useInternalTiming;

   stopTimer();

   if (useInternalTiming) startTimerHz (m_options.refreshRate);
}
//==============================================================================

void MetersPanel::paint (juce::Graphics& g)
{
   g.setColour (m_backgroundColour);
   g.fillRect (getLocalBounds());

   if (! m_options.enabled)
   {
      g.setColour (m_backgroundColour.contrasting (1.0f));
      g.setFont (m_font.withHeight (14.0f));
      g.drawFittedText ("No audio device open for playback. ", getLocalBounds(), juce::Justification::centred, 5);  // NOLINT
   }
}
//==============================================================================

void MetersPanel::resized()
{
   using namespace Constants;

   // Get the number of meters present...
   const int numOfMeters = static_cast<int> (m_meters.size());

   if (numOfMeters == 0) return;

   // Get the meter bounds...
   auto       panelBounds = getLocalBounds();
   const auto panelHeight = panelBounds.getHeight();
   const auto panelWidth  = panelBounds.getWidth();

   // By default show the MASTER strip.
   auto labelStripWidth = m_useLabelStrip ? kLabelWidth : 0;

   // Calculate meter width from available width taking into account the extra width needed when showing the master strip...
   auto meterWidth = std::clamp ((panelWidth - labelStripWidth) / numOfMeters, kMinWidth, kMaxWidth);

   bool minModeEnabled = m_meters[0]->autoSetMinimalMode (meterWidth, panelHeight);

   // Don't show the label strip in minimum mode...
   if (minModeEnabled) labelStripWidth = 0;

   // Re-calculate actual width (taking into account the min. mode)...
   if (m_useLabelStrip) meterWidth = std::clamp ((panelWidth - labelStripWidth) / numOfMeters, kMinWidth, kMaxWidth);

   // Position all meters and adapt them to the current size...
   for (auto meter: m_meters)
   {
      meter->setMinimalMode (minModeEnabled);
      meter->setBounds (panelBounds.removeFromLeft (meterWidth));  // ... set it's width to m_meterWidth

#if SDTK_ENABLE_FADER
      if (minModeEnabled) meter->setFaderActive (false);  // ... do not show the gain fader if it's too narrow.
#endif
   }

   // Position MASTER strip...
   if (labelStripWidth == 0)
   {
      m_labelStrip.setBounds ({});
   }
   else
   {
      // Use the dimensions of the 'meter' part combined with the 'value' part...
      auto labelStripBounds = m_meters[0]->getLabelStripBounds();
      m_labelStrip.setBounds (panelBounds.removeFromRight (labelStripWidth).withY (labelStripBounds.getY()).withHeight (labelStripBounds.getHeight()));
      m_labelStrip.showTickMarks (true);
   }
}
//==============================================================================

void MetersPanel::setChannelNames (const std::vector<juce::String>& channelNames)
{
   using namespace Constants;

   const int numChannelNames = static_cast<int> (channelNames.size());

   const int numMeters = m_meters.size();

   auto defaultMeterWidth = static_cast<float> (kMinWidth);

   // Loop through all meters...
   for (int meterIdx = 0; meterIdx < numMeters; ++meterIdx)
   {
      if (meterIdx < numChannelNames)
      {
         if (channelNames[meterIdx].isNotEmpty())
         {
            m_meters[meterIdx]->setChannelName (channelNames[meterIdx]);  // ... and set the channel name.

            // Calculate the meter width so it fits the largest of channel names...
            defaultMeterWidth = std::max (defaultMeterWidth, m_meters[meterIdx]->getChannelNameWidth());
         }
      }
      else
      {
         // Calculate the meter width so it fits the largest of full type descriptions...
         defaultMeterWidth = std::max (defaultMeterWidth, m_meters[meterIdx]->getChannelTypeWidth());
      }
   }

   if (channelNames.empty())
   {
      for (auto& meter: m_meters)
         meter->setReferredTypeWidth (defaultMeterWidth);
   }

   // Calculate default mixer width...
   // This is the width at which all channel names can be displayed.
   m_autoSizedPanelWidth = static_cast<int> (defaultMeterWidth * static_cast<float> (numMeters));  // Min. width needed for channel names.
   m_autoSizedPanelWidth += numMeters * (2 * kFaderRightPadding);                                  // Add the padding that is on the right side of the channels.
   m_autoSizedPanelWidth += kLabelWidth + kLabelStripLeftPadding;                                  // Add master fader width (incl. padding).
}
//==============================================================================

#if SDTK_ENABLE_FADER

//==============================================================================
void MetersPanel::mouseExit (const juce::MouseEvent& /*event*/)
{
   showFaders (false);
}

//==============================================================================
void MetersPanel::mouseEnter (const juce::MouseEvent& /*event*/)
{
   if (m_meters.isEmpty()) return;

   if (! m_meters[0]->isMinimalModeActive())
   {
      for (auto meter: m_meters)
         meter->setFaderActive (true);
      m_labelStrip.setFaderActive (true);
   }
}

//==============================================================================
void MetersPanel::faderChanged (MeterComponent* sourceMeter, float value)
{
   // Master strip fader moves all channel faders relatively to each other...
   if (sourceMeter == &m_labelStrip)
   {
      jassert (m_faderGains.size() == m_faderGainsBuffer.size());  // NOLINT

      // If the master fader is ACTIVE ...
      if (m_labelStrip.isActive())
      {
         // ... but all meters are muted ...
         if (areAllMetersInactive()) muteAll (false);  // ... un- mute all meters ...

         // Apply the master fader VALUE to all meter faders ...
         for (auto singleMeter: m_meters)
         {
            auto meterIdx = m_meters.indexOf (singleMeter);
            if (juce::isPositiveAndBelow (meterIdx, m_faderGains.size()))
            {
               m_faderGains[meterIdx] = m_faderGainsBuffer[meterIdx] * value;                                // Multiply the gain with the master fader value.
               singleMeter->setFaderValue (m_faderGains[meterIdx], NotificationOptions::dontNotify, false);  // Update the fader to display the new gain value.
            }
         }
      }
      // If the master fader has been DE-ACTIVATED ...
      else
      {
         muteAll();                                                                  // ... mute all meters ...
         m_labelStrip.setFaderValue (1.0f, NotificationOptions::dontNotify, false);  // ... and set the master fader to unity gain.
      }
   }
   // Any meter/fader but the master fader was moved ...
   else
   {
      m_labelStrip.setFaderValue (1.0f, NotificationOptions::dontNotify, false);  // ... reset the master fader.
      getFaderValues (NotificationOptions::dontNotify);
   }

   notifyListeners();
}

//==============================================================================
void MetersPanel::getFaderValues (NotificationOptions notificationOption /*= NotificationOptions::notify*/)
{
   if (m_meters.isEmpty()) return;

   // Set number of mixer gains to match the number of channels...
   jassert (m_faderGains.size() == m_meters.size());  // NOLINT
   // if( static_cast<int>( m_mixerGains.size() ) != m_meters.size() ) m_mixerGains.resize( m_meters.size() );

   // Loop through all meters...
   for (int meterIdx = 0; meterIdx < m_meters.size(); ++meterIdx)
   {
      // If the meter is active, get the value from the fader, otherwise a value of 0.0 is used...
      m_faderGains[meterIdx] = (m_meters[meterIdx]->isActive() ? m_meters[meterIdx]->getFaderValue() : 0.0f);
   }

   // If all meters are in-active, so is the master fader ...
   m_labelStrip.setActive (! areAllMetersInactive(), NotificationOptions::dontNotify);

   m_faderGainsBuffer = m_faderGains;

   if (notificationOption == NotificationOptions::notify) notifyListeners();
}

//==============================================================================
void MetersPanel::notifyListeners()
{
   m_fadersListeners.call ([=] (FadersChangeListener& l) { l.fadersChanged (m_faderGains); });
}

//==============================================================================
void MetersPanel::showFaders (bool mustShowFaders)
{
   m_labelStrip.setFaderActive (mustShowFaders);
   for (auto meter: m_meters)
      meter->setFaderActive (mustShowFaders);
}

//==============================================================================
bool MetersPanel::areAllMetersInactive()
{
   for (auto meter: m_meters)
      if (meter->isActive()) return false;
   return true;
}

//==============================================================================
void MetersPanel::toggleMute()
{
   bool allChannelsInactive = areAllMetersInactive();
   muteAll (! allChannelsInactive);
}

//==============================================================================
void MetersPanel::muteAll (bool mute /*= true */)
{
   bool allChannelsInactive = areAllMetersInactive();
   if (mute == allChannelsInactive) return;  // All meters already muted.

   for (auto meter: m_meters)
   {
      meter->setActive (! mute);
      meter->flashFader();
   }
   getFaderValues();
}

//==============================================================================
void MetersPanel::resetFaders()
{
   std::fill (m_faderGains.begin(), m_faderGains.end(), 1.0f);  // Set all fader gains to unity.
   m_faderGainsBuffer = m_faderGains;                           // Copy the just reset fade gains to it's buffer.

   // Activate (un-mute) all faders and set them to unity gain...
   for (const auto& meter: m_meters)
   {
      meter->setActive (true);
      meter->setFaderValue (1.0f);
   }
   m_labelStrip.setFaderValue (1.0f);
   notifyListeners();
}

//==============================================================================
void MetersPanel::setFadersEnabled (bool faderEnabled) noexcept
{
   for (auto* meter: m_meters)
      meter->setFaderEnabled (faderEnabled);
   m_labelStrip.setFaderEnabled (faderEnabled);
   m_options.faderEnabled = faderEnabled;
}


#endif /* SDTK_ENABLE_FADER */

//==============================================================================
void MetersPanel::setInputLevel (int channel, float value)
{
   if (auto* meter = getMeter (channel)) meter->setInputLevel (value);
}

//==============================================================================
void MetersPanel::createMeters (const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames)
{
   // Create enough meters to match the channel format...
   for (int channelIdx = 0; channelIdx < channelFormat.size(); ++channelIdx)
   {
      auto meter = std::make_unique<MeterComponent> (m_options, MeterPadding (0, kFaderRightPadding, 0, 0), Constants::kMetersPanelId, false,
                                                     channelFormat.getTypeOfChannel (channelIdx),
#if SDTK_ENABLE_FADER
                                                     this
#else
                                                     nullptr
#endif
      );

      meter->setFont (m_font);
      meter->addMouseListener (this, true);

      addChildComponent (meter.get());
      m_meters.add (meter.release());

      m_labelStrip.setActive (true);
   }

   setChannelNames (channelNames);
}

//==============================================================================
void MetersPanel::deleteMeters()
{
#if SDTK_ENABLE_FADER
   for (auto meter: m_meters)
   {
      meter->removeFaderListener (*this);
      meter->removeMouseListener (this);
   }
#endif

   m_meters.clear();
}

//==============================================================================
MeterComponent* MetersPanel::getMeter (const int meterIndex) noexcept
{
   return (juce::isPositiveAndBelow (meterIndex, m_meters.size()) ? m_meters[meterIndex] : nullptr);
}

//==============================================================================
void MetersPanel::setNumChannels (int numChannels, const std::vector<juce::String>& channelNames /*= {}*/)
{
   if (numChannels <= 0) return;

   setChannelFormat (juce::AudioChannelSet::canonicalChannelSet (numChannels), channelNames);
}

//==============================================================================
void MetersPanel::setChannelFormat (const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames)
{
   if (channelFormat.size() == 0) return;

   m_channelFormat = channelFormat;

   // Make sure the number of meters matches the number of channels ...
   if (channelFormat.size() != m_meters.size())
   {
      deleteMeters();                              // ... if not, then delete all previous meters ...
      createMeters (channelFormat, channelNames);  // ... and create new ones, matching the required channel format.
   }

   // Set the channel names...
   setChannelNames (channelNames);

   // Resize the mixer to accommodate any optionally added meters...
   resized();

#if SDTK_ENABLE_FADER

   // Make sure the number of mixer gains matches the number of channels ...
   if (channelFormat.size() != static_cast<int> (m_faderGains.size()))
      m_faderGains.resize (channelFormat.size());  // ... and if not resize the mixer gains to accommodate.

   resetFaders();

#endif /* SDTK_ENABLE_FADER */
}

//==============================================================================
void MetersPanel::resetMeters()
{
   for (auto* meter: m_meters)
      meter->reset();
}

//==============================================================================
void MetersPanel::resetPeakHold()
{
   for (auto* meter: m_meters)
      meter->resetPeakHold();
}

//==============================================================================
void MetersPanel::setMeterDecay (float decay_ms)
{
   m_options.meterDecayTime_ms = decay_ms;
   for (auto* meter: m_meters)
      meter->setDecay (decay_ms);
}
//==============================================================================

void MetersPanel::setFont (const juce::Font& newFont) noexcept
{
   m_font = newFont;
   for (auto* meter: m_meters)
      meter->setFont (newFont);
   m_labelStrip.setFont (newFont);
}
//==============================================================================

void MetersPanel::setOptions (Options meterOptions)
{
   m_options = meterOptions;
   for (auto* meter: m_meters)
      meter->setOptions (meterOptions);
   m_labelStrip.setOptions (meterOptions);
}
//==============================================================================

void MetersPanel::setEnabled (bool enabled /*= true*/)
{
   m_options.enabled = enabled;

   for (auto meter: m_meters)
   {
      meter->setEnabled (enabled);
      meter->setVisible (enabled);
   }

   m_labelStrip.setEnabled (enabled);
   m_labelStrip.setVisible (enabled);

   refresh (true);
}
//==============================================================================

void MetersPanel::showTickMarks (bool showTickMarks)
{
   m_options.showTickMarks = showTickMarks;
   for (auto* meter: m_meters)
      meter->showTickMarks (showTickMarks);
   m_labelStrip.showTickMarks (showTickMarks);
}
//==============================================================================

void MetersPanel::useGradients (bool useGradients) noexcept
{
   m_options.useGradient = useGradients;
   for (auto* meter: m_meters)
      meter->useGradients (useGradients);
}
//==============================================================================

void MetersPanel::useLabelStrip (bool useLabelStrip)
{
   m_useLabelStrip = useLabelStrip;
   resized();
}
//==============================================================================

void MetersPanel::enableHeader (bool headerEnabled)
{
   if (m_options.headerEnabled == headerEnabled) return;

   m_options.headerEnabled = headerEnabled;
   m_labelStrip.enableHeader (headerEnabled);

   for (auto* meter: m_meters)
      meter->enableHeader (headerEnabled);

   resized();
}
//==============================================================================

void MetersPanel::enableValue (bool valueEnabled)
{
   if (m_options.valueEnabled == valueEnabled) return;

   m_labelStrip.enableValue (valueEnabled);

   for (auto* meter: m_meters)
      meter->enableValue (valueEnabled);

   resized();
}


//==============================================================================
void MetersPanel::setRegions (float warningRegion_db, float peakRegion_db)
{
   m_options.warningRegion_db = warningRegion_db;
   m_options.peakRegion_db    = peakRegion_db;

   for (auto* meter: m_meters)
      meter->setRegions (warningRegion_db, peakRegion_db);
}

//==============================================================================
void MetersPanel::setColours()
{
   if (isColourSpecified (MeterComponent::backgroundColourId))
      m_backgroundColour = findColour (MeterComponent::backgroundColourId);
   else if (getLookAndFeel().isColourSpecified (MeterComponent::backgroundColourId))
      m_backgroundColour = getLookAndFeel().findColour (MeterComponent::backgroundColourId);
}

}  // namespace sd::SoundMeter