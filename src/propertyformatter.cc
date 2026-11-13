// ==========================================================================
//  ___                       _        ___                   _   _
// | _ \_ _ ___ _ __  ___ _ _| |_ _  _| __|__ _ _ _ __  __ _| |_| |_ ___ _ _
// |  _/ '_/ _ \ '_ \/ -_) '_|  _| || | _/ _ \ '_| '  \/ _` |  _|  _/ -_) '_|
// |_| |_| \___/ .__/\___|_|  \__|\_, |_|\___/_| |_|_|_\__,_|\__|\__\___|_|
//             |_|                |__/
// ==========================================================================
//
// Property Formatter
// Copyright (C) 2026 by Thomas Dreibholz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Contact: thomas.dreibholz@gmail.com

#include "propertyformatter.h"
#include "displaywidth.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>


// ###### Constructor #######################################################
PropertyFormatter::PropertyFormatter(std::string              indent,
                                     std::vector<std::string> prefixes,
                                     std::string              separator,
                                     bool                     isRtl)
   : Indent(std::move(indent)),
     Prefixes(std::move(prefixes)),
     Separator(std::move(separator)),
     IsRtl(isRtl)
{
}


// ====== Sanitize text inside parentheses for RTL terminals ################
std::string PropertyFormatter::sanitizeBidiBrackets(std::string_view sv,
                                                    bool             isRtl)
{
   if(!isRtl || sv.empty()) {
      return std::string(sv);
   }

   std::string result;
   result.reserve(sv.size() + 16);

   size_t i = 0;
   while(i < sv.size()) {
      if(sv[i] == '(') {
         size_t j = i + 1;
         bool onlyAscii = true;

         while((j < sv.size()) && (sv[j] != ')')) {
            if(static_cast<unsigned char>(sv[j]) >= 0x80) {
               onlyAscii = false;
               break;
            }
            j++;
         }

         if(onlyAscii && (j < sv.size()) && (sv[j] == ')')) {
            result += utf8Lrm;
            result.append(sv.data() + i, (j - i + 1));
            result += utf8Lrm;
            i = j + 1;
            continue;
         }
      }

      result += sv[i];
      i++;
   }
   return result;
}


// ###### Add Key/Value Pair ################################################
PropertyFormatter& PropertyFormatter::add(size_t level, std::string key, std::string value)
{
   const std::string sanitizedKey = sanitizeBidiBrackets(key, IsRtl);
   const size_t width = computeDisplayWidth(sanitizedKey);
   Entries.push_back(Entry{
      .section      = CurrentSection,
      .level        = level,
      .key          = std::move(sanitizedKey),
      .value        = std::move(value),
      .hasSeparator = true,
      .keyWidth     = width
   });
   return *this;
}


// ###### Add Key/Value Pair at Level 0 #####################################
PropertyFormatter& PropertyFormatter::add(std::string key, std::string value)
{
   return add(0, std::move(key), std::move(value));
}


// ###### Add Standalone Text Line ##########################################
PropertyFormatter& PropertyFormatter::addLine(size_t level, std::string text)
{
   std::string sanitizedText = sanitizeBidiBrackets(text, IsRtl);
   Entries.push_back(Entry{
      .section      = CurrentSection,
      .level        = level,
      .key          = std::move(sanitizedText),
      .value        = "",
      .hasSeparator = false,
      .keyWidth     = 0
   });
   return *this;
}


