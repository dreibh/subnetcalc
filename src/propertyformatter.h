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

#ifndef PROPERTYFORMATTER_H
#define PROPERTYFORMATTER_H

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

class PropertyFormatter {
   public:
   // ====== Public methods =================================================
   explicit PropertyFormatter(std::string              indent    = "   ",
                              std::vector<std::string> prefixes  = {"", "- ", "+ "},
                              std::string              separator = "=",
                              bool                     isRtl     = false);

   [[nodiscard]] bool isRtl() const noexcept {
      return IsRtl;
   }

   inline void setIsRtl(bool isRtl) noexcept {
      IsRtl = isRtl;
   }

   inline void newSection() noexcept {
      CurrentSection++;
   }

   PropertyFormatter& add(size_t level, std::string key, std::string value = "");
   PropertyFormatter& add(std::string key, std::string value = "");
   PropertyFormatter& addLine(size_t level, std::string text);

   void print(std::ostream& os) const;
   [[nodiscard]] std::string str() const;

   friend std::ostream& operator<<(std::ostream& os, const PropertyFormatter& pf);

   static size_t skipAnsiSequence(std::string_view sv, size_t offset) noexcept;
   static size_t computeDisplayWidth(std::string_view sv) noexcept;


   // ====== Private methods ================================================
   private:
   static char32_t decodeNextUtf8(std::string_view sv, size_t& offset) noexcept;
   static bool isZeroWidth(char32_t codePoint) noexcept;
   static bool isRegionalIndicator(char32_t codePoint) noexcept;
   static bool isEmojiWide(char32_t codePoint) noexcept;
   static bool isCjkWide(char32_t codePoint) noexcept;
   static void printMultilineValue(std::ostream& os, const std::string& value,
                                   size_t leadingWidth, bool rtl);
   static bool containsAsciiAlnum(std::string_view sv) noexcept;


   // ====== Private attributes =============================================
   private:
   // Unicode directional marks (UTF-8):
   static constexpr std::string_view utf8Lrm = "\xe2\x80\x8e"; // U+200E (Left-to-Right Mark)
   static constexpr std::string_view utf8Rlm = "\xe2\x80\x8f"; // U+200F (Right-to-Left Mark)

   struct Entry {
      size_t      section      { 0 };
      size_t      level        { 0 };
      std::string key          { };
      std::string value        { };
      bool        hasSeparator { true };
      size_t      keyWidth     { 0 };
   };

   std::string              Indent;
   std::vector<std::string> Prefixes;
   std::string              Separator;
   bool                     IsRtl { false };
   std::vector<Entry>       Entries;
   size_t                   CurrentSection{0};


};

#endif // PROPERTYFORMATTER_H
