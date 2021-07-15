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

//==============================================================================
MetersPanel::MetersPanel()
  : m_tickMarks ({ -1.0f, -3.0f, -6.0f, -9.0f, -18.0f }),  // Tick-mark position in db.
    m_meterDecayTime_ms (Constants::kDefaultDecay_ms),
    m_masterFader (Constants::kMetersPanelId, m_tickMarks, MeterPadding (kMasterFaderLeftPadding, 0, 0, 0), m_meterDecayTime_ms, false, false, true,
#if SDTK_ENABLE_FADER
                   this)
#else
                   nullptr)
#endif
{
   // Set master strip options...
   m_masterFader.setRegions (m_warningRegion_db, m_peakRegion_db);

#if SDTK_ENABLE_FADER
   m_masterFader.setFaderActive (m_fadersEnabled);
   m_masterFader.addMouseListener (this, true);
#endif

   addAndMakeVisible (m_masterFader);

   setPaintingIsUnclipped (true);

   startTimerHz (m_panelRefreshRate);
}

//==============================================================================
MetersPanel::MetersPanel (const juce::AudioChannelSet& channelFormat) : MetersPanel()
{
   m_channelFormat = channelFormat;

   createMeters (channelFormat, {});

   setName (Constants::kMetersPanelId);
}

//==============================================================================
MetersPanel::~MetersPanel()
{

#if SDTK_ENABLE_FADER
   m_masterFader.removeFaderListener (*this);
   m_masterFader.removeMouseListener (this);
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
   m_masterFader.setFaderActive (false);
#endif

   m_masterFader.showTickMarks (false);
   m_masterStripWidth = 0;
   m_channelFormat    = juce::AudioChannelSet::stereo();

   refresh (true);
}

//==============================================================================
void MetersPanel::refresh (const bool forceRefresh /*= false*/)
{
   m_masterFader.refresh (forceRefresh);
   for (auto* meter: m_meters)
      meter->refresh (forceRefresh);
}

//==============================================================================
void MetersPanel::setPanelRefreshRate (int refreshRate_hz) noexcept
{
   m_panelRefreshRate = refreshRate_hz;

   if (m_internalTimer)
   {
      stopTimer();
      startTimerHz (refreshRate_hz);
   }
}

//==============================================================================
void MetersPanel::setInternalTiming (bool useInternalTiming) noexcept
{
   m_internalTimer = useInternalTiming;

   stopTimer();

   if (useInternalTiming) startTimerHz (m_panelRefreshRate);
}

//==============================================================================
void MetersPanel::paint (juce::Graphics& g)
{
   g.setColour (m_backgroundColour);
   g.fillRect (getLocalBounds());

   if (! m_enabled)
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
   m_masterStripWidth = kLabelWidth;

   // Calculate meter width from available width taking into account the extra width needed when showing the master strip...
   m_meterWidth = std::clamp ((panelWidth - m_masterStripWidth) / numOfMeters, kMinWidth, kMaxWidth);

   bool minModeEnabled = m_meters[0]->autoSetMinimalMode (m_meterWidth, panelHeight);

   // Don't show the master strip in minimum mode...
   if (minModeEnabled) m_masterStripWidth = 0;

   // Calculate actual width (taking into account the min. mode)...
   m_meterWidth = std::clamp ((panelWidth - m_masterStripWidth) / numOfMeters, kMinWidth, kMaxWidth);

   // Position all meters and adapt them to the current size...
   for (auto meter: m_meters)
   {
      meter->setMinimalMode (minModeEnabled);
      meter->setBounds (panelBounds.removeFromLeft (m_meterWidth));  // ... set it's width to m_meterWidth

#if SDTK_ENABLE_FADER
      if (minModeEnabled) meter->setFaderActive (false);  // ... do not show the gain fader if it's too narrow.
#endif
   }

   // Position MASTER strip...
   if (m_masterStripWidth == 0)
   {
      m_masterFader.setBounds ({});
   }
   else
   {
      // Use the dimensions of the 'meter' part (without the 'header' and 'value' part).
      auto meterPartBounds = m_meters[0]->getMeterBounds();
      m_masterFader.setBounds (panelBounds.removeFromRight (m_masterStripWidth).withY (meterPartBounds.getY()).withHeight (meterPartBounds.getHeight()));
      m_masterFader.showTickMarks (true);
   }
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
      m_masterFader.setFaderActive (true);
   }
}

