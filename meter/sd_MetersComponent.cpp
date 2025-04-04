/*
    ==============================================================================

    This file is part of the sound_meter JUCE module
    Copyright (c) 2019 - 2025 Sound Development - Marcel Huibers
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

#include "sd_MetersComponent.h"

namespace sd  // NOLINT
{
namespace SoundMeter
{
MetersComponent::MetersComponent() : MetersComponent ({}, {}) { }
//==============================================================================

MetersComponent::MetersComponent (const juce::AudioChannelSet& channelFormat) : MetersComponent ({}, channelFormat) { }
//==============================================================================

MetersComponent::MetersComponent (const Options& meterOptions) : MetersComponent (meterOptions, {}) { }
//==============================================================================

MetersComponent::MetersComponent (const Options& meterOptions, const juce::AudioChannelSet& channelFormat)
  : m_meterOptions (meterOptions),
    m_labelStrip (meterOptions, Padding (Constants::kLabelStripLeftPadding, 0, 0, 0), Constants::kLabelStripId, true, juce::AudioChannelSet::ChannelType::unknown),
    m_channelFormat (channelFormat),
    m_font (juce::FontOptions())
{
#if SDTK_ENABLE_FADER
    m_labelStrip.enableFader (m_meterOptions.faderEnabled);
    m_labelStrip.addMouseListener (this, true);
    m_labelStrip.onFaderMove  = [this] (MeterChannel::Ptr meterChannel) { faderChanged (meterChannel); };
    m_labelStrip.onMixerReset = [this]() { resetFaders(); };
#endif

    setName (Constants::kMetersId);

    addAndMakeVisible (m_labelStrip);

    startTimerHz (static_cast<int> (std::round (m_meterOptions.refreshRate)));

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

void MetersComponent::clearMeters()
{
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->setInputLevel (0.0f);

    refresh (true);
}
//==============================================================================

void MetersComponent::refresh (const bool forceRefresh /*= false*/)
{
    if (!isShowing() || m_meterChannels.isEmpty())
        return;

    m_labelStrip.refresh (forceRefresh);
    for (auto* meter: m_meterChannels)
    {
        if (meter)
            meter->refresh (forceRefresh);
    }
}
//==============================================================================

void MetersComponent::setRefreshRate (float refreshRate_hz)
{
    m_meterOptions.refreshRate = refreshRate_hz;

    m_labelStrip.setRefreshRate (static_cast<float> (refreshRate_hz));
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->setRefreshRate (static_cast<float> (refreshRate_hz));

    if (m_useInternalTimer)
    {
        stopTimer();
        startTimerHz (juce::roundToInt (refreshRate_hz));
    }
}
//==============================================================================

void MetersComponent::useInternalTiming (bool useInternalTiming) noexcept
{
    m_useInternalTimer = useInternalTiming;

    stopTimer();

    if (useInternalTiming)
        startTimerHz (static_cast<int> (std::round (m_meterOptions.refreshRate)));
}
//==============================================================================

void MetersComponent::paint (juce::Graphics& g)
{
    g.fillAll (m_backgroundColour);
}
//==============================================================================

