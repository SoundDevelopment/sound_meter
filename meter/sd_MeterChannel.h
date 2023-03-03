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

#include "sd_MeterHelpers.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>


namespace sd  // NOLINT
{
namespace SoundMeter
{

/**
 * @brief An individual meter channel.
 * 
 * This represents a single meter.
 * Use the MetersComponent to create multiple meters matching
 * the specified channel format.
*/
class MeterChannel
  : public juce::Component
  , private juce::SettableTooltipClient
{
public:
    using ChannelType = juce::AudioChannelSet::ChannelType;
    using Ptr         = juce::Component::SafePointer<MeterChannel>;

    /**
     * @brief Default constructor.
    */
    MeterChannel();

    /**
     * @brief Parameterized constructor.
     * 
     * @param meterOptions  Meter options to use (defining appearance and functionality).
     * @param padding       The padding to use (space between meter and the edge of the component).
     * @param channelName   The channel name (set by the user).
     * @param isLabelStrip  When set to true, this meter will function as a label strip (with markers for levels at the tick-marks).
     * @param channelType   The channel type (left, right, center, etc...).
    */
    MeterChannel (const MeterOptions& meterOptions, Padding padding, const juce::String& channelName, bool isLabelStrip = false,
                  ChannelType channelType = ChannelType::unknown);

#if SDTK_ENABLE_FADER
    /**
     * @brief Destructor
    */
    virtual ~MeterChannel() override;
#endif

    /**
     * @brief Reset the meter (but not the peak hold).
     *
     * @see resetPeakHold
    */
    void reset();

    /**
     * @brief Refresh the meter with the current input level.
     *
     * @param forceRefresh When set to true, the meter will be forced to repaint (even if not dirty).
     * @see setRefreshRate
    */
    void refresh (bool forceRefresh);

    /**
     * @brief Sets the meter's refresh rate. 
     *
     * Set this to optimize the meter's decay rate.
     * 
     * @param refreshRate_hz Refresh rate in Hz.
     * @see refresh, setDecay, getDecay
    */
    void setRefreshRate (float refreshRate_hz) { m_level.setRefreshRate (refreshRate_hz); }

    /**
     * @brief Set meter decay.
     *
     * @param decay_ms Meter decay in milliseconds.
     * @see getDecay, setRefreshRate
    */
    void setDecay (float decay_ms) { m_level.setDecay (decay_ms); }

    /**
     * @brief Get meter decay.
     *
     * @return Meter decay in milliseconds.
     * @see setDecay, setRefreshRate
    */
    [[nodiscard]] float getDecay() const noexcept { return m_level.getDecay(); }

    /**
     * @brief Set the input level from the audio engine. 
     *
     * Called from the audio thread!
     *
     * @param inputLevel New input level (in amp).
    */
    inline void setInputLevel (float inputLevel) { m_level.setInputLevel (inputLevel); }

    /**
     * @brief Set the meter's options.
     *
     * The options determine the appearance and functionality of the meter.
     *
     * @param meterOptions Meter options to use.
    */
    void setOptions (const MeterOptions& meterOptions);

    /**
     * @brief Activate or deactivate (mute) the meter.
     *
     * @param isActive When set to true, the meter is active.
     * @param notify   Determine whether to notify all listeners or not.
     * @see isActive
    */
    void setActive (bool isActive, NotificationOptions notify = NotificationOptions::dontNotify);

    /**
     * @brief Check if the meter is active (un-muted).
     *
     * @return True, if the meter is active (un-muted).
     *
     * @see setActive
    */
    [[nodiscard]] bool isActive() const noexcept { return m_active; }

    /**
     * @brief Set whether this meter is a label strip. 
     *
     * A label strip only draws the value labels (at the tick-marks),
     * but does not display any level.
     *
     * @param isLabelStrip when set, this meter behave like a label strip.
    */
    void setIsLabelStrip (bool isLabelStrip = false) noexcept { m_isLabelStrip = isLabelStrip; }

    /**
     * @brief Set the segments the meter is made out of.
     *
     * All segments have a level range, a range within the meter and a colour (or gradient).
     *
     * @param segmentsOptions The segments options to create the segments with.
    */
    void setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions);

    /**
     * @brief Reset the peak hold.
     *
     * Resets the peak hold indicator and value.
     *
     * @see enablePeakHold
    */
    void resetPeakHold();

    /**
     * @brief Enable (or disable) the peak hold indicator.
     *
     * @param enablePeakHold When set true, the peak hold indicator will be shown.
     * @see showPeakValue, resetPeakHold
    */
    void enablePeakHold (bool enablePeakHold = true) { m_level.enablePeakHold (enablePeakHold); }

