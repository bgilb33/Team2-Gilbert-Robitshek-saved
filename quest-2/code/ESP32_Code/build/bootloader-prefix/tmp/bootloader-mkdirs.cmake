# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/benja/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/tmp"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/src"
  "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/benja/Code/School/EC444/Team2-Gilbert-Robitshek/quest-2/code/sample_project/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
