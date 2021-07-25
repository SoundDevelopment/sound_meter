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


namespace sd
{
namespace SoundMeter
{

/**
 * @brief Class responsible for anything relating to the 'meter' and peak 'value' parts.
 * This also includes the peak hold indicator and the tick-marks.
*/
class Level
{
public:
   /**
    * @brief Constructor.
   */
   Level() = default;

   /**
    * @brief Reset the meter (but not the peak hold).
    *
    * @see resetPeakHold
    */
   void reset() noexcept;

   /**
    * @brief Set the level of the meter.
    * 
    * Here the level is actually set from the audio engine.
    * Beware: very likely called from the audio thread!
    * 
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
    * @brief Get the actual meter's level (including ballistics).
    * 
    * Get the decayed meter level.
    * Instant attack, but decayed release.
    * 
    * @return The actual meter's level with ballistics.
    * 
    * @see setMeterLevel, setDecay
   */
   [[nodiscard]] float getMeterLevel() const noexcept { return m_meterLevel; }

   /**
    * @brief Get the level actually drawn on screen.
    *
    * This will return the level in pixels actually drawn
    * and is used to determine if the meter is dirty (needs refresh).
    *
    * @return The level actually drawn on screen (in pixels).
    */
   [[nodiscard]] int getLevelDrawn() const noexcept { return m_levelDrawn_px; }

   /**
    * @brief Set the meter's options.
    *
    * The options determine the appearance and functionality of the meter.
    *
    * @param meterOptions Meter options to use.
    */
   void setOptions (Options meterOptions)
   {
      setDecay (meterOptions.decayTime_ms);
      defineSegments (meterOptions.warningRegion_db, meterOptions.peakRegion_db);
      setTickMarks (meterOptions.tickMarks);
      enableTickMarks (meterOptions.tickMarksEnabled);
      setDecay (meterOptions.decayTime_ms);
      useGradients (meterOptions.useGradient);
      setRefreshRate (meterOptions.refreshRate);
      showPeakHold (meterOptions.showPeakHoldIndicator);

      m_options = meterOptions;
   }

   /**
    * @brief Get the meter's refresh (redraw) rate.
    *
    * @return The refresh rate of the meter in Hz.
    *
    * @see setRefreshRate
    */
   [[nodiscard]] float getRefreshRate() const noexcept { return m_options.refreshRate; }

   /**
    * @brief Sets the meter's refresh rate. 
    *
    * Set this to optimize the meter's decay rate.
    * 
    * @param refreshRate_hz Refresh rate in Hz.
    * @see setDecay, getDecay
    */
   void setRefreshRate (float refreshRate_hz) noexcept;

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
   [[nodiscard]] float getDecay() const noexcept { return m_options.decayTime_ms; }

   /**
    * @brief Set's the visibility of the peak hold indicator.
    * 
    * @param isVisible When true, the peak hold indicator is shown.
    * 
    * @see isPeakHoldVisible, resetPeakHoldLevel
   */
   void showPeakHold (bool isVisible) noexcept { m_options.showPeakHoldIndicator = isVisible; }

   /**
    * @brief Set the levels dividing the different segments of the meter. 
    *
    * The meter has 3 segments. Normal, warning and peak. 
    * The peak segment level supplied need to be larger then the warning segment level. 
    *
    * @param warningRegion_db Sets the level (in db) dividing the normal and warning segments of the meter.
    * @param peakRegion_db    Sets the level (in db) dividing the warning and peak segments of the meter.
    */
   void defineSegments (const float warningRegion_db, const float peakRegion_db);

   /**
    * @brief Check if the peak hold indicator is visible.
    *
    * @return True if the peak hold indicator is visible.
    *
    * @see showPeakHold, resetPeakHoldLevel
    */
   [[nodiscard]] bool isPeakHoldVisible() const noexcept { return m_options.showPeakHoldIndicator; }

   /**
    * @brief Enable the peak 'value' part of the meter.
    *
    * When enabled (and made visible with showValue) the
    * peak 'value' part will be shown below the meter (in dB).
    * It's the same level as the peak hold bar.
    *
    * @param valueEnabled When set true, the 'value' level (in dB) part below the meter will be enabled.
    * 
    * @see isPeakValueVisible, resetPeakHoldLevel, setValueVisible
    */
   void enableValue (bool valueEnabled) noexcept { m_options.valueEnabled = valueEnabled; }