    /**
     * @brief Show the peak 'value' part of the meter.
     *
     * The peak 'value' part will be shown below the meter (in dB).
     * It's the same level as the peak hold bar.
     *
     * @param showPeakValue When set true, shows the 'value' level (in dB) part below the meter.
     * @see enablePeakHold, resetPeakHold, showValue
    */
    void showValue (bool showValue = true);

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
    void showHeader (bool headerVisible);

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
    void enableHeader (bool headerEnabled);

    /**
     * @brief Set the meter font. 
     *
     * Font to be used for the header, value and label strip.
     *
     * @param font The font to use.
    */
    void setFont (juce::Font* font);

    /**
     * @brief Set the channel name.
     * 
     * Set's the channel name belonging to the track
     * feeding the meter.
     * 
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
     * @brief Check if a specified name will fit in a give width (in pixels).
     *
     * This can be a more detailed, multi-line description of the function,
     * and it's containing logic.
     *
     * @param name           The name to check the width of.
     * @param widthAvailable The width (in pixels) available to fit the name in.
     * @return               True, if the name will fit in the given width (in pixels).
     *
     * @see getChannelNameWidth, setChannelNames
    */
    [[nodiscard]] bool nameFits (const juce::String& name, int widthAvailable) const { return m_header.textFits (name, widthAvailable); }

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
     * 
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
    void setReferredTypeWidth (float referredTypeWidth) noexcept { m_header.setReferredWidth (referredTypeWidth); }

    /**
     * @brief Enable tick-marks (divider lines) on the meter.
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     * 
     * The tick-marks will be shown when they are enable and visible.
     *
     * @param tickMarksEnabled When set true, the tick-marks are enabled. 
     * @see showTickMarks, setTickMarks, showTickMarksOnTop
    */
    void showTickMarks (bool tickMarksEnabled) { m_level.showTickMarks (tickMarksEnabled); }

    /**
     * @brief Show the tick-marks on top of the level or below it.
     * 
     * When below the level, the tick-marks will be obscured if the 
     * level is loud enough.
     * 
     * @param showTickMarksOnTop Show the tick-marks on top of the level.
   */
    void showTickMarksOnTop (bool showTickMarksOnTop) { m_level.showTickMarksOnTop (showTickMarksOnTop); }

    /**
     * @brief Set the level of the tick marks. 
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     *
     * @param tickMarks List of tick mark values (in decibels).
     * @see showTickMarks, showTickMarksOnTop, showTickMarks
    */
    void setTickMarks (const std::vector<float>& tickMarks) { m_level.setTickMarks (tickMarks); }

    /**
     * @brief Set the padding of the meter. 
     *
     * The padding is the space between the meter and the edges 
     * of the component.
     *
     * @param padding Amount of padding to apply.
    */
    void setPadding (const Padding& padding) noexcept { m_padding = padding; }

    /**
     * Get the bounds of the 'meter' and 'header' parts combined.
     *
     * @return The bounds of the 'meter' and 'header' parts combined.
    */
    [[nodiscard]] juce::Rectangle<int> getLabelStripBounds() const;

    /**
     * @brief Set the meter in 'minimal' mode.
     * 
     * In minimal mode, the meter is in it's cleanest state possible.
     * This means no header, no tick-marks, no value, no faders and no indicator.
     * 
     * @param minimalMode When set to true, 'minimal' mode will be enabled.
     * @see isMinimalModeActive, autoSetMinimalMode
    */
    void setMinimalMode (bool minimalMode);

    /**
     * @brief Check if the meter is in 'minimal' mode.
     * 
     * In minimal mode, the meter is in it's cleanest state possible.
     * This means no header, no tick-marks, no value, no faders and no indicator.
     * 
     * @return True, if the meter is in 'minimal' mode.
     * @see setMinimalMode, autoSetMinimalMode
    */
    [[nodiscard]] bool isMinimalModeActive() const noexcept { return m_minimalMode; }

    /**
     * @brief Automatically set the meter in 'minimal' mode.
     * 
     * Use the proposed height and width to determine if that would lead
     * to the meter being in 'minimal' mode. Then apply that mode.
     * 
     * In minimal mode, the meter is in it's cleanest state possible.
     * This means no header, no tick-marks, no value, no faders and no indicator.
     * 
     * @param proposedWidth  The width use to determine if the meter would be in 'minimal' mode.
     * @param proposedHeight The height use to determine if the meter would be in 'minimal' mode.
     * @return               True, if the meter is in 'minimal' mode.
     * 
     * @see setMinimalMode, isMinimalModeActive
    */
    [[nodiscard]] bool autoSetMinimalMode (int proposedWidth, int proposedHeight);

    /**
     * @brief Use gradients in stead of hard segment boundaries.
     * 
     * @param useGradients When set to true, uses smooth gradients. False gives hard segment boundaries.
    */
    void useGradients (bool useGradients) { m_level.useGradients (useGradients); }

#if SDTK_ENABLE_FADER

