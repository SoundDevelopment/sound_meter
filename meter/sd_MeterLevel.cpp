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


//==============================================================================
void Level::drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const
{
   if (! m_valueVisible) return;
   if (m_valueBounds.isEmpty()) return;

   // Draw PEAK value...
   const auto peak = getPeakHoldLevel();
   if (m_valueBounds.getHeight() == Constants::kChannelNameHeight && peak > kMin99Db)  // If active, present and enough space is available.
   {
      const float peak_db   = juce::Decibels::gainToDecibels (peak);
      const int   precision = peak_db <= -10.0f ? 1 : 2;  // Set precision depending on peak value. NOLINT

      // Assemble value string...
      std::stringstream peakValue;
      peakValue << std::fixed << std::setprecision (precision) << peak_db;

      g.setColour (textValueColour);
      g.drawFittedText (peakValue.str(), m_valueBounds, juce::Justification::centred, 1);
   }
}

//==============================================================================
void Level::drawPeakHold (juce::Graphics& g, const juce::Colour& peakHoldColour) const
{
   using namespace Constants;

   if (! m_peakHoldVisible) return;

   // Calculate peak hold y coordinate...
   const int y = m_meterBounds.getY() + static_cast<int> (m_meterBounds.getHeight() * (1.0f - m_peakHoldLevel));

   if (y + kPeakHoldHeight < m_meterBounds.getHeight())
   {
      g.setColour (peakHoldColour);
      g.fillRect (m_meterBounds.getX(), y, m_meterBounds.getWidth(), kPeakHoldHeight);
   }
}

//==============================================================================
void Level::drawMeter (juce::Graphics& g, const juce::Colour& peakColour, const juce::Colour& warningColour, const juce::Colour& normalColour)
{
   // Draw meter bar segments (normal, warning, peak)...
   const auto levelDrawn = getMeterLevel();
   calculateLevelDrawn();
   drawMeterSegment (g, levelDrawn, 0.0f, m_warningRegion, normalColour, warningColour);
   drawMeterSegment (g, levelDrawn, m_warningRegion, m_peakRegion, warningColour, peakColour);
   drawMeterSegment (g, levelDrawn, m_peakRegion, 1.0f, peakColour, peakColour.darker());
}

//==============================================================================
void Level::drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const
{
   // Check if there is space enough to write the 'MUTE' text...
   if (m_meterBounds.getWidth() < (g.getCurrentFont().getHeight())) return;

   g.saveState();
   g.addTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::halfPi, static_cast<float> (m_meterBounds.getCentreX()),
                                                    static_cast<float> (m_meterBounds.getY() + (m_meterBounds.getWidth() / 2.0f))));  // NOLINT
   g.setColour (textColour);

   g.drawText (TRANS ("MUTE"), m_meterBounds.withWidth (m_meterBounds.getHeight()).withHeight (m_meterBounds.getWidth()), juce::Justification::centred);
   g.restoreState();
}

//==============================================================================
void Level::drawTickMarks (juce::Graphics& g, const juce::Colour& tickColour) const
{
   if (! m_tickMarksVisible) return;

   g.setColour (tickColour);

   for (const auto& tick: m_tickMarks)  // Loop through all ticks.
   {
      g.fillRect (m_meterBounds.getX(),
                  m_meterBounds.getY() + static_cast<int> (round ((m_meterBounds.getHeight() * (1.0f - tick.gain)) - (Constants::kTickMarkHeight / 2.0f))),  // NOLINT
                  m_meterBounds.getWidth(), Constants::kTickMarkHeight);
   }
}

//==============================================================================
void Level::drawLabels (juce::Graphics& g, const juce::Colour& textColour) const
{
   if (! m_tickMarksVisible) return;

   g.setColour (textColour);
   const float fontsize = std::clamp (m_meterBounds.getHeight() / 4.0f, 1.0f, 15.0f);  // Set font size proportionally. NOLINT
   g.setFont (fontsize);

   for (const auto& tick: m_tickMarks)
   {
      const juce::Rectangle<int> labelrect (m_meterBounds.getX(),
                                            m_meterBounds.getY() + static_cast<int> (round ((m_meterBounds.getHeight() * (1.0f - tick.gain)) - (fontsize / 2.0f))),  // NOLINT
                                            m_meterBounds.getWidth(), static_cast<int> (fontsize));

      g.drawFittedText (juce::String (std::abs (tick.decibels)), labelrect.reduced( Constants::kLabelStripTextPadding, 0 ), juce::Justification::topLeft, 1);
   }
}

