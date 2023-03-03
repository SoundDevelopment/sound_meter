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

#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>


/**
 * @brief Namespace containing all concerning the sound_meter module.
*/
namespace sd
{

namespace SoundMeter
{

/**
 * @brief Component containing one or more meters.
 * 
 * After setting the channel format it 
 * will automatically create the needed meters and give them proper names.
 */
class MetersComponent
  : public juce::Component
  , private juce::Timer
{
 public:
    /**
     * @brief Constructor
    */
    MetersComponent();

    /**
     * @brief Constructor with meter options.
     * 
     * @param meterOptions The options to use with the meters and the label strip.
    */
    MetersComponent (MeterOptions meterOptions);

    /**
     * @brief Constructor which accepts a channel format.
     * 
     * This constructor will automatically setup the panel with the 
     * right amount of meters, named according to the channel format.
     * 
     * @param channelFormat The channel format to use to initialise the panel.
    */
    explicit MetersComponent (const juce::AudioChannelSet& channelFormat);

    /**
     * @brief Constructor with meter options and which accepts a channel format.
     * 
     * This constructor will automatically setup the panel with the 
     * right amount of meters, named according to the channel format.
     * 
     * @param channelFormat The channel format to use to initialise the panel.
    */
    MetersComponent (MeterOptions meterOptions, const juce::AudioChannelSet& channelFormat);

    /** @brief Destructor.*/
    ~MetersComponent() override;

    /**
     * @brief Refresh (redraw) the meters panel.
     * 
     * This can be called manually or internally (see useInternalTiming).
     * 
     * @param forceRefresh When set to true, always redraw the meters panel (not only if it's dirty/changed).
     * 
     * @see setRefreshRate, useInternalTiming
    */
    void refresh (bool forceRefresh = false);

    /**
     * @brief Reset the meters.
     * 
     * Initialise the meters, faders and clears all the levels (but not preserves the peak hold).
     * 
     * @see resetPeakHold, resetMeters
    */
    void reset ();

    /**
     * @brief Reset all meters.
     * 
     * Resets all meters to 0 (but not the peak hold).
     * @see reset, resetPeakHold
    */
    void resetMeters();
 
    /**
     * @brief Clear the level of the meters.
    */
    void clearMeters();

    /**
     * @brief Reset all peak hold indicators and 'values'.
     * 
     * @see reset, resetMeters
    */
    void resetPeakHold();

    /**
     * @brief Set the input level.
     * 
     * This supplies a meter of a specific channel with the peak level from the audio engine.
     * Beware: this will usually be called from the audio thread.
     * 
     * @param channel The channel to set the input level of.
     * @param value   The input level to set to the specified channel.
    */
    void setInputLevel (int channel, float value);

    /**
     * @brief Set the number of channels (meters) in the panel.
     * 
     * @param numChannels  The number of channels (meters).
     * @param channelNames The (optional) channel names to use in the header of the meters.
     * 
     * @see getNumChannels, setChannelFormat
    */
    void setNumChannels (int numChannels, const std::vector<juce::String>& channelNames = {});

    /**
     * @brief Set the channel format (number of channels) to use for the mixer/meters.
     * 
     * @param channels     The channel format to use.
     * @param channelNames The (optional) channel names to use in the header of the meters.
     * 
     * @see setNumChannels, getNumChannels
    */
    void setChannelFormat (const juce::AudioChannelSet& channels, const std::vector<juce::String>& channelNames = {});

    /**
    * @brief Get the number of meters in the panel.
    * 
    * @return The number of meters in the panel.
    * 
    * @see setNumChannels, setChannelFormat
   */
    int getNumChannels() const noexcept { return m_meterChannels.size(); }

    /**
     * @brief Get the default meters panel width.
     * 
     * Get's the default panel width in pixels. 
     * This is the width where all channels are wide enough to display
     * all channel names.
     * 
     * @return The default panel width (in pixels).
    */
    int getAutoSizedPanelWidth() const noexcept { return m_autoSizedPanelWidth; }

    /**
     * @brief Set the channel names to display above the meters.
     * 
     * @param channelNames The list of names to use for the meters.
    */
    void setChannelNames (const std::vector<juce::String>& channelNames);

    /**
     * @brief Set meter options defining appearance and functionality.
     * 
     * @param meterOptions The options to apply to the meters and label strip.
    */
    void setOptions (MeterOptions meterOptions);

    /**
     * @brief Set the refresh (redraw) rate of the meters.
     * 
     * Also used for meter ballistics.
     * When using the internal timer (setInternalTiming) this set's it's refresh rate.
     * When manually redrawing (with refresh) you could (should) still provide the refresh rate
     * to optimize a smooth decay.
     * 
     * @param refreshRate The refresh rate (in Hz).
     * 
     * @see setDecay, refresh, setInternalTiming
    */
    void setRefreshRate (float refreshRate);

    /**
     * @brief Set the timing option to use (internal/external).
     * 
     * When using internal timing, the panel will redraw (refresh) the meters automatically
     * using the refresh rate specified in setPanelRefreshRate.
     * When using external timing, the user has to do this manually with the 'refresh' method.
     * 
     * @param useInternalTiming When set to true, the meter panel will update itself.
     * 
     * @see refresh, setPanelRefreshRate
    */
    void useInternalTiming (bool useInternalTiming);

    /**
     * @brief Set the segments the meter is made out of.
     *
     * All segments have a level range, a range within the meter and a colour (or gradient).
     *
     * @param segmentsOptions The segments options to create the segments with.
    */
    void setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions);

