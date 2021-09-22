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


#ifndef SD_SOUND_METER_HELPERS_H
#define SD_SOUND_METER_HELPERS_H


namespace sd
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
static constexpr auto kDefaultHeaderHeight     = 30;       ///< Default height of the 'header' part (in pixels).
static constexpr auto kDefaultHeaderLabelWidth = 30;       ///< Default 'header' label width (in pixels).
static constexpr auto kDefaultHeaderFontHeight = 12.0f;    ///< Default height of the font used in the 'header' part (in pixels).
static constexpr auto kLabelStripTextPadding   = 2;        ///< Padding around the text in a label strip (in pixels).
static constexpr auto kLabelStripLeftPadding   = 5;        ///< Padding (in pixels) on the left side of the label strip (which can double as a master fader).
static constexpr auto kFaderRightPadding       = 1;        ///< Padding (in pixels) on the right side of the channel faders.
static constexpr auto kMaxLevel_db             = 0.0f;     ///< Maximum meter level (in db).
static constexpr auto kMinDecay_ms             = 100.0f;   ///< Minimum meter decay speed (in milliseconds).
static constexpr auto kMaxDecay_ms             = 4000.0f;  ///< Maximum meter decay speed (in milliseconds).
static constexpr auto kDefaultDecay_ms         = 1000.0f;  ///< Default meter decay speed (in milliseconds).
static constexpr auto kTickMarkHeight          = 2;        ///< Height of a tick mark (in pixels).
static constexpr auto kFaderFadeTime_ms        = 2500;     ///< Fader fade out time (in milliseconds).
static constexpr auto kFaderSensitivity        = 10.0f;    ///< Fader sensitivity value. Must be a positive value > 0.
static constexpr auto kFaderAlphaMax           = 0.8f;     ///< Maximum transparency (alpha) of the fader overlay.
static constexpr auto kWarningLevel_db         = -9.0f;    ///< Dividing level between 'normal' and 'warning' segments (in decibels).
static constexpr auto kPeakLevel_db            = -3.0f;    ///< Dividing level between 'warning' and 'peak' segments (in decibels).
static constexpr auto kMinModeHeightThreshold  = 150;  ///< Meter minimum mode height threshold in pixels (min. mod is just the meter. not value, ticks or fader).
static constexpr auto kMinModeWidthThreshold   = 15;   ///< Meter minimum mode width threshold in pixels (min. mod is just the meter. not value, ticks or fader).
static constexpr auto kMetersPanelId           = "meters_panel";  ///< ID (name) of all components in the meters panel.
static constexpr auto kLabelStripId            = "label_strip";   ///< ID (name) of the label-strip (master fader).
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
    Padding (int newLeft, int newRight, int newTop, int newBottom) : left (newLeft), right (newRight), top (newTop), bottom (newBottom) { }
    int left   = 0;  ///< Space between meter and left border (in pixels).
    int right  = 0;  ///< Space between meter and right border (in pixels).
    int top    = 0;  ///< Space between meter and top border (in pixels).
    int bottom = 0;  ///< Space between meter and bottom border (in pixels).
};

/**
 * @brief All meter options for appearance and functionality.
*/
struct Options
{
    bool  enabled               = true;  ///< Enable the meter.
    bool  headerEnabled         = true;  ///< Enable the 'header' part of the meter.
    bool  valueEnabled          = true;  ///< Enable the 'value' part of the meter.
    bool  faderEnabled          = true;  ///< Enable the fader (overlayed over the meter). Only works if fader have been enabled in the module.
    bool  useGradient           = true;  ///< Use gradients to fill the meter or hard segment boundaries.
    bool  showPeakHoldIndicator = true;  ///< Show the peak hold indicator (a line that marks the highest level up to now).
    bool  useMinimalMode    = true;  ///< Automatically adapt the meter to use the most of the space available (by hiding header, value, tick-marks, etc...).
    float warningSegment_db = Constants::kWarningLevel_db;  ///< Boundary level from normal to warning segment.
    float peakSegment_db    = Constants::kPeakLevel_db;     ///< Boundary level from warning to peak segment.
    float decayTime_ms      = Constants::kDefaultDecay_ms;  ///< Meter decay in milliseconds.
    float refreshRate       = 24.0f;                        ///< Meter refresh rate when using internal timing.
    bool  tickMarksEnabled  = true;                         ///< Show tick-marks. Divider lines on the meter at certain db levels.
    bool  tickMarksOnTop    = false;  ///< Show the tick-marks below the level or above the level (level might obscure the tick-marks if loud enough).
    std::vector<float> tickMarks = { -1.0f, -3.0f, -6.0f, -9.0f, -18.0f };  ///< Tick-mark position in db.
};

/**
 * @brief Type indicating whether to notify the listeners or not.
*/
enum class NotificationOptions
{
    notify,     ///< Notify any listeners.
    dontNotify  ///< Do not notify any listeners.
};

/**
 * @brief Position of the label strip.
*/
enum class LabelStripPosition
{
    left,   ///< Left of the meters.
    right,  ///< Right of the meters.
    none    ///< No label strip will be shown.
};

/**
 * @brief Various meter helper functions.
*/
namespace Helpers
{
/**
 * Apply padding to a rectangle.
 *
 * @param rectToPad Rectangle to apply the padding to.
 * @param paddingToApply The padding to apply (left, right, top, bottom).
 * @return The padded rectangle.
*/
[[nodiscard]] juce::Rectangle<int> applyPadding (const juce::Rectangle<int>& rectToPad, Padding paddingToApply);

}  // namespace Helpers

}  // namespace SoundMeter

}  // namespace sd


#endif /* SD_SOUND_METER_HELPERS_H */
