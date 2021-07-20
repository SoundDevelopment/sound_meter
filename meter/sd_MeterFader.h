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

#ifndef SD_SOUND_METER_FADER_H
#define SD_SOUND_METER_FADER_H

namespace sd::SoundMeter
{

class MeterChannel;

/**
 * @brief Class responsible for the fader.
 * 
 * The fader overlay displayed on top of the 'meter' part
 * (in combination with the 'mute' buttons in the 'header' part)
 * can be used by the user to control gain or any other
 * parameter.
*/
class Fader
{
public:
   /**
    * @brief Parameterized constructor
    * @param parentMeter The parent meter object
   */
   explicit Fader (MeterChannel* parentMeter) : m_parentMeter (parentMeter) {};


   /**
    * @brief Show the fader briefly and fade out (unless overridden and shown longer).
    */
   void flash() noexcept;

   /**
    * @brief Check if the meter is active (un-muted).
    * 
    * @return True, if the meter is active (un-muted).
    * @see setActive, setEnabled, isEnabled
   */
   [[nodiscard]] bool isActive() const noexcept;

   /**
    * @brief Activate (or de-activate) fader.
    * 
    * @param setActive When set to true, will activate the meter's fader.
    * @see isActive, setEnabled, isEnabled
   */
   void setActive (bool setActive = true) noexcept;

   /**
    * @brief Check if the 'fader' overlay is enabled.
    * 
    * @return True, when the fader is enabled.
    * @see setEnabled, isActive, setActive
   */
   [[nodiscard]] bool isEnabled() const noexcept;

   /**
    * @brief Enable the 'fader' overlay.
    * 
    * @param enabled True, when the fader needs to be enabled.
    * @see isEnabled, isActive, setActive
   */
   void setEnabled (bool enabled = true) noexcept;

   /**
    * @brief Set the fader bounds.
    * 
    * @param bounds Bounds to use for the fader.
    * @see getBounds
   */
   void setBounds (const juce::Rectangle<int>& bounds) noexcept;

   /**
    * @brief Get the fader bounds.
    * 
    * @return Bounds used by the fader.
    * @see setBounds
   */
   [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept;

   /**
    * @brief Get the value of the meter fader.
    *
    * @return The current fader value [0..1].
    * @see setValueFromPos, setValue
    */
   [[nodiscard]] float getValue() const noexcept;

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
    * Check whether the fader is currently fading out.
    *
    * @return True, if the fader is currently fading out.
    */
   [[nodiscard]] bool isFading() const noexcept { return m_isFading; }

   /**
    * Draw the fader.
    *
    * @param[in,out] g   The juce graphics context to use.
    * @param faderColour Fader colour to use.
    */
   void draw (juce::Graphics& g, const juce::Colour& faderColour);

   /**
    * @brief Fader listener class.
    *
    * Listeners get notified when the fader has changed.
    */
   struct Listener
   {
      virtual ~Listener() = default;

      /**
       * @brief Fader value has changed.
       * 
       * @param sourceMeter The meter whose fader has been changed.
       * @param value       The new value of the fader.
      */
      virtual void faderChanged (MeterChannel* sourceMeter, float value) = 0;
   };

   /**
    * @brief Add a listener to any changes in the fader.
    * 
    * @param listener The listener to add to the list.
    * 
    * @see removeListener
   */
   void addListener (Listener& listener);

   /**
    * @brief Remove a listener to any changes in the fader.
    * 
    * @param listener The listener to remove from the list.
    * 
    * @see addListener
   */
   void removeListener (Listener& listener);

   /**
    * @brief Notify any listener that the fader has changed.
   */
   void notify();


private:
   std::atomic<float>           m_faderValue { 1.0f };  // Fader value (between 0..1).
   juce::ListenerList<Listener> m_faderListeners;
   SoundMeter::MeterChannel*    m_parentMeter = nullptr;
   bool                         m_active      = false;
   bool                         m_enabled     = false;
   bool                         m_mouseOver   = false;
   bool                         m_isFading    = false;
   int                          m_fadeStart   = 0;
   juce::Rectangle<int>         m_bounds {};

   [[nodiscard]] int getTimeSinceStartFade() const noexcept;

   JUCE_LEAK_DETECTOR (Fader)
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_FADER_H */