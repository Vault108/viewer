/**
 * @file lldrawpoolpbropaque.cpp
 * @brief LLDrawPoolGLTFPBR class implementation
 *
 * $LicenseInfo:firstyear=2022&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2022, Linden Research, Inc.
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

#include "lldrawpool.h"
#include "lldrawpoolpbropaque.h"
#include "llviewershadermgr.h"
#include "pipeline.h"
#include "gltfscenemanager.h"

extern LLCullResult* sCull;

LLDrawPoolGLTFPBR::LLDrawPoolGLTFPBR(U32 type) :
    LLRenderPass(type)
{
    if (type == LLDrawPool::POOL_GLTF_PBR_ALPHA_MASK)
    {
        mRenderType = LLPipeline::RENDER_TYPE_PASS_GLTF_PBR_ALPHA_MASK;
    }
    else
    {
        mRenderType = LLPipeline::RENDER_TYPE_PASS_GLTF_PBR;
    }
}

S32 LLDrawPoolGLTFPBR::getNumDeferredPasses()
{
    return 1;
}

void LLDrawPoolGLTFPBR::renderDeferred(S32 pass)
{
    llassert(!LLPipeline::sRenderingHUDs);

    if (mRenderType == LLPipeline::RENDER_TYPE_PASS_GLTF_PBR_ALPHA_MASK)
    {
        // opaque
        LL::GLTFSceneManager::instance().render(true);
        // opaque rigged
        LL::GLTFSceneManager::instance().render(true, true);
    }

    LLGLTFMaterial::AlphaMode alpha_mode = mRenderType == LLPipeline::RENDER_TYPE_PASS_GLTF_PBR_ALPHA_MASK ? LLGLTFMaterial::ALPHA_MODE_MASK : LLGLTFMaterial::ALPHA_MODE_OPAQUE;

    for (U32 double_sided = 0; double_sided < 2; ++double_sided)
    {
        LLGLDisable cull(double_sided ? GL_CULL_FACE : 0);
        for (U32 planar = 0; planar < 2; ++planar)
        {
            for (U32 tex_anim = 0; tex_anim < 2; ++tex_anim)
            {
                for (U32 tex_mask = 0; tex_mask < LLGLTFBatches::MAX_PBR_TEX_MASK; ++tex_mask)
                {
                    LLGLSLShader& shader = gGLTFPBRShaderPack.mShader[alpha_mode][tex_mask][double_sided][planar][tex_anim];
                    for (U32 rigged = 0; rigged < 2; ++rigged)
                    {
                        pushGLTFBatches(shader, rigged, alpha_mode, tex_mask, double_sided, planar, tex_anim);
                    }

                }
                if (!double_sided && gPipeline.hasRenderType(LLPipeline::RENDER_TYPE_MATERIALS))
                {
                    for (U32 tex_mask = 0; tex_mask < LLGLTFBatches::MAX_BP_TEX_MASK; ++tex_mask)
                    {
                        LLGLSLShader& shader = gBPShaderPack.mShader[alpha_mode][tex_mask][planar][tex_anim];
                        for (U32 rigged = 0; rigged < 2; ++rigged)
                        {
                            pushBPBatches(shader, rigged, alpha_mode, tex_mask, planar, tex_anim);
                        }
                    }
                }
            }
        }
    }
}


S32 LLDrawPoolGLTFPBR::getNumPostDeferredPasses()
{
    return 1;
}

void LLDrawPoolGLTFPBR::renderPostDeferred(S32 pass)
{
    if (LLPipeline::sRenderingHUDs)
    {
        // TODO -- render HUDs
    }
}

