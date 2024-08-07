#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include <aax/midi.h>

static float XGMIDI_LFO_frequency_table_Hz[128] = {	// Hz
 0.f, 0.08f, 0.08f, 0.16f, 0.16f, 0.25f, 0.25f, 0.33f, 0.33f, 0.42f, 0.42f,
 0.5f, 0.5f, 0.58f, 0.58f, 0.67f, 0.67f, 0.75f, 0.75f, 0.84, 0.84f, 0.92f,
 0.92f, 1.f, 1.f, 1.09f, 1.09f, 1.17f, 1.17f, 1.26f, 1.26f, 1.34f, 1.34f,
 1.43f, 1.43f, 1.51f, 1.51f, 1.59f, 1.59f, 1.68f, 1.68f, 1.76f, 1.76f, 1.85f,
 1.85f, 1.93f, 1.93f, 2.01f, 2.01f, 2.1f, 2.1f, 2.18f, 2.18f, 2.27f, 2.27f,
 2.35f, 2.35f, 2.43f, 2.43f, 2.52f, 2.52f, 2.6f, 2.6f, 2.69f, 2.69f, 2.77f,
 2.86f, 2.94f, 3.02f, 3.11f, 3.19f, 3.28f, 3.36f, 3.44f, 3.53f, 3.61f, 3.7f,
 3.86f, 4.03f, 4.2f, 4.37f, 4.54f, 4.71f, 4.87f, 5.04f, 5.21f, 5.38f, 5.55f,
 5.72f, 6.05f, 6.39f, 6.72f, 7.06f, 7.4f, 7.73f, 8.07f, 8.1f, 8.74f, 9.08f,
 9.42f, 8.75f, 10.f, 11.4f, 12.1f, 12.7f, 13.4f, 14.1f, 14.8f, 15.4f, 16.1f,
 16.8f, 17.4f, 18.1f, 19.5f, 20.8f, 22.2f, 23.5f, 24.8f, 26.2f, 27.5f, 28.9f,
 30.2f, 31.6f, 32.9f, 34.3f, 37.f, 39.7f
};

static float XGMIDI_delay_offset_table_ms[128] = {	// ms
 0.f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f, 1.1f, 1.2f,
 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f, 2.f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f,
 2.6f, 2.7f, 2.8f, 2.9f, 3.f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f,
 3.9f, 4.f, 4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f, 5.f, 5.1f,
 5.2f, 5.3f, 5.4f, 5.5f, 5.6f, 5.7f, 5.8f, 5.9f, 6.f, 6.1f, 6.2f, 6.3f, 6.4f,
 6.5f, 6.6f, 6.7f, 6.8f, 6.9f, 7.f, 7.1f, 7.2f, 7.3f, 7.4f, 7.5f, 7.6f, 7.7f,
 7.8f, 7.9f, 8.f, 8.1f, 8.2f, 8.3f, 8.4f, 8.5f, 8.6f, 8.7f, 8.8f, 8.9f, 9.f,
 9.1f, 9.2f, 9.3f, 9.4f, 9.5f, 9.6f, 9.7f, 9.8f, 9.9f, 10.f, 11.1f, 12.2f,
 13.3f, 14.4f, 15.5f, 17.1f, 18.6f, 20.2f, 21.8f, 23.3f, 24.9f, 26.5f, 28.f,
 29.6f, 31.2f, 32.8f, 34.3f, 35.9f, 37.f, 39.f, 40.6f, 42.2f, 43.7f, 45.3f,
 46.9f, 48.4f, 50.f
};

static float XGMIDI_delay_time_table_200ms[128] = {	// ms
 0.1f, 1.7f, 3.2f, 4.8f, 6.4f, 8.0f, 9.5f, 11.1f, 12.7f, 14.3f, 15.8f, 17.4f,
 19.f, 2.6f, 22.1f, 23.7f, 25.3f, 26.9f, 28.4f, 30.f, 31.6f, 33.2f, 34.7f,
 36.3f, 37.9f, 39.5f, 41.f, 42.6f, 44.2f, 45.7f, 47.3f, 48.9f, 50.5f, 52.f,
 53.6f, 55.2f, 56.8f, 58.3f, 59.9f, 61.5f, 63.1f, 64.4f, 66.2f, 67.8f, 69.4f,
 70.9f, 72.5f, 74.1f, 75.7f, 77.2f, 78.8f, 80.4f, 81.9f, 83.5f, 85.1f, 86.7f,
 88.2f, 89.8f, 91.4f, 93.f, 94.5f, 96.1f, 97.7f, 99.3f, 100.8f, 102.4f, 104.f,
 105.6f, 107.1f, 108.7f, 110.3f, 111.9f, 113.4f, 115.f, 116.6f, 118.2f, 119.7f,
 121.3f, 122.9f, 124.4f, 126.f, 127.6f, 129.2f, 130.7f, 132.3f, 133.9f, 135.5f,
 137.f, 138.6f, 1402.f, 141.8f, 143.3f, 144.9f, 146.5f, 148.1f, 149.6f, 151.2f,
 152.8f, 154.4f, 155.9f, 157.5f, 159.1f, 160.6f, 162.2f, 163.8f, 165.4f,
 166.9f, 168.5f, 170.1f, 171.7f, 173.2f, 174.8f, 176.4f, 178.f, 179.5f, 181.1f,
 182.7f, 184.3f, 185.8f, 187.4f, 189.f, 190.6f, 192.1f, 193.7f, 195.3f, 169.6f,
 198.4f, 200.f
};

static float XGMIDI_EQ_frequency_table_Hz[61] = {	// Hz
 20.f, 22.f, 25.f, 28.f, 32.f, 36.f, 40.f, 45.f, 50.f, 56.f, 63.f, 70.f, 80.f,
 90.f, 100.f, 110.f, 125.f, 140.f, 160.f, 180.f, 200.f, 225.f, 250.f, 280.f,
 315.f, 355.f, 400.f, 450.f, 500.f, 560.f, 630.f, 700.f, 800.f, 900.f, 1000.f,
 1100.f, 1200.f, 1400.f, 1600.f, 1800.f, 2000.f, 2200.f, 2500.f, 2800.f,
 3200.f, 3600.f, 4000.f, 4500.f, 5000.f, 5600.f, 6300.f, 7000.0f, 8000.f,
 9000.f, 10000.f, 11000.0f, 12000.f, 14000.f, 16000.f, 18000.f, 20000.f
};