   /**
    * @brief Show the peak 'value' part of the meter.
    *
    * The peak value will be shown below the meter (in dB).
    * It's the same level as the peak hold bar.
    *
    * @param isVisible When set true, shows the 'value' level (in dB) part below the meter.
    *
    * @see isPeakValueVisible, resetPeakHoldLevel, enableValue
    */
   void showValue (bool isVisible) noexcept { m_valueVisible = isVisible; }

   /**
    * @brief Check if the peak 'value' part is visible.
    *
    * The peak value will be shown below the meter (in db).
    * It's the same level as the peak hold bar.
    * 
    * @return True, if the peak hold 'value' part is visible.
    *
    * @see showValue, resetPeakHoldLevel
    */
   [[nodiscard]] bool isPeakValueVisible() const noexcept { return m_valueVisible && m_options.valueEnabled; }

   /**
    * @brief Reset the peak hold level.
    * 
    * Set's the peak hold level to 0.
    * 
    * @see getPeakHoldLevel, isPeakValueVisible, setPeakValueVisible, showPeakHold, showValue, isPeakHoldVisible
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
    * @brief Set the bounds of the 'value' part of the meter.
    * 
    * @param bounds The bounds to use for the 'value' part of the meter.
    * 
    * @see getValueBounds, setMeterBounds, getMeterBounds
   */
   void setValueBounds (juce::Rectangle<int> bounds) noexcept { m_valueBounds = bounds; }

   /**
    * @brief Get the bounds of the 'value' part of the meter.
    *
    * @return The bounds of the 'value' part of the meter.
    * @see setMeterBounds, setValueBounds, getMeterBounds
    */
   [[nodiscard]] juce::Rectangle<int> getValueBounds() const noexcept { return m_valueBounds; }

   /**
    * @brief Set the bounds of the 'meter' part of the meter.
    * 
    * @param bounds The bounds to use for the 'meter' part of the meter.
    * @see getValueBounds, setValueBounds, getMeterBounds
   */
   void setMeterBounds (juce::Rectangle<int> bounds) noexcept;

   /**
    * @brief Get the bounds of the 'meter' part.
    *
    * @return The bounds of the 'meter' part.
    * @see getValueBounds, setValueBounds, setMeterBounds
    */
   [[nodiscard]] juce::Rectangle<int> getMeterBounds() const noexcept { return m_meterBounds; }

   /**
    * @brief Check if the mouse cursor is over the 'value' part of the meter.
    * 
    * @param y The y coordinate (relative to the meter bounds) to use to determine if the mouse if over the 'value' part of the meter.
    * @return True, if the mouse cursor is over the 'value' part of the meter.
   */
   bool isMouseOverValue (const int y) noexcept;

   /**
    * @brief Check if the mouse cursor is over the 'value' part of the meter.
    * 
    * @return True, if the mouse cursor is over the 'value' part of the meter.
   */
   [[nodiscard]] bool isMouseOverValue() const noexcept { return m_mouseOverValue; }

   /**
    * @brief Reset 'mouse over' status of the 'value' part of the meter.
   */
   void resetMouseOverValue() noexcept { m_mouseOverValue = false; }

   /**
    * @brief                       Draws the meter.
    * @param[in,out] g             The juce graphics context to use.
    * @param         peakColour    Colour of the peak region of the meter. 
    * @param         warningColour Colour of the warning region of the meter.
    * @param         normalColour  Colour of the normal region of the meter.
    * 
    * @see drawInactiveMeter, drawPeakValue, drawPeakHold, drawTickMarks, drawLabels
   */
   void drawMeter (juce::Graphics& g, const juce::Colour& peakColour, const juce::Colour& warningColour, const juce::Colour& normalColour);

   /**
    * @brief Draw the 'meter' part in it's inactive (muted) state.
    * 
    * @param[in,out] g   The juce graphics context to use.
    * @param textColour  Colour of the text on the inactive meter.
    * 
    * @see drawMeter, drawTickMarks, drawPeakValue, drawPeakHold, drawLabels
   */
   void drawInactiveMeter (juce::Graphics& g, const juce::Colour& textColour) const;

