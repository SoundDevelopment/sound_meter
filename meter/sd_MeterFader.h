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

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace sd  // NOLINT
{
namespace SoundMeter
{

/**
 * @brief Class responsible for the fader.
 * 
 * The fader overlay displayed on top of the 'meter' part
 * (in combination with the 'mute' buttons in the 'header' part)
 * can be used by the user to control gain or any other
 * parameter.
*/
class Fader final
{
public:
  
    Fader() = default;

    /**
     * @brief Show the fader briefly and fade out (unless overridden and shown longer).
    */
    void flash() noexcept;

    /**
    * @brief Check if the fader is visible.
     * 
     * @return True, if the meter is visible, otherwise the fader is hidden.
     * @see setVisible, setEnabled, isEnabled
    */
    [[nodiscard]] bool isVisible() const noexcept { return m_visible && m_enabled; }

    /**
     * @brief Show or hide the fader.
     * 
     * @param setVisible When set to true, show the fader. Otherwise hide it.
     * @see isVisible, setEnabled, isEnabled
    */
    void setVisible (bool showFader = true) noexcept;

    /**
     * @brief Check if the 'fader' overlay is enabled.
     * 
     * @return True, when the fader is enabled.
     * @see setEnabled, isVisible, setVisible
    */
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }

    /**
     * @brief Enable or disable the 'fader' overlay.
     * 
     * @param enabled True, when the fader needs to be enabled.
     * @see isEnabled, isActive, setActive
    */
    void enable (bool enabled = true) noexcept { m_enabled = enabled; }

    /**
     * @brief Set the fader bounds.
     * 
     * @param bounds The bounds to use for the fader.
     * @see getBounds
    */
    void setBounds (const juce::Rectangle<int>& bounds) noexcept { m_bounds = bounds; }

    /**
     * @brief Get the fader bounds.
     * 
     * @return Bounds used by the fader.
     * @see setBounds
    */
    [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept { return m_bounds; }

    /**
     * @brief Get the value of the meter fader.
     *
     * @return The current fader value [0..1].
     * @see setValueFromPos, setValue
    */
    [[nodiscard]] float getValue() const noexcept { return m_faderValue.load(); }

    /**
     * @brief Set fader value.
     *
     * @param value               The value [0..1] the fader needs to be set to.
     * @param notificationOption  Select whether to notify the listeners.
     * @return                    True, if the value actually changed.
     * 
     * @see setValueFromPos, getValue
    */
    bool setValue (float value, NotificationOptions notificationOption = NotificationOptions::notify);

    /**
     * @brief Set fader value according to a supplied mouse position.
     * 
     * @param position           The mouse position (y coordinate) to use to calculate the fader value.
     * @param notificationOption Select whether to notify the listeners.
     * 
     * @see setValue
    */
    void setValueFromPos (int position, NotificationOptions notificationOption = NotificationOptions::notify);

    /**
     * @brief Check whether the fader is currently fading out.
     *
     * @return True, if the fader is currently fading out.
    */
    [[nodiscard]] bool isFading() const noexcept { return m_isFading; }

    /**
     * @brief Draw the fader.
     *
     * @param[in,out] g   The juce graphics context to use.
     * @param faderColour Fader colour to use.
    */
    void draw (juce::Graphics& g, const juce::Colour& faderColour);

    /**
     * @brief Check if the fader needs redrawing.
    */
    [[nodiscard]] bool needsRedrawing() noexcept { return (m_drawnFaderValue != m_faderValue.load()) || isFading(); }

    std::function<void ()> onFaderValueChanged { nullptr };

private:
    std::atomic<float>   m_faderValue { 1.0f };  // Fader value (between 0..1).
    juce::Rectangle<int> m_bounds {};

    float    m_drawnFaderValue = 1.0f;
    bool     m_visible         = false;
    bool     m_enabled         = false;
    bool     m_isFading        = false;
    int      m_fadeStart       = 0;

    [[nodiscard]] int getTimeSinceStartFade() const noexcept { return static_cast<int> (juce::Time::getMillisecondCounter() - m_fadeStart); }

    JUCE_LEAK_DETECTOR (Fader)
};

}  // namespace SoundMeter

}  // namespace sd