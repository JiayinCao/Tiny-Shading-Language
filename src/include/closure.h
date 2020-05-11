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
#include "tslversion.h"

TSL_NAMESPACE_BEGIN

using ClosureID = int;
constexpr ClosureID INVALID_CLOSURE_ID = 0;
constexpr ClosureID CLOSURE_ADD = -1;
constexpr ClosureID CLOSURE_MUL = -2;

struct ClosureAddNode;
struct ClosureMulNode;

struct ClosureTreeNode {
    ClosureID   m_id = INVALID_CLOSURE_ID;

    const ClosureAddNode* as_add_node() const {
        return reinterpret_cast<const ClosureAddNode*>(this);
    }

    const ClosureMulNode* as_mul_node() const {
        return reinterpret_cast<const ClosureMulNode*>(this);
    }
};

struct ClosureAddNode : public ClosureTreeNode {
    std::unique_ptr<ClosureTreeNode> m_closure0 = nullptr;
    std::unique_ptr<ClosureTreeNode> m_closure1 = nullptr;
};

struct ClosureMulNode : public ClosureTreeNode {
    std::unique_ptr<ClosureTreeNode> m_closure = nullptr;
    float m_weight = 1.0f;
};

struct ClosureTree {
    std::unique_ptr<ClosureTreeNode> m_root = nullptr;
};

TSL_NAMESPACE_END