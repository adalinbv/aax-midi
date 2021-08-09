#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct {
 int program;
 int lsb;
 const char *name;
 int elements;
} _inst_t;

_inst_t inst_table[] = {
 {  1,  0, "Grand Piano", 1 },
 {  1,  1, "Grand Piano (wide)", 1 },
 {  1, 18, "Mellow Grand Piano", 1 },
 {  1, 40, "Piano Strings", 2 },
 {  1, 41, "Dream Piano", 2 },
 {  2,  0, "Bright Piano", 1 },
 {  2,  1, "Bright Piano (wide)", 1 },
 {  3,  0, "Electric Grand Piano", 2 },
 {  3,  1, "Electric Grand Piano (wide)", 2 },
 {  3, 32, "Detuned CP80", 2 },
 {  3, 40, "Layer CP80 1", 2 },
 {  3, 41, "Layer CP80 2", 2 },
 {  4,  0, "Honky Tonk Piano", 2 },
 {  4,  1, "Honky Tonk Piano (wide)", 2 },
 {  5,  0, "Electric Piano 1", 2 },
 {  5,  1, "Electric Piano 1 (wide)", 1 },
 {  5, 18, "Mellow Electric Piano 1", 2 },
 {  5, 32, "Chorused  Electric Piano 1", 2 },
 {  5, 40, "Hard Electric Piano", 2 },
 {  5, 45, "Velocity Crossfade Electric Piano 1", 2 },
 {  5, 64, "60's Electric Piano", 1 },
 {  6,  0, "Electric Piano 2", 2 },
 {  6,  1, "Electric Piano 2 (wide)", 1 },
 {  6, 32, "Chorused Electric Piano 2", 2 },
 {  6, 33, "DX Electric Piano Hard", 2 },
 {  6, 34, "DX Legend", 2 },
 {  6, 40, "DX Phase Electric Piano", 2 },
 {  6, 41, "DX + Analog Electric Piano", 2 },
 {  6, 42, "DX + Koto Electric Piano", 2 },
 {  6, 45, "Velocity Crossfade Electric Piano 2", 2 },
 {  7,  0, "Harpsichord", 1 },
 {  7,  1, "Harpsichord (wide)", 1 },
 {  7, 25, "Harpsichord 2", 2 },
 {  7, 35, "Harpsichord 3", 2 },
 {  8,  0, "Clavi", 1 },
 {  8,  1, "Clavi (wide)", 1 },
 {  8, 27, "Clavi Wah", 2 },
 {  8, 64, "Pulse Clavi", 1 },
 {  8, 65, "Pierce Clavi", 2 },
 {  9,  0, "Celesta", 1 },
 { 10,  0, "Glockenspiel", 1 },
 { 11,  0, "Music Box", 2 },
 { 11, 64, "Orgel", 2 },
 { 12,  0, "Vibraphone", 1 },
 { 12,  1, "Vibraphone (wide)", 1 },
 { 12, 45, "Hard Vibraphone", 2 },
 { 13,  0, "Marimba", 1 },
 { 13,  1, "Marimba (wide)", 1 },
 { 13, 64, "Sine Marimba", 2 },
 { 13, 97, "Balimba", 2 },
 { 13, 98, "Log Drum", 2 },
 { 14,  0, "Xylophone", 1 },
 { 15,  0, "Tubular Bells", 1 },
 { 15, 96, "Church Bells", 2 },
 { 15, 97, "Carillon", 2 },
 { 16,  0, "Dulcimer", 1 },
 { 16, 35, "Dulcimer 2", 2 },
 { 16, 96, "Cimbalom", 2 },
 { 16, 97, "Santur", 2 },
 { 17,  0, "Drawbar Organ", 1 },
 { 17, 32, "Detuned Drawbar Organ", 2 },
 { 17, 33, "60's Drawbar Organ 1", 2 },
 { 17, 34, "60's Drawbar Organ 2", 2 },
 { 17, 35, "70's Drawbar Organ 1", 2 },
 { 17, 36, "Drawbar Organ 2", 2 },
 { 17, 37, "60's Drawbar Organ 3", 2 },
 { 17, 38, "Even Bar", 2 },
 { 17, 40, "16+2'2/3", 2 },
 { 17, 64, "Organ Bass", 1 },
 { 17, 65, "70's Drawbar Organ 2", 2 },
 { 17, 66, "Cheezy Organ", 2 },
 { 17, 67, "Drawbar Organ 3", 2 },
 { 18,  0, "Percussion Organ", 1 },
 { 18, 24, "70's Percussion Organ 1", 2 },
 { 18, 32, "Detuned Percussion Organ", 2 },
 { 18, 33, "Light Organ", 2 },
 { 18, 37, "Percussion Organ 2", 2 },
 { 19,  0, "Rock Organ", 1 },
 { 19, 64, "Rotary Organ", 2 },
 { 19, 65, "Slow Rotary Organ", 2 },
 { 19, 66, "Fast Rotary Organ", 2 },
 { 20,  0, "Church Organ", 2 },
 { 20, 32, "Church Organ 3", 2 },
 { 20, 35, "Church Organ 2", 2 },
 { 20, 40, "Notre Dame", 2 },
 { 20, 64, "Organ Flute", 2 },
 { 20, 65, "Tremolo Organ Flute", 2 },
 { 21,  0, "Reed Organ", 1 },
 { 21, 40, "Puff Organ", 2 },
 { 22,  0, "Acordion", 1 },
 { 22, 32, "Accordion Italian", 2 },
 { 23,  0, "Harmonica", 1 },
 { 23, 32, "Harmonica 2", 2 },
 { 24,  0, "Tango Acordion", 1 },
 { 24, 64, "Tango Acordion 2", 2 },
 { 25,  0, "Nylon Guitar", 1 },
 { 25, 16, "Nylon Guitar 2", 1 },
 { 25, 25, "Nylon Guitar 3", 2 },
 { 25, 43, "Velocity Guitar Harmonics", 1 },
 { 25, 96, "Ukulele", 1 },
 { 26,  0, "Steel Guitar", 1 },
 { 26, 16, "Steel Guitar 2", 1 },
 { 26, 35, "12-string Guitar", 2 },
 { 26, 40, "Nylon & Steel Guitar", 2 },
 { 26, 41, "Steel Guitar with Body Sound", 2 },
 { 26, 96, "Mandolin", 2 },
 { 27,  0, "Jazz Guitar", 1 },
 { 27, 18, "Mellow Guitar", 1 },
 { 27, 32, "Jazz Amp", 2 },
 { 27, 96, "Pedal Steel Guitar", 1 },
 { 28,  0, "Clean Guitar", 1 },
 { 28, 32, "Chorus Guitar", 2 },
 { 28, 65, "Midtone Guitar", 1 },
 { 28, 66, "Midtone Guitar (wide)", 2 },
 { 29,  0, "Muted Guitar", 1 },
 { 29, 40, "Funk Guitar 1", 2 },
 { 29, 41, "Muted Steel Guitar", 2 },
 { 29, 43, "Funk Guitar 2", 1 },
 { 29, 45, "Jazz Man", 2 },
 { 29, 96, "Muted Distorton Guitar", 2 },
 { 30,  0, "Overdrive Guitar", 1 },
 { 30, 43, "Guitar Pinch", 1 },
 { 31,  0, "Distortion Guitar", 1 },
 { 31, 12, "Distorted Rhythm Guitar", 2 },
 { 31, 24, "Distortion Guitar 2", 2 },
 { 31, 35, "Distortion Guitar 3", 2 },
 { 31, 36, "Power Guitar 2", 2 },
 { 31, 37, "Power Guitar 1", 2 },
 { 31, 38, "Distorted Fifths", 2 },
 { 31, 40, "Feedback Guitar", 2 },
 { 31, 41, "Feedback Guitar 2", 2 },
 { 31, 43, "Rock Rhythm Guitar 2", 2 },
 { 31, 45, "Rock Rhythm Guitar 1", 2 },
 { 32,  0, "Guitar Harmonics", 1 },
 { 32, 65, "Guitar Feedback", 1 },
 { 32, 66, "Guitar Harmonics 2", 1 },
 { 33,  0, "Acoustic Bass", 1 },
 { 33, 40, "Jazz Rhythm", 2 },
 { 33, 45, "Velocity Crossfade Upright Bass", 2 },
 { 34,  0, "Finger Bass", 1 },
 { 34, 18, "Finger Bass Dark", 2 },
 { 34, 27, "Flanger Bass", 2 },
 { 34, 40, "Bass & Distorted Electric Guitar", 2 },
 { 34, 43, "Finger Slap Bass", 1 },
 { 34, 45, "Finger Bass 2", 2 },
 { 34, 64, "Jazzy Bass", 1 },
 { 34, 65, "Modulated Bass", 2 },
 { 34,112, "FW-stringed Electric Bass", 1 },
 { 35,  0, "Pick Bass", 1 },
 { 35, 28, "Muted Pick Bass", 1 },
 { 36,  0, "Fretless Bass", 1 },
 { 36, 32, "Fretless Bass 2", 2 },
 { 36, 33, "Fretless Bass 3", 2 },
 { 36, 34, "Fretless Bass 4", 2 },
 { 36, 96, "Synth Fretless Bass", 2 },
 { 36, 97, "Smooth Fretless Bass", 2 },
 { 37,  0, "Slap Bass 1", 1 },
 { 37, 27, "Resonant Slap", 1 },
 { 37, 32, "Punch Thumb Bass", 2 },
 { 37, 64, "Slapper", 1 },
 { 37, 65, "Thumb & Slap", 1 },
 { 38,  0, "Slap Bass 2", 1 },
 { 38, 43, "Velocity Switch Bass", 1 },
 { 39,  0, "Synth Bass 1", 1 },
 { 39, 18, "Synth Bass 1 Dark", 1 },
 { 39, 20, "Fast Resonant Bass", 1 },
 { 39, 24, "Acid Bass", 1 },
 { 39, 27, "Resonant Bass", 1 },
 { 39, 35, "Clavi Bass", 2 },
 { 39, 40, "Techno Bass", 2 },
 { 39, 64, "Orbiter", 2 },
 { 39, 65, "Square Bass", 1 },
 { 39, 66, "Rubber Bass", 2 },
 { 39, 67, "Fish", 1 },
 { 39, 68, "Hard Resonant Bass", 1 },
 { 39, 96, "Hammer", 2 },
 { 39,112, "Analog Bass", 2 },
 { 39,113, "Rezo Bass", 2 },
 { 40,  0, "Synth Bass 2", 2 },
 { 40,  6, "Mellow Synth Bass", 1 },
 { 40, 12, "Sequenced Bass", 2 },
 { 40, 18, "Click Synth Bass", 2 },
 { 40, 19, "Synth Bass 2 Dark", 1 },
 { 40, 32, "Smooth Synth Bass", 2 },
 { 40, 40, "Modular Synth Bass", 2 },
 { 40, 41, "DX Bass", 2 },
 { 40, 64, "X Wire Bass", 2 },
 { 40, 65, "Attack Pulse", 1 },
 { 40, 66, "CS Light", 1 },
 { 40, 67, "Metal Bass", 1 },
 { 40,112, "Smooth Bass", 2 },
 { 40,113, "Octavedved Bass", 1 },
 { 40,114, "Power Bass", 4 },
 { 41,  0, "Violin", 1 },
 { 41,  8, "Slow Violin", 1 },
 { 42,  0, "Viola 1", },
 { 43,  0, "Cello 1", },
 { 44,  0, "Contrabass", 1 },
 { 45,  0, "Tremolo Strings", 1 },
 { 45,  8, "Slow Tremolo Strings", 1 },
 { 45, 40, "Suspension Strings", 2 },
 { 46,  0, "Pizzicato Strings", 1 },
 { 47,  0, "Harp", 1 },
 { 47, 40, "Yang Chin", 2 },
 { 48,  0, "Timpani", 1 },
 { 49,  0, "Strings 1", 1 },
 { 49,  3, "Strings (wide)", 2 },
 { 49,  8, "Slow Strings", 1 },
 { 49, 24, "Arco Strings", 2 },
 { 49, 35, "60's Strings", 2 },
 { 49, 40, "Orchestra", 2 },
 { 49, 41, "Orchestra 2", 2 },
 { 49, 42, "Tremolo Orchestra", 2 },
 { 49, 45, "Velocity Strings", 2 },
 { 50,  0, "Strings 2", 1 },
 { 50,  3, "Slow  Strings (wide)", 2 },
 { 50,  8, "Legato Strings", 2 },
 { 50, 40, "Warm Strings", 2 },
 { 50, 41, "Kingdom", 2 },
 { 50, 64, "70's Strings", 1 },
 { 50, 65, "Strings 3", 1 },
 { 51,  0, "Synth Strings 1", 2 },
 { 51, 27, "Resonant Synth Strings", 2 },
 { 51, 64, "Synth Strings 4", 2 },
 { 51, 65, "Synth Strings 5", 2 },
 { 52,  0, "Synth Strings 2", 2 },
 { 53,  0, "Choir Aahs", 1 },
 { 53,  3, "Choir (wide)", 2 },
 { 53, 16, "Choir Aahs 2", 2 },
 { 53, 32, "Mellow Choir", 2 },
 { 53, 40, "Choir Strings", 2 },
 { 54,  0, "Voice Ooh", 1 },
 { 55,  0, "Synth Voice", 1 },
 { 55, 40, "Synth Voice 2", 2 },
 { 55, 41, "Choral",2 },
 { 55, 64, "Analog Voice", 1 },
 { 56,  0, "Orchestra Hit", 2 },
 { 56, 35, "Orchestra Hit 2", 2 },
 { 56, 64, "Impact", 2 },
 { 56, 68, "Bass Hit", 1 },
 { 56, 70, "6th Hit", 1 },
 { 56, 71, "6th Hit Plus", 2 },
 { 56, 72, "Euro Hit", 1 },
 { 56, 73, "Euro Hit Plus", 2 },
 { 57,  0, "Trumpet", 1 },
 { 57, 16, "Trumpet 2", 1 },
 { 57, 17, "Bright Trumpet", 2 },
 { 57, 32, "Warm Trumpet", 2 },
 { 58,  0, "Trombone", 1 },
 { 58, 18, "Trombone 2", 2 },
 { 59,  0, "Tuba", 1 },
 { 59, 16, "Tuba 2", 1 },
 { 60,  0, "Muted Trumpet", 1 },
 { 61,  0, "French Horn", 1 },
 { 61,  6, "French Horn Solo", 1 },
 { 61, 32, "French Horn 2", 2 },
 { 61, 37, "Horn Orchestra", 2 },
 { 62,  0, "Brass Section", 1 },
 { 62,  3, "Brass Section (wide)", 2 },
 { 62, 35, "Trumpet & Trombone Section", 2 },
 { 62, 40, "Brass Section 2", 2 },
 { 62, 41, "Hit Brass", 2 },
 { 62, 42, "Mellow Brass", 2 },
 { 63,  0, "Synth Brass 1", 2 },
 { 63, 12, "Quack Brass", 2 },
 { 63, 20, "Resonant Synth Brass", 2 },
 { 63, 24, "Poly Brass", 2 },
 { 63, 27, "Synth  Brass 3", 2 },
 { 63, 32, "Jump Brass", 2 },
 { 63, 40, "Synth Brass with Sub Oscillator", 2 },
 { 63, 45, "Analog Velocity Brass 1", 2 },
 { 63, 64, "Analog Brass 1", 2 },
 { 64,  0, "Synth  Brass 2", 1 },
 { 64, 18, "Soft Brass", 2 },
 { 64, 40, "Synth  Brass 4", 2 },
 { 64, 41, "Choir Brass", 2 },
 { 64, 45, "Analog Velocity Brass 2", 2 },
 { 64, 64, "Analog Brass 2", 2 },
 { 65,  0, "Soparno Sax", 1 },
 { 66,  0, "Alto Sax", 1 },
 { 66, 40, "Sax Section", 2 },
 { 66, 43, "Hyper Alto Sax", 1 },
 { 67,  0, "Tenor Sax", 1 },
 { 67, 40, "Breathy Tenor Sax", 2 },
 { 67, 41, "Soft Tenor Sax", 2 },
 { 67, 64, "Tenor Sax 2", 1 },
 { 68,  0, "Baritone Sax", 1 },
 { 69,  0, "Oboe", 1 },
 { 70,  0, "English Horn", 1 },
 { 71,  0, "Bassoon", 1 },
 { 72,  0, "Clarinet", 1 },
 { 73,  0, "Piccolo", 1 },
 { 74,  0, "Flute", 1 },
 { 75,  0, "Recorder", 1 },
 { 76,  0, "Pan Flute", 1 },
 { 77,  0, "Blown Bottle", 2 },
 { 78,  0, "Shakuhachi", 1 },
 { 79,  0, "Whistle", 1 },
 { 80,  0, "Ocarina", 1 },
 { 81,  0, "Square Lead", 2 },
 { 81,  6, "Square Lead 2", 1 },
 { 81,  8, "LM Square", 2 },
 { 81, 18, "Hollow", 1 },
 { 81, 19, "Shroud", 2 },
 { 81, 64, "Mellow", 2 },
 { 81, 65, "Solo Sine", 2 },
 { 81, 66, "Sine Lead", 1 },
 { 81, 67, "Pulse Lead", 1 },
 { 82,  0, "Sawtooth Lead", 2 },
 { 82,  6, "Sawtooth Lead 2",  1 },
 { 82,  8, "Thick Sawtooth", 2 },
 { 82, 18, "Dynamic Sawtooth", 1 },
 { 82, 19, "Digital Sawtooth", 2 },
 { 82, 20, "Big Lead", 2 },
 { 82, 24, "Heavy Synth", 2 },
 { 82, 25, "Waspy Synth", 2 },
 { 82, 27, "Rezzy Sawtooth", 1 },
 { 82, 32, "Double Sawtooth", 2 },
 { 82, 35, "Toy Lead", 2 },
 { 82, 36, "Dim Sawtooth", 2 },
 { 82, 40, "Pulse Sawtooth", 2 },
 { 82, 41, "Dr. Lead", 2 },
 { 82, 45, "Velocity Lead", 2 },
 { 82, 64, "Digger", 1 },
 { 82, 96, "Sequenced Analog", 2 },
 { 83,  0, "Caliope Lead", 2 },
 { 83, 65, "Pure Pad", 2 },
 { 84,  0, "Chiff Lead", 2 },
 { 84, 64, "Rubby", 2 },
 { 84, 65, "Hard Sync", 1 },
 { 85,  0, "Charang Lead", 2 },
 { 85, 64, "Distorted Lead", 2 },
 { 85, 65, "Wire Lead", 2 },
 { 85, 66, "Synth Pluck", 1 },
 { 86,  0, "Voice Lead", 2 },
 { 86, 24, "Synth Aahs", 2 },
 { 86, 64, "Vox Lead", 2 },
 { 87,  0, "Fifth Lead", 2 },
 { 87, 35, "Big Five", 2 },
 { 88,  0, "Bass & Lead", 2 },
 { 88, 16, "Big & Low", 2 },
 { 88, 64, "Fat & Perky", 2 },
 { 88, 65, "Soft Wire Lead", 2 },
 { 89,  0, "New Age Pad", 2 },
 { 89, 64, "Fantasy", 2 },
 { 90,  0, "Warm Pad", 2 },
 { 90, 16, "Thick Pad", 2 },
 { 90, 17, "Soft Pad", 2 },
 { 90, 18, "Sine Pad", 2 },
 { 90, 64, "Horn Pad", 2 },
 { 90, 65, "Rotar Strings", 2 },
 { 91,  0, "Poly Synth Pad", 2 },
 { 91, 64, "Poly Pad 80", 2 },
 { 91, 65, "Click Pad", 2 },
 { 91, 66, "Analog Pad", 2 },
 { 91, 67, "Square Pad", 2 },
 { 92,  0, "Choir Pad", 2 },
 { 92, 64, "Heaven", 2 },
 { 92, 66, "Itopia", 2 },
 { 92, 67, "CC Pad", 2 },
 { 93,  0, "Bowed Pad", 2 },
 { 93, 64, "Glacier", 2 },
 { 93, 65, "Glass Pad", 2 },
 { 94,  0, "Metal Pad", 2 },
 { 94, 64, "Tine Pad", 2 },
 { 94, 65, "Pan Pad", 2 },
 { 95,  0, "Halo Pad", 2 },
 { 96,  0, "Sweep Pad", 2 },
 { 96, 20, "Shwimmer", 2 },
 { 96, 27, "Converge", 2 },
 { 96, 64, "Polar Pad", 2 },
 { 96, 66, "Celstial", 2 },
 { 97,  0, "Rain", 2 },
 { 97, 45, "Clavi  Pad", 2 },
 { 97, 64, "Harmo Rain", 2 },
 { 97, 65, "African Wind", 2 },
 { 97, 66, "Carib", 2 },
 { 98,  0, "Sound Track", 2 },
 { 98, 27, "Prologue", 2 },
 { 98, 64, "Ancestrial", 2 },
 { 99,  0, "Crystal", 2 },
 { 99, 12, "Synth Drum Comp", 2 },
 { 99, 14, "Popcorn", 2 },
 { 99, 18, "Tiny Bells", 2 },
 { 99, 35, "Round Glockenspiel", 2 },
 { 99, 40, "Glockenspiel Chime", 2 },
 { 99, 41, "Clear Bells", 2 },
 { 99, 42, "Chorused Bells", 2 },
 { 99, 64, "Synth Mallet", 1 },
 { 99, 65, "Soft Crystal", 2 },
 { 99, 66, "Loud Glockenspiel", 2 },
 { 99, 67, "Christmas Bells", 2 },
 { 99, 68, "Vibraphone Bells", 2 },
 { 99, 69, "Digital Bells", 2 },
 { 99, 70, "Air Bells", 2 },
 { 99, 71, "Bell Harp", 2 },
 { 99, 72, "Gamelimba", 2 },
 { 100,  0,"Atmosphere", 2 },
 { 100, 18, "Warm Atmosphere", 2 },
 { 100, 19, "Hollow Release", 2 },
 { 100, 40, "Nylon Electric Piano", 2 },
 { 100, 64, "Nylon Harp", 2 },
 { 100, 65, "Harp Vox", 2 },
 { 100, 66, "Atmos Pad", 2 },
 { 100, 67, "Planet", 2 },
 { 101,  0, "Bright", 2 },
 { 101, 64, "Fantasia Bells", 2 },
 { 101, 96, "Smokey", 2 },
 { 102,  0, "Goblins", 2 },
 { 102, 64, "Goblins Synth", 2 },
 { 102, 65, "Creeper", 2 },
 { 102, 66, "Ring Pad", 2 },
 { 102, 67, "Ritual", 2 },
 { 102, 68, "To Heaven", 2 },
 { 102, 70, "Night", 2 },
 { 102, 71, "Glisten", 2 },
 { 102, 96, "Bell Choir", 2 },
 { 103,  0, "Echoes", 2 },
 { 103,  8, "Echoes 2", 2 },
 { 103, 14, "Echo Pan", 2 },
 { 103, 64, "Echo Bells", 2 },
 { 103, 65, "Big Pan", 2 },
 { 103, 66, "Synth Piano", 2 },
 { 103, 67, "Creation", 2 },
 { 103, 68, "Star Dust", 2 },
 { 103, 69, "Resonant & Panning", 2 },
 { 104,  0, "Sci-Fi", 2 },
 { 104, 64, "Starz", 2 },
 { 105,  0, "Sitar", 1 },
 { 105, 32, "Detuned Sitar", 2 },
 { 105, 35, "Sitar 2", 2 },
 { 105, 96, "Tambra", 2 },
 { 105, 97, "Tamboura", 2 },
 { 106,  0, "Banjo", 1 },
 { 106, 28, "Muted Banjo", 1 },
 { 106, 96, "Rabab", 2 },
 { 106, 97, "Gopichant", 2 },
 { 106, 98, "Oud", 2 },
 { 107,  0, "Shamisen", 1 },
 { 108,  0, "Koto", 1 },
 { 108, 96, "Taisho-Kin", 2 },
 { 108, 97, "Kanoon", 2 },
 { 109,  0, "Kalimba", 1 },
 { 110,  0, "Bagpipe", 2 },
 { 111,  0, "Fiddle", 1 },
 { 112,  0, "Shanai", 1 },
 { 112, 64, "Shanai 2", 1 },
 { 112, 96, "Pungi", 1 },
 { 112, 97, "Hichriki", 2 },
 { 113,  0, "Tinkle Bell", 2 },
 { 113, 96, "Bonang", 2 },
 { 113, 97, "Altair", 2 },
 { 113, 98, "Gamelan Gongs", 2 },
 { 113, 99, "Gamelan Gongs (wide)", 2 },
 { 113,100, "Rama Cymbal", 2 },
 { 113,101, "Asian Bells", 2 },
 { 114,  0, "Agogo", 1 },
 { 115,  0, "Steel Drum", 1 },
 { 115, 97, "Glass Percussion", 2 },
 { 115, 98, "Thai Bells", 2 },
 { 116,  0, "Woodblock", 1 },
 { 116, 96, "Castanets", 1 },
 { 117,  0, "Taiko Drum", 1 },
 { 117, 96, "Gran Cassa", 1 },
 { 118,  0, "Melodic Tom", 1 },
 { 118, 64, "Melodic Tom 2", 1 },
 { 118, 65, "Real Tom", 2 },
 { 118, 66, "Rock Tom", 2 },
 { 119,  0, "Synth Drum", 1 },
 { 119, 64, "Analog Tom", 1 },
 { 119, 65, "Electronic Percussion", 2 },
 { 120,  0, "Reverse Cymbal", 1 },
 { 121,  0, "Fret Noise", 1 },
 { 122,  0, "Breath Noise", 1 },
 { 123,  0, "Seashore", 2 },
 { 124,  0, "Tweet", 2 },
 { 125,  0, "Telphone", 1 },
 { 126,  0, "Helicopter", 1 },
 { 127,  0, "Applausse", 1 },
 { 128,  0, "Gunshot", 1 },
 { 128, 65, "Phone Call", 1 },
 { 128, 66, "Door Squeek", 1 },
 { 128, 67, "Door Slam", 1 },
 { 128, 68, "Scratch Cut", 1 },
 { 128, 69, "Scratch Split", 1 },
 { 128, 70, "Wind Chime", 1 },
 { 128, 71, "Telephone Ring 2", 1 },
 { 128, 81, "Car Engine Ignition", 1 },
 { 128, 82, "Car Tires Squeel", 1 },
 { 128, 83, "Car Passing", 1 },
 { 128, 84, "Car Crash", 1 },
 { 128, 85, "Siren", 2 },
 { 128, 86, "Train", 1 },
 { 128, 87, "Jet Plane", 2 },
 { 128, 88, "Starship", 2 },
 { 128, 89, "Burst", 2 },
 { 128, 90, "Coaster", 2 },
 { 128, 91, "Submarine", 1 },
 { 128, 97, "Laugh", 1 },
 { 128, 98, "Scream", 1 },
 { 128, 99, "Punch", 1 },
 { 128, 100, "Heartbeat", 1 },
 { 128, 101, "Footstep", 1 },
 { 128, 113, "Machine Gun", 1 },
 { 128, 114, "Laser Gun", 2 },
 { 128, 115, "Explosion", 2 },
 { 128, 116, "FireWork", 2 },
 { 0, 0,"", 0 }
};


