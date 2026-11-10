// ==========================================================================
//             ___  _         _         __      ___    _ _   _
//            |   \(_)____ __| |__ _ _  \ \    / (_)__| | |_| |_
//            | |) | (_-< '_ \ / _` | || \ \/\/ /| / _` |  _| ' \
//            |___/|_/__/ .__/_\__,_|\_, |\_/\_/ |_\__,_|\__|_||_|
//                      |_|          |__/
// ==========================================================================
//
// Display Width Computation
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

#include "displaywidth.h"


// ###### Skip ANSI sequence ################################################
size_t skipAnsiSequence(std::string_view sv, size_t offset) noexcept
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


// ###### Decode UTF-8 Code Point ###########################################
static char32_t decodeNextUtf8(std::string_view sv, size_t& offset) noexcept
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
static bool isZeroWidth(char32_t codePoint) noexcept
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
          ((codePoint >= 0x0900) && (codePoint <= 0x0902)) || // Devanagari signs (Candrabindu, Anusvara)
          (codePoint == 0x093a)                            || // Devanagari vowel sign oe
          (codePoint == 0x093c)                            || // Devanagari sign nukta
          ((codePoint >= 0x0941) && (codePoint <= 0x0948)) || // Devanagari vowel signs (u, uu, vocalic r/rr, e, ai)
          (codePoint == 0x094d)                            || // Devanagari sign virama (halant)
          ((codePoint >= 0x0951) && (codePoint <= 0x0957)) || // Devanagari stress/accent signs
          ((codePoint >= 0x0962) && (codePoint <= 0x0963)) || // Devanagari vowel signs (vocalic l/ll)
          ((codePoint >= 0x1cd0) && (codePoint <= 0x1cd2)) || // Vedic tone marks
          ((codePoint >= 0x1cd4) && (codePoint <= 0x1ce0)) || // Vedic tone marks
          ((codePoint >= 0x1ce2) && (codePoint <= 0x1ce8)) || // Vedic tone marks
          (codePoint == 0x1ced)                            || // Vedic sign tiryak
          ((codePoint >= 0x1cf8) && (codePoint <= 0x1cf9)) || // Vedic tone ring marks
          ((codePoint >= 0xa8e0) && (codePoint <= 0xa8f1)) || // Combining Devanagari signs
          (codePoint == 0x0981)                            || // Bengali sign candrabindu
          (codePoint == 0x09bc)                            || // Bengali sign nukta
          ((codePoint >= 0x09c1) && (codePoint <= 0x09c4)) || // Bengali vowel signs (u, uu, vocalic r/rr)
          (codePoint == 0x09cd)                            || // Bengali sign virama (hasant)
          ((codePoint >= 0x09e2) && (codePoint <= 0x09e3)) || // Bengali vowel signs (vocalic l/ll)
          (codePoint == 0x09fe)                            || // Bengali sandhi mark
          (codePoint == 0x0e31)                            || // Thai character mai han-akat
          ((codePoint >= 0x0e34) && (codePoint <= 0x0e3a)) || // Thai vowel signs (sara i .. phinthu)
          ((codePoint >= 0x0e47) && (codePoint <= 0x0e4e)) || // Thai tone marks & signs (maitaikhu .. yamakkan)
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
static bool isRegionalIndicator(char32_t codePoint) noexcept
{
   return (codePoint >= 0x1f1e6) && (codePoint <= 0x1f1ff);
}


// ###### Wide emoji check ##################################################
static bool isEmojiWide(char32_t codePoint) noexcept
{
   return ((codePoint >=  0x23e9) && (codePoint <=  0x23f3)) || // ⏱ (U+23F1 Stopwatch), timers
          ((codePoint >=  0x23f8) && (codePoint <=  0x23fa)) || // Media controls
          ((codePoint >=  0x2600) && (codePoint <=  0x27bf)) || // Dingbats & miscellaneous symbols
          ((codePoint >= 0x1f300) && (codePoint <= 0x1f64f)) || // Pictographs, Emoticons (🔥)
          ((codePoint >= 0x1f680) && (codePoint <= 0x1f6ff)) || // Transport, Maps (🛰, 🛡)
          ((codePoint >= 0x1f7e0) && (codePoint <= 0x1f7eb)) || // Colored geometric shapes (🟢)
          ((codePoint >= 0x1f900) && (codePoint <= 0x1f9ff)) || // Supplemental Symbols
          ((codePoint >= 0x1fa70) && (codePoint <= 0x1faff));   // Symbols Extended-A
}


// ###### Wide CJK Check ####################################################
static bool isCjkWide(char32_t codePoint) noexcept
{
   return ((codePoint >=  0x1100) && (codePoint <=  0x115f)) || // Hangul Jamo
          ((codePoint >=  0x2329) && (codePoint <=  0x232a)) || // Angle brackets
          ((codePoint >=  0x2e80) && (codePoint <=  0xa4cf)) || // CJK Radicals, Kanji, Katakana, Hiragana
          ((codePoint >=  0xac00) && (codePoint <=  0xd7a3)) || // Hangul Syllables
          ((codePoint >=  0xf900) && (codePoint <=  0xfaff)) || // CJK Compatibility
          ((codePoint >=  0xfe10) && (codePoint <=  0xfe19)) || // Vertical forms
          ((codePoint >=  0xfe30) && (codePoint <=  0xfe6f)) || // CJK Compatibility Forms
          ((codePoint >=  0xff01) && (codePoint <=  0xff60)) || // Fullwidth Forms
          ((codePoint >=  0xffe0) && (codePoint <=  0xffe6)) || // Fullwidth symbols
          ((codePoint >= 0x20000) && (codePoint <= 0x2fa1f)) || // SIP Ideographs
          ((codePoint >= 0x30000) && (codePoint <= 0x3134f));   // TIP Ideographs
}


// ###### Compute display width #############################################
size_t computeDisplayWidth(std::string_view sv) noexcept
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

      // Bengali digraph O + YA / YYA (ওয়) renders as a single column glyph:
      if(codePoint == 0x0993) {
         size_t peekOffset = offset;
         if(peekOffset < sv.size()) {
            const char32_t nextCp = decodeNextUtf8(sv, peekOffset);
            if((nextCp == 0x09df) || (nextCp == 0x09af)) {
               continue;
            }
         }
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
