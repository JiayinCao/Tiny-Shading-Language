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
	rm -rf tmp
	mkdir tmp

	echo    "Bison parsing ..."
	bison   -d src/grammer.y -o tmp/compiled_grammer.c

	echo    "Laxer parsing ..."
	flex    src/lex.l

	echo    "Compiling generated C ..."
	cd tmp;ls;gcc -m64 -c compiled_lex.c compiled_grammer.c;\
	ar rvs compiled_grammer.a compiled_grammer.o;\
	ar rvs compiled_lex.a compiled_lex.o;

	rm -rf bin;mkdir bin;cd bin;\
	g++ -m64 -std=c++11 ../src/main.cpp ../tmp/compiled_grammer.a ../tmp/compiled_lex.a

	echo    "Executing binary ..."
	./bin/a.out < example/shader_first.tsl

clean:
	rm -rf bin tmp

update:
	git pull