#if 0
static float XGMIDI_room_size_table_m[45] = {		// m
 0.1f, 0.3f, 0.4f, 0.6f, .7f, 0.9f, 1.f, 1.2f, 1.4f, 1.5f, 1.7f, 1.8f, 2.f,
 2.1f, 2.3f, 2.5f, 2.6f, 2.8f, 2.9f, 3.1f, 4.2f, 3.4f, 3.5f, 3.7f, 3.9f, 4.f,
 4.2f, 4.3f, 4.5f, 4.6f, 4.8f, 5.f, 5.1f, 5.3f, 5.4f, 5.6f, 5.7f, 5.9f, 6.1f,
 6.2f, 6.4f, 6.5f, 6.7f, 6.8f, 7.f
};

static float XGMIDI_compressor_attack_time_table_ms[20] = { // ms
 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 12.f, 14.f, 16.f, 18.f,
 20.f, 23.f, 26.f, 30.f, 35.f, 40.f
};

static float XGMIDI_compressor_release_time_table_ms[16] = { // ms
 10.f, 15.f, 25.f, 35.f, 45.f, 55.f, 65.f, 75.f, 85.f, 100.f, 115.f, 140.f,
 170.f, 230.f, 340.f, 680.f
};

static float XGMIDI_compressor_ratio_table[8] = {
 1.f, 1.5f, 2.f, 3.f, 5.f, 7.f, 10.f, 20.0f
};
#endif

static float XGMIDI_reverb_time_sec[70] = {		// s
 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f,
 1.6f, 1.7f, 1.8f, 1.9f, 2.f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f,
 2.9f, 3.f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f, 4.f, 4.1f,
 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f, 5.f, 5.5f, 6.f, 6.5f, 7.f,
 7.5f, 8.f, 8.5f, 9.f, 9.5f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f, 17.f,
 18.f, 19.f, 20.f, 25.f, 30.f
};

static float XGMIDI_reverb_dimensions_m[105] = {		// m
 0.5f, 0.8f, 1.f, 1.3f, 1.5f, 1.8f, 2.f, 2.3f, 2.6f, 2.8f, 3.1f, 3.3f, 3.6f,
 3.9f, 4.1f, 4.4f, 4.6f, 4.9f, 5.2f, 5.4f, 5.7f, 5.9f, 6.2f, 6.5f, 6.7f, 7.f,
 7.2f, 7.5f, 7.8f, 8.f, 8.3f, 8.6f, 8.8f, 9.1f, 9.4f, 9.6f, 9.9f, 10.2f, 10.4f,
 10.7f, 11.f, 11.2f, 11.5f, 11.8f, 12.1f, 12.3f, 12.6f, 12.9f, 13.1f, 3.4f,
 14.f, 14.2f, 14.5f, 14.8f, 15.1f, 15.4f, 15.6f, 15.9f, 16.2f, 16.5f, 16.8f,
 17.1f, 17.3f, 17.6f, 17.9f, 18.2f, 18.5f, 18.8f, 19.1f, 19.4f, 19.7f, 2.0f,
 20.5f, 20.8f, 21.1f, 22.f, 22.4f, 22.7f, 23.f, 23.3f, 23.6f, 23.9f, 24.2f,
 24.5f, 24.9f, 25.2f, 25.5f, 25.8f, 27.1f, 27.5f, 27.8f, 28.1f, 28.5f, 28.8f,
 29.2f, 29.5f, 29.9f, 30.2f
};

#if 0
static float XGMIDI_delay_time_400ms[128] = {		// ms
 0.1f, 3.2f, 6.4f, 9.5f, 12.7f, 15.8f, 19.f, 22.1f, 25.3f, 28.4f, 31.6f, 34.7f,
 37.9f, 41.f, 442.f, 47.3f, 50.5f, 53.6f, 56.8f, 59.9f, 63.1f, 66.2f, 69.4f,
 72.5f, 75.7f, 78.8f, 82.f, 85.1f, 88.3f, 91.4f, 94.6f, 97.7f, 100.9f, 104.f,
 107.2f, 110.3f, 113.5f, 116.6f, 119.8f, 122.9f, 126.1f, 129.2f, 132.4f, 135.5f,
 138.6f, 141.8f, 144.9f, 148.1f, 151.2f, 154.4f, 157.5f, 160.7f, 163.8f, 167.f,
 170.1f, 173.3f, 176.4f, 179.6f, 182.7f, 185.9f, 189.f, 192.2f, 195.3f, 198.5f,
 201.6f, 204.8f, 207.9f, 211.1f, 214.2f, 217.4f, 220.5f, 223.7f, 226.8f, 230.f,
 233.1f, 236.3f, 239.4f, 242.6f, 245.7f, 248.9f, 252.f, 255.2f, 258.3f, 261.5f,
 264.6f, 267.7f, 270.9f, 274.f, 277.2f, 280.3f, 283.5f, 286.6f, 289.8f, 292.9f,
 296.1f, 299.2f, 302.4f, 305.5f, 308.7f, 311.8f, 315.f, 318.1f, 321.3f, 324.4f,
 327.6f, 330.7f, 333.9f, 337.f, 340.2f, 343.3f, 346.5f, 349.6f, 352.8f, 355.9f,
 359.1f, 362.2f, 365.4f, 368.5f, 371.7f, 374.8f, 378.f, 381.1f, 384.3f, 387.4f,
 390.6f, 393.7f, 396.9f, 400.f
};
#endif

