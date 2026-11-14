#include <iostream>
#include "propertyformatter.h"

int main()
{
   PropertyFormatter formatter;

   // Level 0: Top-level fields
   formatter.add("System", "Workstation");
   formatter.add("Diagnostics", "");

   // Level 1: Bulleted items ("- ")
   formatter.addLine(1, "Test");
   formatter.add(1, "Memory", "16 GiB");

   // Level 2: Nested sub-properties ("+ ")
   formatter.add(2, "Check A", "OK");
   formatter.add(2, "Latency Check", "12 ms");

   std::cout << formatter;
   return 0;
}
