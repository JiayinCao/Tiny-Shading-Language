/*
    This file is a part of Tiny-Shading-Language or TSL, an open-source cross
    platform programming shading language.

    Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.

    TSL is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

/*
    This project is a simple ray tracer program with TSL integrated in its tiny material 
    system. The purpose of this project is to demonstrate how TSL could be used in a ray 
    tracer projects with least amount of code.
    
    Of course, TSL is capable of driving shading system for ray tracer way more complex
    than this toy program. For a more sophisticated material system implementation in a 
    reasonable complex ray tracing rendering project, please check out this project
       SORT ( Simple Open-source Ray Tracing )
         main page:              http://sort-renderer.com/
         github repository:      https://github.com/JiayinCao/SORT
*/

int rt_main();
int main(int argc, char* argv[]) {
    return rt_main();
}