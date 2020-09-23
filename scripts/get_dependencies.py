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

# whether to force syncing
forcing_sync = False

if len(sys.argv) > 1:
    # output a message indicating this is a force syncing
    print( 'Force syncing dependencies.' )

    if sys.argv[1] == 'TRUE':
        forcing_sync = sys.argv[0]

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
    zip_file_name = 'tmp.zip'
    urllib.request.urlretrieve(url, zip_file_name)

    # uncompress the zip file and make sure it is in the Dependencies folder
    with zipfile.ZipFile(zip_file_name,"r") as zip_ref:
        zip_ref.extractall(target)

    # delete the temporary file
    os.remove(zip_file_name)

# sync dependencies if needed
if sync_dep:
    # flex and bison is only needed on Windows
    if sys.platform == 'win32':
        # sync flex and bison
        sync_dep_utility('flex and bison', 'https://github.com/lexxmark/winflexbison/releases/download/v2.5.22/win_flex_bison-2.5.22.zip', dep_dir + '/flex_bison')

    # sync llvm
    if sys.platform == 'win32':
        sync_dep_utility('llvm', 'http://45.63.123.194/tsl_dependencies/win/llvm.zip', dep_dir)
    elif sys.platform == "linux" or sys.platform == "linux2":
        sync_dep_utility('llvm', 'http://45.63.123.194/tsl_dependencies/linux/llvm_xenial.zip', dep_dir)
    elif sys.platform == 'darwin':
        sync_dep_utility('llvm', 'http://45.63.123.194/tsl_dependencies/mac/llvm.zip', dep_dir)
