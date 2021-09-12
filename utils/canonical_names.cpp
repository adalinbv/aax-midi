
#include <string>
#include <regex>

void
tokenize_words(std::string& str)
{
    std::regex regex{R"([ ()]+)"}; // split on a space
    std::sregex_token_iterator it{str.begin(), str.end(), regex, -1};
    std::vector<std::string> words{it, {}};

    str = "";
    for (auto it : words)
    {
        it[0] = std::toupper(it[0]);
        str += it + " ";
    }
    if (!str.empty()) str.resize(str.length() - 1);
}

std::size_t
find_word(std::string& name, std::string& section)
{
    bool found = false;
    std::size_t pos = name.find(section);
    if (pos != std::string::npos)
    {
        switch(name[pos])
        {
        case ' ':
        case ')':
        case ',':
            break;
        default:
            pos =  std::string::npos;
            break;
        }
    }
    return pos;
}


void
str_prepend(std::string& name, std::string section, const char *replacement = nullptr)
{
    std::size_t pos;

    tokenize_words(section);

    pos = find_word(name, section);
    if (pos != std::string::npos && !isalpha(name[pos+section.size()]))
    {
        name.replace(pos, section.size(), "");

        if (replacement)
        {
            section = replacement;
            tokenize_words(section);
        }
        name = section + " " + name;
    }
}

void
str_append(std::string& name, std::string& suffix, std::string section, const char *replacement = nullptr)
{
    bool found = false;
    std::size_t pos;

    tokenize_words(section);

    pos = find_word(name, section);
    if (pos != std::string::npos)
    {
        if (!suffix.empty()) suffix.append(", ");
        if (!replacement)
        {
            section[0] = std::tolower(section[0]);
            suffix.append(section);
        }
        else suffix.append(replacement);
        name.replace(pos, section.size(), "");
    }
}

void
str_replace(std::string& name, std::string section, const std::string& replacement = "")
{
    std::string result;
    std::regex expr(section);
    std::regex_replace(std::back_inserter(result), name.begin(), name.end(),
                       expr, replacement);
    name = result;
}

void
str_cleanup(std::string& name)
{
    str_replace(name, "  +", " ");
    str_replace(name, "^ ");
    str_replace(name, "^\\+ ");
    str_replace(name, "\\+$");
    str_replace(name, " $");
    str_replace(name, " ,$");
    str_replace(name, "( \\+ )");
    str_replace(name, "()");
}

std::string
canonical_name(std::string name)
{
    std::string suffix;

    tokenize_words(name);

    str_prepend(name, "square");
    str_prepend(name, "sine wave");
    str_prepend(name, "sine");
    str_prepend(name, "pulse");
    str_prepend(name, "saw + pulse");
    str_prepend(name, "saw");
    str_prepend(name, "sawtooth");
    str_prepend(name, "double");
    str_prepend(name, "finger");
    str_prepend(name, "pick");
    str_prepend(name, "bass");
    str_prepend(name, "Mallet");
    str_prepend(name, "electric");
    str_prepend(name, "acoustic");
    str_prepend(name, "synth");
    str_prepend(name, "analog");
    str_prepend(name, "FM");
    str_prepend(name, "DX");
    str_prepend(name, "LM");
    str_prepend(name, "big");
    str_prepend(name, "thick");
    str_prepend(name, "fat");
    str_prepend(name, "digi", "digital");
    str_prepend(name, "digital");
    str_prepend(name, "heavy");
    str_prepend(name, "wire");
    str_prepend(name, "rhythm");
    str_prepend(name, "funk");
    str_prepend(name, "jazzy", "jazz");
    str_prepend(name, "jazz");
    str_prepend(name, "rock", "rock");
    str_prepend(name, "baroque", "chamber");
    str_prepend(name, "chamber");
    str_prepend(name, "smooth");
    str_prepend(name, "soft");
    str_prepend(name, "reverse");
    str_prepend(name, "clean");
    str_prepend(name, "over driven");
    str_prepend(name, "overdriven", "over driven");
    str_prepend(name, "overdrive", "over driven");
    str_prepend(name, "distorted");
    str_prepend(name, "distrtion");
    str_prepend(name, "attack");
    str_prepend(name, "rubber");
    str_prepend(name, "modulated");
    str_prepend(name, "modular");
    str_prepend(name, "pedal");
    str_prepend(name, "french");
    str_prepend(name, "italian");
    str_prepend(name, "tango");
    str_prepend(name, "diapasom");
    str_prepend(name, "praise");
    str_prepend(name, "halo");
    str_prepend(name, "sweep");
    str_prepend(name, "metallic");
    str_prepend(name, "bowed");
    str_prepend(name, "poly");
    str_prepend(name, "choir");
    str_prepend(name, "itopia");
    str_prepend(name, "new age");
    str_prepend(name, "voice");
    str_prepend(name, "charang");
    str_prepend(name, "calliope");
    str_prepend(name, "shwimmer");
    str_prepend(name, "chiff");
    str_prepend(name, "50's", "early");
    str_prepend(name, "60's", "vintage");
    str_prepend(name, "70's", "classic");
    str_prepend(name, "hammond");
    str_prepend(name, "steel");
    str_prepend(name, "nylon");

    str_append(name, suffix, "dark");
    str_append(name, suffix, "bright");
    str_append(name, suffix, "mellow");
    str_append(name, suffix, "warm");
    str_append(name, suffix, "slow");
    str_append(name, suffix, "slow attack", "slow");
    str_append(name, suffix, "fast");
    str_append(name, suffix, "stereo", "wide");
    str_append(name, suffix, "wide");
    str_append(name, suffix, "octave mix", "octave");
    str_append(name, suffix, "octave mix", "octave");
    str_append(name, suffix, "coupled", "octave");
    str_append(name, suffix, "octave");
    str_append(name, suffix, "phase", "phased");
    str_append(name, suffix, "phased");
    str_append(name, suffix, "chorus", "chorused");
    str_append(name, suffix, "chorused");
    str_append(name, suffix, "flanger", "flanged");
    str_append(name, suffix, "flanged");
    str_append(name, suffix, "detuned");
    str_append(name, suffix, "hard", "wild");
    str_append(name, suffix, "velocity", "wild");
    str_append(name, suffix, "power", "wild");
    str_append(name, suffix, "expressive", "wild");
    str_append(name, suffix, "attack", "velocity");
    str_append(name, suffix, "velocity mix", "velocity");
    str_append(name, suffix, "velocity");
    str_append(name, suffix, "cross-fade");
    str_append(name, suffix, "key-off");
    str_append(name, suffix, "muted");
    str_append(name, suffix, "modern");

    str_cleanup(name);

    if (!suffix.empty()) {
        name += " (" + suffix + ")";
    }

    return name;
}

std::string
to_filename(const std::string& name)
{
   int i, len = name.size();
   std::string filename;

   filename.push_back(std::tolower(name[0]));
   for (i=1; i<len; ++i)
   {
      if (name[i] != '(' && name[i] != ')')
      {
         if (name[i] == ' ') filename.push_back('-');
         else filename.push_back(std::tolower(name[i]));
      }
   }

   str_replace(filename, "--", "-");
   str_replace(filename, "-+-", "+");

   return filename;
}
