//
// Copyright 2017 Pixar
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
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/imaging/hd/unitTestNullRenderDelegate.h""
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdUtils/timeCodeRange.h"
#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/stopwatch.h"
#include "pxr/base/tf/stringUtils.h"
#include "pxr/base/trace/reporter.h"
#include "pxr/base/trace/trace.h"

#include <iostream>

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_ENV_SETTING(TEST_USDIMAGING_POPULATE_PERF_INPUT_USD, "", "Top-level USD to profile");

int 
main(int argc, char** argv)
{
    auto stagePath = TfGetEnvSetting(TEST_USDIMAGING_POPULATE_PERF_INPUT_USD);
    if (stagePath.empty()) {
        return 0;
    }

    // TODO: Support extComputation in Hd_UnitTestNullRenderDelegate.
    bool containsSkel = TfStringContains(stagePath, "Skel");

    TfStopwatch openStageStopwatch;
    UsdStageRefPtr stage = TfNullPtr;
    TraceCollector::GetInstance().SetEnabled(true);
    {
        TRACE_SCOPE("Open stage");
        openStageStopwatch.Start();
        stage = UsdStage::Open(stagePath);
        openStageStopwatch.Stop();
    }
    TraceCollector::GetInstance().SetEnabled(false);

    TF_AXIOM(stage);
    
    TfStopwatch populateDelegateStopwatch;
    if (!containsSkel) {
        TraceCollector::GetInstance().SetEnabled(true);
        {
            TRACE_SCOPE("Populate delegate");
            populateDelegateStopwatch.Start();
            Hd_UnitTestNullRenderDelegate renderDelegate;
            UsdImagingDelegate imagingDelegate(HdRenderIndex::New(&renderDelegate, HdDriverVector()), SdfPath::AbsoluteRootPath());
            imagingDelegate.Populate(stage->GetPseudoRoot());
            populateDelegateStopwatch.Stop();
        }
        TraceCollector::GetInstance().SetEnabled(false);
    }

    UsdUtilsTimeCodeRange timeCodeRange = stage->HasAuthoredTimeCodeRange() ?
        UsdUtilsTimeCodeRange(stage->GetStartTimeCode(), stage->GetEndTimeCode()) :
        UsdUtilsTimeCodeRange(UsdTimeCode::Default());
    TfStopwatch readAttrsStopwatch;
    TraceCollector::GetInstance().SetEnabled(true);
    {
        TRACE_SCOPE("Read attributes");
        readAttrsStopwatch.Start();
        for (auto timeCode : timeCodeRange) {
            TRACE_SCOPE("Read default or time sample");
            for (auto prim : stage->Traverse()) {
                TRACE_SCOPE("Read prim");
                for (auto attr : prim.GetAttributes()) {
                    TRACE_SCOPE("Read value");
                    VtValue dummyVal;
                    attr.Get(&dummyVal, timeCode);
                }
            }
        }
        readAttrsStopwatch.Stop();
    }
    TraceCollector::GetInstance().SetEnabled(false);

    fprintf(stderr, "Open stage took %s.\n", TfStringify(openStageStopwatch).c_str());
    if (!containsSkel) {
        fprintf(stderr, "Populate delegate took %s.\n", TfStringify(populateDelegateStopwatch).c_str());
    }
    fprintf(stderr, "Read attributes took %s.\n", TfStringify(readAttrsStopwatch).c_str());
    TraceReporter::GetGlobalReporter()->Report(std::cerr);

    return 0;
}