    /**
     * @brief Show or hide the fader.
     * 
     * The fader overlay display on top of the 'meter' part
     * (in combination with the 'mute' buttons in the 'header' part)
     * can be used by the user to control gain or any other
     * parameter.
     * 
     * @param faderVisible When set to true, show the fader. Otherwise hide it.
     * @see isFaderVisible, setFaderEnabled
    */
    void showFader (bool faderVisible = true);

    /**
     * @brief Check if the meter is visible or hidden.
     * 
     * @return True, if the meter is visible.
     * @see showFader, setFaderEnabled
    */
    [[nodiscard]] bool isFaderVisible() const noexcept { return m_fader.isVisible(); }

    /**
     * @brief Enable the 'fader' overlay.
     * 
     * The fader overlay display on top of the 'meter' part
     * (in combination with the 'mute' buttons in the 'header' part)
     * can be used by the user to control gain or any other
     * parameter.
     * 
     * @param faderEnabled True, when the fader needs to be enabled.
     * @see isFaderVisible, showFader
    */
    void enableFader (bool faderEnabled = true);

    /**
     * @brief Show the fader briefly and fade out (unless overridden and shown longer).
    */
    void flashFader();

    /**
     * @brief Get the value of the meter fader.
     *
     * @return The current fader value [0..1].
    */
    [[nodiscard]] float getFaderValue() const noexcept { return m_fader.getValue(); }

    /**
     * @brief Set fader value.
     *   
     * The fader overlay display on top of the 'meter' part
     * (in combination with the 'mute' buttons in the 'header' part)
     * can be used by the user to control gain or any other
     * parameter.
     *
     * @param value              The value [0..1] the fader needs to be set to.
     * @param notificationOption Select whether to notify the listeners.
     * @param showFader          When set to true, the fader will briefly appear (when the value is changed).
    */
    void setFaderValue (float value, NotificationOptions notificationOption = NotificationOptions::dontNotify, bool showFader = true);

    /**
     * @brief Notify the parent component that a fader has moved (or a mute button has been pressed).
    */
    void notifyParent();

    /** You can assign a lambda to this callback object to have it called when the button is clicked. */
    std::function<void (sd::SoundMeter::MeterChannel* meter)> onFaderMove;

#endif /* SDTK_ENABLE_FADER */

    /** @internal */
    void paint (juce::Graphics& g) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void lookAndFeelChanged() override;  // On change in lookandfeel or visibility. Cache the colours.
    /** @internal */
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
    Header m_header {};  ///< 'Header' part of the meter with info relating to the meter (name, channel type, info rect, index in a sequence of multiple meters).
    Level        m_level {};         ///< 'Meter' part of the meter. Actually displaying the level.
    MeterOptions m_meterOptions {};  ///< 'Meter' options.

#if SDTK_ENABLE_FADER
    Fader m_fader;
#endif /* SDTK_ENABLE_FADER */

    bool                 m_active { true };
    bool                 m_isLabelStrip { false };
    bool                 m_minimalMode { false };
    juce::Rectangle<int> m_dirtyRect {};

    Padding m_padding { 0, 0, 0, 0 };  ///< Space between meter and component's edge.

    // Cached colours...
    juce::Colour m_backgroundColour    = juce::Colours::black;
    juce::Colour m_inactiveColour      = juce::Colours::grey;
    juce::Colour m_textColour          = juce::Colours::white.darker (0.6F);
    juce::Colour m_textValueColour     = juce::Colours::white.darker (0.6F);
    juce::Colour m_tickColour          = juce::Colours::white.darker (0.3F).withAlpha (0.5F);
    juce::Colour m_muteColour          = juce::Colours::red;
    juce::Colour m_muteMouseOverColour = juce::Colours::black;
    juce::Colour m_faderColour         = juce::Colours::blue.withAlpha (Constants::kFaderAlphaMax);

    void                       setDirty (bool isDirty = true);
    [[nodiscard]] bool         isDirty (const juce::Rectangle<int>& rectToCheck = {}) const noexcept;
    void                       addDirty (const juce::Rectangle<int>& dirtyRect) noexcept { m_dirtyRect = m_dirtyRect.getUnion (dirtyRect); }
    void                       drawMeter (juce::Graphics& g);
    [[nodiscard]] juce::Colour getColourFromLnf (int colourId, const juce::Colour& fallbackColour) const;
    void                       mouseMove (const juce::MouseEvent& event) override;
    void                       mouseExit (const juce::MouseEvent& event) override;
    void                       mouseDoubleClick (const juce::MouseEvent& event) override;
    void                       resetMouseOvers();
    void                       setColours();

#if SDTK_ENABLE_FADER
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseDown (const juce::MouseEvent& event) override;
#endif /* SDTK_ENABLE_FADER */

    // clang-format on
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterChannel)
};

}  // namespace SoundMeter
}  // namespace sd