   /**
    * @brief Draw the peak 'value'.
    * @param[in,out] g        The juce graphics context to use.
    * @param textValueColour  Colour of the text displaying the peak value.
    * 
    * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakHold, drawTickMarks, drawLabels
   */
   void drawPeakValue (juce::Graphics& g, const juce::Colour& textValueColour) const;

   /**
    * @brief Draw the peak hold indicator.
    * @param[in,out] g        The juce graphics context to use.
    * @param colour           The colour of the peak hold indicator.
    * 
    * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakValue, drawTickMarks, drawLabels
   */
   void drawPeakHold (juce::Graphics& g, const juce::Colour& colour) const;

   /**
    * @brief Draw the tick-marks.
    * These are the lines dividing the meters at certain dB levels.
    * @param[in,out] g        The juce graphics context to use.
    * @param tickColour       Colour of the tick marks.
    * 
    * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakValue, drawPeakHold, drawLabels
   */
   void drawTickMarks (juce::Graphics& g, const juce::Colour& tickColour) const;

   /**
    * @brief Draw the value labels at the tick marks.
    * These will be displayed if the meter is a labelStrip.
    * @param[in,out] g        The juce graphics context to use.
    * @param textColour       Colour of the label text.
    * 
    * @see drawMeter, drawInactiveMeter, drawInactiveMeter, drawPeakValue, drawPeakHold, drawTickMarks
   */
   void drawLabels (juce::Graphics& g, const juce::Colour& textColour) const;

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
    * @brief Set the level of the tick marks. 
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param ticks list of tick mark values (in amp).
    * @see setTickMarksVisible, enableTickMarks
    */
   void setTickMarks (const std::vector<float>& ticks) noexcept;

   /**
    * @brief Show tick-marks (divider lines) on the meter.
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    *
    * @param visible When set true, shows the tick-marks. 
    * @see setTickMarks, enableTickMarks
    */
   void showTickMarks (bool visible) noexcept { m_tickMarksVisible = visible; }

   /**
    * @brief Enable tick-marks (divider lines) on the meter.
    *
    * A tick mark is a horizontal line, dividing the meter. 
    * This is also the place the label strip will put it's text values.
    * 
    * The tick-marks will be shown when they are enable and visible.
    *
    * @param enabled When set true, the tick-marks are enabled. 
    * @see setTickMarks, showTickMarks
    */
   void enableTickMarks (bool enabled) noexcept { m_options.tickMarksEnabled = enabled; }

   /**
    * @brief Use gradients in stead of hard region boundaries.
    * @param useGradients When set to true, uses smooth gradients. False gives hard region boundaries.
   */
   void useGradients (bool useGradients) noexcept { m_options.useGradient = useGradients; }

   class Segment
   {
   public:
      int drawMeterSegment (juce::Graphics& g, const juce::Colour& colour, const juce::Colour& nextColour, bool useGradient)
      {
         if (m_segmentBounds.isEmpty()) return 0;

         auto level_px  = static_cast<int> ((m_currentLevel - m_startLevel) * m_levelMultiplier);
         auto levelRect = m_segmentBounds.withTop (m_segmentBounds.getBottom() - level_px);

         int levelDrawn_px = 0;
         if (level_px > 0 && level_px < m_segmentBounds.getHeight()) levelDrawn_px = level_px;

         //const float segmentLevel = std::min (level, stop);

         //if (segmentLevel > start)  // A part of the segment needs to be filled...
         //{
         //   juce::Rectangle<int> segmentRect {};

         //   const auto top    = m_meterBounds.getY() + static_cast<int> (std::round (m_meterBounds.getHeight() * (1.0f - segmentLevel)));
         //   const auto bottom = static_cast<int> (m_meterBounds.getY() + std::round ((1.0f - start) * m_meterBounds.getHeight()));

         //   // Store the actual drawn level. To check later if it needs to redrawn or not...
         //   if (segmentLevel < stop) m_levelDrawn_px = bottom - top;

         //if (useGradient)
         //{
         //   const auto               max            = m_meterBounds.getY() + static_cast<int> (m_meterBounds.getHeight() * (1.0f - stop));
         //   const juce::Point<float> gradientPoint1 = { 0.0f, static_cast<float> (bottom) };
         //   const juce::Point<float> gradientPoint2 = { 0.0f, static_cast<float> (max) };
         //   g.setGradientFill (juce::ColourGradient (colour, gradientPoint1, nextColour, gradientPoint2, false));
         //}
         //else
         //{
         g.setColour (colour);
         //      }
         // g.fillRect (segmentRect.withLeft (m_meterBounds.getX()).withWidth (m_meterBounds.getWidth()).withTop (top).withBottom (bottom));
         g.fillRect (levelRect);
         //}

         return levelDrawn_px;
      }