void MetersComponent::resized()
{
    const auto numOfMeters = static_cast<float> (m_meterChannels.size());
    if (numOfMeters <= 0.0f)
        return;

    auto       panelBounds     = getLocalBounds().toFloat();
    const auto panelHeight     = panelBounds.getHeight();
    const auto panelWidth      = panelBounds.getWidth();
    auto       labelStripWidth = static_cast<float> (m_labelStripPosition != LabelStripPosition::none ? Constants::kDefaultHeaderLabelWidth : 0);

    // Calculate meter width from available width taking into account the extra width needed when showing the master strip...
    auto       meterWidth     = juce::jlimit (1.0f, Constants::kMaxWidth, (panelWidth - labelStripWidth) / numOfMeters);
    const bool minModeEnabled = m_meterChannels[0]->autoSetMinimalMode (static_cast<int> (meterWidth), static_cast<int> (panelHeight));

    // Don't show the label strip in minimum mode...
    if (minModeEnabled)
        labelStripWidth = 0.0f;

    // Re-calculate actual width (taking into account the min. mode)...
    if (m_labelStripPosition != LabelStripPosition::none)
        meterWidth = juce::jlimit (1.0f, Constants::kMaxWidth, (panelWidth - labelStripWidth) / numOfMeters);

    // Position all meters and adapt them to the current size...
    for (auto* meter: m_meterChannels)
    {
        if (meter)
        {
            meter->setMinimalMode (minModeEnabled);
            if (m_labelStripPosition == LabelStripPosition::right)
                meter->setBounds (panelBounds.removeFromLeft (meterWidth).toNearestIntEdges());
            else
                meter->setBounds (panelBounds.removeFromRight (meterWidth).toNearestIntEdges());

#if SDTK_ENABLE_FADER
            if (minModeEnabled)
                meter->showFader (false);  // ... do not show the gain fader if it's too narrow.
#endif
        }
    }

    // Position MASTER strip...
    if (labelStripWidth == 0.0f)
    {
        m_labelStrip.setBounds ({});
    }
    else
    {
        // Use the dimensions of the 'meter' part combined with the 'value' part...
        const auto labelStripBounds = m_meterChannels[0]->getLabelStripBounds().toFloat();
        if (m_labelStripPosition == LabelStripPosition::right)
            m_labelStrip.setBounds (
              panelBounds.removeFromRight (labelStripWidth).withY (labelStripBounds.getY()).withHeight (labelStripBounds.getHeight()).toNearestIntEdges());
        else if (m_labelStripPosition == LabelStripPosition::left)
            m_labelStrip.setBounds (panelBounds.removeFromLeft (labelStripWidth).withY (labelStripBounds.getY()).withHeight (labelStripBounds.getHeight()).toNearestIntEdges());
        m_labelStrip.showTickMarks (true);
    }
}
//==============================================================================

