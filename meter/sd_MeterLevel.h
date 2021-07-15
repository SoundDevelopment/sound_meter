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

#ifndef SD_SOUND_METER_LEVEL_H
#define SD_SOUND_METER_LEVEL_H


namespace sd::SoundMeter
{

/**
 * @brief Class responsible for anything relating to the 'meter' and peak 'value' parts.
 * This also includes the peak hold indicator and the tick-marks.
*/
class Level
{
public:
   // clng-format off

   /**
    * @brief Constructor.
   */
   Level() = default;

   /**
    * @brief                       Draws the meter.
    * @param[in,out] g             The juce graphics context to use.
    * @param         peakColour    Colour of the peak region of the meter. 
    * @param         warningColour Colour of the warning region of the meter.
    * @param         normalColour  Colour of the normal region of the meter.
   */
   void drawMeter (juce::Graphics& g, const juce::Colour& peakColour, const juce::Colour& warningColour, const juce::Colour& normalColour);

   /**
    * @brief Draw the 'meter' part in it's inactive (muted) state.
    * @param[in,out] g   The juce graphics context to use.
    * @param textColour  Colour of the text on the inactive meter.
   */
   void drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const;

   /**
    * @brief Draw the peak 'value'.
    * @param[in,out] g        The juce graphics context to use.
    * @param textValueColour  Colour of the text displaying the peak value.
   */
   void drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const;

   /**
    * @brief Draw the peak hold indicator.
    * @param[in,out] g        The juce graphics context to use.
    * @param colour           The colour of the peak hold indicator.
   */
   void drawPeakHold (juce::Graphics& g, const juce::Colour& colour) const;

   /**
    * @brief Draw the tick-marks.
    * These are the lines dividing the meters at certain dB levels.
    * @param[in,out] g        The juce graphics context to use.
    * @param tickColour       Colour of the tick marks.
   */
   void drawTickMarks (juce::Graphics& g, const juce::Colour& tickColour) const;

   /**
    * @brief Draw the value labels at the tick marks.
    * These will be displayed if the meter is a labelStrip.
    * @param[in,out] g        The juce graphics context to use.
    * @param textColour       Colour of the label text.
   */
   void drawLabels (juce::Graphics& g, const juce::Colour& textColour) const;

   /**
    * @brief Set the level of the meter.
    * Here the level is actually set from the audio engine.
    * Very likely called from the audio thread!
    * @param newLevel The peak level from the audio engine (in amp).
    * 
    * @see getInputLevel
   */
   inline void setInputLevel (float newLevel) noexcept;


   /**
    * @brief Get's the meter's input level.
    * 
    * @return The meter's input level (in amp).
    * 
    * @see setInputLevel
   */
   [[nodiscard]] float getInputLevel() noexcept;

   /**
    * @brief Set the actual meter's level.
    * 
    * Set's the meter's level with ballistics.
    * Instant attack, but decayed release.
    * @param newLevel The level to use as input for the meter's ballistics.
    * 
    * @see getMeterLevel, setDecay
   */
   void setMeterLevel (float newLevel) noexcept;

   /**
    * @brief Get the actual meter's level.
    * 
    * Get the decayed meter level.
    * Instant attack, but decayed release.
    * @returnl The actual meter's level with ballistics.
    * 
    * @see setMeterLevel, setDecay
   */
   [[nodiscard]] float getMeterLevel() const noexcept { return m_meterLevel; }

   /**
    * @brief Get the meter's refresh (redraw) rate.
    *
    * @return The refresh rate of the meter in Hz.
    *
    * @see setRefreshRate
    */
   [[nodiscard]] float getRefreshRate() const noexcept { return m_refreshRate_hz; }

   /**
    * @brief Set the meter's refresh rate.
    *
    * @param refreshRate_hz Refresh rate (in Hz) to use to redraw the meter.
    *
    * @see getRefreshRate()
    */
   void setRefreshRate (float refreshRate_hz);

