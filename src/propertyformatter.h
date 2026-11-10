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


   // ====== Private methods ================================================
   private:
   static void printMultilineValue(std::ostream& os, const std::string& value,
                                   size_t leadingWidth, bool rtl);
   static bool startsWithAscii(std::string_view sv) noexcept;
   static std::string sanitizeBidiBrackets(std::string_view sv, bool isRtl);


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
