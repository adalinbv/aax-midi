
#include <string>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "canonical_names.hpp"

typedef struct {
 int program;
 int msb;
 int lsb;
 const char *name;
 int elements;
} _inst_t;

_inst_t inst_table[] = {
 {   1,  0,   0, "Grand Piano", 1 },
 {   1,  0,   1, "Grand Piano (wide)", 1 },
 {   1,  0,  18, "Mellow Grand Piano", 1 },
 {   1,  0,  40, "Piano Strings", 2 },
 {   1,  0,  41, "Dream Piano", 2 },
 {   1, 64,   0, "Cutting Noise", 1 },
 {   2,  0,   0, "Bright Piano", 1 },
 {   2, 64,   0, "Cutting Noise 2", 2 },
 {   2,  0,   1, "Bright Piano (wide)", 1 },
 {   3,  0,   0, "Electric Grand Piano", 2 },
 {   3, 64,   0, "Distorted Cutting Noise", 2 },
 {   3,  0,   1, "Electric Grand Piano (wide)", 2 },
 {   3,  0,  32, "Detuned CP80", 2 },
 {   3,  0,  40, "Layer CP80 1", 2 },
 {   3,  0,  41, "Layer CP80 2", 2 },
 {   4,  0,   0, "Honky-tonk Piano", 2 },
 {   4,  0,   1, "Honky-tonk Piano (wide)", 2 },
 {   4, 64,   0, "String Slap", 1 },
 {   5,  0,   0, "Electric Piano 1", 2 },
 {   5,  0,   1, "Electric Piano 1 (wide)", 1 },
 {   5,  0,  18, "Mellow Electric Piano 1", 2 },
 {   5,  0,  32, "Chorused Electric Piano 1", 2 },
 {   5,  0,  40, "Hard Electric Piano", 2 },
 {   5,  0,  45, "Velocity Cross-fade Electric Piano 1", 2 },
 {   5,  0,  64, "60's Electric Piano", 1 },
 {   5, 64,   0, "Bass Slide", 2 },
 {   6,  0,   0, "Electric Piano 2", 2 },
 {   6,  0,   1, "Electric Piano 2 (wide)", 1 },
 {   6,  0,  32, "Chorused Electric Piano 2", 2 },
 {   6,  0,  33, "DX Electric Piano Hard", 2 },
 {   6,  0,  34, "DX Legend", 2 },
 {   6,  0,  40, "DX Phase Electric Piano", 2 },
 {   6,  0,  41, "DX + Analog Electric Piano", 2 },
 {   6,  0,  42, "DX + Koto Electric Piano", 2 },
 {   6,  0,  45, "Velocity Cross-fade Electric Piano 2", 2 },
 {   6, 64,   0, "Pick Scrape", 1 },
 {   7,  0,   0, "Harpsichord", 1 },
 {   7,  0,   1, "Harpsichord (wide)", 1 },
 {   7,  0,  25, "Harpsichord 2", 2 },
 {   7,  0,  35, "Harpsichord 3", 2 },
 {   8,  0,   0, "Clavinet", 1 },
 {   8,  0,   1, "Clavinet (wide)", 1 },
 {   8,  0,  27, "Clavinet Wah", 2 },
 {   8,  0,  64, "Pulse Clavinet", 1 },
 {   8,  0,  65, "Pierce Clavinet", 2 },
 {   9,  0,   0, "Celesta", 1 },
 {  10,  0,   0, "Glockenspiel", 1 },
 {  11,  0,   0, "Music Box", 2 },
 {  11,  0,  64, "Orgel", 2 },
 {  12,  0,   0, "Vibraphone", 1 },
 {  12,  0,   1, "Vibraphone (wide)", 1 },
 {  12,  0,  45, "Hard Vibraphone", 2 },
 {  13,  0,   0, "Marimba", 1 },
 {  13,  0,   1, "Marimba (wide)", 1 },
 {  13,  0,  64, "Sine Marimba", 2 },
 {  13,  0,  97, "Balimba", 2 },
 {  13,  0,  98, "Log Drum", 2 },
 {  14,  0,   0, "Xylophone", 1 },
 {  15,  0,   0, "Tubular Bells", 1 },
 {  15,  0,  96, "Church Bells", 2 },
 {  15,  0,  97, "Carillon", 2 },
 {  16,  0,   0, "Dulcimer", 1 },
 {  16,  0,  35, "Dulcimer 2", 2 },
 {  16,  0,  96, "Cimbalom", 2 },
 {  16,  0,  97, "Santur", 2 },
 {  17,  0,   0, "Drawbar Organ", 1 },
 {  17,  0,  32, "Detuned Drawbar Organ", 2 },
 {  17,  0,  33, "60's Drawbar Organ 1", 2 },
 {  17,  0,  34, "60's Drawbar Organ 2", 2 },
 {  17,  0,  35, "70's Drawbar Organ 1", 2 },
 {  17,  0,  36, "Drawbar Organ 2", 2 },
 {  17,  0,  37, "60's Drawbar Organ 3", 2 },
 {  17,  0,  38, "Even Bar", 2 },
 {  17,  0,  40, "16+2'2/3", 2 },
 {  17,  0,  64, "Organ Bass", 1 },
 {  17,  0,  65, "70's Drawbar Organ 2", 2 },
 {  17,  0,  66, "Cheezy Organ", 2 },
 {  17,  0,  67, "Drawbar Organ 3", 2 },
 {  17, 64,   0, "Flute Key Click", 1 },
 {  18,  0,   0, "Percussion Organ", 1 },
 {  18,  0,  24, "70's Percussion Organ 1", 2 },
 {  18,  0,  32, "Detuned Percussion Organ", 2 },
 {  18,  0,  33, "Light Organ", 2 },
 {  18,  0,  37, "Percussion Organ 2", 2 },
 {  19,  0,   0, "Rock Organ", 1 },
 {  19,  0,  64, "Rotary Organ", 2 },
 {  19,  0,  65, "Slow Rotary Organ", 2 },
 {  19,  0,  66, "Fast Rotary Organ", 2 },
 {  20,  0,   0, "Church Organ", 2 },
 {  20,  0,  32, "Church Organ 3", 2 },
 {  20,  0,  35, "Church Organ 2", 2 },
 {  20,  0,  40, "Notre Dame", 2 },
 {  20,  0,  64, "Organ Flute", 2 },
 {  20,  0,  65, "Tremolo Organ Flute", 2 },
 {  21,  0,   0, "Reed Organ", 1 },
 {  21,  0,  40, "Puff Organ", 2 },
 {  22,  0,   0, "Accordion", 1 },
 {  22,  0,  32, "Accordion Italian", 2 },
 {  23,  0,   0, "Harmonica", 1 },
 {  23,  0,  32, "Harmonica 2", 2 },
 {  24,  0,   0, "Tango Accordion", 1 },
 {  24,  0,  64, "Tango Accordion 2", 2 },
 {  25,  0,   0, "Nylon Guitar", 1 },
 {  25,  0,  16, "Nylon Guitar 2", 1 },
 {  25,  0,  25, "Nylon Guitar 3", 2 },
 {  25,  0,  43, "Velocity Guitar Harmonics", 1 },
 {  25,  0,  96, "Ukulele", 1 },
 {  26,  0,   0, "Steel Guitar", 1 },
 {  26,  0,  16, "Steel Guitar 2", 1 },
 {  26,  0,  35, "12-string Guitar", 2 },
 {  26,  0,  40, "Nylon & Steel Guitar", 2 },
 {  26,  0,  41, "Steel Guitar with Body Sound", 2 },
 {  26,  0,  96, "Mandolin", 2 },
 {  27,  0,   0, "Jazz Guitar", 1 },
 {  27,  0,  18, "Mellow Guitar", 1 },
 {  27,  0,  32, "Jazz Amp", 2 },
 {  27,  0,  96, "Pedal Steel Guitar", 1 },
 {  28,  0,   0, "Clean Guitar", 1 },
 {  28,  0,  32, "Chorus Guitar", 2 },
 {  28,  0,  65, "Mid-tone Guitar", 1 },
 {  28,  0,  66, "Mid-tone Guitar (wide)", 2 },
 {  29,  0,   0, "Muted Guitar", 1 },
 {  29,  0,  40, "Funk Guitar 1", 2 },
 {  29,  0,  41, "Muted Steel Guitar", 2 },
 {  29,  0,  43, "Funk Guitar 2", 1 },
 {  29,  0,  45, "Jazz Man", 2 },
 {  29,  0,  96, "Muted Distortion Guitar", 2 },
 {  30,  0,   0, "Overdrive Guitar", 1 },
 {  30,  0,  43, "Guitar Pinch", 1 },
 {  31,  0,   0, "Distortion Guitar", 1 },
 {  31,  0,  12, "Distorted Rhythm Guitar", 2 },
 {  31,  0,  24, "Distortion Guitar 2", 2 },
 {  31,  0,  35, "Distortion Guitar 3", 2 },
 {  31,  0,  36, "Power Guitar 2", 2 },
 {  31,  0,  37, "Power Guitar 1", 2 },
 {  31,  0,  38, "Distorted Fifths", 2 },
 {  31,  0,  40, "Feedback Guitar", 2 },
 {  31,  0,  41, "Feedback Guitar 2", 2 },
 {  31,  0,  43, "Rock Rhythm Guitar 2", 2 },
 {  31,  0,  45, "Rock Rhythm Guitar 1", 2 },
 {  32,  0,   0, "Guitar Harmonics", 1 },
 {  32,  0,  65, "Guitar Feedback", 1 },
 {  32,  0,  66, "Guitar Harmonics 2", 1 },
 {  33,  0,   0, "Acoustic Bass", 1 },
 {  33,  0,  40, "Jazz Rhythm", 2 },
 {  33,  0,  45, "Velocity Cross-fade Upright Bass", 2 },
 {  33, 64,   0, "Shower", 1 },
 {  34,  0,   0, "Finger Bass", 1 },
 {  34,  0,  18, "Finger Bass Dark", 2 },
 {  34,  0,  27, "Flanger Bass", 2 },
 {  34,  0,  40, "Bass & Distorted Electric Guitar", 2 },
 {  34,  0,  43, "Finger Slap Bass", 1 },
 {  34,  0,  45, "Finger Bass 2", 2 },
 {  34,  0,  64, "Jazzy Bass", 1 },
 {  34,  0,  65, "Modulated Bass", 2 },
 {  34,  0, 112, "FW-stringed Electric Bass", 1 },
 {  34, 64,   0, "Thunder", 1 },
 {  35,  0,   0, "Pick Bass", 1 },
 {  35,  0,  28, "Muted Pick Bass", 1 },
 {  33, 64,   0, "Wind", 1 },
 {  36,  0,   0, "Fretless Bass", 1 },
 {  36,  0,  32, "Fretless Bass 2", 2 },
 {  36,  0,  33, "Fretless Bass 3", 2 },
 {  36,  0,  34, "Fretless Bass 4", 2 },
 {  36,  0,  96, "Synth Fretless Bass", 2 },
 {  36,  0,  97, "Smooth Fretless Bass", 2 },
 {  36, 64,   0, "Stream", 2 },
 {  37,  0,   0, "Slap Bass 1", 1 },
 {  37,  0,  27, "Resonant Slap", 1 },
 {  37,  0,  32, "Punch Thumb Bass", 2 },
 {  37,  0,  64, "Slapper", 1 },
 {  37,  0,  65, "Thumb & Slap", 1 },
 {  37, 64,   0, "Bubble", 2 },
 {  38,  0,   0, "Slap Bass 2", 1 },
 {  38,  0,  43, "Velocity Switch Bass", 1 },
 {  38, 64,   0, "Feed", 2 },
 {  39,  0,   0, "Synth Bass 1", 1 },
 {  39,  0,  18, "Synth Bass 1 Dark", 1 },
 {  39,  0,  20, "Fast Resonant Bass", 1 },
 {  39,  0,  24, "Acid Bass", 1 },
 {  39,  0,  27, "Resonant Bass", 1 },
 {  39,  0,  35, "Clavinet Bass", 2 },
 {  39,  0,  40, "Techno Bass", 2 },
 {  39,  0,  64, "Orbiter", 2 },
 {  39,  0,  65, "Square Bass", 1 },
 {  39,  0,  66, "Rubber Bass", 2 },
 {  39,  0,  67, "Fish", 1 },
 {  39,  0,  68, "Hard Resonant Bass", 1 },
 {  39,  0,  96, "Hammer", 2 },
 {  39,  0, 112, "Analog Bass", 2 },
 {  39,  0, 113, "Rezo Bass", 2 },
 {  39, 64,   0, "Cave", 2 },
 {  40,  0,   0, "Synth Bass 2", 2 },
 {  40,  0,   6, "Mellow Synth Bass", 1 },
 {  40,  0,  12, "Sequenced Bass", 2 },
 {  40,  0,  18, "Click Synth Bass", 2 },
 {  40,  0,  19, "Synth Bass 2 Dark", 1 },
 {  40,  0,  32, "Smooth Synth Bass", 2 },
 {  40,  0,  40, "Modular Synth Bass", 2 },
 {  40,  0,  41, "DX Bass", 2 },
 {  40,  0,  64, "X Wire Bass", 2 },
 {  40,  0,  65, "Attack Pulse", 1 },
 {  40,  0,  66, "CS Light", 1 },
 {  40,  0,  67, "Metal Bass", 1 },
 {  40,  0, 112, "Smooth Bass", 2 },
 {  40,  0, 113, "Octaved Bass", 1 },
 {  40,  0, 114, "Power Bass", 4 },
 {  41,  0,   0, "Violin", 1 },
 {  41,  0,   8, "Slow Violin", 1 },
 {  42,  0,   0, "Viola 1", },
 {  43,  0,   0, "Cello 1", },
 {  44,  0,   0, "Contra Bass", 1 },
 {  45,  0,   0, "Tremolo Strings", 1 },
 {  45,  0,   8, "Slow Tremolo Strings", 1 },
 {  45,  0,  40, "Suspension Strings", 2 },
 {  46,  0,   0, "Pizzicato Strings", 1 },
 {  47,  0,   0, "Harp", 1 },
 {  47,  0,  40, "Yang Chin", 2 },
 {  48,  0,   0, "Timpani", 1 },
 {  49,  0,   0, "Strings 1", 1 },
 {  49,  0,   3, "Strings (wide)", 2 },
 {  49,  0,   8, "Slow Strings", 1 },
 {  49,  0,  24, "Arco Strings", 2 },
 {  49,  0,  35, "60's Strings", 2 },
 {  49,  0,  40, "Orchestra", 2 },
 {  49,  0,  41, "Orchestra 2", 2 },
 {  49,  0,  42, "Tremolo Orchestra", 2 },
 {  49,  0,  45, "Velocity Strings", 2 },
 {  49, 64,   0, "Dog", 1 },
 {  50,  0,   0, "Strings 2", 1 },
 {  50,  0,   3, "Slow  Strings (wide)", 2 },
 {  50,  0,   8, "Legato Strings", 2 },
 {  50,  0,  40, "Warm Strings", 2 },
 {  50,  0,  41, "Kingdom", 2 },
 {  50,  0,  64, "70's Strings", 1 },
 {  50,  0,  65, "Strings 3", 1 },
 {  50, 64,   0, "Horse", 1 },
 {  51,  0,   0, "Synth Strings 1", 2 },
 {  51,  0,  27, "Resonant Synth Strings", 2 },
 {  51,  0,  64, "Synth Strings 4", 2 },
 {  51,  0,  65, "Synth Strings 5", 2 },
 {  51, 64,   0, "tweet 2", 1 },
 {  52,  0,   0, "Synth Strings 2", 2 },
 {  53,  0,   0, "Choir Aahs", 1 },
 {  53,  0,   3, "Choir (wide)", 2 },
 {  53,  0,  16, "Choir Aahs 2", 2 },
 {  53,  0,  32, "Mellow Choir", 2 },
 {  53,  0,  40, "Choir Strings", 2 },
 {  54,  0,   0, "Voice Ooh", 1 },
 {  55,  0,   0, "Synth Voice", 1 },
 {  55,  0,  40, "Synth Voice 2", 2 },
 {  55,  0,  41, "Choral",2 },
 {  55,  0,  64, "Analog Voice", 1 },
 {  55, 64,   0, "Ghost", 2 },
 {  56,  0,   0, "Orchestra Hit", 2 },
 {  56,  0,  35, "Orchestra Hit 2", 2 },
 {  56,  0,  64, "Impact", 2 },
 {  56,  0,  68, "Bass Hit", 1 },
 {  56,  0,  70, "6th Hit", 1 },
 {  56,  0,  71, "6th Hit Plus", 2 },
 {  56,  0,  72, "Euro Hit", 1 },
 {  56,  0,  73, "Euro Hit Plus", 2 },
 {  56, 64,   0, "Maou", 2 },
 {  57,  0,   0, "Trumpet", 1 },
 {  57,  0,  16, "Trumpet 2", 1 },
 {  57,  0,  17, "Bright Trumpet", 2 },
 {  57,  0,  32, "Warm Trumpet", 2 },
 {  57, 64,   0, "Insects", 2 },
 {  58,  0,   0, "Trombone", 1 },
 {  58,  0,  18, "Trombone 2", 2 },
 {  58, 64,   0, "Bacteria", 2 },
 {  59,  0,   0, "Tuba", 1 },
 {  59,  0,  16, "Tuba 2", 1 },
 {  60,  0,   0, "Muted Trumpet", 1 },
 {  61,  0,   0, "French Horn", 1 },
 {  61,  0,   6, "French Horn Solo", 1 },
 {  61,  0,  32, "French Horn 2", 2 },
 {  61,  0,  37, "Horn Orchestra", 2 },
 {  62,  0,   0, "Brass Section", 1 },
 {  62,  0,   3, "Brass Section (wide)", 2 },
 {  62,  0,  35, "Trumpet & Trombone Section", 2 },
 {  62,  0,  40, "Brass Section 2", 2 },
 {  62,  0,  41, "Hit Brass", 2 },
 {  62,  0,  42, "Mellow Brass", 2 },
 {  63,  0,   0, "Synth Brass 1", 2 },
 {  63,  0,  12, "Quack Brass", 2 },
 {  63,  0,  20, "Resonant Synth Brass", 2 },
 {  63,  0,  24, "Poly Brass", 2 },
 {  63,  0,  27, "Synth  Brass 3", 2 },
 {  63,  0,  32, "Jump Brass", 2 },
 {  63,  0,  40, "Synth Brass with Sub Oscillator", 2 },
 {  63,  0,  45, "Analog Velocity Brass 1", 2 },
 {  63,  0,  64, "Analog Brass 1", 2 },
 {  64,  0,   0, "Synth  Brass 2", 1 },
 {  64,  0,  18, "Soft Brass", 2 },
 {  64,  0,  40, "Synth  Brass 4", 2 },
 {  64,  0,  41, "Choir Brass", 2 },
 {  64,  0,  45, "Analog Velocity Brass 2", 2 },
 {  64,  0,  64, "Analog Brass 2", 2 },
 {  65,  0,   0, "Soprano Saxophone", 1 },
 {  65, 64,   0, "Phone Call", 1 },
 {  66,  0,   0, "Alto Saxophone", 1 },
 {  66,  0,  40, "Saxophone Section", 2 },
 {  66,  0,  43, "Hyper Alto Saxophone", 1 },
 {  66, 64,   0, "Door Squeak", 1 },
 {  67,  0,   0, "Tenor Saxophone", 1 },
 {  67,  0,  40, "Breathy Tenor Saxophone", 2 },
 {  67,  0,  41, "Soft Tenor Saxophone", 2 },
 {  67,  0,  64, "Tenor Saxophone 2", 1 },
 {  67, 64,   0, "Door Slam", 1 },
 {  68,  0,   0, "Baritone Saxophone", 1 },
 {  68, 64,   0, "Scratch Cut", 1 },
 {  69,  0,   0, "Oboe", 1 },
 {  69, 64,   0, "Scratch Split", 1 },
 {  70,  0,   0, "English Horn", 1 },
 {  70, 64,   0, "Wind Chime", 1 },
 {  71,  0,   0, "Bassoon", 1 },
 {  71, 64,   0, "Telephone 2", 1 },
 {  72,  0,   0, "Clarinet", 1 },
 {  73,  0,   0, "Piccolo", 1 },
 {  74,  0,   0, "Flute", 1 },
 {  75,  0,   0, "Recorder", 1 },
 {  76,  0,   0, "Pan Flute", 1 },
 {  77,  0,   0, "Blown Bottle", 2 },
 {  78,  0,   0, "Shakuhachi", 1 },
 {  79,  0,   0, "Whistle", 1 },
 {  80,  0,   0, "Ocarina", 1 },
 {  81,  0,   0, "Square Lead", 2 },
 {  81,  0,   6, "Square Lead 2", 1 },
 {  81,  0,   8, "LM Square", 2 },
 {  81,  0,  18, "Hollow", 1 },
 {  81,  0,  19, "Shroud", 2 },
 {  81,  0,  64, "Mellow", 2 },
 {  81,  0,  65, "Solo Sine", 2 },
 {  81,  0,  66, "Sine Lead", 1 },
 {  81, 64,   0, "Car Engine Ignite", 1 },
 {  81,  0,  67, "Pulse Lead", 1 },
 {  82,  0,   0, "Sawtooth Lead", 2 },
 {  82,  0,   6, "Sawtooth Lead 2",  1 },
 {  82,  0,   8, "Thick Sawtooth", 2 },
 {  82,  0,  18, "Dynamic Sawtooth", 1 },
 {  82,  0,  19, "Digital Sawtooth", 2 },
 {  82,  0,  20, "Big Lead", 2 },
 {  82,  0,  24, "Heavy Synth", 2 },
 {  82,  0,  25, "Waspy Synth", 2 },
 {  82,  0,  27, "Rezzy Sawtooth", 1 },
 {  82,  0,  32, "Double Sawtooth", 2 },
 {  82,  0,  35, "Toy Lead", 2 },
 {  82,  0,  36, "Dim Sawtooth", 2 },
 {  82,  0,  40, "Pulse Sawtooth", 2 },
 {  82,  0,  41, "Dr. Lead", 2 },
 {  82,  0,  45, "Velocity Lead", 2 },
 {  82,  0,  64, "Digger", 1 },
 {  82,  0,  96, "Sequenced Analog", 2 },
 {  82, 64,   0, "Car Tire Squeal", 1 },
 {  83,  0,   0, "Calliope Lead", 2 },
 {  83,  0,  65, "Pure Pad", 2 },
 {  83, 64,   0, "Car Passing", 1 },
 {  84,  0,   0, "Chiff Lead", 2 },
 {  84,  0,  64, "Rubby", 2 },
 {  84,  0,  65, "Hard Sync", 1 },
 {  84, 64,   0, "Car Crash", 1 },
 {  85,  0,   0, "Charang Lead", 2 },
 {  85,  0,  64, "Distorted Lead", 2 },
 {  85,  0,  65, "Wire Lead", 2 },
 {  85, 64,   0, "Siren", 2 },
 {  85,  0,  66, "Synth Pluck", 1 },
 {  86,  0,   0, "Voice Lead", 2 },
 {  86,  0,  24, "Synth Aahs", 2 },
 {  86,  0,  64, "Vox Lead", 2 },
 {  86, 64,   0, "Train", 1 },
 {  87,  0,   0, "Fifth Lead", 2 },
 {  87,  0,  35, "Big Five", 2 },
 {  87, 64,   0, "Jet Plane", 2 },
 {  88,  0,   0, "Bass & Lead", 2 },
 {  88,  0,  16, "Big & Low", 2 },
 {  88,  0,  64, "Fat & Perky", 2 },
 {  88,  0,  65, "Soft Wire Lead", 2 },
 {  88, 64,   0, "Star Ship", 2 },
 {  89,  0,   0, "New Age Pad", 2 },
 {  89,  0,  64, "Fantasy", 2 },
 {  89, 64,   0, "Burst", 2 },
 {  90,  0,   0, "Warm Pad", 2 },
 {  90,  0,  16, "Thick Pad", 2 },
 {  90,  0,  17, "Soft Pad", 2 },
 {  90,  0,  18, "Sine Pad", 2 },
 {  90,  0,  64, "Horn Pad", 2 },
 {  90,  0,  65, "Rotary Strings", 2 },
 {  90, 64,   0, "Coaster", 2 },
 {  91,  0,   0, "Poly Synth Pad", 2 },
 {  91,  0,  64, "Poly Pad 80", 2 },
 {  91,  0,  65, "Click Pad", 2 },
 {  91,  0,  66, "Analog Pad", 2 },
 {  91,  0,  67, "Square Pad", 2 },
 {  91, 64,   0, "Submarine", 1 },
 {  92,  0,   0, "Choir Pad", 2 },
 {  92,  0,  64, "Heaven", 2 },
 {  92,  0,  66, "Itopia", 2 },
 {  92,  0,  67, "CC Pad", 2 },
 {  92, 64,   0, "Connectivity", 2 },
 {  93,  0,   0, "Bowed Pad", 2 },
 {  93,  0,  64, "Glacier", 2 },
 {  93,  0,  65, "Glass Pad", 2 },
 {  93, 64,   0, "Mystery", 2 },
 {  94,  0,   0, "Metal Pad", 2 },
 {  94,  0,  64, "Tine Pad", 2 },
 {  94,  0,  65, "Pan Pad", 2 },
 {  95,  0,   0, "Halo Pad", 2 },
 {  96,  0,   0, "Sweep Pad", 2 },
 {  96,  0,  20, "Shwimmer", 2 },
 {  96,  0,  27, "Converge", 2 },
 {  96,  0,  64, "Polar Pad", 2 },
 {  96,  0,  66, "Celestial", 2 },
 {  97,  0,   0, "Rain", 2 },
 {  97,  0,  45, "Clavinet  Pad", 2 },
 {  97,  0,  64, "Harmo Rain", 2 },
 {  97,  0,  65, "African Wind", 2 },
 {  97,  0,  66, "Carib", 2 },
 {  97, 64,   0, "Laugh", 1 },
 {  98,  0,   0, "Sound Track", 2 },
 {  98,  0,  27, "Prologue", 2 },
 {  98,  0,  64, "Ancestral", 2 },
 {  98, 64,   0, "Scream", 1 },
 {  99,  0,   0, "Crystal", 2 },
 {  99,  0,  12, "Synth Drum Comp", 2 },
 {  99,  0,  14, "Popcorn", 2 },
 {  99,  0,  18, "Tiny Bells", 2 },
 {  99,  0,  35, "Round Glockenspiel", 2 },
 {  99,  0,  40, "Glockenspiel Chime", 2 },
 {  99,  0,  41, "Clear Bells", 2 },
 {  99,  0,  42, "Chorused Bells", 2 },
 {  99,  0,  64, "Synth Mallet", 1 },
 {  99,  0,  65, "Soft Crystal", 2 },
 {  99,  0,  66, "Loud Glockenspiel", 2 },
 {  99,  0,  67, "Christmas Bells", 2 },
 {  99,  0,  68, "Vibraphone Bells", 2 },
 {  99,  0,  69, "Digital Bells", 2 },
 {  99,  0,  70, "Air Bells", 2 },
 {  99,  0,  71, "Bell Harp", 2 },
 {  99,  0,  72, "Gamelimba", 2 },
 {  99, 64,   0, "Punch", 1 },
 { 100,  0,   0,"Atmosphere", 2 },
 { 100,  0,  18, "Warm Atmosphere", 2 },
 { 100,  0,  19, "Hollow Release", 2 },
 { 100,  0,  40, "Nylon Electric Piano", 2 },
 { 100,  0,  64, "Nylon Harp", 2 },
 { 100,  0,  65, "Harp Vox", 2 },
 { 100,  0,  66, "Atmos Pad", 2 },
 { 100,  0,  67, "Planet", 2 },
 { 100, 64,   0, "Heart", 1 },
 { 101,  0,   0, "Bright", 2 },
 { 101,  0,  64, "Fantasia Bells", 2 },
 { 101,  0,  96, "Smokey", 2 },
 { 101, 64,   0, "Footstep", 1 },
 { 102,  0,   0, "Goblins", 2 },
 { 102,  0,  64, "Goblins Synth", 2 },
 { 102,  0,  65, "Creeper", 2 },
 { 102,  0,  66, "Ring Pad", 2 },
 { 102,  0,  67, "Ritual", 2 },
 { 102,  0,  68, "To Heaven", 2 },
 { 102,  0,  70, "Night", 2 },
 { 102,  0,  71, "Glisten", 2 },
 { 102,  0,  96, "Bell Choir", 2 },
 { 102, 64,   0, "Applause 2", 1 },
 { 103,  0,   0, "Echoes", 2 },
 { 103,  0,   8, "Echoes 2", 2 },
 { 103,  0,  14, "Echo Pan", 2 },
 { 103,  0,  64, "Echo Bells", 2 },
 { 103,  0,  65, "Big Pan", 2 },
 { 103,  0,  66, "Synth Piano", 2 },
 { 103,  0,  67, "Creation", 2 },
 { 103,  0,  68, "Star Dust", 2 },
 { 103,  0,  69, "Resonant & Panning", 2 },
 { 104,  0,   0, "Sci-fi", 2 },
 { 104,  0,  64, "Starz", 2 },
 { 105,  0,   0, "Sitar", 1 },
 { 105,  0,  32, "Detuned Sitar", 2 },
 { 105,  0,  35, "Sitar 2", 2 },
 { 105,  0,  96, "Tambra", 2 },
 { 105,  0,  97, "Tamboura", 2 },
 { 106,  0,   0, "Banjo", 1 },
 { 106,  0,  28, "Muted Banjo", 1 },
 { 106,  0,  96, "Rabab", 2 },
 { 106,  0,  97, "Gopichant", 2 },
 { 106,  0,  98, "Oud", 2 },
 { 107,  0,   0, "Shamisen", 1 },
 { 108,  0,   0, "Koto", 1 },
 { 108,  0,  96, "Taisho-Kin", 2 },
 { 108,  0,  97, "Kanoon", 2 },
 { 109,  0,   0, "Kalimba", 1 },
 { 110,  0,   0, "Bagpipe", 2 },
 { 111,  0,   0, "Fiddle", 1 },
 { 112,  0,   0, "Shanai", 1 },
 { 112,  0,  64, "Shanai 2", 1 },
 { 112,  0,  96, "Pungi", 1 },
 { 112,  0,  97, "Hichiriki", 2 },
 { 113,  0,   0, "Tinkle Bell", 2 },
 { 113,  0,  96, "Bonang", 2 },
 { 113,  0,  97, "Altair", 2 },
 { 113,  0,  98, "Gamelan Gongs", 2 },
 { 113,  0,  99, "Gamelan Gongs (wide)", 2 },
 { 113,  0, 100, "Rama Cymbal", 2 },
 { 113,  0, 101, "Asian Bells", 2 },
 { 113, 64,   0, "Machine Gun", 1 },
 { 114,  0,   0, "Agogo", 1 },
 { 114, 64,   0, "Laser Gun", 1 },
 { 115,  0,   0, "Steel Drum", 1 },
 { 115,  0,  97, "Glass Percussion", 2 },
 { 115,  0,  98, "Thai Bells", 2 },
 { 115, 64,   0, "Explosion", 1 },
 { 116,  0,   0, "Woodblock", 1 },
 { 116,  0,  96, "Castanets", 1 },
 { 116, 64,   0, "Fireworks", 1 },
 { 117,  0,   0, "Taiko Drum", 1 },
 { 117,  0,  96, "Gran Cassa", 1 },
 { 117, 64,   0, "Fireball", 2 },
 { 118,  0,   0, "Melodic Tom", 1 },
 { 118,  0,  64, "Melodic Tom 2", 1 },
 { 118,  0,  65, "Real Tom", 2 },
 { 118,  0,  66, "Rock Tom", 2 },
 { 119,  0,   0, "Synth Drum", 1 },
 { 119,  0,  64, "Analog Tom", 1 },
 { 119,  0,  65, "Electronic Percussion", 2 },
 { 120,  0,   0, "Reverse Cymbal", 1 },
 { 121,  0,   0, "Fret Noise", 1 },
 { 122,  0,   0, "Breath Noise", 1 },
 { 123,  0,   0, "Seashore", 2 },
 { 124,  0,   0, "Tweet", 2 },
 { 125,  0,   0, "Telphone", 1 },
 { 126,  0,   0, "Helicopter", 1 },
 { 127,  0,   0, "Applause", 1 },
 { 128,  0,   0, "Gunshot", 1 },
 { 128,  0,  65, "Phone Call", 1 },
 { 128,  0,  66, "Door Squeak", 1 },
 { 128,  0,  67, "Door Slam", 1 },
 { 128,  0,  68, "Scratch Cut", 1 },
 { 128,  0,  69, "Scratch Split", 1 },
 { 128,  0,  70, "Wind Chime", 1 },
 { 128,  0,  71, "Telephone Ring 2", 1 },
 { 128,  0,  81, "Car Engine Ignition", 1 },
 { 128,  0,  82, "Car Tires Squeal", 1 },
 { 128,  0,  83, "Car Passing", 1 },
 { 128,  0,  84, "Car Crash", 1 },
 { 128,  0,  85, "Siren", 2 },
 { 128,  0,  86, "Train", 1 },
 { 128,  0,  87, "Jet Plane", 2 },
 { 128,  0,  88, "Star Ship", 2 },
 { 128,  0,  89, "Burst", 2 },
 { 128,  0,  90, "Coaster", 2 },
 { 128,  0,  91, "Submarine", 1 },
 { 128,  0,  97, "Laugh", 1 },
 { 128,  0,  98, "Scream", 1 },
 { 128,  0,  99, "Punch", 1 },
 { 128,  0, 100, "Heartbeat", 1 },
 { 128,  0, 101, "Footstep", 1 },
 { 128,  0, 113, "Machine Gun", 1 },
 { 128,  0, 114, "Laser Gun", 2 },
 { 128,  0, 115, "Explosion", 2 },
 { 128,  0, 116, "Fireworks", 2 },
 {   0,  0,   0, "", 0 }
};


