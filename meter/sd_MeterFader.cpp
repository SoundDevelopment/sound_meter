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

namespace sd::SoundMeter
{

void Fader::flash()
{
   if( !m_enabled ) return;

   m_fadeStart = static_cast<int>( juce::Time::getMillisecondCounter() );
}


[[nodiscard]] bool Fader::isActive() const noexcept
{
   return m_active && m_enabled;
}


void Fader::setActive( bool setActive /*= true*/ ) noexcept
{
   if( !m_enabled ) return;

   // If fader needs to be DE-ACTIVATED...
   if( !setActive )
   {
      // If it was activated, FADE it out...
      if( m_active ) m_fadeStart = static_cast<int>( juce::Time::getMillisecondCounter() );

      // DE-ACTIVATE fader...
      m_active = false;
   }
   // ... if fader needs to be ACTIVATED...
   else
   {
      // ACTIVATE fader...
      m_active    = true;
      m_fadeStart = 0;
   }
}


void Fader::setBounds( const juce::Rectangle<int>& bounds ) noexcept
{
   m_bounds = bounds;
}


[[nodiscard]] juce::Rectangle<int> Fader::getBounds() const noexcept
{
   return m_bounds;
}

//==============================================================================
[[nodiscard]] void Fader::draw( juce::Graphics& g, const juce::Colour& faderColour )
{
   using namespace Constants;

   m_isFading = false;

   if( !m_enabled ) return;

   const auto timeSinceStartFade = getTimeSinceStartFade();
   if( !m_active && timeSinceStartFade >= kFaderFadeTime_ms ) return;

   auto alpha = Constants::kFaderAlphaMax;
   if( timeSinceStartFade < kFaderFadeTime_ms )
   {
      const float fadePortion = 2.0f;
      jassert( kFaderFadeTime_ms > 0.0f );
      alpha      = std::clamp( fadePortion - ( ( timeSinceStartFade * fadePortion ) / kFaderFadeTime_ms ), 0.0f, 1.0f ) * kFaderAlphaMax;
      m_isFading = alpha > 0.0f;
   }

   g.setColour( faderColour.withAlpha( alpha ) );
   auto faderRect = m_bounds;
   g.fillRect( faderRect.removeFromBottom( m_bounds.proportionOfHeight( getValue() ) ) );
}

//==============================================================================
[[nodiscard]] bool Fader::isEnabled() const noexcept
{
   return m_enabled;
}


void Fader::setEnabled( bool enabled /*= true*/ ) noexcept
{
   m_enabled = enabled;
}

[[nodiscard]] float Fader::getValue() const noexcept
{
   return m_faderValue.load();
}


bool Fader::setValue( float value, NotificationOptions notificationOption /*= NotificationOptions::Notify*/ )
{
   if( !m_enabled ) return false;
   if( m_faderValue.load() == value ) return false;
   m_faderValue.store( value );
   if( notificationOption == NotificationOptions::notify ) notify();
   return true;
}


void Fader::setValueFromPos( const int pos, NotificationOptions notificationOption /*= NotificationOptions::Notify*/ )
{
   const auto height = static_cast<float>( m_bounds.getHeight() );
   if( height <= 0.0f ) return;

   setValue( 1.0f - std::clamp( static_cast<float>( pos - m_bounds.getY() ) / height, 0.0f, 1.0f ), notificationOption );
}


[[nodiscard]] int Fader::getTimeSinceStartFade() const noexcept
{
   return static_cast<int>( juce::Time::getMillisecondCounter() ) - m_fadeStart;
}

void Fader::notify()
{
   if( !m_parentMeter || !m_enabled ) return;
   m_faderListeners.call( [=]( Listener& l ) { l.faderChanged( m_parentMeter, getValue() ); } );  // Fader changed CALLBACK.
}


void Fader::addListener( Listener& listener )
{
   m_faderListeners.add( &listener );
}


void Fader::removeListener( Listener& listener )
{
   m_faderListeners.remove( &listener );
}

}  // namespace sd::SoundMeter