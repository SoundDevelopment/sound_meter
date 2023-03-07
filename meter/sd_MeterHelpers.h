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

namespace sd  // NOLINT
{
namespace SoundMeter
{
/**
 * @brief Various meter helper constants.
*/
namespace Constants
{
static constexpr auto kMinWidth                = 2;        ///< Minimum meter width (in pixels).
static constexpr auto kMaxWidth                = 500;      ///< Maximum meter width (in pixels).
static constexpr auto kPeakHoldHeight          = 2;        ///< Height of the peak hold strip (in pixels).
static constexpr auto kDefaultHeaderHeight     = 25;       ///< Default height of the 'header' part (in pixels).
static constexpr auto kDefaultHeaderLabelWidth = 30;       ///< Default 'header' label width (in pixels).
static constexpr auto kDefaultHeaderFontHeight = 14.0f;    ///< Default height of the font used in the 'header' part (in pixels).
static constexpr auto kLabelStripTextPadding   = 2;        ///< Padding around the text in a label strip (in pixels).
static constexpr auto kLabelStripLeftPadding   = 5;        ///< Padding (in pixels) on the left side of the label strip (which can double as a master fader).
static constexpr auto kFaderRightPadding       = 1;        ///< Padding (in pixels) on the right side of the channel faders.
static constexpr auto kMaxLevel_db             = 0.0f;     ///< Maximum meter level (in db).
static constexpr auto kMinLevel_db             = -96.0f;   ///< Minimum meter level (in db).
static constexpr auto kMinDecay_ms             = 100.0f;   ///< Minimum meter decay speed (in milliseconds).
static constexpr auto kMaxDecay_ms             = 4000.0f;  ///< Maximum meter decay speed (in milliseconds).
static constexpr auto kDefaultDecay_ms         = 1000.0f;  ///< Default meter decay speed (in milliseconds).
static constexpr auto kTickMarkHeight          = 2;        ///< Height of a tick mark (in pixels).
static constexpr auto kFaderFadeTime_ms        = 2500;     ///< Fader fade out time (in milliseconds).
static constexpr auto kFaderSensitivity        = 10.0f;    ///< Fader sensitivity value. Must be a positive value > 0.
static constexpr auto kFaderAlphaMax           = 0.3f;     ///< Maximum transparency (alpha) of the fader overlay.
static constexpr auto kMinModeHeightThreshold = 150;  ///< Meter minimum mode height threshold in pixels (min. mod is just the meter. not value, ticks or fader).
static constexpr auto kMinModeWidthThreshold = 15;    ///< Meter minimum mode width threshold in pixels (min. mod is just the meter. not value, ticks or fader).
static constexpr auto kMetersPanelId         = "meters_panel";  ///< ID (name) of all components in the meters panel.
static constexpr auto kLabelStripId          = "label_strip";   ///< ID (name) of the label-strip (master fader).
}  // namespace Constants

/**
 * @brief Amount of padding to use on the meters.
 * Padding is the space between the meter and the component's edge.
*/
struct Padding
{
    /**
    * @brief Constructor.
    * @param newLeft   New left padding amount (in pixels).
    * @param newRight  New right padding amount (in pixels).
    * @param newTop    New top padding amount (in pixels).
    * @param newBottom New bottom padding amount (in pixels).
   */
    Padding (int newLeft, int newRight, int newTop, int newBottom) noexcept : left (newLeft), right (newRight), top (newTop), bottom (newBottom) { }
    int left   = 0;  ///< Space between meter and left border (in pixels).
    int right  = 0;  ///< Space between meter and right border (in pixels).
    int top    = 0;  ///< Space between meter and top border (in pixels).
    int bottom = 0;  ///< Space between meter and bottom border (in pixels).
};

/**
 * @brief Options defining the meter segments.
 */
struct SegmentOptions
{
    juce::Range<float> levelRange { Constants::kMinLevel_db, Constants::kMaxLevel_db };  ///<  The range of the segment in decibels.
    juce::Range<float> meterRange { 0.0f, 1.0f };  ///< The range of the segment in the meter (0.0f - 1.0f, with 0.0f being the bottom of the meter).
    juce::Colour       segmentColour { juce::Colours::yellow };         ///< The colour of the segment.
    juce::Colour       nextSegmentColour { segmentColour.brighter() };  ///< The second colour of the segment (for use in gradients).
};

/**
 * @brief All meter options for appearance and functionality.
*/
struct Options
{
    bool  enabled          = true;  ///< Enable the meter.
    bool  headerEnabled    = true;  ///< Enable the 'header' part of the meter.
    bool  valueEnabled     = true;  ///< Enable the 'value' part of the meter.
    bool  faderEnabled     = true;  ///< Enable the fader (overlay-ed over the meter). Only works if fader have been enabled in the module.
    bool  useMinimalMode   = true;  ///< Automatically adapt the meter to use the most of the space available (by hiding header, value, tick-marks, etc...).
    float decayTime_ms     = Constants::kDefaultDecay_ms;  ///< Meter decay in milliseconds.
    float refreshRate      = 30.0f;                        ///< Meter refresh rate when using internal timing.
    bool  tickMarksEnabled = true;                         ///< Show tick-marks. Divider lines on the meter at certain db levels.
    bool  tickMarksOnTop   = false;       ///< Show the tick-marks below the level or above the level (level might obscure the tick-marks if loud enough).
    bool  useGradient      = true;        ///< Use gradients for the meter segments, in stead of solid colours.
    bool  showPeakHoldIndicator  = true;  ///< Enable peak hold indicator.
    std::vector<float> tickMarks = { 0.0f, -3.0f, -6.0f, -9.0f, -12.0f, -18.0f, -30.0f, -40.0f, -50.0f };  ///< Tick-mark position in db.
};

/**
 * @brief All meter colours for the appearance of the meters.
*/
struct MeterColours
{
    juce::Colour backgroundColour    = juce::Colours::black;                                       ///< Background colour of the meter.
    juce::Colour inactiveColour      = juce::Colours::grey;                                        ///< Colour of the meter when inactive.
    juce::Colour textValueColour     = juce::Colours::white.darker (0.6f);                         ///< Colour of the peak value text.
    juce::Colour muteColour          = juce::Colours::red;                                         ///< Colour of the mute button.
    juce::Colour muteMouseOverColour = juce::Colours::black;                                       ///< Colour of the mute button when the mouse is over it.
    juce::Colour faderColour         = juce::Colours::blue.withAlpha (Constants::kFaderAlphaMax);  ///< Colour of the fader overlay.
    juce::Colour textColour          = juce::Colours::white.darker (0.6f);                         ///< Colour of the text (in the header and label strip).
    juce::Colour tickMarkColour      = juce::Colours::white.darker (0.3f).withAlpha (0.5f);        ///< Colour of the tick-marks.
    juce::Colour peakHoldColour      = juce::Colours::red;                                         ///< Colour of the peak hold indicator.
    juce::Colour solodColour         = juce::Colours::yellow;                                      ///< Colour of the solo button.
};

/**
 * @brief A class with static functions to create different types of meter scales.
*/
class MeterScales
{
public:
    /**
    * @brief Default meter scale. 3 segments, from -60db to 0db. 
    */
    [[nodiscard]] static std::vector<SegmentOptions> getDefaultScale()
    {
        return { { { -60.0f, -18.0f }, { 0.0f, 0.5f }, juce::Colours::green, juce::Colours::green },
                 { { -18.0f, -3.0f }, { 0.5f, 0.90f }, juce::Colours::green, juce::Colours::yellow },
                 { { -3.0f, 0.0f }, { 0.90f, 1.0f }, juce::Colours::yellow, juce::Colours::red } };
    }

