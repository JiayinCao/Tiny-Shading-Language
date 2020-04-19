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

all:
	make release

full:
	make update
	make clean
	make
	make test

clean:
	echo Cleaning all temporary file
	rm -rf bin generated_src

update:
	echo Sycning latest code
	git pull

generate_src:
	echo Generating flex and bison source code
	rm -rf generated_src
	mkdir generated_src

	bison   -d src/grammer.y -o generated_src/compiled_grammer.cpp
	flex    src/lex.l

release:
	echo Building release
	rm -rf proj_release;mkdir proj_release;cd proj_release;cmake -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;

debug:
	echo Building debug
	rm -rf proj_debug;mkdir proj_debug;cd proj_debug;cmake -DCMAKE_BUILD_TYPE=Debug ..;make -j 4;cd ..;

test:
	echo Running unit tests
	./bin/tsl_r