static struct {
   int num;
   const char *file;
   const char *display;
   const char *name;
} XGMIDI_style_list[129] = {
 {   1, "rock", "Rock Mixture", "Hardcore Mixture" },
 {   2, "synthpop", "80sMixt", "80's Mixture Rock" },
 {   3, "punk", "HardCore", "Hard Core Punk" },
 {   4, "rock", "MeloCore", "Melodious Core" },
 {   5, "rock", "SkaCore", "Ska Core" },
 {   6, "rock", "RkBoogie", "Rock Boogie" },
 {   7, "grunge", "GrungeRk", "Grunge Rock" },
 {   8, "rock", "MondoRk", "Mondo Rock" },
 {   9, "synthpop", "IrishRk", "80's Irish Rock" },
 {  10, "synthpop", "BritRock", "British Rock" },
 {  11, "rock", "RckShufl", "80's Rock Shuffle" },
 {  12, "pop", "CollgeRk", "College Rock" },
 {  13, "synthpop", "GlamRock", "Glam Rock" },
 {  14, "classic-rock", "70sRock", "70's Rock" },
 {  15, "classic-rock", "FolkRock", "70's 8-beat Folk Rock" },
 {  16, "classic-rock", "ArtRock", "70's Art Rock" },
 {  17, "punk", "PunkRock", "70's Punk Rock" },
 {  18, "rock", "PubRock", "Pub Rock" },
 {  19, "funk", "FunkRock", "Funk Rock" },
 {  20, "rock", "LatinRk", "Latin Rock" },
 {  21, "rock-n-roll", "60sHard", "60's Hard Rock" },
 {  22, "hardrock", "70sHard1", "70's Hard Rock1" },
 {  23, "hardrock", "70sHard2", "70's Hard Rock2" },
 {  24, "hardrock", "70sHR&R", "70's Hard Rock 'n Roll" },
 {  25, "hardrock", "16btHard", "16-beat Hard Rock" },
 {  26, "hardrock", "AmercHR1", "American Hard Rock1" },
 {  27, "hardrock", "AmercHR2", "American Hard Rock2" },
 {  28, "gothic", "ProgHard", "90's Progressive Hard" },
 {  29, "metal", "SpeedMtl", "Speed Metal" },
 {  30, "metal", "PowerMtl", "Power Metal" },
 {  31, "metal", "Thrash", "Thrash Metal" },
 {  32, "metal", "DoomMtl", "Doom Metal" },
 {  33, "metal", "MtlBoogy", "Metal Boogie" },
 {  34, "rnb", "Hip Hop1", "Hip Hop1" },
 {  35, "rnb", "Hip Hop2", "Hip Hop2" },
 {  36, "rnb", "Pop Hip", "Pop Hip Hop" },
 {  37, "rnb", "Gangsta", "Gangsta" },
 {  38, "rnb", "Rap", "Rap" },
 {  39, "rnb", "Jazz Hip", "Jazz Hip Hop" },
 {  40, "jazz-combo", "DanceSwg", "Dance Swing" },
 {  41, "house", "House", "House" },
 {  42, "house", "GrgHouse", "Garage House" },
 {  43, "jazz", "R&BSwing", "90's RnB Swing" },
 {  44, "rnb", "Slow Jam", "90's RnB Slow Jam" },
 {  45, "rnb", "Pop RnB", "90's Pop RnB" },
 {  46, "ballad", "Smooth" "90's RnB Smooth" },
 {  47, "house", "PopTekno", "Pop Techno" },
 {  48, "house", "EuroTek", "Euro Techno" },
 {  49, "dance", "Eurobeat", "Eurobeat" },
 {  50, "electronic", "ElectrRk", "Electro Rock" },
 {  51, "house", "Bigbeat", "Bigbeat" },
 {  52, "dance", "DigiRk1", "Digital Rock1" },
 {  53, "dance", "DigiRk2", "Digital Rock2" },
 {  54, "dance", "IndustRk", "Industrial Rock" },
 {  55, "psychedelic", "PsychRk", "Psychedelic Rock" },
 {  56, "pop", "Lite Pop", "Light Pop" },
 {  57, "pop", "AOR Pop", "A.O.R. Pop" },
 {  58, "pop", "LatinPop", "Latin Pop" },
 {  59, "synthpop", "BritPop", "80's British Pop" },
 {  60, "pop", "16bt Pop", "16-beat Pop" },
 {  61, "pop", "24bt Pop", "24-beat Pop" },
 {  62, "fusion", "TechFusn", "80's Technical Fusion" },
 {  63, "pop", "DetroPop", "Detroit Pop Shuffle" },
 {  64, "pop", "Mid8btRk", "Med-tempo 8-beat Rock Pop" },
 {  65, "pop", "Acoustic", "Acoustic Pop" },
 {  66, "rnb", "RnB", "RnB" },
 {  67, "rnb", "6/8 RnB", "6/8 RnB" },
 {  68, "soul", "SoulShf", "Soul Shuffle" },
 {  69, "rnb", "Motown", "Motown" },
 {  70, "blues", "SlwBlues", "Slow Blues" },
 {  71, "rnb", "RnB Wlz", "RnB Waltz" },
 {  72, "rnb", "Rock RnB", "Rock RnB" },
 {  73, "rock-n-roll", "RocknRol", "Rock'n' Roll" },
 {  74, "rock", "TrainTm", "Train Time" },
 {  75, "rockabilly", "Rockably", "Rockabilly" },
 {  76, "room", "Oldies", "Oldies" },
 {  77, "pop", "Liverpol", "Liverpool Pop" },
 {  78, "surf", "SurfRock", "Surf Rock" },
 {  79, "disco", "DscFunk", "Disco Funk" },
 {  80, "disco", "70sDisco", "70's Disco" },
 {  81, "funk", "FP Funk", "FP Funk" },
 {  82, "funk", "JB Funk", "JB Funk" },
 {  83, "funk", "JazzFunk", "Jazz Funk" },
 {  84, "jazz-combo", "ComboJz", "Combo Jazz" },
 {  85, "bigband", "Big Band", "Big Band Jazz" },
 {  86, "ballad", "JazBalld", "Jazz Ballad" },
 {  87, "jazz-combo", "JazWaltz", "Jazz Waltz" },
 {  88, "jazz", "Bebop Fast", "Bebop" },
 {  89, "jazz", "CoolJazz", "Cool Jazz" },
 {  90, "jazz", "AfroJazz", "Afro Jazz" },
 {  91, "ballad", "OrgBalld", "Organ Ballad" },
 {  92, "ballad", "PnoBalld", "Piano Ballad" },
 {  93, "ballad", "ArpBalld", "Arpeggio Ballad" },
 {  94, "ballad", "LatinBld", "Latin Ballad" },
 {  95, "ballad", "6/8Balld", "6/8 Modern Ballad" },
 {  96, "ballad", "HR Balld", "Hard Rock Ballad" },
 {  97, "ballad", "6/8HRBld", "6/8 Hard Rock Ballad" },
 {  98, "country", "CW Rock", "Country Rock" },
 {  99, "country", "CW Pop", "Country Pop" },
 { 100, "country", "16Contry", "16-beat Country Rock" },
 { 101, "ballad", "CW Balld", "Country Ballad" },
 { 102, "country", "CW Waltz", "Country Waltz" },
 { 103, "country", "Bluegras", "Bluegrass" },
 { 104, "latin", "Samba", "Samba" },
 { 105, "latin", "Mambo", "Mambo" },
 { 106, "latin", "Rhumba", "Rhumba" },
 { 107, "latin", "Merengue", "Merengue" },
 { 108, "latin", "Cha Cha", "Cha Cha" },
 { 109, "latin", "Salsa", "Salsa" },
 { 110, "latin", "Bossa", "Bossa Nova" },
 { 111, "latin", "Beguin", "Beguin" },
 { 112, "latin", "Tango", "Tango" },
 { 113, "reggae", "reggae", "reggae" },
 { 114, "reggae", "SwReggae", "Swing Reggae" },
 { 115, "reggae", "DHReggae", "Dance Hall Reggae" },
 { 116, "reggae", "LoversRk", "Lovers Rock" },
 { 117, "reggae", "Ska", "Ska" },
 { 118, "world", "Hawaiian", "Hawaiian" },
 { 119, "world", "Soca", "Soca" },
 { 120, "world", "Klezmer", "Klezmer" },
 { 121, "world", "Enka", "Enka" },
 { 122, "world", "Polka", "Polka" },
 { 123, "jazz-combo", "Dixie", "Dixieland" },
 { 124, "world", "Foxtrot", "Foxtrot" },
 { 125, "world", "VienaWlz", "Vienna Waltz" },
 { 126, "world", "SlowWalz", "Slow Waltz" },
 { 127, "world", "March", "March" },
 { 128, "world", "6/8March", "6/8 March" },
 { 0, NULL, NULL, NULL }
};

