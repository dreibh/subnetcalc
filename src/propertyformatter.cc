// ==========================================================================
//             ____        _     _   _      _    ____      _
//            / ___| _   _| |__ | \ | | ___| |_ / ___|__ _| | ___
//            \___ \| | | | '_ \|  \| |/ _ \ __| |   / _` | |/ __|
//             ___) | |_| | |_) | |\  |  __/ |_| |__| (_| | | (__
//            |____/ \__,_|_.__/|_| \_|\___|\__|\____\__,_|_|\___|
//
//                    ---  IPv4/IPv6 Subnet Calculator  ---
//                   https://www.nntb.no/~dreibh/subnetcalc/
// ==========================================================================
//
// SubNetCalc - IPv4/IPv6 Subnet Calculator
// Copyright (C) 2009-2026 by Thomas Dreibholz
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
std::string PropertyFormatter::sanitizeBidiBrackets(std::string_view sv, bool isRtl)
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


// ###### Skip ANSI sequence ################################################
size_t PropertyFormatter::skipAnsiSequence(std::string_view sv, size_t offset) noexcept
{
   if(offset + 1 >= sv.size()) {
      return offset + 1;
   }

   const char nextChar = sv[offset + 1];

   // CSI: ESC [ ... [0x40 - 0x7e] (Colors, bold, underline, italic, etc.)
   if(nextChar == '[') {
      size_t cursor = offset + 2;
      while(cursor < sv.size()) {
         const auto byte = static_cast<unsigned char>(sv[cursor]);
         if((byte >= 0x40) && (byte <= 0x7e)) {
            return cursor + 1;
         }
         cursor++;
      }
      return sv.size();
   }

   // OSC: ESC ] ... (BEL | ESC \)
   if(nextChar == ']') {
      size_t cursor = offset + 2;
      while(cursor < sv.size()) {
         if(sv[cursor] == 0x07) {
            return cursor + 1;
         }
         if((sv[cursor] == 0x1b) && (cursor + 1 < sv.size()) && (sv[cursor + 1] == '\\')) {
            return cursor + 2;
         }
         cursor++;
      }
      return sv.size();
   }

   // Character set shifts: ESC (, ESC ), ESC *, ESC +
   if((nextChar == '(') || (nextChar == ')') || (nextChar == '*') || (nextChar == '+')) {
      return (offset + 2 < sv.size()) ? (offset + 3) : sv.size();
   }

   return offset + 2;
}


// ###### Compute display width #############################################
size_t PropertyFormatter::computeDisplayWidth(std::string_view sv) noexcept
{
   size_t colWidth = 0;
   size_t offset   = 0;

   while(offset < sv.size()) {
      if(sv[offset] == 0x1b) {
         offset = skipAnsiSequence(sv, offset);
         continue;
      }

      const char32_t codePoint = decodeNextUtf8(sv, offset);

      if(isZeroWidth(codePoint)) {
         continue;
      }

      // Emoji and CJK ideographs occupy 2 terminal columns:
      if(isEmojiWide(codePoint) || isCjkWide(codePoint)) {
         colWidth += 2;
         continue;
      }

      // Regional Indicator symbols (Flags: two combined indicators span 2 columns):
      if(isRegionalIndicator(codePoint)) {
         colWidth += 1;
         continue;
      }

      colWidth += 1;
   }
   return colWidth;
}


// ###### Decode UTF-8 Code Point ###########################################
char32_t PropertyFormatter::decodeNextUtf8(std::string_view sv, size_t& offset) noexcept
{
   const auto byte0 = static_cast<unsigned char>(sv[offset]);
   if(byte0 < 0x80) {
      offset += 1;
      return byte0;
   }
   if(((byte0 & 0xe0) == 0xc0) && (offset + 1 < sv.size())) {
      const char32_t codePoint = ((byte0 & 0x1f) << 6) | (static_cast<unsigned char>(sv[offset + 1]) & 0x3f);
      offset += 2;
      return codePoint;
   }
   if(((byte0 & 0xf0) == 0xe0) && (offset + 2 < sv.size())) {
      const char32_t codePoint = ((byte0 & 0x0f) << 12) |
                                 ((static_cast<unsigned char>(sv[offset + 1]) & 0x3f) << 6) |
                                 (static_cast<unsigned char>(sv[offset + 2]) & 0x3f);
      offset += 3;
      return codePoint;
   }
   if(((byte0 & 0xf8) == 0xf0) && (offset + 3 < sv.size())) {
      const char32_t codePoint = ((byte0 & 0x07) << 18) |
                                 ((static_cast<unsigned char>(sv[offset + 1]) & 0x3f) << 12) |
                                 ((static_cast<unsigned char>(sv[offset + 2]) & 0x3f) << 6) |
                                 (static_cast<unsigned char>(sv[offset + 3]) & 0x3f);
      offset += 4;
      return codePoint;
   }
   offset += 1;
   return 0xfffd;
}


