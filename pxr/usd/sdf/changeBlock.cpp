//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#include "pxr/pxr.h"
#include "pxr/usd/sdf/changeBlock.h"
#include "pxr/usd/sdf/changeManager.h"

// #nv begin #fast-updates
#include "pxr/base/tf/envSetting.h"
// nv end

PXR_NAMESPACE_OPEN_SCOPE

// #nv begin #fast-updates
TF_DEFINE_ENV_SETTING(SDF_CHANGEBLOCK_IMPLICIT_FAST_UPDATES, false,
    "Enable Fast Updates in SdfChangeBlocks by default");
// nv end

SdfChangeBlock::SdfChangeBlock(bool fastUpdates)
{
    Sdf_ChangeManager::Get().OpenChangeBlock(fastUpdates);
}

SdfChangeBlock::~SdfChangeBlock() 
{
    Sdf_ChangeManager::Get().CloseChangeBlock();
}

// #nv begin #fast-updates
SdfChangeBlock::SdfChangeBlock()
{
    Sdf_ChangeManager::Get().OpenChangeBlock(TfGetEnvSetting(SDF_CHANGEBLOCK_IMPLICIT_FAST_UPDATES));
}

bool SdfChangeBlock::IsFastUpdating()
{
    return Sdf_ChangeManager::Get().IsFastUpdating();
}
// nv end

PXR_NAMESPACE_CLOSE_SCOPE
