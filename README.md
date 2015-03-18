# Kinderegg

Packages Shadertoys into desktop executables.

## Purpose
 - For the lazy/wannabe demo coder who would like to submit something to their local demoparty without investing too much time in a proper framework and tools  
 - For taking advantage of [Shadertoy](https://www.shadertoy.com/)'s excellent browser-based shader IDE to create desktop apps  

## Usage

Pick out a favorite shadertoy and remember its 6-character id. Save your Shadertoy API key in the file **tools/apikey.txt**.

    cd tools
    python parse_shadertoy_json.py <id>

The script will:  

 - Create a directory under tools/ named for the given shadertoy id.  
 - Query Shadertoy.com for the given id's json data  
 - Write the shader's name, author and description to a README file in the directory  
 - Invoke CMake and Visual Studio to create an executable  
 - Copy the exe into the new directory  
 - Copy SDL2.dll in there as well  

Once finished, there should be a complete, running production in the new directory ready for submission.

## Working Examples

 - [lssXWS]() Music - Harry Potter by iq
 - [4dl3zn]() Bubbles by iq
 - [4dfXWj]() Music - Mario by iq
 - [XdfXWS]() Music - Pulsating by iq
 - [lsfXDl]() Whitney wheel by lost4468
 - [lts3zn]() Cardboard Waves by jimbo00000
 - [4tfGRM]() Shulgins' Eye by jimbo00000

## Features
 - Image rendering  
 - Audio  
 - Textures(todo)

## Shortcomings
Some features are not supported:  

 - Cubemaps  
 - Video/audio textures  
 - Keyboard  
 - Mouse  
 - Playback pause/reset  
 - Recompilation  

The exe size is rather large due to SDL dependency.

## Thanks

All glory to iq and Beautypi for the coolest shader IDE around. Tremendous thanks and respect to all shadertoy users, democoders, party organizers, Khronos contributors, driver engineers, hardware designers and graphics enthusiasts everywhere.
