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


void Header::draw( juce::Graphics& g, bool meterActive, bool faderEnabled, const juce::Colour& mutedColour, const juce::Colour& mutedMouseOver,
                   const juce::Colour& textColour, const juce::Colour& inactiveColour )
{
   if( !m_visible ) return;
   if( m_bounds.isEmpty() ) return;

   // Draw channel names...
   juce::String headerText = getHeader();

   if( m_mouseOver && faderEnabled )  // Draw 'button' for enabling/disabling channel.
   {
      g.setColour( meterActive ? mutedColour : mutedMouseOver );
      g.fillRect( m_bounds );
      g.setColour( mutedColour.contrasting( 0.8f ) );  // NOLINT
   }
   else
   {
      if( meterActive )
      {
         g.setColour( textColour );
      }
      else
      {
         g.setColour( inactiveColour );
         g.fillRect( m_bounds );
         g.setColour( inactiveColour.contrasting( 0.8f ) );  // NOLINT
      }
   }
   g.drawFittedText( headerText, m_bounds, juce::Justification::centred, 1 );
}

void Header::setType( const juce::AudioChannelSet::ChannelType& type )
{
   m_type                 = type;
   m_typeDescription      = juce::AudioChannelSet::getChannelTypeName( type );
   m_typeAbbrDecscription = juce::AudioChannelSet::getAbbreviatedChannelTypeName( type );
   if( m_name.isEmpty() ) setName( m_typeDescription );
}


[[nodiscard]] const juce::AudioChannelSet::ChannelType& Header::getType() const noexcept
{
   return m_type;
}


void Header::setName( const juce::String name )
{
   m_name = name;
   calculateNameWidth();
}


void Header::calculateNameWidth()
{
   m_nameWidth = m_font.getStringWidthFloat( m_name );
}


[[nodiscard]] float Header::getNameWidth() const noexcept
{
   return m_nameWidth;
}


[[nodiscard]] juce::String Header::getName() const noexcept
{
   return m_name;
}


[[nodiscard]] juce::String Header::getHeader() const noexcept
{
   juce::String result = m_typeAbbrDecscription;  // Worst case: just use abbreviated name.
   if( m_nameWidth <= m_bounds.getWidth() && m_name.isNotEmpty() ) result = m_name;
   if( result.isEmpty() ) result = "-";
   return result;
}


void Header::setBounds( const juce::Rectangle<int>& bounds ) noexcept
{
   m_bounds = bounds;
}


[[nodiscard]] juce::Rectangle<int> Header::getBounds() const noexcept
{
   return m_bounds;
}


[[nodiscard]] const juce::Font& Header::getFont() const noexcept
{
   return m_font;
}


void Header::setFont( const juce::Font& font ) noexcept
{
   m_font = font;
   calculateNameWidth();
}


bool Header::nameFits( const juce::String& name, const int widthAvailable ) const
{
   return m_font.getStringWidthFloat( name ) <= static_cast<float>( widthAvailable );
}

}  // namespace sd::SoundMeter