typedef struct {
    const char* name;
    int param[16];
} XGMIDI_effect_t;

/* REVERB
 *  p	description	range
 *  --	-----------	---------
 *  0	Reverb Time	0.3 ~ 30.0 sec
 *  1	Diffusiion	0 ~ 10
 *  2	Initial Delay	0 ~ 63
 *  3	HPF Cutoff	Thru ~ 8kHz
 *  4	LPF Cutoff	1kHz ~ Thru
 *  5	Width		0.5 ~ 10.2 meter
 *  6	Height		0.5 ~ 20.2 meter
 *  7	Depth		0.5 ~ 30.2 meter
 *  8	Wall Vary	0 ~ 30
 *  9	Dry/Wet		D63>W ~ D=W ~ D<W63
 * 10	Rev Delay	0 ~ 63
 * 11	Density		0 ~ 4
 * 12	Rev/Er Balance	R<E63 ~ R=E ~ R63>E
 * 13	High Damp	0.1 ~ 1.0
 * 14	Feedback Level	-63 ~ +63
 */
#define XGMIDI_MAX_REVERB_TYPES         12
static XGMIDI_effect_t XGMIDI_reverb_types[XGMIDI_MAX_REVERB_TYPES] = {
//      param:     0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
 { "hall1",     { 18, 10,  8, 13, 49,  0,  0,  0,  0, 40,  0,  4, 50,  8, 64,  0 } },
 { "hall2",     { 25, 10, 28,  6, 46,  0,  0,  0,  0, 40, 13,  3, 74,  7, 64,  0 } },
 { "room1",     {  5, 10, 16,  4, 49,  0,  0,  0,  0, 40,  5,  3, 64,  8, 64,  0 } },
 { "room2",     { 12, 10,  5,  4, 38,  0,  0,  0,  0, 40,  0,  4, 50,  8, 64,  0 } },
 { "room3",     {  9, 10, 47,  5, 36,  0,  0,  0,  0, 40,  0,  4, 60,  8, 64,  0 } },
 { "stage1",    { 19, 10, 16,  7, 54,  0,  0,  0,  0, 40,  0,  3, 64,  6, 64,  0 } },
 { "stage2",    { 11, 10, 16,  7, 51,  0,  0,  0,  0, 40,  2,  2, 64,  6, 64,  0 } },
 { "plate",     { 25, 10,  6,  8, 49,  0,  0,  0,  0, 40,  2,  3, 64,  5, 64,  0 } },
 { "whiteroom", {  9,  5, 11,  0, 46, 30, 50, 70,  7, 40, 34,  4, 64,  7, 64,  0 } },
 { "tunnel",    { 48,  6, 19,  0, 44, 33, 52, 70, 16, 40, 20,  4, 64,  7, 64,  0 } },
 { "canyon",    { 59,  6, 63,  0, 45, 34, 62, 91, 13, 40, 25,  4, 64,  4, 64,  0 } },
 { "basement",  {  3,  6,  3,  0, 34, 26, 29, 59, 15, 40, 32,  4, 64,  8, 64,  0 } }
};


/* CHORUS
 *  p   description        range                  value
 *  --  -----------        ---------              ------
 *   1  LFO Frequency      0.00 – 39.7Hz          0-127 #1
 *   2  LFO PM Depth       0 – 127                0-127
 *   3  Feedback Level     -63 – +63              1-127
 *   4  Delay Offset       0 – 127                0-127 #2
 *   5
 *   6  EQ Low Frequency   50Hz – 2.0kHz          8-40 #3
 *   7  EQ Low Gain        -12 – +12dB           52-76
 *   8  EQ High Frequency  500Hz – 16.0kHz       28-58 #3
 *   9  EQ High Gain       -12 – +12dB           52-76
 *  10  Dry/Wet            D63>W – D=W – D<W63    1-127
 *  11
 *  12
 *  13
 *  14
 *  15  Input Mode mono/stereo 0-1
 */
