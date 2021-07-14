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
 * @brief The meter component.
*/
class NewMeterComponent
  : public juce::Component
  , public juce::SettableTooltipClient
{
public:
   //==============================================================================
   // clang-format off

   using ChannelType = juce::AudioChannelSet::ChannelType;

   NewMeterComponent() : m_fader( this ) { setPaintingIsUnclipped( true ); }

   NewMeterComponent( const juce::String&          name,
                      const std::vector<float>&    ticks, 
                      MeterPadding                 padding, 
                      float                        meterDecy, 
                      bool                         headerVisible, 
                      bool                         valueVisible, 
                      bool                         isLabelStrip    = false, 
                      Fader::Listener*             faderListener   = nullptr,       
                      int                          channelIdx      = 0,
                      ChannelType                  channelType     = ChannelType::unknown );

   ~NewMeterComponent() override = default;

   /**
    * Reset the meter (but not the peak hold).
    *
    * @see resetPeakHold
    */
   void reset();
    
   /**
    * Set the input level from the audio engine. 
    *
    * Called from the audio thread!
    *
    * @param inputLevel new input level (in amp).
    */
   inline void setInputLevel( float inputLevel ) noexcept { m_level.setInputLevel( inputLevel ); }
 
   /**
    * Show the fader briefly and fade out, unless overridden and shown longer.
    */

   // Show the fader briefly and fade out, unless overridden and shown longer.
   void flashFader();
   
   /**
    * Refresh the meter with the current input level.
    *
    * @param forceRefresh when set, the meter will be repainted no matter what.
    */
   void refresh( bool forceRefresh );

   //==============================================================================
   // Properties.

   /**
    * Sets the gui refresh rate. 
    *
    * This determines the refresh rate when using internal clocking.
    * 
    * @param guiRefreshRate refresh rate in hz.
    */
   void setGuiRefreshRate( float guiRefreshRate ) noexcept { m_level.setRefreshRate( guiRefreshRate ); }  

   /**
    * Set meter decay.
    *
    * @param decay_ms meter decay in milliseconds.
    */
   void setDecay( float decay_ms ) noexcept { m_level.setDecay( decay_ms); }
    
   /**
    * Get meter decay.
    *
    * @return meter decay in milliseconds
    */
   [[nodiscard]] float getDecay() const noexcept { return m_level.getDecay(); }

   /**
    * Set whether this meter is a label strip. 
    *
    * A label strip only draws the value labels, but does display any level.
    *
    * @param isLabelStrip when set, this meter behave like a label strip.
    */
   void setIsLabelStrip( bool isLabelStrip = false ) noexcept { m_isLabelStrip = isLabelStrip; } // When enabled, just draw the labels, not the meter.
 
   /**
    * Set the levels dividing the different regions of the meter. 
    *
    * The meter has 3 regions. Normal, warning and peak. 
    * The peak region level supplied need to be larger then the warning region level. 
    *
    * @param warningRegion_db sets the level (in db) dividing the normal and warning regions of the meter.
    * @param peakRegion_db sets the level (in db) dividing the warning and peak regions of the meter.
    */
   void setRegions( float warningRegion_db, float peakRegion_db ) { m_level.setRegions( warningRegion_db, peakRegion_db ); refresh( true ); }
   
   /**
    * Set the padding of the meter. 
    *
    * The padding is the space between the meter and the edges 
    * of the component.
    *
    * @param padding amount of padding to apply.
    */
   void setPadding( MeterPadding padding ) noexcept { m_padding = padding; }
   
   /**
    * Set the meter's 'header' part's info.
    *
    * This meter's header part contains info like channel name, index, type, etc..
    *
    * @param header the header info to setup the 
    */
   void setHeader( Header header ) noexcept { m_header = header; refresh( true ); }

   void                                   setChannelName          ( const juce::String& channelName ) { m_header.setName( channelName ); addDirty( m_header.getBounds() ); }
   void                                   setChannelType          ( ChannelType channelType ) { m_header.setType( channelType ); }
   void                                   setHeaderVisible        ( bool showChannelName = true ) { m_header.setVisible( showChannelName ); addDirty( m_header.getBounds() ); }
   [[nodiscard]] bool                     nameFits                ( const juce::String& name, int widthAvailable ) const { return m_header.nameFits( name, widthAvailable ); }

   /**
    * Get the width (in pixels) of the channel name.
    *
    * @return the width (in pixels) taken by the channel name in the header.
    */
   [[nodiscard]] float                    getChannelNameWidth     () const noexcept { return m_header.getNameWidth(); }

   /**
    * Active or de-activate (mute) meter.
    *
    * @param isActive when set, the meter is active.
    * @param notify determine whether to notify all listeners.
    */
   void                                   setActive               ( bool isActive, NotificationOptions notify = NotificationOptions::dontNotify );
   [[nodiscard]] bool                     isActive                () const noexcept { return m_active; }

   /**
    * @brief Active (or de-activate) fader.
    * @param faderActive When set to true, will activate the meter's fader.
    * @see isFaderActive
   */
   void                                   setFaderActive          ( bool faderActive = true );
   [[nodiscard]] bool                     isFaderActive           () const noexcept { return m_fader.isActive(); }

   void                                   setFaderEnabled         ( bool faderEnabled = true ) { m_fader.setEnabled( faderEnabled ); addDirty( m_fader.getBounds() ); }

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
   void setFaderValue( float value, NotificationOptions notificationOption = NotificationOptions::dontNotify, bool showFader = true ) noexcept;
   
   /**
    * @brief Set the meter font. 
    *
    * Font to be used for the header, value and label strip.
    *
    * @param font The font to use.
    */
   void setFont( const juce::Font& font ) noexcept { m_header.setFont( font.withHeight( Constants::kChannelNameFontHeight ) ); setDirty(); }

   /**
    * @brief Reset the peak hold.
    *
    * Resets the peak hold indicator and value.
    *
    * @see showPeakHold
    */
   void resetPeakHold() noexcept { m_level.resetPeakHoldLevel(); setDirty(); }

   /**
    * @brief Show the peak hold indicator.
    *
    * @param showPeakHold When set true, the peak hold indicator will be shown.
    * @see showPeakValue, resetPeakHold
    */
   void showPeakHold( bool showPeakHold = true ) { m_level.setPeakHoldVisible( showPeakHold ); setDirty(); }

   /**
    * @brief Show the peak value.
    *
    * The peak value will be shown below the meter (in dB).
    * It's the same level as the peak hold bar.
    *
    * @param showPeakValue When set true, shows the value level (in dB) below the meter.
    * @see showPeakHold
    */
   void showPeakValue( bool showPeakValue = true ) { m_level.setPeakValueVisible( showPeakValue ); setDirty(); }

   /**
    * @brief Show tick-marks (divider lines) on the meter.
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param showTickMarks When set true, shows the tick-marks. 
    * @see setTickMarks
    */
   void showTickMarks( bool showTickMarks = true ) { m_level.setTickMarksVisible( showTickMarks ); setDirty(); }
  
   /**
    * @brief Set the level of the tick marks. 
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param ticks list of tick mark values (in amp).
    * @see showTickMarks
    */
   void setTickMarks( const std::vector<float>& ticks ) { m_level.setTickMarks( ticks ); }

   /**
    * Get the bounds of the 'meter' part.
    *
    * @return the bounds of the meter.
    */
   [[nodiscard]] juce::Rectangle<int> getMeterBounds() const noexcept { return m_level.getMeterBounds(); }
   
   void                                   setMinimalMode          ( bool minimalMode ) noexcept;
   [[nodiscard]] bool                     isMinimalModeActive     () const noexcept { return m_minimalMode; }
   [[nodiscard]] bool                     autoSetMinimalMode      ( int proposedWidth, int proposedHeight );

   void                                   addFaderListener        ( Fader::Listener& listener ) { m_fader.addListener( listener ); }
   void                                   removeFaderListener     ( Fader::Listener& listener ) { m_fader.removeListener( listener ); }

   //==============================================================================
   // Internal
   void                                   paint                   ( juce::Graphics& g ) override;
   void                                   resized                 () override;
   void                                   lookAndFeelChanged      () override { setColours(); setDirty(); }  // On change in lookandfeel or visibility. Cache the colours.
   void                                   visibilityChanged       () override { setColours(); setDirty(); }
     
   /**
    * @brief Colour IDs that can be used to customise the colours.
    * This can be done by overriding juce's LookAndFeel class.
   */
   enum ColourIds
   {
      backgroundColourId                  = 0x1a03201,            ///< Background colour.
      peakColourId                        = 0x1a03202,            ///< Peak area colour.
      warningColourId                     = 0x1a03203,            ///< Warning area colour.
      normalColourId                      = 0x1a03204,            ///< Normal colour.
      tickMarkColourId                    = 0x1a03205,            ///< Tick-mark colour.
      textColourId                        = 0x1a03206,            ///< Text colour.
      faderColourId                       = 0x1a03207,            ///< Fader colour.  
      textValueColourId                   = 0x1a03208,            ///< Value text colour.
      textBackgroundColourId              = 0x1a03209,            ///< Text background colour.
      mutedColourId                       = 0x1a03210,            ///< Muted button colour.
      solodColourId                       = 0x1a03211,            ///< Soloed button colour..
      mutedMouseOverColourId              = 0x1a03212,            ///< Muted mouse over button colour.
      inactiveColourId                    = 0x1a03213             ///< Inactive (muted) colour.
   };

private:

   //==============================================================================
   // Private members.
   Header                                 m_header;                                 // Info relating to the meter (name, channel type, info rect, index in a sequence of multiple meters).

   Fader                                  m_fader;

   Level                                  m_level;

   // GUI
   MeterPadding                           m_padding               { 0, 0, 0, 0 };  // Space between meter and component's edge.

   // Flags
   bool                                   m_active                = true;
   bool                                   m_isLabelStrip          = false;
   bool                                   m_minimalMode           = false;
   juce::Rectangle<int>                   m_dirtyRect             {};

   // Cached colours...
   juce::Colour                           m_mutedColour           = juce::Colours::red;
   juce::Colour                           m_mutedMouseOverColour  = juce::Colours::blue;
   juce::Colour                           m_textColour            = juce::Colours::white;
   juce::Colour                           m_textValueColour       = juce::Colours::white;
   juce::Colour                           m_inactiveColour        = juce::Colours::red;
   juce::Colour                           m_faderColour           = juce::Colours::yellow.withAlpha( Constants::kFaderAlphaMax );
   juce::Colour                           m_backgroundColour      = juce::Colours::black;
   juce::Colour                           m_tickColour            = juce::Colours::white;
   juce::Colour                           m_peakColour            = juce::Colours::red;
   juce::Colour                           m_warningColour         = juce::Colours::yellow;
   juce::Colour                           m_normalColour          = juce::Colours::green;
   
   //==============================================================================
   // Private methods.
   void                                   setDirty                ( bool isDirty = true ) noexcept { m_dirtyRect = {0,0,0,0}; if( isDirty ) m_dirtyRect = getLocalBounds(); }
   [[nodiscard]] bool                     isDirty                 ( const juce::Rectangle<int>& rectToCheck = {} ) const noexcept;
   void                                   addDirty                ( juce::Rectangle<int> dirtyRect ) noexcept { m_dirtyRect = m_dirtyRect.getUnion( dirtyRect ); }
 
   void                                   drawMeter               ( juce::Graphics& g );                                                                                                         

   [[nodiscard]] juce::Colour             getColourFromLnf        ( int colourId, const juce::Colour& fallbackColour ) const;
      
   void                                   mouseDown               ( const juce::MouseEvent& event ) override;
   void                                   mouseDrag               ( const juce::MouseEvent& event ) override;
   void                                   mouseMove               ( const juce::MouseEvent& event ) override;
   void                                   mouseExit               ( const juce::MouseEvent& event ) override;
   void                                   mouseWheelMove          ( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel ) override; 
   void                                   mouseDoubleClick        ( const juce::MouseEvent& event ) override; 
   void                                   resetMouseOvers         () noexcept;    
   void                                   setColours              () noexcept;

   // clang-format on
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( NewMeterComponent )
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_COMPONENT_H */