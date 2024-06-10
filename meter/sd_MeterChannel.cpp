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

#include "sd_MeterChannel.h"

namespace sd
{
namespace SoundMeter
{
MeterChannel::MeterChannel() noexcept
{
#if SDTK_ENABLE_FADER
    m_fader.onFaderValueChanged = [this]() { notifyParent(); };
#endif
}
//==============================================================================

MeterChannel::MeterChannel (const Options& meterOptions, Padding padding, const juce::String& channelName, bool isLabelStrip /*= false*/,
                            ChannelType channelType /*= ChannelType::unknown*/)
  : MeterChannel()
{
    setName (channelName);
    setBufferedToImage (true);

    setChannelType (channelType);
    setOptions (meterOptions);
    setIsLabelStrip (isLabelStrip);
    setPadding (padding);
}
//==============================================================================

void MeterChannel::reset()
{
    m_level.reset();
    setDirty();
}
//==============================================================================

#if SDTK_ENABLE_FADER

MeterChannel::~MeterChannel()
{
    onFaderMove  = nullptr;
    onMixerReset = nullptr;
    onFaderMove  = nullptr;
}
//==============================================================================

void MeterChannel::flashFader()
{
    m_fader.flash();
    addDirty (m_fader.getBounds());
}
//==============================================================================

void MeterChannel::notifyParent()
{
    if (onFaderMove)
        onFaderMove (this);
}
//==============================================================================

#endif /* SDTK_ENABLE_FADER */
//==============================================================================

bool MeterChannel::autoSetMinimalMode (int proposedWidth, int proposedHeight)
{
    bool minimalMode = !nameFits ("-99.99", proposedWidth);
    if (proposedWidth < Constants::kMinModeWidthThreshold)
        minimalMode = true;
    if (proposedHeight < Constants::kMinModeHeightThreshold)
        minimalMode = true;

    setMinimalMode (minimalMode);

    return m_minimalMode;
}
//==============================================================================

void MeterChannel::setMinimalMode (bool minimalMode)
{
    if (!m_meterOptions.useMinimalMode)
        return;

    m_minimalMode = minimalMode;
    m_level.setMinimalMode (m_minimalMode);
    showHeader (!m_minimalMode);  // ... show channel ID if it's not too narrow for ID and not in minimum mode.
}
//==============================================================================

void MeterChannel::showHeader (bool headerVisible)
{
    if (m_meterOptions.headerEnabled == headerVisible)
        return;

    m_meterOptions.headerEnabled = headerVisible;

    resized();
    addDirty (m_header.getBounds());
}
//==============================================================================

void MeterChannel::setOptions (const Options& meterOptions)
{
    m_meterOptions = meterOptions;

    setVisible (meterOptions.enabled);
    setEnabled (meterOptions.enabled);

    m_level.setMeterOptions (meterOptions);

    showHeader (meterOptions.headerEnabled);

#if SDTK_ENABLE_FADER
    enableFader (meterOptions.faderEnabled);
#endif

    refresh (true);  // TODO: - [mh] Change this to refresh(true)?
    resized();
}
//==============================================================================

void MeterChannel::lookAndFeelChanged()
{
    visibilityChanged();
}
//==============================================================================

void MeterChannel::visibilityChanged()
{
    setColours();
}
//==============================================================================

juce::Colour MeterChannel::getColourFromLnf (int colourId, const juce::Colour& fallbackColour) const
{
    if (isColourSpecified (colourId))
        return findColour (colourId);
    if (getLookAndFeel().isColourSpecified (colourId))
        return getLookAndFeel().findColour (colourId);

    return fallbackColour;
}
//==============================================================================

void MeterChannel::setColours()
{
    m_meterColours.backgroundColour    = getColourFromLnf (backgroundColourId, juce::Colours::black);
    m_meterColours.inactiveColour      = getColourFromLnf (inactiveColourId, juce::Colours::grey);
    m_meterColours.textValueColour     = getColourFromLnf (textValueColourId, juce::Colours::white.darker (0.6f));  // NOLINT
    m_meterColours.muteColour          = getColourFromLnf (mutedColourId, juce::Colours::red);
    m_meterColours.muteMouseOverColour = getColourFromLnf (mutedMouseOverColourId, juce::Colours::black);
    m_meterColours.solodColour         = getColourFromLnf (solodColourId, juce::Colours::red);
    m_meterColours.faderColour         = getColourFromLnf (faderColourId, juce::Colours::blue.withAlpha (Constants::kFaderAlphaMax));
    m_meterColours.textColour          = getColourFromLnf (textColourId, juce::Colours::white.darker (0.6f));                       // NOLINT
    m_meterColours.tickMarkColour      = getColourFromLnf (tickMarkColourId, juce::Colours::white.darker (0.3f).withAlpha (0.5f));  // NOLINT
    m_meterColours.peakHoldColour      = getColourFromLnf (peakHoldColourId, juce::Colours::red);
}
//==============================================================================

void MeterChannel::useGradients (bool useGradients)
{
    m_meterOptions.useGradient = useGradients;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::showTickMarks (bool showTickMarks)
{
    m_meterOptions.tickMarksEnabled = showTickMarks;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::showTickMarksOnTop (bool showTickMarksOnTop)
{
    m_meterOptions.tickMarksOnTop = showTickMarksOnTop;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::setTickMarks (const std::vector<float>& tickMarks)
{
    m_meterOptions.tickMarks = tickMarks;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::showValue (bool showValue /*= true*/)
{
    m_meterOptions.valueEnabled = showValue;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::showPeakHold (bool showPeakHold)
{
    m_meterOptions.showPeakHoldIndicator = showPeakHold;
    setOptions (m_meterOptions);
}
//==============================================================================

void MeterChannel::resized()
{
    auto meterBounds = getLocalBounds();
    if (!m_minimalMode)
        meterBounds = SoundMeter::Helpers::applyPadding (getLocalBounds(), m_padding);

    // Meter header...
    m_header.setBounds (meterBounds.withHeight (0));
    if (m_meterOptions.headerEnabled && !m_minimalMode)
        m_header.setBounds (meterBounds.removeFromTop (Constants::kDefaultHeaderHeight));

    m_level.setMeterBounds (meterBounds);

#if SDTK_ENABLE_FADER
    m_fader.setBounds (m_level.getLevelBounds());
#endif
}
//==============================================================================

void MeterChannel::paint (juce::Graphics& g)
{
    if (getLocalBounds().isEmpty())
        return;

    g.setFont (m_font);

    // Draw the 'HEADER' part of the meter...
    if (!m_header.getBounds().isEmpty() && m_meterOptions.headerEnabled)
    {
#if SDTK_ENABLE_FADER
        m_header.draw (g, isActive(), m_fader.isEnabled(), m_meterColours);
#else
        m_header.draw (g, isActive(), false, m_meterColours);
#endif
    }

    drawMeter (g);

#if SDTK_ENABLE_FADER
    m_fader.draw (g, m_meterColours);  // Draw FADER....
#endif
}
//==============================================================================

void MeterChannel::drawMeter (juce::Graphics& g)
{
    // Draw meter BACKGROUND...
    g.setColour (m_active ? m_meterColours.backgroundColour : m_meterColours.inactiveColour);
    g.fillRect (m_level.getMeterBounds());

    m_active ? m_level.drawMeter (g, m_meterColours) : m_level.drawInactiveMeter (g, m_meterColours);
}
//==============================================================================

bool MeterChannel::isDirty (const juce::Rectangle<int>& rectToCheck /*= {}*/) const noexcept
{
    if (rectToCheck.isEmpty())
        return !m_dirtyRect.isEmpty();
    return m_dirtyRect.intersects (rectToCheck);
}
//==============================================================================

void MeterChannel::addDirty (const juce::Rectangle<int>& dirtyRect) noexcept
{
    if (!isShowing())
        return;
    m_dirtyRect = m_dirtyRect.getUnion (dirtyRect);
}
//==============================================================================

void MeterChannel::setDirty (bool isDirty /*= true*/) noexcept
{
    if (!isShowing())
        return;
    m_dirtyRect = { 0, 0, 0, 0 };
    if (isDirty)
        m_dirtyRect = getLocalBounds();
}
//==============================================================================

void MeterChannel::refresh (const bool forceRefresh)
{
    if (!isShowing())
        return;

    if (getBounds().isEmpty())
        return;

    if (m_active)
    {
        m_level.refreshMeterLevel();
        const auto levelDirtyBounds = m_level.getDirtyBounds();
        if (!levelDirtyBounds.isEmpty())
            addDirty (levelDirtyBounds);

#if SDTK_ENABLE_FADER
        if (m_fader.needsRedrawing())
            addDirty (m_level.getMeterBounds());  // Repaint if the faders are being moved/faded out...
#endif
    }

    // Redraw if dirty or forced to...
    if (isDirty())
    {
        repaint (m_dirtyRect);
        setDirty (false);
    }
    else if (forceRefresh)
    {
        repaint();
    }
}
//==============================================================================

void MeterChannel::setActive (bool isActive, NotificationOptions notify /*= NotificationOptions::dontNotify*/)
{
    if (m_active == isActive)
        return;
    m_active = isActive;

#if SDTK_ENABLE_FADER
    if (notify == NotificationOptions::notify)
        notifyParent();
#else
    juce::ignoreUnused (notify);
#endif

    reset();
    repaint();
}
//==============================================================================

void MeterChannel::resetMouseOvers() noexcept
{
    m_header.resetMouseOver();
    m_level.resetMouseOverValue();
}
//==============================================================================

void MeterChannel::setFont (const juce::Font& font)
{
    m_font = font;
    m_header.setFont (m_font);
    setDirty();  // TODO: - [mh] Change this to refresh(true)?
}
//==============================================================================

void MeterChannel::resetPeakHold()
{
    m_level.resetPeakHold();
    // setDirty(); // TODO: - [mh] Change this to refresh(true)?
}
//==============================================================================

void MeterChannel::setChannelName (const juce::String& channelName)
{
    m_header.setName (channelName);
    addDirty (m_header.getBounds());
}
//==============================================================================

void MeterChannel::setIsLabelStrip (bool isLabelStrip) noexcept
{
    m_isLabelStrip = isLabelStrip;
    m_level.setIsLabelStrip (isLabelStrip);
}
//==============================================================================

void MeterChannel::setMeterSegments (const std::vector<SegmentOptions>& segmentsOptions)
{
#if SDTK_ENABLE_FADER
    m_fader.setMeterSegments (segmentsOptions);
#endif
    m_level.setMeterSegments (segmentsOptions);
    setDirty();  // TODO: - [mh] Change this to refresh(true)?
    resized();
}
//==============================================================================

#if SDTK_ENABLE_FADER

void MeterChannel::showFader (const bool faderVisible /*= true */)
{
    m_fader.setVisible (faderVisible);

    // If slider needs to be DE-ACTIVATED...
    if (!faderVisible || !m_fader.isEnabled())
        resetMouseOvers();
    addDirty (m_fader.getBounds());
}
//==============================================================================

void MeterChannel::setFaderValue (const float value, NotificationOptions notificationOption /*= NotificationOptions::DontNotify*/, const bool mustShowFader /*= true*/)
{
    if (m_fader.setValue (value, notificationOption))
    {
        if (mustShowFader && !m_fader.isVisible())
            flashFader();
        addDirty (m_fader.getBounds());
    }
}
//==============================================================================

void MeterChannel::enableFader (bool faderEnabled /*= true*/) noexcept
{
    m_fader.enable (faderEnabled);
    addDirty (m_fader.getBounds());
}

//==============================================================================

void MeterChannel::mouseDown (const juce::MouseEvent& event)
{
    // Left mouse button down and fader is active...
    if (event.mods.isLeftButtonDown() && m_fader.isEnabled())
    {
        // Clicked on the METER part...
        if (!m_header.isMouseOver (event.y) && !m_level.isMouseOverValue (event.y) && m_fader.isVisible())
        {
            if (!isActive())
                setActive (true);               // Activate if it was deactivated.
            m_fader.setValueFromPos (event.y);  // Set the fader level at the value clicked.
            addDirty (m_fader.getBounds());
        }

        // Clicked on the HEADER part...
        if (m_header.isMouseOver (event.y))
        {
            if (event.mods.isShiftDown())
            {
                setActive (true, NotificationOptions::dontNotify);
                if (onChannelSolo)
                    onChannelSolo (this);
            }
            else
            {
                setActive (!isActive(), NotificationOptions::notify);
            }
        }
    }
}

#endif /* SDTK_ENABLE_FADER */
//==============================================================================

void MeterChannel::mouseMove (const juce::MouseEvent& event)
{
    // Check if the FADER is enabled...

#if SDTK_ENABLE_FADER
    const bool faderEnabled = m_fader.isEnabled();
#else
    const bool faderEnabled = false;
#endif

    // Check if the mouse is over the header part...
    const auto isMouseOverHeader      = m_header.isMouseOver();                                 // Get the previous mouse over flag, to check if it has changed.
    const bool mouseOverHeaderChanged = (isMouseOverHeader != m_header.isMouseOver (event.y));  // Check if it has changed.
    if (m_header.isMouseOver() && mouseOverHeaderChanged && faderEnabled)  // If the mouse entered the 'header' part for the first time and the fader is enabled...
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        setTooltip ("Mute or un-mute channel");
    }
    if (mouseOverHeaderChanged)
        addDirty (m_header.getBounds());  // Mouse over status has changed. Repaint.

    // Check if the mouse is over the value part...
    const auto isMouseOverValue      = m_level.isMouseOverValue();
    const bool mouseOverValueChanged = (isMouseOverValue != m_level.isMouseOverValue (event.y));
    if (m_level.isMouseOverValue() && mouseOverValueChanged)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);  // NOLINT
        setTooltip ("Double click to clear peak hold.");
    }
    if (mouseOverValueChanged)
        addDirty (m_level.getValueBounds());

    // Check if the mouse is over the meter part...
    if (!m_header.isMouseOver() && !m_level.isMouseOverValue())
    {
#if SDTK_ENABLE_FADER
        if (m_fader.isVisible())
        {
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
            setTooltip (m_isLabelStrip ? "Drag to move master fader." : "Drag to move fader.");
        }
        else
#endif /* SDTK_ENABLE_FADER */

        {
            setMouseCursor (juce::MouseCursor::NormalCursor);
        }
    }
}
//==============================================================================

void MeterChannel::mouseExit (const juce::MouseEvent& /*event*/)
{
    resetMouseOvers();
    repaint();
}
//==============================================================================

void MeterChannel::mouseDoubleClick (const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {

        if (!m_header.isMouseOver (event.y))
        {
            if (m_level.isMouseOverValue (event.y))  // Double clicking on VALUE resets peak hold...
                resetPeakHold();
#if SDTK_ENABLE_FADER
            else if (event.mods.isShiftDown())  // Shift double click resets the full mixer...
            {
                if (onMixerReset)
                    onMixerReset();
            }
            else if (isActive())  // Double clicking on the FADER resets it to 0...
                setFaderValue (1.0f, NotificationOptions::notify);
#endif /* SDTK_ENABLE_FADER */
        }
    }
}
#if SDTK_ENABLE_FADER
//==============================================================================

void MeterChannel::mouseDrag (const juce::MouseEvent& event)
{
    // When left button down, the meter is active, the fader is active and the mouse is not over the 'info' area...
    if (event.mods == juce::ModifierKeys::leftButtonModifier && isActive() && m_fader.isVisible() && !m_level.isMouseOverValue (event.y))
    {
        m_fader.setValueFromPos (event.y);
        addDirty (m_level.getMeterBounds());
    }
}
//==============================================================================

void MeterChannel::mouseWheelMove (const juce::MouseEvent& /*event*/, const juce::MouseWheelDetails& wheel)
{
    setFaderValue (juce::jlimit (0.0f, 1.0f, m_fader.getValue() + (wheel.deltaY / Constants::kFaderSensitivity)), NotificationOptions::notify, false);
}

#endif /* SDTK_ENABLE_FADER */

}  // namespace SoundMeter
}  // namespace sd