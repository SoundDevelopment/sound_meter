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

class MeterComponent;

/**
 * @brief Class responsible for the fader.
*/
class Fader
{
public:
   explicit Fader( MeterComponent* parentMeter ) : m_parentMeter( parentMeter ) {};

   /**
    * Draw the fader.
    *
    * @param g juce graphics context to draw into.
    * @param faderColour colour to draw the fader in.
    */
   void draw( juce::Graphics& g, const juce::Colour& faderColour );

   void                               flash();
   [[nodiscard]] bool                 isActive() const noexcept;
   void                               setActive( bool setActive = true ) noexcept;
   void                               setBounds( const juce::Rectangle<int>& bounds ) noexcept;
   [[nodiscard]] juce::Rectangle<int> getBounds() const noexcept;
   [[nodiscard]] bool                 isEnabled() const noexcept;
   void                               setEnabled( bool enabled = true ) noexcept;
   [[nodiscard]] float                getValue() const noexcept;

   /**
    * Set fader value.
    *
    * @param value               The value [0..1] the fader needs to be set to.
    * @param notificationOption  Select whether to notify the listeners.
    * @return                    True, if the value actually changed.
    */
   bool               setValue( float value, NotificationOptions notificationOption = NotificationOptions::notify );
   void               setValueFromPos( int pos, NotificationOptions notificationOption = NotificationOptions::notify );
   [[nodiscard]] int  getTimeSinceStartFade() const noexcept;
   [[nodiscard]] bool isFading() const noexcept { return m_isFading; }
   void               notify();

   /**
    * @brief Fader listener class.
    *
    * Listeners get notified when the fader has changed.
    */
   struct Listener
   {
      virtual ~Listener()                                                      = default;
      virtual void faderChanged( MeterComponent* sourceMeter, float value ) = 0;
   };

   void addListener( Listener& listener );
   void removeListener( Listener& listener );

private:
   std::atomic<float>             m_faderValue { 1.0f };  // Fader value (between 0..1).
   juce::ListenerList<Listener>   m_faderListeners;
   SoundMeter::MeterComponent* m_parentMeter = nullptr;
   bool                           m_active      = false;
   bool                           m_enabled     = false;
   bool                           m_mouseOver   = false;
   bool                           m_isFading    = false;
   int                            m_fadeStart   = 0;
   juce::Rectangle<int>           m_bounds {};

   JUCE_LEAK_DETECTOR( Fader )
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_FADER_H */