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

namespace sd
{

namespace SoundMeter
{

MetersComponent::MetersComponent() : MetersComponent (Options()) { }
//==============================================================================

MetersComponent::MetersComponent (Options meterOptions)
  : m_options (meterOptions),
    m_labelStrip (meterOptions, Padding (Constants::kLabelStripLeftPadding, 0, 0, 0), Constants::kLabelStripId, true, juce::AudioChannelSet::ChannelType::unknown)
{
#if SDTK_ENABLE_FADER
    m_labelStrip.enableFader (m_options.faderEnabled);
    m_labelStrip.addMouseListener (this, true);
    m_labelStrip.onFaderMove = [this] (MeterChannel* meterChannel) { faderChanged (meterChannel); };
#endif

    addAndMakeVisible (m_labelStrip);

    setName (Constants::kMetersPanelId);

    startTimerHz (static_cast<int> (std::round (m_options.refreshRate)));
}
//==============================================================================

MetersComponent::MetersComponent (const juce::AudioChannelSet& channelFormat) : MetersComponent (Options(), channelFormat) { }
//==============================================================================

MetersComponent::MetersComponent (Options meterOptions, const juce::AudioChannelSet& channelFormat) : MetersComponent (meterOptions)
{
    m_channelFormat = channelFormat;

    createMeters (channelFormat, {});
}
//==============================================================================

MetersComponent::~MetersComponent()
{

#if SDTK_ENABLE_FADER
    m_labelStrip.removeMouseListener (this);
#endif

    deleteMeters();
}
//==============================================================================

void MetersComponent::reset()
{
    deleteMeters();

#if SDTK_ENABLE_FADER
    resetFaders();
    m_labelStrip.showFader (false);
#endif

    m_labelStrip.showTickMarks (false);
    m_channelFormat = juce::AudioChannelSet::stereo();

    refresh (true);
}
//==============================================================================

void MetersComponent::refresh (const bool forceRefresh /*= false*/)
{
    m_labelStrip.refresh (forceRefresh);
    for (auto* meterChannel: m_meterChannels)
        meterChannel->refresh (forceRefresh);
}
//==============================================================================

void MetersComponent::setRefreshRate (float refreshRate_hz) noexcept
{
    m_options.refreshRate = refreshRate_hz;

    m_labelStrip.setRefreshRate (static_cast<float> (refreshRate_hz));
    for (auto* meter: m_meterChannels)
        meter->setRefreshRate (static_cast<float> (refreshRate_hz));

    if (m_useInternalTimer)
    {
        stopTimer();
        startTimerHz (static_cast<int> (std::round (refreshRate_hz)));
    }
}
//==============================================================================

void MetersComponent::useInternalTiming (bool useInternalTiming) noexcept
{
    m_useInternalTimer = useInternalTiming;

    stopTimer();

    if (useInternalTiming) startTimerHz (static_cast<int> (std::round (m_options.refreshRate)));
}
//==============================================================================

void MetersComponent::paint (juce::Graphics& g)
{
    g.fillAll (m_backgroundColour);

    if (! m_options.enabled)
    {
        g.setColour (m_backgroundColour.contrasting (1.0f));
        g.setFont (m_font.withHeight (14.0f));
        g.drawFittedText ("No audio device open for playback. ", getLocalBounds(), juce::Justification::centred, 5);  // NOLINT
    }
}
//==============================================================================

void MetersComponent::resized()
{
    using namespace Constants;

    // Get the number of meters present...
    const int numOfMeters = static_cast<int> (m_meterChannels.size());

    if (numOfMeters == 0) return;

    // Get the meter bounds...
    auto       panelBounds = getLocalBounds();
    const auto panelHeight = panelBounds.getHeight();
    const auto panelWidth  = panelBounds.getWidth();

    // By default show the MASTER strip.
    auto labelStripWidth = (m_labelStripPosition != LabelStripPosition::none ? kDefaultHeaderLabelWidth : 0);

    // Calculate meter width from available width taking into account the extra width needed when showing the master strip...
    auto meterWidth = juce::jlimit (kMinWidth, kMaxWidth, (panelWidth - labelStripWidth) / numOfMeters);

    bool minModeEnabled = m_meterChannels[0]->autoSetMinimalMode (meterWidth, panelHeight);

    // Don't show the label strip in minimum mode...
    if (minModeEnabled) labelStripWidth = 0;

    // Re-calculate actual width (taking into account the min. mode)...
    if (m_labelStripPosition != LabelStripPosition::none) meterWidth = juce::jlimit (kMinWidth, kMaxWidth, (panelWidth - labelStripWidth) / numOfMeters);

    // Position all meters and adapt them to the current size...
    for (auto meterChannel: m_meterChannels)
    {
        meterChannel->setMinimalMode (minModeEnabled);
        if (m_labelStripPosition == LabelStripPosition::right)
            meterChannel->setBounds (panelBounds.removeFromLeft (meterWidth));  // ... set it's width to m_meterWidth
        else
            meterChannel->setBounds (panelBounds.removeFromRight (meterWidth));  // ... set it's width to m_meterWidth

#if SDTK_ENABLE_FADER
        if (minModeEnabled) meterChannel->showFader (false);  // ... do not show the gain fader if it's too narrow.
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
        auto labelStripBounds = m_meterChannels[0]->getLabelStripBounds();
        if (m_labelStripPosition == LabelStripPosition::right)
            m_labelStrip.setBounds (panelBounds.removeFromRight (labelStripWidth).withY (labelStripBounds.getY()).withHeight (labelStripBounds.getHeight()));
        else if (m_labelStripPosition == LabelStripPosition::left)
            m_labelStrip.setBounds (panelBounds.removeFromLeft (labelStripWidth).withY (labelStripBounds.getY()).withHeight (labelStripBounds.getHeight()));
        m_labelStrip.showTickMarks (true);
    }
}
//==============================================================================

void MetersComponent::setChannelNames (const std::vector<juce::String>& channelNames)
{
    using namespace Constants;

    const auto numChannelNames = static_cast<int> (channelNames.size());

    const auto numMeters = static_cast<int> (m_meterChannels.size());

    auto defaultMeterWidth = static_cast<float> (kMinWidth);

    // Loop through all meters...
    for (int meterIdx = 0; meterIdx < numMeters; ++meterIdx)
    {
        if (meterIdx < numChannelNames)
        {
            if (channelNames[(size_t) meterIdx].isNotEmpty())
            {
                m_meterChannels[meterIdx]->setChannelName (channelNames[(size_t) meterIdx]);  // ... and set the channel name.

                // Calculate the meter width so it fits the largest of channel names...
                defaultMeterWidth = std::max (defaultMeterWidth, m_meterChannels[meterIdx]->getChannelNameWidth());
            }
        }
        else
        {
            // Calculate the meter width so it fits the largest of full type descriptions...
            defaultMeterWidth = std::max (defaultMeterWidth, m_meterChannels[meterIdx]->getChannelTypeWidth());
        }
    }

    if (channelNames.empty())
    {
        for (auto& meter: m_meterChannels)
            meter->setReferredTypeWidth (defaultMeterWidth);
    }

    // Calculate default mixer width...
    // This is the width at which all channel names can be displayed.
    m_autoSizedPanelWidth = static_cast<int> (defaultMeterWidth * static_cast<float> (numMeters));  // Min. width needed for channel names.
    m_autoSizedPanelWidth += numMeters * (2 * kFaderRightPadding);               // Add the padding that is on the right side of the channels.
    m_autoSizedPanelWidth += kDefaultHeaderLabelWidth + kLabelStripLeftPadding;  // Add master fader width (incl. padding).
}
//==============================================================================

#if SDTK_ENABLE_FADER

//==============================================================================

void MetersComponent::mouseExit (const juce::MouseEvent& /*event*/)
{
    showFaders (false);
}
//==============================================================================

void MetersComponent::mouseEnter (const juce::MouseEvent& /*event*/)
{
    if (m_meterChannels.isEmpty()) return;

    if (! m_meterChannels[0]->isMinimalModeActive())
    {
        for (auto meterChannel: m_meterChannels)
            meterChannel->showFader (true);
        m_labelStrip.showFader (true);
    }
}
//==============================================================================

void MetersComponent::faderChanged (MeterChannel* sourceChannel)
{
    // Master strip fader moves all channel faders relatively to each other...
    if (sourceChannel == &m_labelStrip)
    {
        jassert (m_faderGains.size() == m_faderGainsBuffer.size());  // NOLINT

        // If the master fader is ACTIVE ...
        if (m_labelStrip.isActive())
        {
            // ... but all meters are muted ...
            if (areAllMetersInactive()) muteAll (false);  // ... un- mute all meters ...

            // Apply the master fader VALUE to all meter faders ...
            for (auto singleMeter: m_meterChannels)
            {
                auto meterIdx = static_cast<size_t> (m_meterChannels.indexOf (singleMeter));
                if (juce::isPositiveAndBelow (meterIdx, m_faderGains.size()))
                {
                    m_faderGains[meterIdx] = m_faderGainsBuffer[meterIdx] * sourceChannel->getFaderValue();  // Multiply the gain with the master fader value.
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

void MetersComponent::getFaderValues (NotificationOptions notificationOption /*= NotificationOptions::notify*/)
{
    if (m_meterChannels.isEmpty()) return;

    // Set number of mixer gains to match the number of channels...
    jassert (m_faderGains.size() == m_meterChannels.size());  // NOLINT
    // if( static_cast<int>( m_mixerGains.size() ) != m_meters.size() ) m_mixerGains.resize( m_meters.size() );

    // Loop through all meters...
    for (int channelIdx = 0; channelIdx < (int) m_meterChannels.size(); ++channelIdx)
    {
        // If the meter is active, get the value from the fader, otherwise a value of 0.0 is used...
        m_faderGains[(size_t) channelIdx] = (m_meterChannels[channelIdx]->isActive() ? m_meterChannels[channelIdx]->getFaderValue() : 0.0f);
    }

    // If all meters are in-active, so is the master fader ...
    m_labelStrip.setActive (! areAllMetersInactive(), NotificationOptions::dontNotify);

    m_faderGainsBuffer = m_faderGains;

    if (notificationOption == NotificationOptions::notify) notifyListeners();
}
//==============================================================================

void MetersComponent::notifyListeners()
{
    Component::BailOutChecker checker (this);

    if (checker.shouldBailOut()) return;

    m_fadersListeners.callChecked (checker, [=] (FadersChangeListener& l) { l.fadersChanged (m_faderGains); });

    if (checker.shouldBailOut()) return;
}
//==============================================================================

void MetersComponent::showFaders (bool mustShowFaders)
{
    m_labelStrip.showFader (mustShowFaders);
    for (auto meterChannel: m_meterChannels)
        meterChannel->showFader (mustShowFaders);
}
//==============================================================================

bool MetersComponent::areAllMetersInactive()
{
    for (auto meterChannel: m_meterChannels)
        if (meterChannel->isActive()) return false;
    return true;
}
//==============================================================================

void MetersComponent::toggleMute()
{
    bool allChannelsInactive = areAllMetersInactive();
    muteAll (! allChannelsInactive);
}
//==============================================================================

void MetersComponent::muteAll (bool mute /*= true */)
{
    bool allChannelsInactive = areAllMetersInactive();
    if (mute == allChannelsInactive) return;  // All meters already muted.

    for (auto meterChannel: m_meterChannels)
    {
        meterChannel->setActive (! mute);
        meterChannel->flashFader();
    }
    getFaderValues();
}
//==============================================================================

void MetersComponent::resetFaders()
{
    if (std::any_of (m_faderGains.begin(), m_faderGains.end(), [] (auto gain) { return gain != 1.0F; }) )
    {
        std::fill (m_faderGains.begin(), m_faderGains.end(), 1.0f);  // Set all fader gains to unity.
        notifyListeners();
    }
    m_faderGainsBuffer = m_faderGains;                           // Copy the just reset fader gains to it's buffer.

    // Activate (un-mute) all faders and set them to unity gain...
    for (const auto& meterChannel: m_meterChannels)
    {
        meterChannel->setActive (true);
        meterChannel->setFaderValue (1.0f);
        meterChannel->flashFader();
    }
    m_labelStrip.setActive (true);
    m_labelStrip.setFaderValue (1.0f);
    m_labelStrip.flashFader();
}
//==============================================================================

void MetersComponent::setFadersEnabled (bool faderEnabled) noexcept
{
    for (auto* meterChannel: m_meterChannels)
        meterChannel->enableFader (faderEnabled);
    m_labelStrip.enableFader (faderEnabled);
    m_options.faderEnabled = faderEnabled;
}


#endif /* SDTK_ENABLE_FADER */

//==============================================================================
void MetersComponent::setNumChannels (int numChannels, const std::vector<juce::String>& channelNames /*= {}*/)
{
    if (numChannels <= 0) return;

    setChannelFormat (juce::AudioChannelSet::canonicalChannelSet (numChannels), channelNames);
}

//==============================================================================
void MetersComponent::setChannelFormat (const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames)
{
    if (channelFormat.size() == 0) return;

    m_channelFormat = channelFormat;

    // Make sure the number of meters matches the number of channels ...
    if (channelFormat.size() != m_meterChannels.size())
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
        m_faderGains.resize (static_cast<size_t> (channelFormat.size()));  // ... and if not resize the mixer gains to accommodate.

    resetFaders();

#endif /* SDTK_ENABLE_FADER */
}

//==============================================================================

void MetersComponent::setInputLevel (int channel, float value)
{
    if (auto* meterChannel = getMeterChannel (channel)) meterChannel->setInputLevel (value);
}
//==============================================================================

void MetersComponent::createMeters (const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames)
{
    // Create enough meters to match the channel format...
    for (int channelIdx = 0; channelIdx < channelFormat.size(); ++channelIdx)
    {
        auto meterChannel = std::make_unique<MeterChannel> (m_options, Padding (0, Constants::kFaderRightPadding, 0, 0), Constants::kMetersPanelId, false,
                                                            channelFormat.getTypeOfChannel (channelIdx));

#if SDTK_ENABLE_FADER
        meterChannel->onFaderMove = [this] (MeterChannel* channel) { faderChanged (channel); };
#endif

        meterChannel->setFont (&m_font);
        meterChannel->addMouseListener (this, true);

        addChildComponent (meterChannel.get());
        m_meterChannels.add (meterChannel.release());

        m_labelStrip.setActive (true);
    }

    setChannelNames (channelNames);
}
//==============================================================================

void MetersComponent::deleteMeters()
{
#if SDTK_ENABLE_FADER
    for (auto meterChannel: m_meterChannels)
        meterChannel->removeMouseListener (this);
#endif

    m_meterChannels.clear();
}
//==============================================================================

MeterChannel* MetersComponent::getMeterChannel (const int meterIndex) noexcept
{
    return (juce::isPositiveAndBelow (meterIndex, m_meterChannels.size()) ? m_meterChannels[meterIndex] : nullptr);
}
//==============================================================================

void MetersComponent::resetMeters()
{
    for (auto* meterChannel: m_meterChannels)
        meterChannel->reset();
}
//==============================================================================

void MetersComponent::resetPeakHold()
{
    for (auto* meterChannel: m_meterChannels)
        meterChannel->resetPeakHold();
}
//==============================================================================

void MetersComponent::setDecay (float decay_ms)
{
    m_options.decayTime_ms = decay_ms;
    for (auto* meterChannel: m_meterChannels)
        meterChannel->setDecay (decay_ms);
}
//==============================================================================

void MetersComponent::setFont (const juce::Font& newFont) noexcept
{
    m_font = newFont;
    for (auto* meterChannel: m_meterChannels)
        meterChannel->setFont (&m_font);
    m_labelStrip.setFont (&m_font);
}
//==============================================================================

void MetersComponent::setOptions (Options meterOptions)
{
    m_options = meterOptions;
    for (auto* meterChannel: m_meterChannels)
        meterChannel->setOptions (meterOptions);
    m_labelStrip.setOptions (meterOptions);
}
//==============================================================================

void MetersComponent::setEnabled (bool enabled /*= true*/)
{
    m_options.enabled = enabled;

    for (auto meterChannel: m_meterChannels)
    {
        meterChannel->setEnabled (enabled);
        meterChannel->setVisible (enabled);
    }

    m_labelStrip.setEnabled (enabled);
    m_labelStrip.setVisible (enabled);

    refresh (true);
}
//==============================================================================

void MetersComponent::showTickMarks (bool showTickMarks)
{
    m_options.tickMarksEnabled = showTickMarks;
    for (auto* meterChannel: m_meterChannels)
        meterChannel->showTickMarks (showTickMarks);
    m_labelStrip.showTickMarks (showTickMarks);
}
//==============================================================================

void MetersComponent::useGradients (bool useGradients) noexcept
{
    m_options.useGradient = useGradients;
    for (auto* meterChannel: m_meterChannels)
        meterChannel->useGradients (useGradients);
}
//==============================================================================

void MetersComponent::setLabelStripPosition (LabelStripPosition labelStripPosition)
{
    m_labelStripPosition = labelStripPosition;
    resized();
}
//==============================================================================

void MetersComponent::enableHeader (bool headerEnabled)
{
    if (m_options.headerEnabled == headerEnabled) return;

    m_options.headerEnabled = headerEnabled;
    m_labelStrip.enableHeader (headerEnabled);

    for (auto* meterChannel: m_meterChannels)
        meterChannel->enableHeader (headerEnabled);

    resized();
}
//==============================================================================

void MetersComponent::enableValue (bool valueEnabled)
{
    if (m_options.valueEnabled == valueEnabled) return;

    m_labelStrip.enableValue (valueEnabled);

    for (auto* meterChannel: m_meterChannels)
        meterChannel->enableValue (valueEnabled);

    resized();
}
//==============================================================================

void MetersComponent::defineSegments (float warningSegment_db, float peakSegment_db)
{
    m_options.warningSegment_db = warningSegment_db;
    m_options.peakSegment_db    = peakSegment_db;

    for (auto* meterChannel: m_meterChannels)
        meterChannel->defineSegments (warningSegment_db, peakSegment_db);
}
//==============================================================================

void MetersComponent::setColours()
{
    if (isColourSpecified (MeterChannel::backgroundColourId))
        m_backgroundColour = findColour (MeterChannel::backgroundColourId);
    else if (getLookAndFeel().isColourSpecified (MeterChannel::backgroundColourId))
        m_backgroundColour = getLookAndFeel().findColour (MeterChannel::backgroundColourId);
}

}  // namespace SoundMeter
}  // namespace sd
