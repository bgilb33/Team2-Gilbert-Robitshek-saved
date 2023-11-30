# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/noahrobitshek/esp/esp-idf/components/bootloader/subproject"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/tmp"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/src"
  "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/noahrobitshek/esp/EC 444/Team2-Gilbert-Robitshek/quest-5/code/MeterDisplayTest1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