void MetersComponent::setChannelNames (const std::vector<juce::String>& channelNames)
{
    if (m_meterChannels.isEmpty())
        return;

    const auto numChannelNames   = static_cast<int> (channelNames.size());
    const auto numMeters         = static_cast<int> (m_meterChannels.size());
    auto       defaultMeterWidth = static_cast<float> (Constants::kMinModeWidthThreshold);

    // Loop through all meters...
    for (int meterIdx = 0; meterIdx < numMeters; ++meterIdx)
    {
        if (meterIdx < numChannelNames)
        {
            if (channelNames[static_cast<size_t> (meterIdx)].isNotEmpty())
            {
                m_meterChannels[meterIdx]->setChannelName (channelNames[static_cast<size_t> (meterIdx)]);  // ... and set the channel name.

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
        for (auto* meter: m_meterChannels)
            if (meter)
                meter->setReferredTypeWidth (defaultMeterWidth);
    }

    // Calculate default mixer width...
    // This is the width at which all channel names can be displayed.
    m_autoSizedPanelWidth = static_cast<int> (defaultMeterWidth * static_cast<float> (numMeters));  // Min. width needed for channel names.
    m_autoSizedPanelWidth += numMeters * (2 * Constants::kFaderRightPadding);  // Add the padding that is on the right side of the channels.
    m_autoSizedPanelWidth += Constants::kDefaultHeaderLabelWidth + Constants::kLabelStripLeftPadding;  // Add master fader width (incl. padding).
}
//==============================================================================

void MetersComponent::mouseDoubleClick (const juce::MouseEvent& /*event*/) 
{
    if (m_meterOptions.faderEnabled)
        return;

    resetPeakHold();
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
    if (m_meterChannels.isEmpty())
        return;

    if (!m_meterChannels[0]->isMinimalModeActive())
    {
        for (auto* meter: m_meterChannels)
            if (meter)
                meter->showFader (true);
        m_labelStrip.showFader (true);
    }
}
//==============================================================================

void MetersComponent::faderChanged (MeterChannel::Ptr sourceChannel)
{
    jassert (m_faderGains.size() == m_faderGainsBuffer.size());
    if (m_faderGains.size() != m_faderGainsBuffer.size())
        return;

    // Master strip fader moves all channel faders relatively to each other...
    if (sourceChannel == &m_labelStrip)
    {
        // If the master fader is ACTIVE ...
        if (m_labelStrip.isActive())
        {
            // ... but all meters are muted ...
            if (areAllMetersInactive())
                muteAll (false);  // ... un- mute all meters ...

            // Apply the master fader VALUE to all meter faders ...
            for (auto* meter: m_meterChannels)
            {
                if (meter)
                {
                    auto meterIdx = static_cast<size_t> (m_meterChannels.indexOf (meter));
                    if (juce::isPositiveAndBelow (meterIdx, m_faderGains.size()))
                    {
                        m_faderGains[meterIdx] = m_faderGainsBuffer[meterIdx] * sourceChannel->getFaderValue();  // Multiply the gain with the master fader value.
                        meter->setFaderValue (m_faderGains[meterIdx], NotificationOptions::dontNotify, false);  // Update the fader to display the new gain value.
                    }
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
        assembleFaderGains (NotificationOptions::dontNotify);
    }

    notifyListeners();
}
//==============================================================================

void MetersComponent::channelSolo (MeterChannel::Ptr sourceChannel)
{
    bool alreadySoloed { true };
    for (auto* meter: m_meterChannels)
    {
        if (meter)
        {
            if (meter != sourceChannel && meter->isActive())
            {
                meter->setActive (false, NotificationOptions::dontNotify);
                alreadySoloed = false;
            }
        }
    }
    if (alreadySoloed)
    {
        for (auto* meter: m_meterChannels)
            if (meter)
                meter->setActive (true, NotificationOptions::dontNotify);
    }

    assembleFaderGains (NotificationOptions::notify);
}
//==============================================================================

void MetersComponent::setFaderValues (const std::vector<float>& faderValues, NotificationOptions notificationOption /*= NotificationOptions::dontNotify*/)
{
    for (size_t meterIdx = 0; meterIdx < static_cast<size_t> (m_meterChannels.size()); ++meterIdx)
    {
        if (meterIdx < faderValues.size())
            m_meterChannels[static_cast<int> (meterIdx)]->setFaderValue (faderValues[meterIdx], notificationOption);
    }

    m_faderGains = faderValues;
}
//==============================================================================

void MetersComponent::assembleFaderGains (NotificationOptions notificationOption /*= NotificationOptions::notify*/)
{
    if (m_meterChannels.isEmpty())
        return;

    // Set number of mixer gains to match the number of channels...
    jassert (static_cast<int> (m_faderGains.size()) == m_meterChannels.size());  // NOLINT

    for (int channelIdx = 0; channelIdx < static_cast<int> (m_meterChannels.size()); ++channelIdx)
    {
        // If the meter is active, get the value from the fader, otherwise a value of 0.0 is used...
        m_faderGains[(size_t) channelIdx] = (m_meterChannels[channelIdx]->isActive() ? m_meterChannels[channelIdx]->getFaderValue() : 0.0f);
    }

    // If all meters are in-active, so is the master fader ...
    m_labelStrip.setActive (!areAllMetersInactive(), NotificationOptions::dontNotify);

    m_faderGainsBuffer = m_faderGains;

    if (notificationOption == NotificationOptions::notify)
        notifyListeners();
}
//==============================================================================

juce::String MetersComponent::serializeFaderGains()
{
    assembleFaderGains (NotificationOptions::dontNotify);

    juce::StringArray faderGains {};
    for (const auto& gain: m_faderGains)
        faderGains.add (juce::String (gain));

    return faderGains.joinIntoString ("|");
}
//==============================================================================

void MetersComponent::notifyListeners()
{
    Component::BailOutChecker checker (this);

    if (checker.shouldBailOut())
        return;

    m_fadersListeners.callChecked (checker, [&] (FadersChangeListener& l) { l.fadersChanged (m_faderGains); });

    if (checker.shouldBailOut())
        return;
}
//==============================================================================

void MetersComponent::showFaders (bool mustShowFaders)
{
    m_labelStrip.showFader (mustShowFaders);
    for (auto* meter: m_meterChannels)
        meter->showFader (mustShowFaders);
}
//==============================================================================

bool MetersComponent::areAllMetersInactive()
{
    for (const auto* meter: m_meterChannels)
        if (meter->isActive())
            return false;
    return true;
}
//==============================================================================

void MetersComponent::toggleMute()
{
    const bool allChannelsInactive = areAllMetersInactive();
    muteAll (!allChannelsInactive);
}
//==============================================================================

void MetersComponent::muteAll (bool mute /*= true */)
{
    const bool allChannelsInactive = areAllMetersInactive();
    if (mute == allChannelsInactive)
        return;  // All meters already muted.

    for (auto* meter: m_meterChannels)
    {
        meter->setActive (!mute);
        meter->flashFader();
    }
    assembleFaderGains();
}
//==============================================================================

void MetersComponent::resetFaders()
{
    if (std::any_of (m_faderGains.begin(), m_faderGains.end(), [] (auto gain) { return gain != 1.0f; }))
    {
        std::fill (m_faderGains.begin(), m_faderGains.end(), 1.0f);  // Set all fader gains to unity.
        notifyListeners();
    }
    m_faderGainsBuffer = m_faderGains;  // Copy the just reset fader gains to it's buffer.

    // Activate (un-mute) all faders and set them to unity gain...
    for (auto* meter: m_meterChannels)
    {
        meter->setActive (true);
        meter->setFaderValue (1.0f);
        meter->flashFader();
    }
    m_labelStrip.setActive (true);
    m_labelStrip.setFaderValue (1.0f);
    m_labelStrip.flashFader();
}
//==============================================================================

void MetersComponent::setFadersEnabled (bool faderEnabled)
{
    for (auto* meter: m_meterChannels)
        meter->enableFader (faderEnabled);
    m_labelStrip.enableFader (faderEnabled);
    m_meterOptions.faderEnabled = faderEnabled;
}
//==============================================================================

void MetersComponent::flashFaders()
{
    for (auto* meter: m_meterChannels)
        meter->flashFader();

    m_labelStrip.flashFader();
}

#endif /* SDTK_ENABLE_FADER */

//==============================================================================

void MetersComponent::setNumChannels (int numChannels, const std::vector<juce::String>& channelNames /*= {}*/)
{
    if (numChannels <= 0)
        return;

    setChannelFormat (juce::AudioChannelSet::canonicalChannelSet (numChannels), channelNames);
}
//==============================================================================

void MetersComponent::setChannelFormat (const juce::AudioChannelSet& channels, const std::vector<juce::String>& channelNames)
{
    if (channels.size() <= 0)
        return;

    m_channelFormat = channels;

    // Make sure the number of meters matches the number of channels ...
    if (channels.size() != m_meterChannels.size())
    {
        deleteMeters();                         // ... if not, then delete all previous meters ...
        createMeters (channels, channelNames);  // ... and create new ones, matching the required channel format.
    }

    // Set the channel names...
    setChannelNames (channelNames);

    // Resize the mixer to accommodate any optionally added meters...
    resized();

#if SDTK_ENABLE_FADER

    // Make sure the number of mixer gains matches the number of channels ...
    const auto numFaderGains = static_cast<int> (m_faderGains.size());

    if (channels.size() != numFaderGains)
    {
        if (numFaderGains > channels.size() || m_faderGains.empty())
        {
            m_faderGains.resize (static_cast<size_t> (channels.size()), 1.0f);  // ... and if not resize the mixer gains to accommodate.
            m_faderGainsBuffer.resize (static_cast<size_t> (channels.size()), 1.0f);
        }
        else
        {
            const auto numChannelsToAdd = static_cast<size_t> (channels.size() - numFaderGains);
            const auto lastGain         = m_faderGains.back();
            const auto lastBufferedGain = m_faderGainsBuffer.back();

            m_faderGains.insert (m_faderGains.end(), numChannelsToAdd, lastGain);
            m_faderGainsBuffer.insert (m_faderGainsBuffer.end(), numChannelsToAdd, lastBufferedGain);
        }
    }

    setFaderValues (m_faderGains);

#endif /* SDTK_ENABLE_FADER */
}

//==============================================================================

void MetersComponent::setInputLevel (int channel, float value)
{
    if (auto* meterChannel = getMeterChannel (channel))
        if (meterChannel)
            meterChannel->setInputLevel (value);
}
//==============================================================================

void MetersComponent::createMeters (const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames)
{
    // Create enough meters to match the channel format...
    for (int channelIdx = 0; channelIdx < channelFormat.size(); ++channelIdx)
    {
        auto meterChannel = std::make_unique<MeterChannel> (m_meterOptions, Padding (0, Constants::kFaderRightPadding, 0, 0), Constants::kMetersId, false,
                                                            channelFormat.getTypeOfChannel (channelIdx));

#if SDTK_ENABLE_FADER
        meterChannel->onFaderMove   = [this] (MeterChannel* channel) { faderChanged (channel); };
        meterChannel->onChannelSolo = [this] (MeterChannel* channel) { channelSolo (channel); };
        meterChannel->onMixerReset  = [this]() { resetFaders(); };
#endif

        meterChannel->setFont (m_font);
        meterChannel->addMouseListener (this, true);

        addChildComponent (meterChannel.get());
        m_meterChannels.add (meterChannel.release());

        m_labelStrip.setActive (true);
    }

    setChannelNames (channelNames);
    setMeterSegments (m_segmentsOptions);
}
//==============================================================================

void MetersComponent::deleteMeters()
{
#if SDTK_ENABLE_FADER
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->removeMouseListener (this);
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
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->reset();
}
//==============================================================================

void MetersComponent::resetPeakHold()
{
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->resetPeakHold();
}
//==============================================================================

void MetersComponent::setDecay (float decay_ms)
{
    m_meterOptions.decayTime_ms = decay_ms;
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->setDecay (decay_ms);
}
//==============================================================================

void MetersComponent::setFont (const juce::Font& newFont)
{
    m_font = newFont;
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->setFont (m_font);
    m_labelStrip.setFont (m_font);
}
//==============================================================================

void MetersComponent::setOptions (const Options& meterOptions)
{
    m_meterOptions = meterOptions;
    for (auto* meter: m_meterChannels)
    {
        if (meter)
            meter->setOptions (meterOptions);
    }
    m_labelStrip.setOptions (meterOptions);

    setRefreshRate (meterOptions.refreshRate);
}
//==============================================================================

void MetersComponent::enable (bool enabled /*= true*/)
{
    m_meterOptions.enabled = enabled;

    for (auto* meter: m_meterChannels)
    {
        if (meter)
        {
            meter->setEnabled (enabled);
            meter->setVisible (enabled);
        }
    }

    m_labelStrip.setEnabled (enabled);
    m_labelStrip.setVisible (enabled);

    refresh (true);
}
//==============================================================================

void MetersComponent::showTickMarks (bool showTickMarks)
{
    m_meterOptions.tickMarksEnabled = showTickMarks;
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->showTickMarks (showTickMarks);

    m_labelStrip.showTickMarks (showTickMarks);
}
//==============================================================================

bool MetersComponent::isMinimalModeActive() const noexcept
{
    if (m_meterChannels.isEmpty())
        return true;

    return m_meterChannels[0]->isMinimalModeActive();
}
//==============================================================================

void MetersComponent::useGradients (bool useGradients)
{
    for (auto* meter: m_meterChannels)
        if (meter)
            meter->useGradients (useGradients);
}
//==============================================================================

void MetersComponent::setLabelStripPosition (LabelStripPosition labelStripPosition)
{
    m_labelStripPosition = labelStripPosition;
    resized();
}
//==============================================================================

void MetersComponent::showHeader (bool showHeader)
{
    if (m_meterOptions.headerEnabled == showHeader)
        return;

    m_meterOptions.headerEnabled = showHeader;
    m_labelStrip.showHeader (showHeader);

    for (auto* meter: m_meterChannels)
        if (meter)
            meter->showHeader (showHeader);

    resized();
}
//==============================================================================

void MetersComponent::showValue (bool showValue)
{
    if (m_meterOptions.valueEnabled == showValue)
        return;

    m_labelStrip.showValue (showValue);

    for (auto* meter: m_meterChannels)
        if (meter)
            meter->showValue (showValue);

    resized();
}
//==============================================================================

void MetersComponent::setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions)
{
    m_segmentsOptions = segmentsOptions;

    m_labelStrip.setMeterSegments (segmentsOptions);

    for (auto* meter: m_meterChannels)
        if (meter)
            meter->setMeterSegments (m_segmentsOptions);
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
