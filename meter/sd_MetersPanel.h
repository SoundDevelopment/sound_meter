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


#ifndef SD_SOUND_METERS_PANEL_H
#define SD_SOUND_METERS_PANEL_H

/**
 * @brief Namespace containing all concerning the sound_meter module.
*/
namespace sd::SoundMeter
{

/**
 * @brief Panel containing one or more meters.
 */
class MetersPanel
  : public juce::Component
  , public sd::SoundMeter::Fader::Listener
{
public:
   static constexpr auto kMasterFaderLeftPadding = 10;  ///< Padding (in pixels) on the left side of the master fader.
   static constexpr auto kFaderRightPadding      = 1;   ///< Padding (in pixels) on the right side of the channel faders.

   /**
    * @brief Constructor.
   */
   MetersPanel();

   /**
    * @brief Constructor which accepts a channel format.
    * 
    * This constructor will automatically setup the panel with the 
    * right amount of meters, named according to the channel format.
    * @param channelFormat The channel format to use to initialise the panel.
    * @see MixerPanel, ~MixerPanel
   */
   explicit MetersPanel( const juce::AudioChannelSet& channelFormat );

   /** @brief Destructor.*/
   ~MetersPanel() override;

   /**
    * @brief Refresh (redraw) the meters panel.
    * 
    * @param forceRefresh When set to true, always redraw the meters panel (not only if it's dirty/changed).
    */
   void refresh( bool forceRefresh );

   /**
    * @brief Reset the meters.
    * 
    * Initialise the meters, faders and clears all the levels (but not the peak hold).
   */
   void reset();

   /**
    * @brief Set all faders to unity gain.
    */
   void resetFaders();

   /**
    * @brief Reset all meters.
    * Resets all meters (but not the peak hold.
   */
   void resetMeters();

   /**
    * @brief Set the input levels.
    * 
    * This supplies a meter of a specific channel with the peak level from the audio engine.
    * Beware: this will usually be called from the audio thread.
    * 
    * @param channel The channel to set the input level of.
    * @param value   The input level to set to the specified channel.
   */
   void setInputLevel( int channel, float value );

   /**
    * @brief Notify the listeners the faders have been moved.
   */
   void notifyListeners();

   /**
    * @brief Set the channel format (number of channels) to use for the mixer/meters.
    * 
    * @param channels     The channel format to use.
    * @param channelNames The (optional) channel names to use in the header of the meters.
    */
   void setChannelFormat( const juce::AudioChannelSet& channels, const std::vector<juce::String>& channelNames = {} );

   /**
    * @brief Set the channel names to display above the meters.
    * 
    * @param channelNames The list of names to use for the meters.
   */
   void setChannelNames( const std::vector<juce::String>& channelNames );

   /**
    * @brief Set the levels dividing the different regions of the meter.
    *
    * The meter has 3 regions. Normal, warning and peak. 
    * The peak region level supplied need to be larger then the warning region level. 
    *
    * @param warningRegion_db The level (in dB) dividing the normal and warning regions of the meter.
    * @param peakRegion_db    The level (in dB) dividing the warning and peak regions of the meter.
    */
   void setRegions( float warningRegion_db, float peakRegion_db );

   /**
    * @brief Set the refresh (redraw) rate of the meters.
    * 
    * Also used for meter ballistics.
    * 
    * @param refreshRate The refresh rate (in Hz).
    * 
    * @see setMeterDecay
   */
   void setPanelRefreshRate( int refreshRate ) noexcept { m_guiRefreshRate = refreshRate; }

   /**
    * @brief Set meter decay rate.
    * 
    * Set's the meter's decay rate in milliseconds.
    * The meter's attack is instant.
    * 
    * @param decay The time it takes the meter to decay (in ms).
    * 
    * @see setPanelRefreshRate
   */
   void setMeterDecay( float decay_ms );

   /**
    * @brief Set the font to be used in the panel and it's meters.
    * @param newFont The font to use.
   */
   void setFont( const juce::Font& newFont ) noexcept;

   /**
    * @brief Enable or disable the panel.
    * 
    * @param enabled When set to true, the meters panel will be displayed.
    */
   void setEnabled( bool enabled = true );

   /**
    * @brief Enable the faders on the meters.
    *
    * @param fadersEnabled When set to true, the faders are enabled.
    */
   void setFadersEnabled( bool fadersEnabled ) noexcept;

   /**
    * @brief Get values from all faders.
    *
    * @param notificationOption Set whether to notify the listeners of the gathered fader values.
    * @see notifyListeners()
    */
   void getFaderValues( sd::SoundMeter::NotificationOptions notificationOption = sd::SoundMeter::NotificationOptions::notify );

   /**
    * @brief Get the master fader.
    * Get a reference to the master fader component.
    * @return A reference to the master fader component.
   */
   const sd::SoundMeter::NewMeterComponent& getMasterFader() const noexcept { return m_masterFader; }

   /**
    * @brief Show (or hide) all the faders.
    * @param showMixer When set to true, will show the faders. Otherwise it will hide them.
   */
   void showFaders( bool showMixer );

   /**
    * @brief Toggle mute channels.
    * Mute all channels, or if all were muted, un-mute all.
    */
   void toggleMute();

   /**
    * Check if all channels have been set inactive..
    * 
    * @return True, if all channels are inactive (muted).
    */
   [[nodiscard]] bool areAllChannelsInactive();

   /**
    * @brief Reset all peak hold indicators and 'values'.
   */
   void peakHoldReset();

   /**
    * @brief Get the default mixer width.
    * Get's the default mixer width in pixels. 
    * This is the width where all channels are wide enough to display
    * all channel names.
    * @return The default mixer width (in pixels).
   */
   int getDefaultMixerWidth() const noexcept { return m_mixerDefaultWidth; }

   /**
    * @brief Get the number of meters in the panel.
    * @return The number of meters in the panel.
   */
   int getNumMeters() const noexcept { return m_meters.size(); }

   /**
	 * @brief A listener to any fader changes in the meters panel.
	*/
   struct FadersChangeListener
   {
      virtual ~FadersChangeListener()                             = default;
      virtual void fadersChanged( std::vector<float> faderGains ) = 0;
   };

   /**
    * @brief Add a listener to any fader movement in the panel.
    * 
    * @param listener The listener to add.
    * 
    * @see removeFadersListener
   */
   void addFadersListener( FadersChangeListener& listener ) { m_fadersListeners.add( &listener ); }

   /**
    * @brief Remove listener to any fader movement in the panel.
    * 
    * @param listener The listener to remove.
    * 
    * @see addFadersListener
   */
   void removeFadersListener( FadersChangeListener& listener ) { m_fadersListeners.remove( &listener ); }

   //==============================================================================
   // Internal juce methods...

   void visibilityChanged() override { setColours(); }
   void lookAndFeelChanged() override { setColours(); }
   void paint( juce::Graphics& g ) override;
   void resized() override;
   void mouseEnter( const juce::MouseEvent& event ) override;
   void mouseExit( const juce::MouseEvent& event ) override;

private:
   // clang-format off

   using                            MetersType           = juce::OwnedArray<NewMeterComponent>;       ///< Container type for multiple meters.
   using                            FadersListenerList   = juce::ListenerList<FadersChangeListener>;  ///< List of listeners to fader changes.

   // Private members...
   FadersListenerList               m_fadersListeners;                                                ///< List of listeners to fader changes.

   std::vector<float>               m_faderGainsBuffer;
   std::vector<float>               m_faderGains;

   juce::AudioChannelSet            m_channelFormat      = juce::AudioChannelSet::stereo();

   MetersType                       m_meters;                                                         ///< All meter objects.

   std::vector<float>               m_tickMarks          = { -1.0f, -3.0f, -6.0f, -9.0f, -18.0f };    ///< Tick-mark position in db.
   int                              m_meterWidth         = 20;                                        ///< Width of the meter (in pixels).
   int                              m_masterStripWidth   = m_meterWidth;                              ///< Width of the tick-mark labels (in pixels).
   int                              m_mixerDefaultWidth  = 0;                                       
   float                            m_meterDecayTime_ms  = Constants::kDefaultDecay_ms;               // NOLINT

   NewMeterComponent                m_masterFader;

   bool                             m_enabled            = true;
   bool                             m_fadersEnabled      = false;
   juce::Font                       m_font               {};
   int                              m_guiRefreshRate     = 24;
   float                            m_warningRegion_db   = Constants::kWarningLevel_db;
   float                            m_peakRegion_db      = Constants::kPeakLevel_db;

   juce::Colour                     m_backgroundColour   = juce::Colours::black;

   // Private methods...
   void                             setColours           ();
   void                             faderChanged         ( NewMeterComponent* sourceMeter, float value ) override;
   void                             createMeters         ( const juce::AudioChannelSet& channelFormat, const std::vector<juce::String>& channelNames );
   void                             deleteMeters         ();
   [[nodiscard]] NewMeterComponent* getMeter             ( const int meterIndex ) noexcept;


   // clang-format on
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( MetersPanel )
};

}

#endif /* SD_SOUND_METERS_PANEL_H */