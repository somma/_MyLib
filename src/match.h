// File:        match.cpp
// Author:      Robert A. van Engelen, engelen@genivia.com
// Date:        August 5, 2019
// License:     The Code Project Open License (CPOL)
//              https://www.codeproject.com/info/cpol10.aspx
#pragma once
#include <string>

// set to 1 to enable dotglob: *. ?, and [] match a . (dotfile) at the begin or after each /
#define DOTGLOB 1

// set to 1 to enable case-insensitive glob matching
#define NOCASEGLOB 0

#define CASE(c) (NOCASEGLOB ? tolower(c) : (c))

// Windows \ versus normal / path separator
//#ifdef OS_WIN
#define PATHSEP '\\'
//#else
//#define PATHSEP '/'
//#endif

/// @brief	Returns true if text string matches wild pattern with * and ?
bool match_string(const std::string& text, const std::string& wild);

/// @brief	Returns TRUE if text string matches glob pattern with * and ?
bool glob_match_string(const std::string& text, const std::string& glob);

/// @brief Returns TRUE if text string matches gitignore-style glob pattern
bool gitignore_glob_match_string(const std::string& text, const std::string& glob);