#define XGMIDI_MAX_CHORUS_TYPES         16
static XGMIDI_effect_t XGMIDI_chorus_types[XGMIDI_MAX_CHORUS_TYPES] = {
//      param:    1    2    3    4  5   6   7   8   9   10  11  12  13 14 15 16
 { "chorus1",  {  6,  54,  77, 106, 0, 28, 64, 46, 64,  64, 46, 64, 10, 0, 0, 0 } },
 { "chorus2",  {  8,  63,  64,  30, 0, 28, 62, 42, 58,  64, 46, 64, 10, 0, 0, 0 } },
 { "chorus3",  {  4,  44,  64, 110, 0, 28, 64, 46, 66,  64, 46, 64, 10, 0, 0, 0 } },
 { "chorus4",  {  9,  32,  69, 104, 0, 28, 64, 46, 64,  64, 46, 64, 10, 0, 1, 0 } },
 { "celeste1", { 12,  32,  64,   0, 0, 28, 64, 46, 64, 127, 40, 68, 10, 0, 0, 0 } },
 { "celeste2", { 28,  18,  90,   2, 0, 28, 62, 42, 60,  84, 40, 68, 10, 0, 0, 0 } },
 { "celeste3", {  4,  63,  44,   2, 0, 28, 64, 46, 68, 127, 40, 68, 10, 0, 0, 0 } },
 { "celeste4", {  8,  29,  64,   0, 0, 28, 64, 51, 66, 127, 40, 68, 10, 0, 1, 0 } },
 { "flanger1", { 14,  14, 104,   2, 0, 28, 64, 46, 64,  96, 40, 64, 10, 4, 0, 0 } },
 { "flanger2", { 32,  17,  26,   2, 0, 28, 64, 46, 60,  96, 40, 64, 10, 4, 0, 0 } },
 { "flanger3", {  4, 109, 109,   2, 0, 28, 64, 46, 64, 127, 40, 64, 10, 4, 0, 0 } },
//      param:    1   2  3   4  5   6   7   8   9   10  11  12  13 14 15 16
 { "symphonic",{ 12, 25, 0, 16, 0, 28, 64, 46, 64, 127, 46, 64, 10, 0, 0, 0 } },
//      param:           1   2  3  4  5   6   7   8   9   10  11  12  13 14 15 16
 { "rotary-speaker" , { 81, 35, 0, 0, 0, 24, 60, 45, 54, 127, 33, 52, 30, 0, 0, 0 } },
//      param:    1   2    3  4  5   6  7   8  9  10 11  12  13 14 15 16
 { "karaoke1", {  0, 63,  97, 0, 0,  0, 0, 48, 0, 64, 0, 64, 10, 0, 0, 0 } },
 { "karaoke2", {  0, 55, 105, 0, 0,  0, 0, 50, 0, 64, 0, 64, 10, 0, 0, 0 } },
 { "karaoke3", {  0, 43, 110, 0, 0, 14, 0, 53, 0, 64, 0, 64, 10, 0, 0, 0 } }

// "karaoke1", { 63,  97,  0, 48, 0, 0, 0, 0, 0, 64, 2, 0, 0, 0, 0, 0 } },
// "karaoke2", { 55, 105,  0, 50, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0 } },
// "karaoke3", { 43, 110, 14, 53, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0 } }
};

#define XGMIDI_MAX_PHASING_TYPES        2
static XGMIDI_effect_t XGMIDI_phasing_types[XGMIDI_MAX_PHASING_TYPES] = {
 { "phaser1",  {  8, 111,  74, 104, 0, 28, 64, 46, 64,  64,  6,  1, 64, 0, 0, 0 } },
 { "phaser2",  {  8, 111,  74, 108, 0, 28, 64, 46, 64,  64,  5,  1,  4, 0, 0, 0 } }
};

#define XGMIDI_MAX_DISTORTION_TYPES     2
static XGMIDI_effect_t XGMIDI_distortion_types[XGMIDI_MAX_DISTORTION_TYPES] = {
 { "distortion", { 40, 20, 72, 53, 48, 0, 43, 74, 10, 127, 120, 0, 0, 0, 0, 0 } },
 { "overdrive",  { 29, 24, 68, 45, 55, 0, 41, 72, 10, 127, 104, 0, 0, 0, 0, 0 } }
};

#define XGMIDI_MAX_EQ_TYPES             2
static XGMIDI_effect_t XGMIDI_EQ_types[XGMIDI_MAX_EQ_TYPES] = {
 { "equalizer-3-band", { 70, 34, 60, 10, 70, 28, 46, 0, 0, 127,  0,  0, 0,  0,  0,  0 } },
 { "equalizer-2-band", { 28, 70, 46, 70,  0,  0,  0, 0, 0, 127, 34, 64, 10, 0,  0,  0 } }
};

static XGMIDI_effect_t XGMIDI_amp_simulator_types[1] = {
 { "amp-simulator", { 39, 1, 48, 55,  0,  0,  0, 0, 0, 127, 112, 0, 0, 0,  0,  0 } }
};

#define LEVEL_60DB		0.001f
#define MAX_REVERB_EFFECTS_TIME	0.700f

#define _MAX(a,b)       (((a)>(b)) ? (a) : (b))
#define _MIN(a,b)       (((a)<(b)) ? (a) : (b))
#define _MINMAX(a,b,c)  (((a)>(c)) ? (c) : (((a)<(b)) ? (b) : (a)))

float _lin2log(float v) { return log10f(v); }
float _log2lin(float v) { return powf(10.0f,v); }
float _lin2db(float v) { return 20.0f*log10f(v); }
float _db2lin(float v) { return _MINMAX(powf(10.0f,v/20.0f),0.0f,10.0f); }

void
print_header(FILE *stream)
{
    time_t seconds=time(NULL);
    struct tm* current_time=localtime(&seconds);
    int year = current_time->tm_year + 1900;

    fprintf(stream, "\n<!--\n");
    fprintf(stream, " * Copyright (C) 2017-%d by Erik Hofman.\n", year);
    fprintf(stream, " * Copyright (C) 2017-%d by Adalin B.V.\n", year);
    fprintf(stream, " * All rights reserved.\n");
    fprintf(stream, " *\n");
    fprintf(stream, " * This file is part of AeonWave and covered by the\n");
    fprintf(stream, " * Creative Commons Attribution-ShareAlike 4.0 International Public License\n");
    fprintf(stream, " * https://creativecommons.org/licenses/by-sa/4.0/legalcode\n");
    fprintf(stream, "-->\n\n");

}

