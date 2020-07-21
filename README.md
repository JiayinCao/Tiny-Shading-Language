# Tiny-Shading-Language
[![Build TSL](https://github.com/JiayinCao/Tiny-Shading-Language/workflows/Build%20TSL/badge.svg)](https://actions-badge.atrox.dev/Jiayincao/Tiny-Shading-Language/goto)
[![License](https://img.shields.io/badge/License-GPL3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

TSL ( Tiny Shading Language ) is my own shading language designed for my offline renderer [SORT](http://sort-renderer.com/).

The goal of TSL is to provide shader programming ability to open source ray tracer projects. 
Though, it is specifically designed for my own renderer. 
This programming language can totally be used in any other CPU based ray tracing project.

Following is the image generated in the sample ray tracing program with TSL integrated in this project (I'm still working on it to implement more fun stuff in this image.)
![](https://github.com/JiayinCao/Tiny-Shading-Language/blob/master/gallery/tsl_sample.jpg?raw=true)

# Note

TSL is functional enough to replace all of the OSL features used in SORT at this point.
However, due to crunching features in TSL, lots of code is terribly designed.
I'm refactoring this library now to make it a bit more robust and friendly.