//==============================================================================
void Level::drawMeterSegment (juce::Graphics& g, const float level, const float start, const float stop, const juce::Colour& colour, const juce::Colour& nextColour) const
{
   const float segmentLevel = std::min (level, stop);

   if (segmentLevel > start)  // A part of the segment needs to be filled...
   {
      juce::Rectangle<int> segmentRect {};

      const auto top    = m_meterBounds.getY() + static_cast<int> (m_meterBounds.getHeight() * (1.0f - segmentLevel));
      const auto bottom = static_cast<int> (m_meterBounds.getY() + std::round ((1.0f - start) * m_meterBounds.getHeight()));

      if (m_useGradients)
      {
         const auto        max            = m_meterBounds.getY() + static_cast<int> (m_meterBounds.getHeight() * (1.0f - stop));
         const juce::Point gradientPoint1 = { 0.0f, static_cast<float> (bottom) };
         const juce::Point gradientPoint2 = { 0.0f, static_cast<float> (max) };
         g.setGradientFill (juce::ColourGradient (colour, gradientPoint1, nextColour, gradientPoint2, false));
      }
      else
      {
         g.setColour (colour);
      }
      g.fillRect (segmentRect.withLeft (m_meterBounds.getX()).withWidth (m_meterBounds.getWidth()).withTop (top).withBottom (bottom));
   }
}

//==============================================================================
void Level::setTickMarks (const std::vector<float>& ticks) noexcept
{
   m_tickMarks.clear();
   for (const auto& tick: ticks)
      m_tickMarks.emplace_back (Tick (tick));
}

//==============================================================================
[[nodiscard]] float Level::getInputLevel() noexcept
{
   m_inputLevelRead.store (true);
   return std::clamp (m_inputLevel.load(), -m_maxLevel, m_maxLevel);
}

//==============================================================================
inline void Level::setInputLevel (float newLevel) noexcept
{
   m_inputLevel.store (m_inputLevelRead.load() ? newLevel : std::max (m_inputLevel.load(), newLevel));
   m_inputLevelRead.store (false);
}

//==============================================================================
void Level::setMeterLevel (float newLevel) noexcept
{
   m_meterLevel    = (newLevel > m_meterLevel ? newLevel : getDecayedLevel (newLevel));
   m_peakHoldLevel = std::max<float> (m_peakHoldLevel, newLevel);
}

//==============================================================================
void Level::setRefreshRate (float refreshRate_hz)
{
   m_refreshRate_hz = refreshRate_hz;
   calculateDecayCoeff();
}

//==============================================================================
void Level::setDecay (float decay_ms) noexcept
{
   m_decay_ms = std::clamp (decay_ms, Constants::kMinDecay_ms, Constants::kMaxDecay_ms);
   calculateDecayCoeff();
}

//==============================================================================
int Level::calculateLevelDrawn() noexcept
{
   m_levelDrawn_px = static_cast<int> (m_meterLevel * m_meterBounds.getHeight());
   return m_levelDrawn_px;
}

//==============================================================================
void Level::setRegions (const float warningRegion_db, const float peakRegion_db)
{
   jassert (m_peakRegion > m_warningRegion);  // NOLINT
   if (m_peakRegion <= m_warningRegion) return;

   m_warningRegion = juce::Decibels::decibelsToGain (warningRegion_db);
   m_peakRegion    = juce::Decibels::decibelsToGain (peakRegion_db);
}

//==============================================================================
void Level::reset() noexcept
{
   m_inputLevel.store (0.0f);
   m_meterLevel          = 0.0f;
   m_levelDrawn_px       = 0;
   m_previousRefreshTime = 0;
}

//==============================================================================
[[nodiscard]] float Level::getDecayedLevel (const float callbackLevel)
{
   if (m_refreshRate_hz <= 0.0f) return callbackLevel;

   // Measure time passed...
   const auto currentTime = juce::Time::getMillisecondCounter();
   const auto timePassed  = juce::Time::getMillisecondCounter() - m_previousRefreshTime;
   m_previousRefreshTime  = currentTime;

   if (timePassed > m_decay_ms) return callbackLevel;  // The signal has fully decayed.

   // Convert that to refreshed frames...
   auto numberOfFramePassed = static_cast<int> (std::round ((timePassed * m_refreshRate_hz) / 1000.0f));
   auto level               = m_meterLevel;
   for (int frame = 0; frame < numberOfFramePassed; ++frame)
      level = callbackLevel + (m_decayCoeff * (level - callbackLevel));

   if (level < 0.0001f) return 0.0f;

   return level;
}

//==============================================================================
void Level::calculateDecayCoeff() noexcept
{
   // Rises to 99% of in value over duration of time constant.
   m_decayCoeff = std::pow (0.01f, (1000.0f / (m_decay_ms * m_refreshRate_hz)));  // NOLINT
}

}  // namespace sd::SoundMeter