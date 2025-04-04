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
    Level();

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
    void refreshMeterLevel();

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
    void setMeterOptions (const Options& meterOptions);

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
    [[nodiscard]] bool isPeakValueVisible() const noexcept { return !m_valueBounds.isEmpty(); }

    /**
     * @brief Reset the peak hold level.
     * @see getPeakHoldLevel, isPeakValueVisible, setPeakValueVisible, showPeakHold, showValue, isPeakHoldEnabled
    */
    void resetPeakHold();

    /**
     * @brief Get the current peak hold level.
     * @return The current peak hold level (in decibels).
     * @see resetPeakHold, isPeakValueVisible, setPeakValueVisible, setPeakHoldVisible, isPeakHoldEnabled
    */
    [[nodiscard]] float getPeakHoldLevel() const noexcept;

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
     * @brief Sets the meter's refresh rate.
     *
     * Set this to optimize the meter's decay rate.
     *
     * @param refreshRate_hz Refresh rate in Hz.
     * @see refresh, setDecay, getDecay
    */
    void setRefreshRate (float refreshRate_hz);

    /**
     * @brief Get the meter's refresh (redraw) rate.
     *
     * @return The refresh rate of the meter in Hz.
     *
     * @see setRefreshRate
    */
    [[nodiscard]] float getRefreshRate() const noexcept { return m_meterOptions.refreshRate; }

    /**
     * @brief Set meter decay.
     *
     * @param decay_ms Meter decay in milliseconds.
     * @see getDecay, setRefreshRate
    */
    void setDecay (float decay_ms);

    /**
     * @brief Get meter decay.
     *
     * @return Meter decay in milliseconds
     *
     * @see setDecay
    */
    [[nodiscard]] float getDecay() const noexcept { return m_meterOptions.decayTime_ms; }

    /**
     * @brief Set the segments the meter is made out of.
     *
     * All segments have a level range, a range within the meter and a colour (or gradient).
     *
     * @param segmentsOptions The segments options to create the segments with.
    */
    void setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions);

    /**
     * @brief Set whether this meter is a label strip.
     *
     * A label strip only draws the value labels (at the tick-marks),
     * but does not display any level.
     *
     * @param isLabelStrip when set, this meter behave like a label strip.
    */
    void setIsLabelStrip (bool isLabelStrip = false) noexcept;

    /**
     * @brief Set the bounds of the 'meter' part of the meter.
     *
     * @param bounds The bounds to use for the 'meter' part of the meter.
     * @see getValueBounds, setValueBounds, getMeterBounds, getDirtyBounds, getLevelBounds
    */
    void setMeterBounds (const juce::Rectangle<int>& bounds);

    /**
     * @brief Get the bounds of the 'value' part of the meter.
     *
     * @return The bounds of the 'value' part of the meter.
     * @see setMeterBounds, setValueBounds, getMeterBounds, getDirtyBounds, getLevelBounds
    */
    [[nodiscard]] juce::Rectangle<int> getValueBounds() const noexcept { return m_valueBounds; }

    /**
     * @brief Get the bounds of the 'meter' part.
     *
     * @return The bounds of the 'meter' part.
     * @see getValueBounds, setValueBounds, setMeterBounds, getDirtyBounds, getLevelBounds
    */
    [[nodiscard]] juce::Rectangle<int> getMeterBounds() const noexcept { return m_meterBounds; }

    /**
     * @brief Get the bounds of the 'level' part.
     *
     * @return The bounds of the 'level' part.
     * @see getValueBounds, setValueBounds, setMeterBounds, getDirtyBounds, getMeterBounds
    */
    [[nodiscard]] juce::Rectangle<int> getLevelBounds() const noexcept { return m_levelBounds; }

    /** @brief Get the dirty part of the meter.*/
    [[nodiscard]] juce::Rectangle<int> getDirtyBounds();

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

    /**
     * @brief Draws the meter.
     *
     * @param[in,out] g            The juce graphics context to use.     
     * @param         meterColours The colours to use to draw the meter.
     *
     * @see drawInactiveMeter, drawPeakValue, drawPeakHold, drawTickMarks, drawLabels
    */
    void drawMeter (juce::Graphics& g, const MeterColours& meterColours);

    /**
     * @brief Draw the 'meter' part in it's inactive (muted) state.
     *
     * @param[in,out] g            The juce graphics context to use.
     * @param         meterColours The colours to use to draw the meter.
     *
     * @see drawMeter, drawTickMarks, drawPeakValue, drawPeakHold, drawLabels
    */
    void drawInactiveMeter (juce::Graphics& g, const MeterColours& meterColours) const;

    /**
     * @brief Draw the peak 'value'.
     * @param[in,out] g            The juce graphics context to use.              
     * @param         meterColours The colours to use to draw the meter.
     *
     * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakHold, drawTickMarks, drawLabels
    */
    void drawPeakValue (juce::Graphics& g, const MeterColours& meterColours) const;

private:
    Options                     m_meterOptions;
    std::vector<SegmentOptions> m_segmentOptions = MeterScales::getDefaultScale();

    std::vector<Segment> m_segments;  // List of meter segments.
    juce::Range<float>   m_meterRange { Constants::kMaxLevel_db, Constants::kMinLevel_db };
    juce::Rectangle<int> m_valueBounds;  // Bounds of the value area.
    juce::Rectangle<int> m_meterBounds;  // Bounds of the meter area.
    juce::Rectangle<int> m_levelBounds;  // Bounds of the level area.

    // Meter levels...
    std::atomic<float> m_inputLevel { 0.0f };  // Audio peak level.
    std::atomic<bool>  m_inputLevelRead { false };
    float              m_meterLevel_db       = Constants::kMinLevel_db;  // Current meter level.
    float              m_nominalLevel_db     = 0.0f;                     // The level (in dB) where the nominal level should be. e.g. -20.0 for K20.
    bool               m_peakHoldDirty       = false;
    bool               m_mouseOverValue      = false;
    bool               m_minimalModeActive   = false;
    bool               m_isLabelStrip        = false;
    float              m_decayCoeff          = 0.0f;
    float              m_refreshPeriod_ms    = (1.0f / m_meterOptions.refreshRate) * 1000.0f;  // NOLINT
    int                m_previousRefreshTime = 0;
    float              m_decayRate           = 0.0f;  // Decay rate in dB/ms.

    [[nodiscard]] float getDecayedLevel (float newLevel_db);
    [[nodiscard]] float getLinearDecayedLevel (float newLevel_db);
    void                calculateDecayCoeff (const Options& meterOptions);
    void                synchronizeMeterOptions();

    // clang-format on
    JUCE_LEAK_DETECTOR (Level)
};
}  // namespace SoundMeter
}  // namespace sd
