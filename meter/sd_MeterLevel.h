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
#include "sd_MeterSegment.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace sd  // NOLINT
{
namespace SoundMeter
{

struct Options;

/**
 * @brief Class responsible for anything relating to the 'meter' and peak 'value' parts.
 * This also includes the peak hold indicator and the tick-marks.
*/
class Level final
{
public:
    /**
     * @brief Constructor.
    */
    Level() = default;

    /**
     * @brief Reset the meter (but not the peak hold).
     *
     * @see resetPeakHold
    */
    void reset();

    /**
     * @brief Set the level of the meter.
     * 
     * Here the level is actually set from the audio engine.
     * Beware: very likely called from the audio thread!
     * 
     * @param newLevel The peak level from the audio engine (in amp).
     * 
     * @see getInputLevel
    */
    void setInputLevel (float newLevel);

    /**
     * @brief Get's the meter's input level.
     * 
     * @return The meter's input level (in decibels).
     * 
     * @see setInputLevel
    */
    [[nodiscard]] float getInputLevel();

    /**
     * @brief Calculate the actual meter level (ballistics included).
     * 
     * Calculate the meter's level including ballistics.
     * Instant attack, but decayed release.
     * 
     * @see getMeterLevel, setDecay
    */
    void refreshMeterLevel ();

    /**
     * @brief Get the actual meter's level (including ballistics).
     * 
     * Get the decayed meter level.
     * Instant attack, but decayed release.
     * 
     * @return The actual meter's level (in decibels) with ballistics.
     * 
     * @see setMeterLevel, setDecay
    */
    [[nodiscard]] float getMeterLevel() const noexcept { return m_meterLevel_db; }

    /**
     * @brief Set the meter's options.
     *
     * The options determine the appearance and functionality of the meter.
     *
     * @param meterOptions Meter options to use.
    */
    void setOptions (const Options& meterOptions);

    /**
     * @brief Get the meter's refresh (redraw) rate.
     *
     * @return The refresh rate of the meter in Hz.
     *
     * @see setRefreshRate
    */
    [[nodiscard]] float getRefreshRate() const noexcept { return m_options.refreshRate; }

    /**
     * @brief Sets the meter's refresh rate. 
     *
     * Set this to optimize the meter's decay rate.
     * 
     * @param refreshRate_hz Refresh rate in Hz.
     * @see setDecay, getDecay
    */
    void setRefreshRate (float refreshRate_hz);

    /**
     * @brief Set meter decay.
     *
     * @param decay_ms Meter decay in milliseconds.
     * 
     * @see getDecay
    */
    void setDecay (float decay_ms);

    /**
     * @brief Get meter decay.
     *
     * @return Meter decay in milliseconds
     * 
     * @see setDecay
    */
    [[nodiscard]] float getDecay() const noexcept { return m_options.decayTime_ms; }

    /**
     * @brief Set's the visibility of the peak hold indicator.
     * 
     * @param isVisible When true, the peak hold indicator is shown.
     * 
     * @see isPeakHoldEnabled, resetPeakHold
    */
    void enablePeakHold (bool isVisible) noexcept;

    /**
     * @brief Set the segments the meter is made out of.
     *
     * All segments have a level range, a range within the meter and a colour (or gradient).
     *
     * @param segmentsOptions The segments options to create the segments with.
    */
    void defineSegments (const std::vector<SegmentOptions>& segmentsOptions);

    /**
     * @brief Check if the peak hold indicator is visible.
     *
     * @return True if the peak hold indicator is visible.
     *
     * @see enablePeakHold, resetPeakHold
    */
    [[nodiscard]] bool isPeakHoldEnabled() const noexcept;

    /**
     * @brief Enable the peak 'value' part of the meter.
     *
     * When enabled (and made visible with showValue) the
     * peak 'value' part will be shown below the meter (in dB).
     * It's the same level as the peak hold bar.
     *
     * @param valueEnabled When set true, the 'value' level (in dB) part below the meter will be enabled.
     * 
     * @see isPeakValueVisible, resetPeakHold, setValueVisible
    */
    void enableValue (bool valueEnabled) noexcept { m_options.valueEnabled = valueEnabled; }

    /**
     * @brief Show the peak 'value' part of the meter.
     *
     * The peak value will be shown below the meter (in dB).
     * It's the same level as the peak hold bar.
     *
     * @param isVisible When set true, shows the 'value' level (in dB) part below the meter.
     *
     * @see isPeakValueVisible, resetPeakHold, enableValue
    */
    void showValue (bool isVisible) noexcept { m_valueVisible = isVisible; }

    /**
     * @brief Check if the peak 'value' part is visible.
     *
     * The peak value will be shown below the meter (in db).
     * It's the same level as the peak hold bar.
     * 
     * @return True, if the peak hold 'value' part is visible.
     *
     * @see showValue, resetPeakHold
    */
    [[nodiscard]] bool isPeakValueVisible() const noexcept { return m_valueVisible && m_options.valueEnabled; }

    /**
     * @brief Reset the peak hold level.
     * @see getPeakHoldLevel, isPeakValueVisible, setPeakValueVisible, enablePeakHold, showValue, isPeakHoldEnabled
    */
    void resetPeakHold();

    /**
     * @brief Get the current peak hold level.
     * @return The current peak hold level (in decibels).
     * @see resetPeakHold, isPeakValueVisible, setPeakValueVisible, setPeakHoldVisible, isPeakHoldEnabled
    */
    [[nodiscard]] float getPeakHoldLevel() const noexcept;

    /**
     * @brief Set the bounds of the 'value' part of the meter.
     * 
     * @param bounds The bounds to use for the 'value' part of the meter.
     * 
     * @see getValueBounds, setMeterBounds, getMeterBounds
    */
    void setValueBounds (const juce::Rectangle<int>& bounds) noexcept { m_valueBounds = bounds; }

    /**
     * @brief Get the bounds of the 'value' part of the meter.
     *
     * @return The bounds of the 'value' part of the meter.
     * @see setMeterBounds, setValueBounds, getMeterBounds
    */
    [[nodiscard]] juce::Rectangle<int> getValueBounds() const noexcept { return m_valueBounds; }

    /**
     * @brief Set the bounds of the 'meter' part of the meter.
     * 
     * @param bounds The bounds to use for the 'meter' part of the meter.
     * @see getValueBounds, setValueBounds, getMeterBounds
    */
    void setMeterBounds (const juce::Rectangle<int>& bounds);

    /**
     * @brief Get the bounds of the 'meter' part.
     *
     * @return The bounds of the 'meter' part.
     * @see getValueBounds, setValueBounds, setMeterBounds
    */
    [[nodiscard]] juce::Rectangle<int> getMeterBounds() const noexcept { return m_meterBounds; }

    /**
     * @brief Check if the mouse cursor is over the 'value' part of the meter.
     * 
     * @param y The y coordinate (relative to the meter bounds) to use to determine if the mouse if over the 'value' part of the meter.
     * @return True, if the mouse cursor is over the 'value' part of the meter.
    */
    bool isMouseOverValue (int y);

    /**
     * @brief Check if the mouse cursor is over the 'value' part of the meter.
     * 
     * @return True, if the mouse cursor is over the 'value' part of the meter.
    */
    [[nodiscard]] bool isMouseOverValue() const noexcept { return m_mouseOverValue; }

    /**
     * @brief Reset 'mouse over' status of the 'value' part of the meter.
    */
    void resetMouseOverValue() noexcept { m_mouseOverValue = false; }

    /** @brief Get the dirty part of the meter.*/
    [[nodiscard]] juce::Rectangle<int> getDirtyBounds();

    /**
     * @brief Draws the meter.
     * 
     * @param[in,out] g The juce graphics context to use.
     * 
     * @see drawInactiveMeter, drawPeakValue, drawPeakHold, drawTickMarks, drawLabels
    */
    void drawMeter (juce::Graphics& g);

    /**
     * @brief Draw the 'meter' part in it's inactive (muted) state.
     * 
     * @param[in,out] g   The juce graphics context to use.
     * @param textColour  Colour of the text on the inactive meter.
     * 
     * @see drawMeter, drawTickMarks, drawPeakValue, drawPeakHold, drawLabels
    */
    void drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const;

    /**
     * @brief Draw the peak 'value'.
     * @param[in,out] g        The juce graphics context to use.
     * @param textValueColour  Colour of the text displaying the peak value.
     * 
     * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakHold, drawTickMarks, drawLabels
    */
    void drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const;

    /**
     * @brief Draw the tick-marks.
     * These are the lines dividing the meters at certain dB levels.
     * @param[in,out] g        The juce graphics context to use.
     * @param tickColour       Colour of the tick marks.
     * 
     * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakValue, drawPeakHold, drawLabels
    */
    void drawTickMarks (juce::Graphics& g, const juce::Colour& tickColour) const;

    /**
     * @brief Draw the value labels at the tick marks.
     * These will be displayed if the meter is a labelStrip.
     * @param[in,out] g        The juce graphics context to use.
     * @param textColour       Colour of the label text.
     * 
     * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakValue, drawPeakHold, drawTickMarks
    */
    void drawLabels (juce::Graphics& g, const juce::Colour& textColour) const;

    /**
     * @brief Tick-mark class.
     * Data class for drawing tick-marks.
     * These are the divider lines on the meter at certain
     * dB levels.
    */
    struct Tick
    {
        /**
        * @brief Constructor.
        * @param value_db Value of the tick-mark (in dB).
       */
        explicit Tick (float value_db) : decibels (value_db), gain (juce::Decibels::decibelsToGain (value_db)) { }
        float gain     = 0;  ///< Value of the tick mark (in amp).
        float decibels = 0;  ///< Value of the tick mark (in dB).
    };

    /**
     * @brief Set the level of the tick marks. 
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     *
     * @param ticks list of tick mark values (in amp).
     * @see setTickMarksVisible, enableTickMarks
    */
    void setTickMarks (const std::vector<float>& ticks);

    /**
     * @brief Show tick-marks (divider lines) on the meter.
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     *
     * @param visible When set true, shows the tick-marks. 
     * @see setTickMarks, enableTickMarks
    */
    void showTickMarks (bool visible) noexcept { m_tickMarksVisible = visible; }

    /**
     * @brief Enable tick-marks (divider lines) on the meter.
     *
     * A tick mark is a horizontal line, dividing the meter. 
     * This is also the place the label strip will put it's text values.
     * 
     * The tick-marks will be shown when they are enable and visible.
     *
     * @param enabled When set true, the tick-marks are enabled. 
     * @see setTickMarks, showTickMarks
    */
    void enableTickMarks (bool enabled) noexcept { m_options.tickMarksEnabled = enabled; }

    /**
     * @brief Use gradients in stead of hard segment boundaries.
     * @param gradientsUsed When set to true, smooth gradients will be used. False gives hard segment boundaries.
    */
    void useGradients (bool gradientsUsed) noexcept;

private:
    Options              m_options;
    std::vector<Segment> m_segments {};  // List of meter segments.

    // Meter levels...
    std::atomic<float> m_inputLevel { 0.0f };  // Audio peak level.
    std::atomic<bool>  m_inputLevelRead { false };
    float              m_meterLevel_db { Constants::kMinLevel_db };  // Current meter level.

    juce::Rectangle<int> m_valueBounds;  // Bounds of the value area.
    juce::Rectangle<int> m_meterBounds;  // Bounds of the meter area.

    std::vector<Tick> m_tickMarks {};  // List of user definable tick marks (in db).

    bool        m_tickMarksVisible    = true;
    bool        m_peakHoldDirty       = false;
    bool        m_valueVisible        = false;
    bool        m_mouseOverValue      = false;
    float       m_decayCoeff          = 0.0f;
    float       m_refreshPeriod_ms    = (1.0f / m_options.refreshRate) * 1000.0f;  // NOLINT
    int         m_previousRefreshTime = 0;

    //==============================================================================
    [[nodiscard]] float getDecayedLevel (float newLevel_db);
    void                calculateDecayCoeff();

    // clang-format on
    JUCE_LEAK_DETECTOR (Level)
};

}  // namespace SoundMeter
}  // namespace sd
