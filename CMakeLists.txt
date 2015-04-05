
#/**
# Copyright (C) 2012-2014 Chincisan Octavian-Marius(mariuschincisan@gmail.com) - getic.net - N/A
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#*/

cmake_minimum_required(VERSION 2.6)

################################################################################################
# To HERE                                                                   }
################################################################################################
PROJECT(MITZAWC)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -fexceptions -fpermissive -D__USE_MISC")


ADD_EXECUTABLE(mitza config.cpp database.cpp glb.cpp htpargs.cpp main.cpp mimes.cpp site.cpp strops.cpp thr.cpp wcurl.cpp webpage.cpp)

include_directories (include)

TARGET_LINK_LIBRARIES(mitza pthread curl mysqlclient gnutls-openssl pcrecpp htmlcxx)

SET(EXECUTABLE_OUTPUT_PATH ../bin)