// ###### Zero-width code point check #######################################
bool PropertyFormatter::isZeroWidth(char32_t codePoint) noexcept
{
   return (codePoint <= 0x001f) ||
          ((codePoint >= 0x007f) && (codePoint <= 0x009f)) || // Control characters
          ((codePoint >= 0x0300) && (codePoint <= 0x036f)) || // Combining Diacritical Marks
          ((codePoint >= 0x0591) && (codePoint <= 0x05bd)) || // Hebrew accents / cantillation
          (codePoint == 0x05bf)                            || // Hebrew rafe
          ((codePoint >= 0x05c1) && (codePoint <= 0x05c2)) || // Hebrew shin/sin dots
          ((codePoint >= 0x05c4) && (codePoint <= 0x05c5)) || // Hebrew upper/lower dots
          (codePoint == 0x05c7)                            || // Hebrew qamats qatan
          ((codePoint >= 0x0610) && (codePoint <= 0x061a)) || // Arabic Quranic marks
          ((codePoint >= 0x064b) && (codePoint <= 0x065f)) || // Arabic Tashkeel (Fatha, Shaddah, etc.)
          (codePoint == 0x0670)                            || // Arabic superscript alef
          ((codePoint >= 0x06d6) && (codePoint <= 0x06dc)) || // Arabic Quranic small marks
          ((codePoint >= 0x06df) && (codePoint <= 0x06e8)) || // Arabic Quranic small marks
          ((codePoint >= 0x06ea) && (codePoint <= 0x06ed)) || // Arabic tone marks
          ((codePoint >= 0x1ab0) && (codePoint <= 0x1aff)) || // Combining Diacritical Extended
          ((codePoint >= 0x1dc0) && (codePoint <= 0x1dff)) || // Combining Diacritical Supplement
          ((codePoint >= 0x20d0) && (codePoint <= 0x20ff)) || // Combining Marks for Symbols
          ((codePoint >= 0xfe20) && (codePoint <= 0xfe2f)) || // Combining Half Marks
          ((codePoint >= 0x200b) && (codePoint <= 0x200f)) || // Zero-width spaces, LRM, RLM
          ((codePoint >= 0x202a) && (codePoint <= 0x202e)) || // Directional embeddings / overrides
          ((codePoint >= 0x2060) && (codePoint <= 0x206f)) || // Word joiners, isolates
          ((codePoint >= 0xfe00) && (codePoint <= 0xfe0f)) || // Variation Selectors (VS1-VS16)
          ((codePoint >= 0xe0100) && (codePoint <= 0xe01ef)); // Variation Selectors Supplement
}


// ###### Regional Indicator check ##########################################
bool PropertyFormatter::isRegionalIndicator(char32_t codePoint) noexcept
{
   return (codePoint >= 0x1f1e6) && (codePoint <= 0x1f1ff);
}


// ###### Wide emoji check ##################################################
bool PropertyFormatter::isEmojiWide(char32_t codePoint) noexcept
{
   return ((codePoint >= 0x23e9) && (codePoint <= 0x23f3)) || // ⏱ (U+23F1 Stopwatch), timers
          ((codePoint >= 0x23f8) && (codePoint <= 0x23fa)) || // Media controls
          ((codePoint >= 0x2600) && (codePoint <= 0x27bf)) || // Dingbats & miscellaneous symbols
          ((codePoint >= 0x1f300) && (codePoint <= 0x1f64f)) || // Pictographs, Emoticons (🔥)
          ((codePoint >= 0x1f680) && (codePoint <= 0x1f6ff)) || // Transport, Maps (🛰, 🛡)
          ((codePoint >= 0x1f7e0) && (codePoint <= 0x1f7eb)) || // Colored geometric shapes (🟢)
          ((codePoint >= 0x1f900) && (codePoint <= 0x1f9ff)) || // Supplemental Symbols
          ((codePoint >= 0x1fa70) && (codePoint <= 0x1faff));   // Symbols Extended-A
}


// ###### Wide CJK Check ####################################################
bool PropertyFormatter::isCjkWide(char32_t codePoint) noexcept
{
   return ((codePoint >= 0x1100) && (codePoint <= 0x115f))   || // Hangul Jamo
          ((codePoint >= 0x2329 && codePoint <= 0x232a))   || // Angle brackets
          ((codePoint >= 0x2e80) && (codePoint <= 0xa4cf))   || // CJK Radicals, Kanji, Katakana, Hiragana
          ((codePoint >= 0xac00) && (codePoint <= 0xd7a3))   || // Hangul Syllables
          ((codePoint >= 0xf900) && (codePoint <= 0xfaff))   || // CJK Compatibility
          ((codePoint >= 0xfe10) && (codePoint <= 0xfe19))   || // Vertical forms
          ((codePoint >= 0xfe30) && (codePoint <= 0xfe6f))   || // CJK Compatibility Forms
          ((codePoint >= 0xff01) && (codePoint <= 0xff60))   || // Fullwidth Forms
          ((codePoint >= 0xffe0) && (codePoint <= 0xffe6))   || // Fullwidth symbols
          ((codePoint >= 0x20000) && (codePoint <= 0x2fa1f)) || // SIP Ideographs
          ((codePoint >= 0x30000) && (codePoint <= 0x3134f));   // TIP Ideographs
}


// ###### Print multiline value #############################################
void PropertyFormatter::printMultilineValue(std::ostream& os, const std::string& value, size_t leadingWidth, bool rtl)
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
         // Technical values (IPs, formulas, brackets, error strings) start with ASCII (< 0x80)
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
