
#pragma once

#if 0
void remove_double_spaces(std::string& str);
void tokenize_words(std::string& str);
void str_prepend(std::string& name, std::string section, const char *replacement = nullptr);
void str_append(std::string& name, std::string& suffix, std::string section, const char *replacement = nullptr);
void str_remove(std::string& name, std::string section);
void str_cleanup(std::string& name);
#endif

std::string to_filename(const std::string& name);
std::string canonical_name(std::string name);