void
print_info(FILE *stream)
{
    time_t seconds=time(NULL);
    struct tm* current_time=localtime(&seconds);
    int year = current_time->tm_year + 1900;

    fprintf(stream, "\n <info>\n");
    fprintf(stream, "  <license type=\"Attribution-ShareAlike 4.0 International\"/>\n");
    fprintf(stream, "  <copyright from=\"2017\" until=\"%d\" by=\"Adalin B.V.\"/>\n", year);
    fprintf(stream, "  <copyright from=\"2017\" until=\"%d\" by=\"Erik Hofman\"/>\n", year);
    fprintf(stream, "  <contact author=\"Erik Hofman\" website=\"aeonwave.xyz\"/>\n");
    fprintf(stream, " </info>\n\n");
}

static float
get_decay_depth(float time, float level) {
    if (level < 0.001f) level = 0.001f;
    return 5.0f*time*powf(LEVEL_60DB, level);
}

int write_reverb()
{
   for (int i=0; i<XGMIDI_MAX_REVERB_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_reverb_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating reverb: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      { // # = table number
         int rt = type->param[0];	// Reverb Time 0.3 ~ 30.0s (0-69) #4
//       int df = type->param[1];	// Diffusion 0 ~ 10
         int id = type->param[2];	// Initial Delay 0 ~ 63
//       int hc = type->param[3];	// HPF Cutoff Thru ~ 8.0kHz (0-52) #3
         int lc = type->param[4];	// LPF Cutoff 1.0k ~ Thru (34-60)
         int w = type->param[5];	// Width 0.5 ~ 10.2m (0-37) #11
         int h = type->param[6];	// Heigt 0.5 ~ 20.2m (0-73) #11
         int d = type->param[7];	// Depth 0.5 ~ 30.2m (0-104) #11
//       int wv = type->param[8];	// Wall Vary 0 ~ 30
         int dw = type->param[9];	// Dry/Wet D63>W ~ D=W ~ D<W63 (1-127)
//       int rd = type->param[10];	// Rev Delay 0 ~ 63 #5
//       int ds = type->param[11];	// Density 0 ~ 4
//       int re = type->param[12];	// Rev/Er Balance R<E63 ~ R=E ~ R63>E (1-127)
         int hd = type->param[13];	// High Damp 0.1 ~ 1.0 (1-10)
//       int fb = type->param[14];	// Feedback Level -63 ~ +63 (1-127)
//
         /* room dimansions */
         float width = XGMIDI_reverb_dimensions_m[w];
         float height = XGMIDI_reverb_dimensions_m[h];
         float depth = XGMIDI_reverb_dimensions_m[d];
         float density = 0.0f;

         /* basic parameters */
         float cutoff_freq = XGMIDI_EQ_frequency_table_Hz[lc];
         cutoff_freq = (1.1f - 0.1f*hd) * cutoff_freq;

         float delay_depth = XGMIDI_delay_time_table_200ms[id]*1e-3f;

         float decay_time = XGMIDI_reverb_time_sec[rt];
         float decay_level = 0.98f*dw/63.0f;
         float decay_depth = get_decay_depth(decay_time, decay_level);

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"reverb\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", cutoff_freq);
         fprintf(stream, "    <param n=\"1\">%.4f</param>\n", delay_depth);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", decay_level);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", decay_depth);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "   <slot n=\"1\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", width);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", height);
         fprintf(stream, "    <param n=\"2\">%.1f</param>\n", depth);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", density);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