static char filename[256];
char *lowercase(const char *name)
{
   int i, j ,len = strlen(name);
   if (len > 256) len = 255;
 
   j = 1;
   filename[0] = tolower(name[0]);
   for (i=1; i<len; ++i)
   {
      if (name[i] != '(' && name[i] != ')')
      {
         if (name[i] == ' ') filename[j] = '-';
         else filename[j] = tolower(name[i]);
         j++;
      }
   }
   filename[j] = 0;
    return filename;
}

int main(int argc, char **argv)
{
   const char *pname = strrchr(argv[0], '/');
   int b;

   if (!pname) pname = argv[0];
   else pname++;

   printf("<?xml version=\"1.0\"?>\n\n");
   printf("<!--\n * Auto generated by %s\n", pname);
   printf(" *\n * Instrument list for XG-MIDI\n");
   printf("-->\n");
   printf("<aeonwave>\n\n");
   printf(" <midi name=\"XG-MIDI\">\n\n");

   for (b=0; b<128; ++b)
   {
      char f = 0;
      int i = 0;
      do
      {
         if (inst_table[i].lsb == b )
         {
            if (!f)
            {
               printf("  <bank n=\"0\" l=\"%i\">\n", b);
               f = 1;
            }

            printf("   <instrument n=\"%i\" name=\"%s\"",
                    inst_table[i].program-1, inst_table[i].name);
 
            printf(" file=\"instruments/%s\"", lowercase(inst_table[i].name));

            if (strstr(inst_table[i].name, "(wide)")) {
               printf(" wide=\"true\"");
            }
 
            printf("/>\n");
         }
      } while (inst_table[++i].program);
      if (f) printf("  </bank>\n\n");
   }

   printf(" </midi>\n\n");
   printf("</aeonwave>\n");

  return 0;
}
  
