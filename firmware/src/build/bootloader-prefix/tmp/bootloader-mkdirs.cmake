# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/jaden/esp/esp-idf/components/bootloader/subproject"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/tmp"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/src/bootloader-stamp"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/src"
  "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/jaden/esp/aj-sr04m/src/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