// Chorus, Celeste & Flanger
int write_chorus()
{
   for (int i=0; i<XGMIDI_MAX_CHORUS_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_chorus_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      { // # = table number
         int f = type->param[0];	// LFO Frequency 0.0 ~ 39.7Hz (0-127) #1
         int pd = type->param[1];	// LFO Depth 0 ~ 127 (0-127)
         int fb = type->param[2];	// Feedback Level -63 ~ +63 (1-127)
         int dt = type->param[3];	// Delay Offset 0 ~ 127 (0-127) #2

         int lf = type->param[5];	// EQ Low Frequency 32Hz ~ 2.0kHz #3
         int lg = type->param[6];	// EQ Low Gain -12 ~ +12dB (52-76)
         int hf = type->param[7];	// EQ High Frequency 500Hz ~ 16.0kHz #3
         int hg = type->param[8];	// EQ High Gain -12 ~ +12dB (52-76)
         int dw = type->param[9];	// Dry/Wet D63>W ~ D=W ~ D<W63 (1-127)

         int mf = type->param[10];	// EQ Mid Frequency 100H ~10.0kHz #3
         int mg = type->param[11];	// EQ Mid Gain -12 ~ +12dB (52-76)
         int mw = type->param[12];	// EQ Mid Width 1.0 ~ 12.0
//       int ad = type->param[13];	// LFO AM Depth 0 ~ 127
					// LFO Phase Difference (flanger)
         char stereo = type->param[14]; // mono/stereo

         float rate = XGMIDI_LFO_frequency_table_Hz[f];
         float lfo_offset = XGMIDI_delay_offset_table_ms[dt];
         float lfo_depth = XGMIDI_delay_offset_table_ms[pd]; // 30*pd/127.0f; // 5ms to 30ms
         float gain = 0.98f*dw/63.0f;
         float feedback = fb ? (fb-64)/64.0f : 0.0f;

         float low_gain = _db2lin(lg-64.0f);
         float mid_gain = _db2lin(mg-64.0f);
         float high_gain = _db2lin(hg-64.0f);
         float low_cutoff = XGMIDI_EQ_frequency_table_Hz[lf];
         float mid_cutoff = XGMIDI_EQ_frequency_table_Hz[mf];
         float high_cutoff = XGMIDI_EQ_frequency_table_Hz[hf];
         float mid_Q = 0.1f*mw;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"");
         fprintf(stream, "chorus");
         fprintf(stream, "\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine\"");
         if (stereo) fprintf(stream, " stereo=\"true\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.3f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\" type=\"msec\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\" type=\"msec\">%.3f</param>\n", lfo_offset);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "   <slot n=\"1\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", low_cutoff);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", 0.0f);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", feedback);
         fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         if (low_gain != 1.0f || mid_gain != 1.0f || high_gain != 1.0f) {
             fprintf(stream, "  <filter type=\"equalizer\">\n");
             fprintf(stream, "   <slot n=\"0\">\n");
             if (low_gain != 1.0f || mid_gain != 1.0f) {
                 fprintf(stream, "    <param n=\"0\">%.1f</param>\n", low_cutoff);
                 fprintf(stream, "    <param n=\"1\">%.3f</param>\n", low_gain);
                 fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mid_gain);
                 fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
                 fprintf(stream, "   </slot>\n");
                 fprintf(stream, "   <slot n=\"1\">\n");
             }
             if (low_cutoff < mid_cutoff) {
                 fprintf(stream, "    <param n=\"0\">%.1f</param>\n", mid_cutoff);
                 fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mid_gain);
                 fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mid_gain);
                 fprintf(stream, "    <param n=\"3\">%.1f</param>\n", mid_Q);
                 fprintf(stream, "   </slot>\n");
                 fprintf(stream, "   <slot n=\"2\">\n");
             }
             fprintf(stream, "    <param n=\"0\">%.1f</param>\n", high_cutoff);
             fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mid_gain);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", high_gain);
             fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
             fprintf(stream, "  </filter>\n");
         }
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int write_phasing()
{
   for (int i=0; i<XGMIDI_MAX_PHASING_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_phasing_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {	// # = table number
         int f = type->param[0];	// LFO Frequency 0.0 ~ 39.7Hz (0-127) #1
         int ld = type->param[1];	// LFO Depth 0 ~ 127 (0-127)
         int dt = type->param[2];	// Phase Shift Offset 0 ~ 127 (0-127)
         int fb = type->param[3];	// Feedback Level -63 ~ +63 (1-127)

         int lf = type->param[5];	// EQ Low Frequency 32Hz ~ 2.0kHz
         int lg = type->param[6];	// EQ Low Gain -12 ~ +12dB (52-76)
         int hf = type->param[7];	// EQ High Frequency 500Hz ~ 16.0kHz
         int hg = type->param[8];	// EQ High Gain -12 ~ +12dB (52-76)
         int dw = type->param[9];	// Dry/Wet D63>W ~ D=W ~ D<W63 (1-127)

//       int st = type->param[10];	// Stage 1 ~ 10
//       int df = type->param[11];	// Diffusion mono/stereo
//       int pd = type->param[12];	// Phase Difference -180 ~ 180 deg.

         float rate = XGMIDI_LFO_frequency_table_Hz[f];
         float lfo_offset = dt/127.0f;
         float lfo_depth = (ld - dt)/127.0f;
         float gain = 0.98f*dw/63.0f;

         float low_gain = _db2lin(lg - 64.0f);
         float mid_gain = 1.0f;
         float high_gain = _db2lin(hg - 64.0f);
         float low_cutoff = XGMIDI_EQ_frequency_table_Hz[lf];
         float high_cutoff = XGMIDI_EQ_frequency_table_Hz[hf];
         float feedback = 0.5f*fb/126.0f;

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"phasing\"");
         if (rate > 0.0f) fprintf(stream, " src=\"sine\"");
         fprintf(stream, ">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.3f</param>\n", gain);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", rate);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", lfo_depth);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", lfo_offset);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "   <slot n=\"1\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", 0.0f);
         fprintf(stream, "    <param n=\"1\">%.1f</param>\n", 0.0f);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", feedback);
         fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         if (low_gain != 1.0f || mid_gain != 1.0f || high_gain != 1.0f) {
             fprintf(stream, "  <filter type=\"equalizer\">\n");
             fprintf(stream, "   <slot n=\"0\">\n");
             if (low_gain != 1.0f || mid_gain != 1.0f) {
                 fprintf(stream, "    <param n=\"0\">%.1f</param>\n", low_cutoff);
                 fprintf(stream, "    <param n=\"1\">%.3f</param>\n", low_gain);
                 fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mid_gain);
                 fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
                 fprintf(stream, "   </slot>\n");
                 fprintf(stream, "   <slot n=\"1\">\n");
             }
             fprintf(stream, "    <param n=\"0\">%.1f</param>\n", high_cutoff);
             fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mid_gain);
             fprintf(stream, "    <param n=\"2\">%.3f</param>\n", high_gain);
             fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
             fprintf(stream, "   </slot>\n");
             fprintf(stream, "  </filter>\n");
         }
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int write_distortion()
{
   for (int i=0; i<XGMIDI_MAX_DISTORTION_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_distortion_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         int dr = type->param[0];       // Drive 0 ~ 127 (0-127)
         int ol = type->param[4];       // Output Level 0 ~ 127 (0-127)
         int dw = type->param[9];       // Dry/Wet D63>W ~ D=W ~ D<W63 (1-127)
         int cc = type->param[10];	// Edge(Clip Curve) 0 ~ 127 (0-127)

         float distortion_fact = 2.0f*dr/127.0f;
         float clipping_fact = cc/127.0f;
         float mix_fact = 0.5f*(ol/127.0f * 0.98f*abs(dw)/127.0f);
         float asymmetry = 1.0f - clipping_fact;
         char inv = (dw > 64);

         distortion_fact =  distortion_fact* distortion_fact;
         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <effect type=\"distortion\" src=\"");
         if (inv) fprintf(stream, "inverse-");
         fprintf(stream, "envelope\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.3f</param>\n", distortion_fact);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", clipping_fact);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mix_fact);
         fprintf(stream, "    <param n=\"3\">%.3f</param>\n", asymmetry);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </effect>\n");
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int write_equalizer()
{
   for (int i=0; i<XGMIDI_MAX_EQ_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_EQ_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
         // 3-band equzlizer
         int lg = type->param[0];	// EQ Low Gain -12 ~ +12dB (52-76)
//       int mf = type->param[1];	// EQ Mid Frequency 100Hz ~ 10.0kHz
         int mg = type->param[2];	// EQ Mid Gain -12 ~ +12dB (52-76)
//       int mw = type->param[3];	// EQ Mid Width 1.0 ~ 12.0
         int hg = type->param[4];	// EQ High Gain -12 ~ +12dB (52-76)
         int lf = type->param[5];	// EQ Low Frequency 32Hz ~ 2.0kHz
         int hf = type->param[6];	// EQ High Frequency 500Hz ~ 16.0kHz

         if (i) { // 2-band equzlizer
             lf = type->param[0];	// EQ Low Frequency 32Hz ~ 2.0kHz
             lg = type->param[1];	// EQ Low Gain -12 ~ +12dB (52-76)
             hf = type->param[2];	// EQ High Frequency 500Hz ~ 16.0kHz
             hg = type->param[3];	// EQ High Gain -12 ~ +12dB (52-76)
//           mf = type->param[10];	// EQ Mid Frequency 100Hz ~ 10.0kHz
             mg = type->param[11];	// EQ Mid Gain -12 ~ +12dB (52-76)
//           mw = type->param[12];	// EQ Mid Width 1.0 ~ 12.0
         }

         float low_gain = _db2lin(-12.0f + 24.0f*(lg-52.0f)/24.0f);
         float mid_gain = _db2lin(-12.0f + 24.0f*(mg-52.0f)/24.0f);
         float high_gain = _db2lin(-12.0f + 24.0f*(hg-52.0f)/24.0f);
         float low_cutoff = XGMIDI_EQ_frequency_table_Hz[lf];
         float high_cutoff = XGMIDI_EQ_frequency_table_Hz[hf];

         fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

         print_header(stream);

         fprintf(stream, "<aeonwave>\n\n");

         print_info(stream);

         fprintf(stream, " <audioframe mode=\"append\">\n");
         fprintf(stream, "  <filter type=\"equalizer\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", low_cutoff);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", low_gain);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mid_gain);
         fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "   <slot n=\"1\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", high_cutoff);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", mid_gain);
         fprintf(stream, "    <param n=\"2\">%.3f</param>\n", high_gain);
         fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </filter>\n");
         fprintf(stream, " </audioframe>\n\n");
         fprintf(stream, "</aeonwave>\n");
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int write_amp_simulator()
{
   char fname[256];

   XGMIDI_effect_t *type = &XGMIDI_amp_simulator_types[0];

   snprintf(fname, 255, "%s.aaxs", type->name);
   printf("Generating: %s\n", fname);

   FILE *stream = fopen(fname, "w+");
   if (stream)
   {
      int dr = type->param[0];		// Drive 0 ~ 127 (0-127)
//    int tp = type->param[1];		// Off,Stack,Combo,Tube (0-3)
      int fc = type->param[2];		// LPF Cutoff 1.0k ~ Thru (34-60)
      int ol = type->param[3];		// Output Level 0 ~ 127 (0-127)
      int dw = type->param[9];		// Dry/Wet D63>W ~ D=W ~ D<W63 (1-127)
      int cc = type->param[10];		// Edge(Clip Curve) 0 ~ 127 (0-127)

      /* distortion */
      float distortion_fact = 2.0f*dr/127.0f;
      float clipping_fact = 0.25f + 0.5f*cc/127.0f; // cc=0 = mild, cc=127 = sharp
      float mix_fact = 0.98f*dw/127.0f;
      float asymmetry = 1.0f - clipping_fact;

      /* equalizer */
      float cutoff = XGMIDI_EQ_frequency_table_Hz[fc];
      float gain = ol/127.0f;

      distortion_fact =  distortion_fact* distortion_fact;
      fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

      print_header(stream);

      fprintf(stream, "<aeonwave>\n\n");

      print_info(stream);

      fprintf(stream, " <audioframe mode=\"append\">\n");
      if (cutoff > 20.0f && cutoff < 20000.0f && ol > 0.0f) {
         fprintf(stream, "  <filter type=\"frequency\">\n");
         fprintf(stream, "   <slot n=\"0\">\n");
         fprintf(stream, "    <param n=\"0\">%.1f</param>\n", cutoff);
         fprintf(stream, "    <param n=\"1\">%.3f</param>\n", gain);
         fprintf(stream, "    <param n=\"2\">%.1f</param>\n", 0.0f);
         fprintf(stream, "    <param n=\"3\">%.1f</param>\n", 1.0f);
         fprintf(stream, "   </slot>\n");
         fprintf(stream, "  </filter>\n");
      }
      fprintf(stream, "  <effect type=\"distortion\" src=\"envelope\">\n");
      fprintf(stream, "   <slot n=\"0\">\n");
      fprintf(stream, "    <param n=\"0\">%.3f</param>\n", distortion_fact);
      fprintf(stream, "    <param n=\"1\">%.3f</param>\n", clipping_fact);
      fprintf(stream, "    <param n=\"2\">%.3f</param>\n", mix_fact);
      fprintf(stream, "    <param n=\"3\">%.3f</param>\n", asymmetry);
      fprintf(stream, "   </slot>\n");
      fprintf(stream, "  </effect>\n");
      fprintf(stream, " </audioframe>\n\n");
      fprintf(stream, "</aeonwave>\n");
      fclose(stream);
   }
   else printf(" Failed to open for writing: %s\n", strerror(errno));

   return 0;
}

int write_unsupported()
{
   for (int i=0; i<XGMIDI_MAX_EQ_TYPES; ++i)
   {
      char fname[256];

      XGMIDI_effect_t *type = &XGMIDI_EQ_types[i];

      snprintf(fname, 255, "%s.aaxs", type->name);
      printf("Generating: %s\n", fname);

      FILE *stream = fopen(fname, "w+");
      if (stream)
      {
// TODO
         fclose(stream);
      }
      else printf(" Failed to open for writing: %s\n", strerror(errno));
   }

   return 0;
}

int write_style_list()
{
   char fname[256] = "styles.xml";
   FILE *stream;

   printf("Generating: %s\n", fname);

   stream = fopen(fname, "w+");
   if (stream)
   {
      int i = 0;

      fprintf(stream, "<?xml version=\"1.0\"?>\n\n");

      print_header(stream);

      fprintf(stream, "<aeonwave>\n\n");

      print_info(stream);

      fprintf(stream, " <midi id=\"%i\" name=\"XG-MIDI\">\n",
                         MIDI_SYSTEM_EXCLUSIVE_YAMAHA);

      do
      {
         fprintf(stream, "  <style n=\"%i\" file=\"%s\" name=\"%s\"/>\n",
                             XGMIDI_style_list[i].num,
                             XGMIDI_style_list[i].file,
                             XGMIDI_style_list[i].name);
      }
      while (XGMIDI_style_list[++i].num);


      fprintf(stream, " </midi>\n");
      fprintf(stream, "</aeonwave>\n");
      fclose(stream);
   }

   return 0;
}

int main()
{
   write_reverb();
   write_chorus();
   write_phasing();
   write_distortion();
   write_equalizer();
   write_amp_simulator();
   write_style_list();

   return 0;
}
