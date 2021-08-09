#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct {
  int program;
  int bank;
  const char *name;
} _inst_t;

_inst_t inst_table[] = {
 {   1, 127, "Acoustic Piano 1 (D-series)" },
 {   2, 127, "Acoustic Piano 2" },
 {   3, 127, "Acoustic Piano 3 (Acoustic)" },
 {   4, 127, "Electric Piano 1 (Rhodes)" },
 {   5, 127, "Electric Piano 2 (FM)" },
 {   6, 127, "Electric Piano 3 (Wurly)" },
 {   7, 127, "Electric Piano 4" },
 {   8, 127, "Honky-tonk Piano" },
 {   9, 127, "Electric Organ 1 (60's)" },
 {  10, 127, "Electric Organ 2 (Hammond+Lesslie)" },
 {  11, 127, "Electric Organ 3 (Early)" },
 {  12, 127, "Electric Organ 4 (Vox/Farfisa)" },
 {  13, 127, "Pipe Organ 1" },
 {  14, 127, "Pipe Organ 2 (Dark)" },
 {  15, 127, "Pipe Organ 3 (Soft)" },
 {  16, 127, "Accordion" },
 {  17, 127, "Harpsichord 1" },
 {  18, 127, "Harpsichord 2" },
 {  19, 127, "Harpsichord 3 (synth)" },
 {  20, 127, "Clavinet 1" },
 {  21, 127, "Clavinet 2" },
 {  22, 127, "Clavinet 3 (stevie)" },
 {  23, 127, "Celesta 1" },
 {  24, 127, "Celesta 2" },
 {  25, 127, "Synth Brass 1 (LA sheen)" },
 {  26, 127, "Synth Brass 2" },
 {  27, 127, "Synth Brass 3 (80's Soft)" },
 {  28, 127, "Synth Brass 4 (Spandau Balet)" },
 {  29, 127, "Synth Bass 1 (Mood)" },
 {  30, 127, "Synth Bass 2 (Metallic)" },
 {  31, 127, "Synth Bass 3 (Full Deep)" },
 {  32, 127, "Synth Bass 4 (TB-303/SH-101)" },
 {  33, 127, "Fantasy (D-50)" },
 {  34, 127, "Harmo Pan (Stereo)" },
 {  35, 127, "Chorale (Breathy)" },
 {  36, 127, "Glasses" },
 {  37, 127, "Soundtrack (D-50)" },
 {  38, 127, "Atmosphere" },
 {  39, 127, "Warm Bell" },
 {  40, 127, "Funny Vox (Synth)" },
 {  41, 127, "Echo Bell (Bell+Synth Brass)" },
 {  42, 127, "Ice Rain" },
 {  43, 127, "Oboe 2001" },
 {  44, 127, "Echo Pan (Staccato)" },
 {  45, 127, "Doctor Solo (Minimoog)" },
 {  46, 127, "School Daze" },
 {  47, 127, "Bellsinger (ethnic Bowl)" },
 {  48, 127, "Square Wave (Minimoog)" },
 {  49, 127, "String Section 1 (Quartet)" },
 {  50, 127, "String Section 2" },
 {  51, 127, "String Section 3 (Warm)" },
 {  52, 127, "Pizzicato" },
 {  53, 127, "Violin 1" },
 {  54, 127, "Violin 2" },
 {  55, 127, "Cello 1 (Velocity)" },
 {  56, 127, "Cello 2" },
 {  57, 127, "Contrabass" },
 {  58, 127, "Harp 1 (Nylon)" },
 {  59, 127, "Harp 2" },
 {  60, 127, "Guitar 1 (Nylon)" },
 {  61, 127, "Guitar 2 (Steel)" },
 {  62, 127, "Electric Guitar 1 (Jazz)" },
 {  63, 127, "Electric Guitar 2 (Single-coil)" },
 {  64, 127, "Sitar" },
 {  65, 127, "Acoustic Bass 1" },
 {  66, 127, "Acoustic Bass 2 (Wild)" },
 {  67, 127, "Electric Bass 1 (Overdriven)" },
 {  68, 127, "Electric Bass 2 (Picked)" },
 {  69, 127, "Slap Bass 1" },
 {  70, 127, "Slap Bass 2" },
 {  71, 127, "Fretless 1" },
 {  72, 127, "Fretless 2" },
 {  73, 127, "Flute 1 (Velocity)" },
 {  74, 127, "Flute 2" },
 {  75, 127, "Piccolo 1" },
 {  76, 127, "Piccolo 2" },
 {  77, 127, "Recorder" },
 {  78, 127, "Pan Pipes" },
 {  79, 127, "Saxophone 1 (Soprano)" },
 {  80, 127, "Saxophone 2 (Alto)" },
 {  81, 127, "Saxophone 3 (tenor)" },
 {  82, 127, "Saxophone 4 (Baritone)" },
 {  83, 127, "Clarinet 1" },
 {  84, 127, "Clarinet 2" },
 {  85, 127, "Oboe" },
 {  86, 127, "English Horn" },
 {  87, 127, "Bassoon" },
 {  88, 127, "Harmonica" },
 {  89, 127, "Trumpet 1 (Velocity)" },
 {  90, 127, "Trumpet 2" },
 {  91, 127, "Trombone 1" },
 {  92, 127, "Trombone 2 (Dark)" },
 {  93, 127, "French Horn 1" },
 {  94, 127, "French Horn 2 (Dark)" },
 {  95, 127, "Tuba" },
 {  96, 127, "Brass Section 1" },
 {  97, 127, "Brass Section 2" },
 {  98, 127, "Vibe 1 (Virbaphone)" },
 {  99, 127, "Vibe 2 (Warm Marimba)" },
 { 100, 127, "Synth Mallet" },
 { 101, 127, "Windbell (detuned)" },
 { 102, 127, "Glock" },
 { 103, 127, "Tube Bell" },
 { 104, 127, "Xylophone" },
 { 105, 127, "Marimba" },
 { 106, 127, "Koto" },
 { 107, 127, "Sho (Kokyu)" },
 { 108, 127, "Shakuhachi" },
 { 109, 127, "Whistle 1 (Breathy)" },
 { 110, 127, "Whistle 2" },
 { 111, 127, "Bottle Blow" },
 { 112, 127, "Breathpipe" },
 { 113, 127, "Timpani" },
 { 114, 127, "Melodic Tom (Electric)" },
 { 115, 127, "Deep Snare" },
 { 116, 127, "Electric Percussion 1 (Synth)" },
 { 117, 127, "Electric Percussion 2" },
 { 118, 127, "Taiko" },
 { 119, 127, "Taiko Rim" },
 { 120, 127, "Cymbal" },
 { 121, 127, "Castanets" },
 { 122, 127, "Triangle" },
 { 123, 127, "Orchestra Hit (Brass+Euro)" },
 { 124, 127, "Telephone (Mechanical)" },
 { 125, 127, "Bird Tweet (Blackbird)" },
 { 126, 127, "One Note Jam" },
 { 127, 127, "Water Bell" },
 { 128, 127, "Jungle Tune" },
 {   0,   0, "" }
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
   printf(" *\n * Instrument list for the MT-32\n");
   printf("-->\n");
   printf("<aeonwave>\n\n");
   printf(" <midi name=\"MT-32\">\n\n");

   for (b=127; b<128; ++b)
   {
      char f = 0;
      int i = 0;
      do
      {
         if (!f)
         {  
            printf("  <bank n=\"%i\">\n", b);
            f = 1;
         }

         if (inst_table[i].bank == 127)
         {
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
   