   /**
    * @brief Set meter decay.
    *
    * @param decay_ms Meter decay in milliseconds.
    * 
    * @see getDecay
    */
   void setDecay (float decay_ms) noexcept;

   /**
    * @brief Get meter decay.
    *
    * @return Meter decay in milliseconds
    * 
    * @see setDecay
    */
   [[nodiscard]] float getDecay() const noexcept { return m_decay_ms; }

   /**
    * @brief Set's the visibility of the peak hold indicator.
    * 
    * @param isVisible When true, the peak hold indicator is shown.
    * 
    * @see isPeakHoldVisible, resetPeakHoldLevel
   */
   void setPeakHoldVisible (bool isVisible) noexcept { m_peakHoldVisible = isVisible; }

   /**
    * @brief Check if the peak hold indicator is visible.
    *
    * @return True if the peak hold indicator is visible.
    *
    * @see setPeakHoldVisible, resetPeakHoldLevel
    */
   [[nodiscard]] bool isPeakHoldVisible() const noexcept { return m_peakHoldVisible; }

   /**
    * @brief Show the peak value.
    *
    * The peak value will be shown below the meter (in db).
    * It's the same level as the peak hold bar.
    *
    * @param isVisible when set, shows the value level (in db) below the meter.
    * 
    * @see isPeakValueVisible, resetPeakHoldLevel
    */
   void setPeakValueVisible (bool isVisible) noexcept { m_peakValueVisible = isVisible; }

   /**
    * @brief Check if the peak 'value' part is visible.
    *
    * The peak value will be shown below the meter (in db).
    * It's the same level as the peak hold bar.
    * 
    * @return True, if the peak hold 'value' part is visible.
    *
    * @see setPeakValueVisible, resetPeakHoldLevel
    */
   [[nodiscard]] bool isPeakValueVisible() const noexcept { return m_peakValueVisible; }

   /**
    * @brief Reset the peak hold level.
    * 
    * Set's the peak hold level to 0.
    * 
    * @see getPeakHoldLevel, isPeakValueVisible, setPeakValueVisible, setPeakHoldVisible, isPeakHoldVisible
   */
   void resetPeakHoldLevel() noexcept { m_peakHoldLevel = 0.0f; }

   /**
    * @brief Get the current peak hold level.
    * 
    * Set's the peak hold level to 0.
    * 
    * @return The current peak hold level (in amp).
    * @see resetPeakHoldLevel, isPeakValueVisible, setPeakValueVisible, setPeakHoldVisible, isPeakHoldVisible
   */
   [[nodiscard]] float getPeakHoldLevel() const noexcept { return m_peakHoldLevel; }

   /**
    * @brief Return the maximum level of the meter.
    *
    * @return The maximum level of the meter (in amp).
    */
   [[nodiscard]] float getMaxLevel() const noexcept { return m_maxLevel; }

   /**
    * Get the level actually drawn on screen.
    *
    * This will return the level in pixels actually drawn
    * and is used to determine if the meter is dirty (needs refresh).
    *
    * @return The level actually drawn on screen (in pixels).
    */
   [[nodiscard]] int   getLevelDrawn() const noexcept { return m_levelDrawn_px; }
   [[nodiscard]] float getWarningRegion() const noexcept { return m_warningRegion; }
   [[nodiscard]] float getPeakRegion() const noexcept { return m_peakRegion; }
   int                 calculateLevelDrawn() noexcept;

   /**
    * Set the levels dividing the different regions of the meter. 
    *
    * The meter has 3 regions. Normal, warning and peak. 
    * The peak region level supplied need to be larger then the warning region level. 
    *
    * @param warningRegion_db sets the level (in db) dividing the normal and warning regions of the meter.
    * @param peakRegion_db sets the level (in db) dividing the warning and peak regions of the meter.
    */
   void setRegions (const float warningRegion_db, const float peakRegion_db);
   void reset() noexcept;

   void setValueBounds (juce::Rectangle<int> bounds) noexcept { m_valueBounds = bounds; }