    /**
     * @brief Set meter decay rate.
     * 
     * Set's the meter's decay rate in milliseconds.
     * The meter's attack is instant.
     * 
     * @param decay The time it takes the meter to decay (in ms).
     * 
     * @see setRefreshRate
    */
    void setDecay (float decay_ms);

    /**
     * @brief Use gradients instead of hard segment boundaries.
     * 
     * @param useGradients When set to true, uses smooth gradients. False gives hard segment boundaries.
    */
    void useGradients (bool useGradients);

    /**
     * @brief Set the position of the label-strip relative to the meters.
     * 
     * @param labelStripPosition The position of the label-strip relative to the meters.
    */
    void setLabelStripPosition (LabelStripPosition labelStripPosition);

    /**
     * @brief Enable the 'header' part above the meters.
     * 
     * This will display the channel name (a custom one that the user can set)
     * or the channel type (left, right, center, etc...).
     * This also doubles as a mute button for the specific channel.
     * 
     * @param headerEnabled When set to true, the 'header' part will be shown.
     * @see showValue, setChannelNames
    */
    void enableHeader (bool headerEnabled);

    /**
     * @brief Show the 'value' part below the meters.
     * 
     * This will display the peak value, in decibels, below the meter.
     * The level displayed here matches the peak level indicator on the meter.
     * Double clicking will reset the peak hold value (as well as the indicator).
     * 
     * @param showValue When set to true, the 'value' part will be shown.
     * @see enableHeader, resetPeakHold
    */
    void showValue (bool showValue);

    /**
     * @brief Set the font to be used in the panel and it's meters.
     * @param font The font to use.
    */
    void setFont (const juce::Font& font);

    /**
     * @brief Enable or disable the panel.
     * 
     * @param enabled When set to true, the meters panel will be displayed.
    */
    void enable (bool enabled = true);

    /**
     * @brief Show tick-marks (divider lines) on the meter.
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     *
     * @param showTickMarks When set true, shows the tick-marks. 
    */
    void showTickMarks (bool showTickMarks);

#if SDTK_ENABLE_FADER

    /**
     * @brief Show (or hide) all the faders.
     * 
     * @param showMixer When set to true, will show the faders. Otherwise it will hide them.
    */
    void showFaders (bool showMixer);

    /**
     * @brief Show the faders briefly and fade out (unless overridden and shown longer).
    */
    void flashFaders();

    /**
     * @brief Enable the faders on the meters.
     *
     * @param faderEnabled When set to true, the faders are enabled.
    */
    void setFadersEnabled (bool faderEnabled);

    /**
     * @brief Get values from all faders.
     *
     * @param notificationOption Set whether to notify the listeners of the gathered fader values.
     * @see notifyListeners()
    */
    void getFaderValues (NotificationOptions notificationOption = NotificationOptions::notify);