      void setLevel (float level)
      {
         if (m_segmentBounds.isEmpty()) return;

         auto previousLevel = m_currentLevel;
         m_currentLevel     = juce::jlimit (m_startLevel, m_stopLevel, level);
         auto level_px      = static_cast<int> ((level - m_startLevel) / m_segmentBounds.getHeight());
      }

      void setRange (float newStartLevel, float newStopLevel)
      {
         const auto startLevel = juce::jlimit (0.0f, 1.0f, newStartLevel);
         const auto stopLevel  = juce::jlimit (0.0f, 1.0f, newStopLevel);

         if (startLevel >= stopLevel)
         {
            jassertfalse;  // NOLINT
            return;
         }

         m_startLevel = startLevel;
         m_stopLevel  = stopLevel;

         calculateSegment();
      }


      void setMeterBounds (juce::Rectangle<int> bounds) noexcept
      {
         m_meterBounds = bounds;
         calculateSegment();
      }

      [[nodiscard]] juce::Rectangle<int> getSegmentBounds() const noexcept { return m_segmentBounds; }

   private:
      float                m_startLevel   = 0.0f;
      float                m_stopLevel    = 1.0f;
      float                m_currentLevel = 0.0f;
      int                  m_levelDrawn   = 0;
      juce::Rectangle<int> m_meterBounds {};
      juce::Rectangle<int> m_segmentBounds {};
      float                m_levelMultiplier = 0.0f;


      void calculateSegment()
      {
         if (m_meterBounds.isEmpty()) return;

         // Calculate segment bounds...
         m_segmentBounds = m_meterBounds.getProportion<float> ({ 0.0f, 1.0f - m_stopLevel, 1.0f, m_stopLevel - m_startLevel });

         // Calculate level multiplier to optimize level drawing...
         m_levelMultiplier = m_segmentBounds.getHeight() / (m_stopLevel - m_startLevel);

         jassert (! m_segmentBounds.isEmpty());
         setLevel (m_currentLevel);
      }

      JUCE_LEAK_DETECTOR (Segment)
   };

private:
   Options m_options;

   std::atomic<float> m_inputLevel { 0.0f };  // Audio peak level.
   std::atomic<bool>  m_inputLevelRead { false };
   float              m_peakHoldLevel = 0.0f;
   float              m_meterLevel    = 0.0f;  // Current meter level.
   int                m_levelDrawn_px = 0;

   Segment m_normalSegment;
   Segment m_warningSegment;
   Segment m_peakSegment;

   float m_warningSegmentLevel = 0.2f;
   float m_peakSegmentLevel    = 0.8f;

   juce::Rectangle<int> m_valueBounds;  // Bounds of the value area.
   juce::Rectangle<int> m_meterBounds;  // Bounds of the meter area.

   std::vector<Tick> m_tickMarks {};  // List of user definable tick marks (in db).

   bool        m_tickMarksVisible    = true;
   bool        m_valueVisible        = false;
   bool        m_mouseOverValue      = false;
   float       m_decayCoeff          = 0.0f;
   float       m_refreshPeriod_ms    = (1.0f / m_options.refreshRate) * 1000.0f;
   int         m_previousRefreshTime = 0;
   const float kMin99Db              = juce::Decibels::decibelsToGain (-99.0f);
   const float kMinMeter_db          = juce::Decibels::decibelsToGain (-65.0f);
   const float m_maxLevel            = juce::Decibels::decibelsToGain (Constants::kMaxLevel_db);  // Maximum meter level.

   //==============================================================================
   [[nodiscard]] float getDecayedLevel (const float callbackLevel);
   void                calculateDecayCoeff() noexcept;
   void                drawMeterSegment (juce::Graphics& g, float level, float start, float stop, const juce::Colour& colour, const juce::Colour& nextColour);

   // clang-format on
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Level)
};

}  // namespace SoundMeter
}  // namespace sd

#endif /* SD_SOUND_METER_LEVEL_H */