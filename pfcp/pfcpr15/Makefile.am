############################################################################
# Copyright (c) 2009-2019 Brian Waters
# Copyright (c) 2019 Sprint
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
############################################################################

#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example #usr#lib
lib_LIBRARIES = libpfcpr15.a

#######################################
# Build information for each library

# Sources for epc
libpfcpr15_a_SOURCES = \
   ../../src/pfcpr15.cpp

includedir = /usr/local/include/epc
include_HEADERS =          \
   ../../include/epc/pfcpr15.h    \
   ../../include/epc/pfcpr15inl.h

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libpfcpr15_a_CPPFLAGS = -fPIC -Wall -std=c++11 -I$(top_srcdir)/include/epc -I$(top_srcdir)/modules/rapidjson/include -I$(top_srcdir)/modules/spdlog/include -I$(top_srcdir)/modules/libpfcp/include
