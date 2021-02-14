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

# This is a helper script to get dependencies

import os
import urllib.request
import zipfile
import shutil
import sys
import subprocess

# whether to force syncing
forcing_sync = False
arch = 'x86_64'

if len(sys.argv) > 1:
    # output a message indicating this is a force syncing
    print( 'Force syncing dependencies.' )

    if sys.argv[1] == 'TRUE':
        forcing_sync = sys.argv[0]

    if len(sys.argv) > 2:
        if sys.argv[2] == 'arm64':
            arch = sys.argv[2]

# dependencies folder
dep_dir = 'dependencies'

# whether to sync dependencies
sync_dep = False

# if forcing syncing is enabled, delete the dependencies even if it exists
if forcing_sync:
    # check if the folder already exists, if it does, remove it
    if os.path.isdir(dep_dir):
        # output a warning
        print('The dependencies are purged.')

        # remove the folder
        shutil.rmtree(dep_dir)

    # re-create the folder again
    os.makedirs(dep_dir)

    sync_dep = True
else:
    # this might not be very robust since it just check the folder
    # if there is a broken dependencies folder, it will fail to build
    if os.path.isdir(dep_dir) is False:
        sync_dep = True
    else:
        print('Dependencies are up to date, no need to sync.')

# sync a dependency from server
def sync_dep_utility( name, url, target ):
    print( 'Syncing ' + name )

    # sync file
    dummy_folder = 'dummy/'

    # check if the folder already exists, if it does, remove it
    if os.path.isdir(dummy_folder):
        shutil.rmtree(dummy_folder)
    # make the dir
    os.makedirs(dummy_folder)

    # sync the file
    index_file_name = dummy_folder + 'index.txt'
    urllib.request.urlretrieve(url + 'files.txt', index_file_name)

    # Using readlines() 
    index_file = open(index_file_name, 'r') 
    Lines = index_file.readlines() 
  
    # sync all files down
    for index, line in enumerate(Lines, start=1):
        zip_file_name = line.rstrip()
        file_name = dummy_folder + zip_file_name
        print( "zip file: " + file_name )
        urllib.request.urlretrieve(url + zip_file_name, file_name)

    # uncompress the zip file and make sure it is in the Dependencies folder
    for line in Lines:
        zip_file_name = dummy_folder + line.rstrip()
        with zipfile.ZipFile(zip_file_name,"r") as zip_ref:
            print( "extracing file: " + zip_file_name)
            zip_ref.extractall(target)

    # close the file
    index_file.close()

    # delete the temporary file
    shutil.rmtree(dummy_folder)

# sync dependencies if needed
if sync_dep:
    # flex and bison is only needed on Windows
    if sys.platform == 'win32':
        #sync flex and bison
        sync_dep_utility('flex and bison', 'https://raw.githubusercontent.com/JiayinCao/Tiny-Shading-Language/dependencies/flex_bison/win/x86_64/', dep_dir + '/flex_bison')

    # sync llvm
    if sys.platform == 'win32':
        sync_dep_utility('llvm', 'https://raw.githubusercontent.com/JiayinCao/Tiny-Shading-Language/dependencies/llvm_10_0_0/win/x86_64/', dep_dir)
    elif sys.platform == "linux" or sys.platform == "linux2":
        sync_dep_utility('llvm', 'https://raw.githubusercontent.com/JiayinCao/Tiny-Shading-Language/dependencies/llvm_10_0_0/linux/x86_64/', dep_dir)
    elif sys.platform == 'darwin':
        if arch == 'arm64':
            print('Sycning arm version llvm...')
            sync_dep_utility('llvm', 'https://raw.githubusercontent.com/JiayinCao/Tiny-Shading-Language/dependencies/llvm_10_0_0/mac/arm64/', dep_dir)
        elif arch == 'x86_64':
            print('Syncing x86_64 version llvm...')
            sync_dep_utility('llvm', 'https://raw.githubusercontent.com/JiayinCao/Tiny-Shading-Language/dependencies/llvm_10_0_0/mac/x86_64/', dep_dir)
        else:
            print('Error, unknown archtecture!')
