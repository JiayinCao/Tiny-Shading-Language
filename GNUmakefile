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

clean:
	rm -rf bin generated_src

update:
	git pull

generate_src:
	rm -rf generated_src
	mkdir generated_src

	echo    "Bison parsing ..."
	bison   -d src/grammer.y -o generated_src/compiled_grammer.c

	echo    "Laxer parsing ..."
	flex    src/lex.l

	echo    "Compiling generated C ..."
	cd generated_src;gcc -m64 -c compiled_lex.c compiled_grammer.c;\
	ar rvs compiled_grammer.a compiled_grammer.o;\
	ar rvs compiled_lex.a compiled_lex.o;

release:
	rm -rf proj_release;mkdir proj_release;cd proj_release;cmake -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;

debug:
	rm -rf proj_debug;mkdir proj_debug;cd proj_debug;cmake -DCMAKE_BUILD_TYPE=Debug ..;make -j 4;cd ..;

test:
	./bin/tsl_r < ./example/shader_first.tsl