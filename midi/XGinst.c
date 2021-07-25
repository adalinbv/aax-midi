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
 {  1,  1, "Grand Piano K", 1 },
 {  1, 18, "Mellow Grand Piano", 1 },
 {  1, 40, "Piano Strings", 2 },
 {  1, 41, "Dream", 2 },
 {  2,  0, "Bright Piano", 1 },
 {  2,  1, "Bright Piano K", 1 },
 {  3,  0, "Electric Grand", 2 },
 {  3,  1, "Electric Grand Piano K", 2 },
 {  3, 32, "Detuned CP80", 2 },
 {  3, 40, "Layer CP80 1", 2 },
 {  3, 41, "Layer CP80 2", 2 },
 {  4,  0, "Honky Tonk", 2 },
 {  4,  1, "Honky Tonk K", 2 },
 {  5,  0, "Electric Piano 1", 2 },
 {  5,  1, "Electric Piano 1 K", 1 },
 {  5, 18, "Mellow Electric Piano 1", 2 },
 {  5, 32, "Chorused  Electric Piano 1", 2 },
 {  5, 40, "Hard Electric Piano", 2 },
 {  5, 45, "VX Electric Piano 1", 2 },
 {  5, 64, "60s Electric Piano", 1 },
 {  6,  0, "Electric Piano 2", 2 },
 {  6,  1, "Electric Piano 2 K", 1 },
 {  6, 32, "Chorused  Electric Piano 2", 2 },
 {  6, 33, "DX Hard", 2 },
 {  6, 34, "DX Legend", 2 },
 {  6, 40, "DX Phase", 2 },
 {  6, 41, "DX  +Analg", 2 },
 {  6, 42, "DX Koto Electric Piano", 2 },
 {  6, 45, "VX Electric P2", 2 },
 {  7,  0, "Harpsichord", 1 },
 {  7,  1, "Harpsichord K", 1 },
 {  7, 25, "Harpsichord 2", 2 },
 {  7, 35, "Harpsichord 3", 2 },
 {  8,  0, "Clavinet", 1 },
 {  8,  1, "Clavinet  K", 1 },
 {  8, 27, "Clavinet Wah", 2 },
 {  8, 64, "PulseClv", 1 },
 {  8, 65, "PierceCl", 2 },
 {  9,  0, "Celesta", 1 },
 { 10,  0, "Glocken", 1 },
 { 11,  0, "MusicBox", 2 },
 { 11, 64, "Orgel", 2 },
 { 12,  0, "Vibes", 1 },
 { 12,  1, "Vibes K", 1 },
 { 12, 45, "HardVibe", 2 },
 { 13,  0, "Marimba", 1 },
 { 13,  1, "MarimbaK", 1 },
 { 13, 64, "SineMrmb", 2 },
 { 13, 97, "Balimba", 2 },
 { 13, 98, "Log Drum", 2 },
 { 14,  0, "Xylophon", 1 },
 { 15,  0, "TubulBel", 1 },
 { 15, 96, "chrchBel", 2 },
 { 15, 97, "carillon", 2 },
 { 16,  0, "Dulcimer", 1 },
 { 16, 35, "Dulcimr2", 2 },
 { 16, 96, "Cimbalom", 2 },
 { 16, 97, "Santur", 2 },
 { 17,  0, "DrawOrgn", 1 },
 { 17, 32, "DetDrwOr", 2 },
 { 17, 33, "60sDrOr1", 2 },
 { 17, 34, "60sDrOr2", 2 },
 { 17, 35, "70sDrOr1", 2 },
 { 17, 36, "DrawOrg2", 2 },
 { 17, 37, "60sDrOr3", 2 },
 { 17, 38, "EvenBar", 2 },
 { 17, 40, "16+2'2/3", 2 },
 { 17, 64, "Organ Ba", 1 },
 { 17, 65, "70sDrOr2", 2 },
 { 17, 66, "CheezOrg", 2 },
 { 17, 67, "DrawOrg3", 2 },
 { 18,  0, "PercOrgn", 1 },
 { 18, 24, "70sPcOr1", 2 },
 { 18, 32, "DetPrcOr", 2 },
 { 18, 33, "Lite Org", 2 },
 { 18, 37, "PercOrg2", 2 },
 { 19,  0, "RockOrgn", 1 },
 { 19, 64, "RotaryOr", 2 },
 { 19, 65, "SloRotar", 2 },
 { 19, 66, "FstRotar", 2 },
 { 20,  0, "ChrchOrg", 2 },
 { 20, 32, "ChurOrg3", 2 },
 { 20, 35, "ChurOrg2", 2 },
 { 20, 40, "NotreDam", 2 },
 { 20, 64, "OrgFlute", 2 },
 { 20, 65, "TrmOrgFl", 2 },
 { 21,  0, "ReedOrgn", 1 },
 { 21, 40, "Puff Org", 2 },
 { 22,  0, "Acordion", 1 },
 { 22, 32, "AccordIt", 2 },
 { 23,  0, "Harmnica", 1 },
 { 23, 32, "Harmo 2", 2 },
 { 24,  0, "TangoAcd", 1 },
 { 24, 64, "TngoAcd2", 2 },
 { 25,  0, "NylonGtr", 1 },
 { 25, 16, "NylonGt2", 1 },
 { 25, 25, "NylonGt3", 2 },
 { 25, 43, "VelGtHrm", 1 },
 { 25, 96, "Ukulele", 1 },
 { 26,  0, "SteelGtr", 1 },
 { 26, 16, "SteelGt2", 1 },
 { 26, 35, "12 Strings Gtr", 2 },
 { 26, 40, "Nyln&Stl", 2 },
 { 26, 41, "Stl&Body", 2 },
 { 26, 96, "Mandolin", 2 },
 { 27,  0, "Jazz Gtr", 1 },
 { 27, 18, "MelloGtr", 1 },
 { 27, 32, "JazzAmp", 2 },
 { 27, 96, "PdlSteel", 1 },
 { 28,  0, "CleanGtr", 1 },
 { 28, 32, "ChorusGt", 2 },
 { 28, 65, "MidT.Gtr", 1 },
 { 28, 66, "MidTGtSt", 2 },
 { 29,  0, "Mute.Gtr", 1 },
 { 29, 40, "FunkGtr1", 2 },
 { 29, 41, "MuteStlG", 2 },
 { 29, 43, "FunkGtr2", 1 },
 { 29, 45, "Jazz Man", 2 },
 { 29, 96, "Mu.DstGt", 2 },
 { 30,  0, "Ovrdrive", 1 },
 { 30, 43, "Gt.Pinch", 1 },
 { 31,  0, "Dist.Gtr", 1 },
 { 31, 12, "DstRthmG", 2 },
 { 31, 24, "DistGtr2", 2 },
 { 31, 35, "DistGtr3", 2 },
 { 31, 36, "PowerGt2", 2 },
 { 31, 37, "PowerGt1", 2 },
 { 31, 38, "Dst.5ths", 2 },
 { 31, 40, "FeedbkGt", 2 },
 { 31, 41, "FeedbkG2", 2 },
 { 31, 43, "RckRthm2", 2 },
 { 31, 45, "RckRthm1", 2 },
 { 32,  0, "GtrHarmo", 1 },
 { 32, 65, "GtFeedbk", 1 },
 { 32, 66, "GtrHrmo2", 1 },
 { 33,  0, "Aco.Bass", 1 },
 { 33, 40, "JazzRthm", 2 },
 { 33, 45, "VXUprght", 2 },
 { 34,  0, "FngrBass", 1 },
 { 34, 18, "FingrDrk", 2 },
 { 34, 27, "FlangeBa", 2 },
 { 34, 40, "Ba&DstEG", 2 },
 { 34, 43, "FngrSlap", 1 },
 { 34, 45, "FngBass2", 2 },
 { 34, 64, "Jazzy Ba", 1 },
 { 34, 65, "Mod.Bass", 2 },
 { 34, 112 ,"FW EBass", 1 },
 { 35,  0, "PickBass", 1 },
 { 35, 28, "MutePkBa", 1 },
 { 36,  0, "Fretless", 1 },
 { 36, 32, "Fretles2", 2 },
 { 36, 33, "Fretles3", 2 },
 { 36, 34, "Fretles4", 2 },
 { 36, 96, "SynFretl", 2 },
 { 36, 97, "SmthFrtl", 2 },
 { 37,  0, "SlapBas1", 1 },
 { 37, 27, "ResoSlap", 1 },
 { 37, 32, "PunchThm", 2 },
 { 37, 64, "Slapper", 1 },
 { 37, 65, "Thum&Slp", 1 },
 { 38,  0, "SlapBas2", 1 },
 { 38, 43, "VeloSlap", 1 },
 { 39,  0, "SynBass1", 1 },
 { 39, 18, "SynBa1Dk", 1 },
 { 39, 20, "FastResB", 1 },
 { 39, 24, "AcidBass", 1 },
 { 39, 27, "ResoBass", 1 },
 { 39, 35, "Clv Bass", 2 },
 { 39, 40, "TechnoBa", 2 },
 { 39, 64, "Orbiter", 2 },
 { 39, 65, "Sqr.Bass", 1 },
 { 39, 66, "RubberBa", 2 },
 { 39, 67, "Fish", 1 },
 { 39, 68, "HardReso", 1 },
 { 39, 96, "Hammer", 2 },
 { 39, 112 ,"AnlgBass", 2 },
 { 39, 113 ,"RezoBass", 2 },
 { 40,  0, "SynBass2", 2 },
 { 40,  6, "MelloSBa", 1 },
 { 40, 12, "Seq Bass", 2 },
 { 40, 18, "ClkSynB", 2 },
 { 40, 19, "SynBa2Dk", 1 },
 { 40, 32, "SmthSynB", 2 },
 { 40, 40, "ModulrBa", 2 },
 { 40, 41, "DX Bass", 2 },
 { 40, 64, "X WireBa", 2 },
 { 40, 65, "AtkPulse", 1 },
 { 40, 66, "CS Light", 1 },
 { 40, 67, "MetlBass", 1 },
 { 40, 112 ,"SmoothBs", 2 },
 { 40, 113 ,"Oct. Bass", 1 },
 { 40, 114 ,"PowerBas", 4 },
 { 41,  0, "Violin", 1 },
 { 41,  8, "Slow Vln", 1 },
 { 42,  0, "Viola 1", },
 { 43,  0, "Cello 1", },
 { 44,  0, "Contrabs", 1 },
 { 45,  0, "Trem. Strings", 1 },
 { 45,  8, "SlwTr Strings", 1 },
 { 45, 40, "Susp. Strings", 2 },
 { 46,  0, "Pizz. Strings", 1 },
 { 47,  0, "Harp", 1 },
 { 47, 40, "YangChin", 2 },
 { 48,  0, "Timpani", 1 },
 { 49,  0, "Strings ings1", 1 },
 { 49,  3, "S. Strings ngs", 2 },
 { 49,  8, "Slow Strings", 1 },
 { 49, 24, "Arco Strings", 2 },
 { 49, 35, "60s Strings ng", 2 },
 { 49, 40, "Orchestr", 2 },
 { 49, 41, "Orchstr2", 2 },
 { 49, 42, "TremOrch", 2 },
 { 49, 45, "Velo. Strings", 2 },
 { 50,  0, "Strings ings2", 1 },
 { 50,  3, "S.Slw Strings", 2 },
 { 50,  8, "LegatoSt", 2 },
 { 50, 40, "Warm Strings", 2 },
 { 50, 41, "Kingdom", 2 },
 { 50, 64, "70s Strings", 1 },
 { 50, 65, "Strings ings3", 1 },
 { 51,  0, "Syn Strings 1", 2 },
 { 51, 27, "Reso Strings", 2 },
 { 51, 64, "Syn Strings 4", 2 },
 { 51, 65, "Syn Strings 5", 2 },
 { 52,  0, "Syn Strings 2", 2 },
 { 53,  0, "ChoirAah", 1 },
 { 53,  3, "S.Choir", 2 },
 { 53, 16, "Ch.Aahs2", 2 },
 { 53, 32, "MelChoir", 2 },
 { 53, 40, "Choir Strings", 2 },
 { 54,  0, "VoiceOoh", 1 },
 { 55,  0, "SynVoice", 1 },
 { 55, 40, "SynVoice2", 2 },
 { 55, 41, "Choral",2 },
 { 55, 64, "AnaVoice", 1 },
 { 56,  0, "Orch.Hit", 2 },
 { 56, 35, "OrchHit2", 2 },
 { 56, 64, "Impact", 2 },
 { 56, 68, "Bass Hit", 1 },
 { 56, 70, "6th Hit", 1 },
 { 56, 71, "6thHit +", 2 },
 { 56, 72, "Euro Hit", 1 },
 { 56, 73, "EuroHit+", 2 },
 { 57,  0, "Trumpet", 1 },
 { 57, 16, "Trumpet2", 1 },
 { 57, 17, "BriteTrp", 2 },
 { 57, 32, "Warm Trp", 2 },
 { 58,  0, "Trombone", 1 },
 { 58, 18, "Trmbone2", 2 },
 { 59,  0, "Tuba", 1 },
 { 59, 16, "Tuba 2", 1 },
 { 60,  0, "Mute Trp", 1 },
 { 61,  0, "Fr.Horn", 1 },
 { 61,  6, "FrHrSolo", 1 },
 { 61, 32, "FrHorn 2", 2 },
 { 61, 37, "HornOrch", 2 },
 { 62,  0, "BrssSect", 1 },
 { 62,  3, "StBrsSec", 2 },
 { 62, 35, "Tp&TbSec", 2 },
 { 62, 40, "BrssSec2", 2 },
 { 62, 41, "Hi Brass", 2 },
 { 62, 42, "MelloBrs", 2 },
 { 63,  0, "SynBrss1", 2 },
 { 63, 12, "Quack Br", 2 },
 { 63, 20, "RezSynBr", 2 },
 { 63, 24, "PolyBrss", 2 },
 { 63, 27, "SynBrss3", 2 },
 { 63, 32, "JumpBrss", 2 },
 { 63, 40, "SyBrsSub", 2 },
 { 63, 45, "AnVelBr1", 2 },
 { 63, 64, "AnaBrss1", 2 },
 { 64,  0, "SynBrss2", 1 },
 { 64, 18, "Soft Brs", 2 },
 { 64, 40, "SynBrss4", 2 },
 { 64, 41, "ChoirBrs", 2 },
 { 64, 45, "AnVelBr2", 2 },
 { 64, 64, "AnaBrss2", 2 },
 { 65,  0, "SprnoSax", 1 },
 { 66,  0, "Alto Sax", 1 },
 { 66, 40, "Sax Sect", 2 },
 { 66, 43, "HyprAlto", 1 },
 { 67,  0, "TenorSax", 1 },
 { 67, 40, "BrthTnSx", 2 },
 { 67, 41, "SoftTenr", 2 },
 { 67, 64, "TnrSax 2", 1 },
 { 68,  0, "Bari.Sax", 1 },
 { 69,  0, "Oboe", 1 },
 { 70,  0, "Eng.Horn", 1 },
 { 71,  0, "Bassoon", 1 },
 { 72,  0, "Clarinet", 1 },
 { 73,  0, "Piccolo", 1 },
 { 74,  0, "Flute", 1 },
 { 75,  0, "Recorder", 1 },
 { 76,  0, "PanFlute", 1 },
 { 77,  0, "Bottle", 2 },
 { 78,  0, "Shakhchi", 1 },
 { 79,  0, "Whistle", 1 },
 { 80,  0, "Ocarina", 1 },
 { 81,  0, "Square Lead", 2 },
 { 81,  6, "Square 2", 1 },
 { 81,  8, "LMSquare", 2 },
 { 81, 18, "Hollow", 1 },
 { 81, 19, "Shroud", 2 },
 { 81, 64, "Mellow", 2 },
 { 81, 65, "SoloSine", 2 },
 { 81, 66, "SineLead", 1 },
 { 81, 67, "Pulse Lead", 1 },
 { 82,  0, "Saw Lead", 2 },
 { 82,  6, "Saw Lead 2",  1 },
 { 82,  8, "ThickSaw", 2 },
 { 82, 18, "Dyna Saw", 1 },
 { 82, 19, "Digi Saw", 2 },
 { 82, 20, "Big Lead", 2 },
 { 82, 24, "HeavySyn", 2 },
 { 82, 25, "WaspySyn", 2 },
 { 82, 27, "RezzySaw", 1 },
 { 82, 32, "DoublSaw", 2 },
 { 82, 35, "Toy Lead", 2 },
 { 82, 36, "Dim Saw", 2 },
 { 82, 40, "PulseSaw", 2 },
 { 82, 41, "Dr. Lead", 2 },
 { 82, 45, "VeloLead", 2 },
 { 82, 64, "Digger", 1 },
 { 82, 96, "Seq Ana.", 2 },
 { 83,  0, "Caliop Lead", 2 },
 { 83, 65, "Pure Pad", 2 },
 { 84,  0, "Chiff Lead", 2 },
 { 84, 64, "Rubby", 2 },
 { 84, 65, "HardSync", 1 },
 { 85,  0, "Charan Lead", 2 },
 { 85, 64, "DistLead", 2 },
 { 85, 65, "WireLead", 2 },
 { 85, 66, "SynPluck", 1 },
 { 86,  0, "Voice Lead", 2 },
 { 86, 24, "SynthAah", 2 },
 { 86, 64, "VoxLead", 2 },
 { 87,  0, "Fifth Lead", 2 },
 { 87, 35, "Big Five", 2 },
 { 88,  0, "Bass& Lead", 2 },
 { 88, 16, "Big&Low", 2 },
 { 88, 64, "Fat&Prky", 2 },
 { 88, 65, "Soft Wrl", 2 },
 { 89,  0, "NewAgePd", 2 },
 { 89, 64, "Fantasy", 2 },
 { 90,  0, "Warm Pad", 2 },
 { 90, 16, "Thick Pad", 2 },
 { 90, 17, "Soft Pad", 2 },
 { 90, 18, "Sine Pad", 2 },
 { 90, 64, "Horn Pad", 2 },
 { 90, 65, "Rotar Strings", 2 },
 { 91,  0, "PolySyPd", 2 },
 { 91, 64, "PolyPd80", 2 },
 { 91, 65, "Click Pad", 2 },
 { 91, 66, "Ana. Pad", 2 },
 { 91, 67, "Squar Pad", 2 },
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
 { 97, 45, "Clavinet  Pad", 2 },
 { 97, 64, "HrmoRain", 2 },
 { 97, 65, "AfrcnWnd", 2 },
 { 97, 66, "Carib", 2 },
 { 98,  0, "SoundTrk", 2 },
 { 98, 27, "Prologue", 2 },
 { 98, 64, "Ancestrl", 2 },
 { 99,  0, "Crystal", 2 },
 { 99, 12, "SynDrCmp", 2 },
 { 99, 14, "Popcorn", 2 },
 { 99, 18, "TinyBell", 2 },
 { 99, 35, "RndGlock", 2 },
 { 99, 40, "GlockChi", 2 },
 { 99, 41, "ClearBel", 2 },
 { 99, 42, "ChorBell", 2 },
 { 99, 64, "SynMalet", 1 },
 { 99, 65, "SftCryst", 2 },
 { 99, 66, "LoudGlok", 2 },
 { 99, 67, "ChrsBel", 2 },
 { 99, 68, "VibeBell", 2 },
 { 99, 69, "DigiBell", 2 },
 { 99, 70, "AirBells", 2 },
 { 99, 71, "BellHarp", 2 },
 { 99, 72, "Gamelmba", 2 },
 { 100,  0,"Atmosphr", 2 },
 { 100, 18, "WarmAtms", 2 },
 { 100, 19, "HollwRls", 2 },
 { 100, 40, "Nylon Electric Piano", 2 },
 { 100, 64, "NylnHarp", 2 },
 { 100, 65, "Harp Vox", 2 },
 { 100, 66, "Atmos Pad", 2 },
 { 100, 67, "Planet", 2 },
 { 101,  0, "Bright", 2 },
 { 101, 64, "FantaBel", 2 },
 { 101, 96, "Smokey", 2 },
 { 102,  0, "Goblins", 2 },
 { 102, 64, "GobSynth", 2 },
 { 102, 65, "Creeper", 2 },
 { 102, 66, "Ring Pad", 2 },
 { 102, 67, "Ritual", 2 },
 { 102, 68, "ToHeaven", 2 },
 { 102, 70, "Night", 2 },
 { 102, 71, "Glisten", 2 },
 { 102, 96, "BelChoir", 2 },
 { 103,  0, "Echoes", 2 },
 { 103,  8, "Echoes 2", 2 },
 { 103, 14, "Echo Pan", 2 },
 { 103, 64, "EchoBell", 2 },
 { 103, 65, "Big Pan", 2 },
 { 103, 66, "SynPiano", 2 },
 { 103, 67, "Creation", 2 },
 { 103, 68, "StarDust", 2 },
 { 103, 69, "Reso&Pan", 2 },
 { 104,  0, "Sci-Fi", 2 },
 { 104, 64, "Starz", 2 },
 { 105,  0, "Sitar", 1 },
 { 105, 32, "DetSitar", 2 },
 { 105, 35, "Sitar 2", 2 },
 { 105, 96, "Tambra", 2 },
 { 105, 97, "Tamboura", 2 },
 { 106,  0, "Banjo", 1 },
 { 106, 28, "MuteBnjo", 1 },
 { 106, 96, "Rabab", 2 },
 { 106, 97, "Gopichnt", 2 },
 { 106, 98, "Oud", 2 },
 { 107,  0, "Shamisen", 1 },
 { 108,  0, "Koto", 1 },
 { 108, 96, "Taisho- K", 2 },
 { 108, 97, "Kanoon", 2 },
 { 109,  0, "Kalimba", 1 },
 { 110,  0, "Bagpipe", 2 },
 { 111,  0, "Fiddle", 1 },
 { 112,  0, "Shanai", 1 },
 { 112, 64, "Shanai 2", 1 },
 { 112, 96, "Pungi", 1 },
 { 112, 97, "Hichriki", 2 },
 { 113,  0, "TnklBell", 2 },
 { 113, 96, "Bonang", 2 },
 { 113, 97, "Altair", 2 },
 { 113, 98, "Gamelan", 2 },
 { 113, 99, "S.Gamlan", 2 },
 { 113, 100 ,"Rama Cym", 2 },
 { 113, 101 ,"AsianBel", 2 },
 { 114,  0, "Agogo", 1 },
 { 115,  0, "SteelDrm", 1 },
 { 115, 97, "GlasPerc", 2 },
 { 115, 98, "ThaiBell", 2 },
 { 116,  0, "Woodblok", 1 },
 { 116, 96, "Castanet", 1 },
 { 117,  0, "TaikoDrm", 1 },
 { 117, 96, "Gr.Cassa", 1 },
 { 118,  0, "MelodTom", 1 },
 { 118, 64, "Mel Tom2", 1 },
 { 118, 65, "Real Tom", 2 },
 { 118, 66, "Rock Tom", 2 },
 { 119,  0, "Syn Drum", 1 },
 { 119, 64, "Ana Tom", 1 },
 { 119, 65, "ElecPerc", 2 },
 { 120,  0, "RevCymbl", 1 },
 { 121,  0, "FretNoiz", 1 },
 { 122,  0, "BrthNoiz", 1 },
 { 123,  0, "Seashore", 2 },
 { 124,  0, "Tweet", 2 },
 { 125,  0, "Telphone", 1 },
 { 126,  0, "Helicoptr", 1 },
 { 127,  0, "Applause", 1 },
 { 128,  0, "Gunshot", 1 },
 { 128, 65, "Phone Call", 1 },
 { 128, 66, "Door Squeek", 1 },
 { 128, 67, "Door Slam", 1 },
 { 128, 68, "Scratch C", 1 },
 { 128, 69, "Scratch S", 1 },
 { 128, 70, "Wind Chime", 1 },
 { 128, 71, "Telphon2 2", 1 },
 { 128, 81, "Car Engine Ignition", 1 },
 { 128, 82, "Car Tyre Squeel", 1 },
 { 128, 83, "Car Pass", 1 },
 { 128, 84, "Car Crash", 1 },
 { 128, 85, "Siren", 2 },
 { 128, 86, "Train", 1 },
 { 128, 87, "JetPlane", 2 },
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

int main()
{
   int b;
   for (b=0; b<128; ++b)
   {
      int i = 0;
      printf("  <bank n=\"0\"l=\"%i\">\n", b);
      do
      {
         if (inst_table[i].lsb == b )
         {
            printf("   <instrument n=\"%i\" name=\"%s\"",
                    inst_table[i].program-1, inst_table[i].name);
 
            printf(" file=\"instruments/%s\"", lowercase(inst_table[i].name));
 
            printf("/>\n");
         }
      } while (inst_table[++i].program);
      printf("  </bank>\n\n");
   }

  return 0;
}
  