//==============================================================================
void MetersPanel::faderChanged (MeterComponent* sourceMeter, float value)
{
   if (sourceMeter == &m_masterFader)  // Master strip fader moves all channel faders relatively to each other.
   {
      jassert (m_faderGains.size() == m_faderGainsBuffer.size());  // NOLINT

      for (auto singleMeter: m_meters)  // ... apply it to the 'other' meters.
      {
         auto meterIdx = m_meters.indexOf (singleMeter);
         if (juce::isPositiveAndBelow (meterIdx, m_faderGains.size()) && singleMeter->isActive())
         {
            m_faderGains[meterIdx] = m_faderGainsBuffer[meterIdx] * value;                                // Multiply the gain with the master fader value.
            singleMeter->setFaderValue (m_faderGains[meterIdx], NotificationOptions::dontNotify, false);  //  Update the fader to display the new gain value.
         }
      }
   }
   else  // Any meter/fader but the master fader was moved ...
   {
      m_masterFader.setFaderValue (1.0f, NotificationOptions::dontNotify, false);  // ... reset the master fader.
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
   m_masterFader.setFaderActive (mustShowFaders);
   for (auto meter: m_meters)
      meter->setFaderActive (mustShowFaders);
}

//==============================================================================
bool MetersPanel::areAllChannelsInactive()
{
   for (auto meter: m_meters)
      if (meter->isActive()) return false;
   return true;
}

//==============================================================================
void MetersPanel::toggleMute()
{
   bool allChannelsInactive = areAllChannelsInactive();
   for (auto meter: m_meters)
   {
      meter->setActive (allChannelsInactive);
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
   m_masterFader.setFaderValue (1.0f);
   notifyListeners();
}

//==============================================================================
void MetersPanel::setFadersEnabled (bool fadersEnabled) noexcept
{
   for (auto* meter: m_meters)
      meter->setFaderEnabled (fadersEnabled);
   m_masterFader.setFaderEnabled (fadersEnabled);
   m_fadersEnabled = fadersEnabled;
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
      auto meter = std::make_unique<MeterComponent> (Constants::kMetersPanelId, m_tickMarks, MeterPadding (0, kFaderRightPadding, 0, 0), m_meterDecayTime_ms,
                                                     true, true, false,
#if SDTK_ENABLE_FADER
                                                     this,
#else
                                                     nullptr,
#endif
                                                     channelFormat.getTypeOfChannel (channelIdx));

      meter->setRegions (m_warningRegion_db, m_peakRegion_db);
      meter->setGuiRefreshRate (static_cast<float> (m_panelRefreshRate));
      meter->setFont (m_font);
      meter->addMouseListener (this, true);
      meter->setVisible (m_enabled);
      meter->setEnabled (m_enabled);
      meter->useGradients (m_useGradients);
#if SDTK_ENABLE_FADER
      meter->setFaderEnabled (m_fadersEnabled);
#endif
      addChildComponent (meter.get());

      m_meters.add (meter.release());
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

void calculateDefaultPanelWidth() { }

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

            // Calculate the default meter width so it fits the largest of channel names...
            defaultMeterWidth = std::max (defaultMeterWidth, m_meters[meterIdx]->getChannelNameWidth());
         }
      }
      else
      {
         // Calculate the default meter width so it fits the largest of full type descriptions...
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
   m_defaultPanelWidth = static_cast<int> (defaultMeterWidth * static_cast<float> (numMeters));  // Min. width needed for channel names.
   m_defaultPanelWidth += numMeters * kFaderRightPadding;                                        // Add the padding that is on the right side of the channels.
   m_defaultPanelWidth += kLabelWidth + kMasterFaderLeftPadding;                                 // Add master fader width (incl. padding).
   // m_mixerDefaultWidth += kLabelWidth;
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
   m_meterDecayTime_ms = decay_ms;
   for (auto* meter: m_meters)
      meter->setDecay (m_meterDecayTime_ms);
}

//==============================================================================
void MetersPanel::setFont (const juce::Font& newFont) noexcept
{
   for (auto* meter: m_meters)
      meter->setFont (newFont);
   m_masterFader.setFont (newFont);
   m_font = newFont;
}

//==============================================================================
void MetersPanel::setEnabled (bool enabled /*= true*/)
{
   m_enabled = enabled;

   for (auto meter: m_meters)
   {
      meter->setEnabled (enabled);
      meter->setVisible (enabled);
   }

   m_masterFader.setEnabled (enabled);
   m_masterFader.setVisible (enabled);

   refresh (true);
}

//==============================================================================
void MetersPanel::useGradients (bool useGradients) noexcept
{
   m_useGradients = useGradients;
   for (auto* meter: m_meters)
      meter->useGradients (useGradients);
}

//==============================================================================
void MetersPanel::setRegions (float warningRegion_db, float peakRegion_db)
{
   m_warningRegion_db = warningRegion_db;
   m_peakRegion_db    = peakRegion_db;

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