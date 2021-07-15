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

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.

#if 0
BEGIN_JUCE_MODULE_DECLARATION

    ID:             sound_meter
    vendor:         Sound Development
    name:           Resize-able peak meter module with fader and container panel.
    version:        0.8.0
    description:    Resize-able peak meter juce module, with an (optional) fader and accompanying container panel (mixer).
    website:        https://www.sounddevelopment.nl
    license:        MIT

    dependencies:   juce_audio_basics, juce_gui_basics, juce_events, juce_graphics
END_JUCE_MODULE_DECLARATION

#endif

*******************************************************************************/

#ifndef SOUND_METER_HEADER_H
#define SOUND_METER_HEADER_H

/** Config: SDTK_ENABLE_FADER
    Enable a fader, to be overlayed on top of the meter.
*/
#ifndef SDTK_ENABLE_FADER
   #define SDTK_ENABLE_FADER 1
#endif

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "meter/sd_MeterHelpers.h"
#include "meter/sd_MeterLevel.h"
#include "meter/sd_MeterFader.h"
#include "meter/sd_MeterHeader.h"
#include "meter/sd_MeterComponent.h"
#include "meter/sd_MetersPanel.h"

#endif  //SOUND_METER_HEADER_H
