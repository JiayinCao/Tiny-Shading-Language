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

import sys
import os

files_to_verify = ("./bin/tsl_test_r", "./bin/tsl_test_d", "./bin/llvm_test_r", 
                   "./bin/llvm_test_d", "./bin/tsl_sample_r", "./bin/tsl_sample_d")
def main():
    missing_build = False

    for file in files_to_verify:
        if os.path.exists(file) is False:
            missing_build = True
            print('Missing build ' + file)
        else:
            os.system('file ' + file)

    if missing_build:
        sys.exit(1)

if __name__=="__main__": 
    main()