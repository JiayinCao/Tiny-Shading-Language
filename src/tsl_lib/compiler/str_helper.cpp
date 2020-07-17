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

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include <emmintrin.h>
#include "str_helper.h"

TSL_NAMESPACE_BEGIN

// since this is only a very small amount of memory needed, there is no cleanning interface for simplicity.
std::unordered_set<std::string> g_string_container;

// a lock free version of spin 'lock'
class spinlock_mutex {
public:
    void lock() {
        // std::memory_order_acquire is neccessary here to prevent the out-of-order execution optimization.
        // It makes sure all memory load will happen after the lock is acquired.
        while (locked.test_and_set(std::memory_order_acquire)) {
            // In a very contended multi-threading environment, full busy loop may not be the most efficient thing to do since
            // they consume CPU cycles all the time. This instruction could allow delaying CPU instructions for a few cycles in
            // some cases to allow other threads to take ownership of hardware resources.
            // https://software.intel.com/en-us/comment/1134767
            _mm_pause();
        }
    }
    void unlock() {
        // std::memory_order_release will make sure all memory writting operations will be finished by the time this is done.
        locked.clear(std::memory_order_release);
    }

private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
};

// This is by no means the most performance implementation, but it works and this will unlikely to be the bottleneck
// I will leave it this way unless there is a performance problem.
const char* make_str_unique(const char* s) {
    static spinlock_mutex g_spin_lock;
    std::lock_guard<spinlock_mutex> guard(g_spin_lock);

    if (s == nullptr)
        return nullptr;

    const auto str = std::string(s);
    std::unordered_set<std::string>::iterator it = g_string_container.find(str);
    if (it == g_string_container.end()) {
        g_string_container.insert(str);
        it = g_string_container.find(str);
    }
    return it->c_str();
}

TSL_NAMESPACE_END