int main(int argc, char **argv)
{
   const char *pname = strrchr(argv[0], '/');
   int m, b;

   if (!pname) pname = argv[0];
   else pname++;

   printf("<?xml version=\"1.0\"?>\n\n");
   printf("<!--\n * Auto generated by %s\n", pname);
   printf(" *\n * Instrument list for XG-MIDI\n");
   printf("-->\n");
   printf("<aeonwave>\n\n");
   printf(" <midi name=\"XG-MIDI\">\n\n");

   for (m=0; m<65; m+= 64)
   {
       for (b=0; b<128; ++b)
       {
          char f = 0;
          int i = 0;
          do
          {
             if (inst_table[i].msb == m && inst_table[i].lsb == b )
             {
                if (!f)
                {
                   printf("  <bank n=\"%i\" l=\"%i\">\n", m, b);
                   f = 1;
                }

                std::string name = canonical_name(inst_table[i].name);
                printf("   <instrument n=\"%i\" name=\"%s\"",
                        inst_table[i].program-1, name.c_str());
 
                printf(" file=\"instruments/%s\"", to_filename(name).c_str());

                if (strstr(inst_table[i].name, "(wide)")) {
                   printf(" wide=\"true\"");
                }
 
                printf("/>\n");
             }
          } while (inst_table[++i].program);
          if (f) printf("  </bank>\n\n");
       }
    }

   printf(" </midi>\n\n");
   printf("</aeonwave>\n");

  return 0;
}
  