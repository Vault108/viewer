/**
 * @file llgltfdrawinfo.cpp
 * @brief LLGLTFDrawInfo implementation
 *
 * $LicenseInfo:firstyear=2024&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llgltfdrawinfo.h"

void LLGLTFBatches::clear()
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_DRAWPOOL;

    mBatchList.clear();
    mSkinnedBatchList.clear();

    for (U32 i = 0; i < 3; i++)
    {
        for (U32 j = 0; j < 2; j++)
        {
            for (U32 k = 0; k < 2; ++k)
            {
                for (U32 l = 0; l < 2; ++l)
                {
                    mDrawInfo[i][j][k][l].clear();
                    mSkinnedDrawInfo[i][j][k][l].clear();
                }
            }
        }
    }
}

void LLGLTFDrawInfo::texNameCheck(U32 texName)
{
    if (LLImageGL::sTexNames[mBaseColorMap] == texName)
    {
        LL_WARNS_ONCE("GLTF") << "Base color map (or diffuse map) dangling reference: " << mBaseColorMap << LL_ENDL;
    }

    if (LLImageGL::sTexNames[mMetallicRoughnessMap] == texName)
    {
        LL_WARNS_ONCE("GLTF") << "Metallic roughness map (or specular map) dangling reference: " << mMetallicRoughnessMap << LL_ENDL;
    }

    if (LLImageGL::sTexNames[mNormalMap] == texName)
    {
        LL_WARNS_ONCE("GLTF") << "Normal map dangling reference: " << mNormalMap << LL_ENDL;
    }

    if (LLImageGL::sTexNames[mEmissiveMap] == texName)
    {
        LL_WARNS_ONCE("GLTF") << "Emissive map dangling reference: " << mEmissiveMap << LL_ENDL;
    }
}

LLGLTFDrawInfo* LLGLTFBatches::create(LLGLTFMaterial::AlphaMode alpha_mode, bool double_sided, bool planar, bool tex_anim, LLGLTFDrawInfoHandle &handle)
{
    auto& draw_info = mDrawInfo[alpha_mode][double_sided][planar][tex_anim];

    if (draw_info.empty())
    {
        mBatchList.push_back({ alpha_mode, double_sided, planar, tex_anim, &draw_info });
    }

    handle.mSkinned = false;
    handle.mContainer = &draw_info;
    handle.mIndex = (S32)draw_info.size();

    return &draw_info.emplace_back();
}

LLSkinnedGLTFDrawInfo* LLGLTFBatches::createSkinned(LLGLTFMaterial::AlphaMode alpha_mode, bool double_sided, bool planar, bool tex_anim, LLGLTFDrawInfoHandle& handle)
{
    auto& draw_info = mSkinnedDrawInfo[alpha_mode][double_sided][planar][tex_anim];

    if (draw_info.empty())
    {
        mSkinnedBatchList.push_back({ alpha_mode, double_sided, planar, tex_anim, &draw_info });
    }

    handle.mSkinned = true;
    handle.mSkinnedContainer = &draw_info;
    handle.mIndex = (S32)draw_info.size();

    return &draw_info.emplace_back();
}

void LLGLTFBatches::add(const LLGLTFBatches& other)
{
    for (auto& batch : other.mBatchList)
    {
        auto& draw_info = mDrawInfo[batch.alpha_mode][batch.double_sided][batch.planar][batch.tex_anim];
        draw_info.insert(draw_info.end(), batch.draw_info->begin(), batch.draw_info->end());
    }

    for (auto& batch : other.mSkinnedBatchList)
    {
        auto& draw_info = mSkinnedDrawInfo[batch.alpha_mode][batch.double_sided][batch.planar][batch.tex_anim];
        draw_info.insert(draw_info.end(), batch.draw_info->begin(), batch.draw_info->end());
    }
}

void LLGLTFBatches::texNameCheck(U32 texName)
{
    for (auto& batch : mBatchList)
    {
        for (auto& draw_info : *batch.draw_info)
        {
            draw_info.texNameCheck(texName);
        }
    }

    for (auto& batch : mSkinnedBatchList)
    {
        for (auto& draw_info : *batch.draw_info)
        {
            draw_info.texNameCheck(texName);
        }
    }
}

LLGLTFDrawInfo* LLGLTFDrawInfoHandle::get()
{
    if (mIndex == -1)
    {
        return nullptr;
    }

    if (mSkinned)
    {
        llassert(mIndex >= 0 && mIndex < mSkinnedContainer->size());
        return &mSkinnedContainer->at(mIndex);
    }
    else
    {
        llassert(mIndex >= 0 && mIndex < mContainer->size());
        return &mContainer->at(mIndex);
    }
}

void LLGLTFDrawInfoHandle::clear()
{
    mIndex = -1;
}