    /**
     * @brief SMPTE meter scale. 3 segments, from -44db to 0db.
     */
    [[nodiscard]] static std::vector<SegmentOptions> getSmpteScale()
    {
        return { { { -44.0f, -12.0f }, { 0.0f, 0.7273f }, juce::Colours::green, juce::Colours::yellow },
                 { { -12.0f, -3.0f }, { 0.7273f, 0.9318f }, juce::Colours::yellow, juce::Colours::red },
                 { { -3.0f, 0.0f }, { 0.9318f, 1.0f }, juce::Colours::red, juce::Colours::red } };
    }

    /**
     * @brief Yamaha mixer meter scale. 3 segments, from -60db to 0db.
     */
    [[nodiscard]] static std::vector<SegmentOptions> getYamaha60()
    {
        return { { { -60.0f, -30.0f }, { 0.0f, 0.2751f }, juce::Colours::yellow, juce::Colours::yellow },
                 { { -30.0f, -18.0f }, { 0.2751f, 0.4521f }, juce::Colours::yellow, juce::Colours::yellow },
                 { { -18.0f, 0.0f }, { 0.4521f, 1.0f }, juce::Colours::red, juce::Colours::red } };
    }

private:
    MeterScales() = default;
};

/** @brief Type indicating whether to notify the listeners or not. */
enum class NotificationOptions
{
    notify,     ///< Notify any listeners.
    dontNotify  ///< Do not notify any listeners.
};

/** @brief Position of the label strip. */
enum class LabelStripPosition
{
    left,   ///< Left of the meters.
    right,  ///< Right of the meters.
    none    ///< No label strip will be shown.
};

}  // namespace SoundMeter
}  // namespace sd