// ###### Print Output ######################################################
void PropertyFormatter::print(std::ostream& os) const
{
   struct SectionLevelKey {
      size_t section;
      size_t level;
      bool operator==(const SectionLevelKey& other) const noexcept {
         return ( (section == other.section) &&
                  (level == other.level) );
      }
   };

   struct Hasher {
      size_t operator()(const SectionLevelKey& slKey) const noexcept {
         return (std::hash<size_t>()(slKey.section) ^ (std::hash<size_t>()(slKey.level) << 1));
      }
   };

   std::unordered_map<SectionLevelKey, size_t, Hasher> maxWidths;
   for(const auto& entry : Entries) {
      if(entry.hasSeparator) {
         const SectionLevelKey slKey{entry.section, entry.level};
         // Protected against Windows SDK #define max macro
         maxWidths[slKey] = (std::max)(maxWidths[slKey], entry.keyWidth);
      }
   }

   for(const auto& entry : Entries) {
      if(!entry.hasSeparator) {
         if(IsRtl) {
            // Space + RLM binds the RTL mark to cell 0, forcing RTL paragraph direction:
            const size_t totalIndentWidth = entry.level * computeDisplayWidth(Indent);
            if(totalIndentWidth > 0) {
               os << ' ' << utf8Rlm;
               for(size_t i = 1; i < totalIndentWidth; ++i) {
                  os << ' ';
               }
            }
            else {
               os << ' ' << utf8Rlm;
            }

            if(entry.level < Prefixes.size()) {
               os << Prefixes[entry.level];
            }
         }
         else {
            for(size_t i = 0; i < entry.level; ++i) {
               os << Indent;
            }
            if(entry.level < Prefixes.size()) {
               os << Prefixes[entry.level];
            }
         }

         os << entry.key << '\n';
         continue;
      }

      for(size_t i = 0; i < entry.level; ++i) {
         os << Indent;
      }

      if(entry.level < Prefixes.size()) {
         os << Prefixes[entry.level];
      }

      const SectionLevelKey slKey{entry.section, entry.level};
      const size_t maxKeyWidth = maxWidths[slKey];
      const size_t padding     = (maxKeyWidth >= entry.keyWidth) ? (maxKeyWidth - entry.keyWidth) : 0;

      os << entry.key;
      if(IsRtl) {
         os << utf8Rlm;
      }

      for(size_t i = 0; i < padding; ++i) {
         os << ' ';
      }

      if(entry.value.empty()) {
         os << ' ' << Separator << '\n';
      }
      else {
         os << ' ' << Separator << ' ';

         const size_t prefixWidth  = (entry.level < Prefixes.size()) ?
            computeDisplayWidth(Prefixes[entry.level]) : 0;
         const size_t indentWidth  = entry.level * computeDisplayWidth(Indent);
         const size_t leadingWidth = indentWidth + prefixWidth + maxKeyWidth +
                                        1 + computeDisplayWidth(Separator) + 1;

         printMultilineValue(os, entry.value, leadingWidth, IsRtl);
      }
   }
}


// ###### Convert to string #################################################
std::string PropertyFormatter::str() const
{
   std::ostringstream ss;
   print(ss);
   return ss.str();
}


// ###### Stream insertion operator #########################################
std::ostream& operator<<(std::ostream& os, const PropertyFormatter& pf)
{
   pf.print(os);
   return os;
}


// ###### Print multiline value #############################################
void PropertyFormatter::printMultilineValue(std::ostream&      os,
                                            const std::string& value,
                                            size_t             leadingWidth,
                                            bool               rtl)
{
   std::istringstream stream(value);
   std::string line;
   bool isFirstLine = true;

   while(std::getline(stream, line)) {
      if(!isFirstLine) {
         if(!line.empty()) {
            if(rtl) {
               // Space + RLM binds the RTL mark to cell 0, forcing RTL paragraph direction:
               os << ' ' << utf8Rlm;
               for(size_t i = 1; i < leadingWidth; ++i) {
                  os << ' ';
               }
            }
            else {
               for(size_t i = 0; i < leadingWidth; ++i) {
                  os << ' ';
               }
            }
         }
      }

      if(!line.empty()) {
         // Technical values (IPs, formulas, brackets, error strings) start with ASCII (< 0x80):
         const bool valIsLTR = rtl && startsWithAscii(line);
         if(valIsLTR) {
            os << utf8Lrm << line << utf8Lrm;
         }
         else {
            os << line;
         }
      }
      os << '\n';
      isFirstLine = false;
   }
}


// ###### Check if string starts with ASCII character #######################
bool PropertyFormatter::startsWithAscii(std::string_view sv) noexcept
{
   size_t offset = 0;
   while(offset < sv.size()) {
      if(sv[offset] == 0x1b) {
         offset = skipAnsiSequence(sv, offset);
         continue;
      }
      return static_cast<unsigned char>(sv[offset]) < 0x80;
   }
   return false;
}
