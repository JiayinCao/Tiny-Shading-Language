#
#   This file is a part of Tiny-Shading-Language or TSL, an open-source cross
#   platform programming shading language.
#
#   Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.
#
#   TSL is a free software written for educational purpose. Anyone can distribute
#   or modify it under the the terms of the GNU General Public License Version 3 as
#   published by the Free Software Foundation. However, there is NO warranty that
#   all components are functional in a perfect manner. Without even the implied
#   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with
#   this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
#

MAKEFLAGS += --silent

# Operating system name, it could be Darwin or Linux
OS             := $(shell uname -s | tr A-Z a-z)

# Mac OS
ifeq ($(OS), darwin)
	UPDATE_DEP_COMMAND = sh ./build-files/mac/getdep.sh
endif

# Ubuntu
ifeq ($(OS), linux)
	UPDATE_DEP_COMMAND = sh ./build-files/ubuntu/getdep.sh

	# It looks like the library built on one verion of Ubuntu can be shared across multiple
	# Then there is no need to separate the download anymore.

	# Different Ubuntu have different version of libraries, we need to tell which version it is.
	# I have only built dependencies for Ubuntu Xenial and Bionic. In order to build other versions,
	# it is necessary to build the library first.
	#OS_VERS:=$(shell lsb_release -a 2>/dev/null | grep Description | awk '{ print $$2 "-" $$3 }')
	#ifeq ($(findstring Ubuntu-16,$(OS_VERS)),Ubuntu-16)
	#	UPDATE_DEP_COMMAND = sh ./build-files/ubuntu/getdep_xenial.sh
	#endif
	#ifeq ($(findstring Ubuntu-18,$(OS_VERS)),Ubuntu-18)
	#	UPDATE_DEP_COMMAND = sh ./build-files/ubuntu/getdep_bionic.sh
	#endif
endif

YELLOW=`tput setaf 3`
NOCOLOR=`tput sgr0`

all:
	make release

full:
	make update
	make clean
	make
	make test

clean:
	echo ${YELLOW}Cleaning all temporary file${NOCOLOR}
	rm -rf bin generated_src

update:
	echo ${YELLOW}Sycning latest code${NOCOLOR}
	git pull

generate_src:
	echo ${YELLOW}Generating flex and bison source code${NOCOLOR}
	rm -rf generated_src
	mkdir generated_src

	bison   -d src/tsl_lib/compiler/grammar.y -o generated_src/compiled_grammar.cpp
	flex    src/tsl_lib/compiler/lex.l

release:
	echo ${YELLOW}Building release${NOCOLOR}
	rm -rf proj_release;mkdir proj_release;cd proj_release;cmake -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;

debug:
	echo ${YELLOW}Building debug${NOCOLOR}
	rm -rf proj_debug;mkdir proj_debug;cd proj_debug;cmake -DCMAKE_BUILD_TYPE=Debug ..;make -j 4;cd ..;

test:
	echo ${YELLOW}Running unit tests${NOCOLOR}
	./bin/tsl_test_r
	./bin/llvm_test_r

update_dep:
	echo ${YELLOW}Downloading dependencies ${NOCOLOR}
	$(UPDATE_DEP_COMMAND)

install:
	echo ${YELLOW}Build and install TSL${NOCOLOR}
	make release
	cmake -DCMAKE_INSTALL_PREFIX=./tsl/ -P ./_out/cmake_install.cmake