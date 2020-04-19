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

	bison   -d src/grammer.y -o generated_src/compiled_grammer.cpp
	flex    src/lex.l

release:
	echo ${YELLOW}Building release${NOCOLOR}
	rm -rf proj_release;mkdir proj_release;cd proj_release;cmake -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;

debug:
	echo ${YELLOW}Building debug${NOCOLOR}
	rm -rf proj_debug;mkdir proj_debug;cd proj_debug;cmake -DCMAKE_BUILD_TYPE=Debug ..;make -j 4;cd ..;

test:
	echo ${YELLOW}Running unit tests${NOCOLOR}
	./bin/tsl_r