    /**
     * @brief Set the values of all channel faders.
     * 
     * @param faderValues A list of all values.
     * @param notificationOption Set whether to notify the listeners of the gathered fader values.
    */
    void setFaderValues (const std::vector<float>& faderValues, NotificationOptions notificationOption = NotificationOptions::dontNotify);

    /**
     * @brief Get the master fader.
     * 
     * Get a reference to the master fader component.
     * 
     * @return A reference to the master fader component.
    */
    const MeterChannel& getMasterFader() const noexcept { return m_labelStrip; }

    /**
     * @brief Set all faders to unity gain.
    */
    void resetFaders();

    /**
     * @brief Toggle mute channels.
     * 
     * Mute all channels, or if all were muted, un-mute all.
     * 
     * @see muteAll
    */
    void toggleMute();

    /**
     * @brief Mute (or un-mute) all meters.
     * 
     * @param mute When set to true, all meters will be muted.
     * @see toggleMute
    */
    void muteAll (bool mute = true);

    /**
     * Check if all channels have been set inactive..
     * 
     * @return True, if all channels are inactive (muted).
    */
    [[nodiscard]] bool areAllMetersInactive();

    /**
	 * @brief A listener to any fader changes in the meters panel.
	*/
    struct FadersChangeListener
    {
        virtual ~FadersChangeListener() = default;

        /**
         * @brief This virtual function gets called when a fader has changed.
         * @param faderValues All fader values.
        */
        virtual void fadersChanged (std::vector<float> faderValues) = 0;
    };

    /**
     * @brief Add a listener to any fader movement in the panel.
     * 
     * @param listener The listener to add.
     * 
     * @see removeFadersListener
    */
    void addFadersListener (FadersChangeListener& listener) { m_fadersListeners.add (&listener); }

    /**
     * @brief Remove listener to any fader movement in the panel.
     * 
     * @param listener The listener to remove.
     * 
     * @see addFadersListener
    */
    void removeFadersListener (FadersChangeListener& listener) { m_fadersListeners.remove (&listener); }

#endif /* SDTK_ENABLE_FADER */


    /** @internal */
    void visibilityChanged() override { setColours(); }
    /** @internal */
    void lookAndFeelChanged() override { setColours(); }
    /** @internal */
    void paint (juce::Graphics& g) override;
    /** @internal */
    void resized() override;

 private:
    // clang-format off
   
   MeterOptions                          m_options               {};
   
   using                            MetersType              = juce::OwnedArray<MeterChannel>;  
   MetersType                       m_meterChannels;   
   MeterChannel                     m_labelStrip;
   LabelStripPosition               m_labelStripPosition    = LabelStripPosition::right;
   
   juce::AudioChannelSet            m_channelFormat         = juce::AudioChannelSet::stereo();
   
   bool                             m_useInternalTimer      = true; 
   juce::Font                       m_font;  
   int                              m_autoSizedPanelWidth   = 0;                                       

   juce::Colour                     m_backgroundColour      = juce::Colours::black;
      
#if SDTK_ENABLE_FADER

   // Fader members and methods...
   using                            FadersListenerList      = juce::ListenerList<FadersChangeListener>;  // List of listeners to fader changes.
                                                                           
   FadersListenerList               m_fadersListeners;                                                   // List of listeners to fader changes.
   std::vector<float>               m_faderGainsBuffer;
   std::vector<float>               m_faderGains;  
   
   void                             notifyListeners         ();                                          // Notify the listeners the faders have been moved.
   void                             mouseEnter              (const juce::MouseEvent& event) override;
   void                             mouseExit               (const juce::MouseEvent& event) override;
   void                             faderChanged            ( MeterChannel* sourceMeter);
   
#endif

   // Private methods...
   void                             timerCallback           () override { refresh(); }
   void                             setColours              ();
   void                             createMeters            ( const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames );
   void                             deleteMeters            ();
   [[nodiscard]] MeterChannel*      getMeterChannel         ( const int meterIndex );


    // clang-format on
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetersComponent)
};

}  // namespace SoundMeter
}  // namespace sd