   /**
    * Get the bounds of the 'value' part.
    *
    * @return the bounds of the value part.
    */
   [[nodiscard]] juce::Rectangle<int> getValueBounds() const noexcept { return m_valueBounds; }
   void                               setMeterBounds (juce::Rectangle<int> bounds) noexcept { m_meterBounds = bounds; }

   /**
    * Get the bounds of the 'meter' part.
    *
    * @return the bounds of the meter.
    */
   [[nodiscard]] juce::Rectangle<int> getMeterBounds() const noexcept { return m_meterBounds; }


   bool isMouseOverValue (const int y) noexcept
   {
      m_mouseOverValue = (y >= m_valueBounds.getY() && ! m_valueBounds.isEmpty());
      return m_mouseOverValue;
   }
   [[nodiscard]] bool isMouseOverValue() const noexcept { return m_mouseOverValue; }
   void               resetMouseOverValue() noexcept { m_mouseOverValue = false; }

   /**
    * @brief Tick-mark class.
    * Data class for drawing tick-marks.
    * These are the divider lines on the meter at certain
    * dB levels.
   */
   struct Tick
   {
      /**
       * @brief Constructor.
       * @param dbValue Value of the tick-mark (in dB).
      */
      explicit Tick (float dbValue)
      {
         decibels = dbValue;
         gain     = juce::Decibels::decibelsToGain (dbValue);
      }
      float gain     = 0;  ///< Value of the tick mark (in amp).
      float decibels = 0;  ///< Value of the tick mark (in dB).
   };

   /**
    * Set the level of the tick marks. 
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param ticks list of tick mark values (in amp).
    */
   void setTickMarks (const std::vector<float>& ticks) noexcept;

   /**
    * Show tick-marks (divider lines) on the meter.
    *
    * @param visible when set, shows the tick-marks. 
    */
   void setTickMarksVisible (bool visible) noexcept { m_tickMarksVisible = visible; }

   /**
    * @brief Use gradients in stead of hard region boundaries.
    * @param useGradients When set to true, uses smooth gradients. False gives hard region boundaries.=
   */
   void useGradients (bool useGradients) noexcept {  m_useGradients = useGradients; }

private:
   juce::Rectangle<int> m_valueBounds;  // Bounds of the value area.
   juce::Rectangle<int> m_meterBounds;  // Bounds of the meter area.

   std::vector<Tick> m_tickMarks {};  // List of user definable tick marks (in db).

   std::atomic<float> m_inputLevel { 0.0f };  // Audio peak level.
   float              m_peakHoldLevel = 0.0f;
   float              m_meterLevel    = 0.0f;  // Current meter level.
   int                m_levelDrawn_px = 0;
   const float        m_maxLevel      = juce::Decibels::decibelsToGain (Constants::kMaxLevel_db);  // Maximum meter level.

   std::atomic<bool> m_inputLevelRead { false };

   float m_warningRegion = juce::Decibels::decibelsToGain (-10.0f);  // Meter warning region. NOLINT
   float m_peakRegion    = juce::Decibels::decibelsToGain (-3.0f);   // Meter peak region. NOLINT

   bool m_tickMarksVisible = true;
   bool m_peakHoldVisible  = true;
   bool m_peakValueVisible = false;
   bool m_mouseOverValue   = false;
   bool m_useGradients     = true;

   float m_decay_ms            = Constants::kDefaultDecay_ms;
   float m_decayCoeff          = 0.0f;
   float m_refreshRate_hz      = 25.0f;  // Meter refresh rate in Hz. NOLINT
   int   m_previousRefreshTime = 0;

   const float kMin99Db = juce::Decibels::decibelsToGain (-99.0f);

   //==============================================================================
   [[nodiscard]] float getDecayedLevel (const float callbackLevel);
   void                calculateDecayCoeff() noexcept;
   void drawMeterSegment (juce::Graphics& g, float level, float start, float stop, const juce::Colour& colour, const juce::Colour& nextColour) const;

   // clang-format on
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Level)
};

}  // namespace sd::SoundMeter

#endif /* SD_SOUND_METER_LEVEL_H */