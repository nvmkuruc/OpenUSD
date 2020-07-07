//
// Copyright 2019 Pixar
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

// #nv begin #fast-updates
#include "pxr/pxr.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/schema.h"
#include "pxr/usd/sdf/notice.h"

#include "pxr/imaging/hd/unitTestNullRenderDelegate.h""

#include "pxr/usdImaging/usdImaging/delegate.h"

#include "pxr/base/tf/errorMark.h"
#include "pxr/base/trace/reporter.h"
#include "pxr/base/trace/trace.h"

#include <iostream>
#include <random>

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEBUG_CODES(
    TEST_USDIMAGING_FAST_UPDATES_PERF,
    TEST_USDIMAGING_FAST_UPDATES_BASELINE_PERF,
    TEST_USDIMAGING_FAST_UPDATES_NO_IMAGING
);

TF_REGISTRY_FUNCTION(TfDebug)
{
    TF_DEBUG_ENVIRONMENT_SYMBOL(TEST_USDIMAGING_FAST_UPDATES_PERF, "Run testUsdImagingFastUpdates as a performance test");
    TF_DEBUG_ENVIRONMENT_SYMBOL(TEST_USDIMAGING_FAST_UPDATES_BASELINE_PERF, "Run testUsdImagingFastUpdates without fast updates as a performance test");
    TF_DEBUG_ENVIRONMENT_SYMBOL(TEST_USDIMAGING_FAST_UPDATES_NO_IMAGING, "Run testUsdImagingFastUpdates with no imaging overhead");
}

static std::default_random_engine gRandomEngine(1515);
static const double NUM_INITIAL_TIME_SAMPLES = 1000.0;

class LayerAttrChangeHelper
{
    SdfLayerRefPtr _layer;
    UsdStagePtr _stage;

    struct AttrData
    {
        UsdAttribute usdAttr;
    };

    std::vector<SdfPath> _pathList;
    TfHashMap<SdfPath, AttrData, SdfPath::Hash> _attrMap;

public:
    LayerAttrChangeHelper(SdfLayerRefPtr layer, UsdStagePtr stage)
        : _layer(layer), _stage(stage)
    {
        _layer->Traverse(SdfPath::AbsoluteRootPath(), [this](const SdfPath& path)
        {
            if (path.IsPropertyPath())
            {
                if (_layer->GetSpecType(path) == SdfSpecTypeAttribute)
                {
                    _pathList.push_back(path);
                    AttrData attrData = {
                        _stage->GetPrimAtPath(path.GetPrimPath()).GetAttribute(path.GetNameToken())
                    };
                    _attrMap[path] = attrData;
                }
            }
        });
    }

    ~LayerAttrChangeHelper() {
    }

    size_t GetAttrCount() const { return _pathList.size(); }

    void ExecuteRandomChange(bool writeDefaults, bool isBaselinePerfTest, bool isPerfTest)
    {
        static const double timeSampleToSet = 1.0;

        SdfChangeBlock changeBlock(!isBaselinePerfTest /* fastUpdates */);

        for (size_t i = 0; i < _pathList.size(); ++i)
        {
            const size_t attrIdx = i;
            const SdfPath &attrPath = _pathList[i];
            const AttrData& attrData = _attrMap[attrPath];
            double oldValue = 1.0;
            double newValue = 0.0;
            if (!isPerfTest) {
                if (writeDefaults) {
                    VtValue oldVtVal = _layer->GetField(attrData.usdAttr.GetPath(), SdfFieldKeys->Default);
                    if (oldVtVal.IsHolding<double>())
                        oldValue = oldVtVal.UncheckedGet<double>();
                } else {
                    VtValue oldVtVal = _layer->GetField(attrData.usdAttr.GetPath(), SdfFieldKeys->TimeSamples);
                    TF_AXIOM(oldVtVal.IsHolding<SdfTimeSampleMap>());
                    SdfTimeSampleMap oldTimeSamples = oldVtVal.UncheckedGet<SdfTimeSampleMap>();
                    TF_AXIOM((oldTimeSamples.size() == NUM_INITIAL_TIME_SAMPLES) || (oldTimeSamples.size() == NUM_INITIAL_TIME_SAMPLES + 1));
                    SdfTimeSampleMap::iterator oldEntry = oldTimeSamples.find(timeSampleToSet);
                    if (oldEntry != oldTimeSamples.end() && oldEntry->second.IsHolding<double>()) {
                        oldValue = oldEntry->second.UncheckedGet<double>();
                    }
                }
                TF_AXIOM(!std::isnan(oldValue));
                newValue = std::uniform_int_distribution<int>(1, _pathList.size() - 1)(gRandomEngine) + oldValue;
                TF_AXIOM(!std::isnan(newValue));
            } else {
                newValue = std::uniform_int_distribution<int>(1, _pathList.size() - 1)(gRandomEngine);
            }

            if (writeDefaults) {
                attrData.usdAttr.Set(newValue);
            } else {
                attrData.usdAttr.Set(newValue, timeSampleToSet);
            }

            if (!isPerfTest) {
                double modValue = 0.0;
                if (writeDefaults) {
                    modValue = _layer->GetField(attrData.usdAttr.GetPath(), SdfFieldKeys->Default).UncheckedGet<double>();
                    TF_AXIOM(!std::isnan(modValue));
                }
                else {
                    SdfTimeSampleMap modTimeSamples =
                        _layer->GetField(attrData.usdAttr.GetPath(), SdfFieldKeys->TimeSamples).UncheckedGet<SdfTimeSampleMap>();
                    TF_AXIOM(modTimeSamples.find(timeSampleToSet) != modTimeSamples.end());
                    TF_AXIOM(modTimeSamples[timeSampleToSet].IsHolding<double>());
                    modValue = modTimeSamples[timeSampleToSet].UncheckedGet<double>();

                    TF_AXIOM(!std::isnan(modValue));
                }
                TF_AXIOM(oldValue != modValue);
                TF_AXIOM(modValue == newValue);
            }
        }
    }
};

static
void PopulateInitialLayerContent(SdfLayerRefPtr layer, int numPrims)
{
    // XXX:aluk
    // When authoring in Sdf, there is no API to get the usdPrimTypeName for a schema, so we have to hardcode type names here.
    auto parentPrim = SdfPrimSpec::New(layer->GetPseudoRoot(), "world", SdfSpecifierDef, "Scope");

    for (int primIdx = 0; primIdx < numPrims; ++primIdx)
    {
        std::string primName("sphere");
        primName.append("_").append(std::to_string(primIdx));

        auto prim = SdfPrimSpec::New(parentPrim, primName, SdfSpecifierDef, "Sphere");

        auto attr = SdfAttributeSpec::New(prim, UsdGeomTokens->radius, SdfValueTypeNames->Double);

        // Prepopulate timesample map with many entries-- the runtime performance of authoring new or existing
        // time samples should be comparable regardless of the size of the timesample map.
        for (double time=2.0; time < 2.0 + NUM_INITIAL_TIME_SAMPLES; ++time)
            layer->SetTimeSample(attr->GetPath(), time, time);

        TF_AXIOM(layer->GetNumTimeSamplesForPath(attr->GetPath()) == NUM_INITIAL_TIME_SAMPLES);
    }
}

static
void BenchmarkFieldUpdate(const std::string &fileExtension, bool writeDefaults, bool isBaselinePerfTest, bool isPerfTest, bool enableImaging)
{
    const std::string assetPath = "benchmarkAsset." + fileExtension;

    auto layer = SdfLayer::CreateNew(assetPath);

    PopulateInitialLayerContent(layer, 300);

    layer->Save();

    layer = nullptr;

    layer = SdfLayer::OpenAsAnonymous(assetPath);
    auto stage = UsdStage::Open(layer);
    Hd_UnitTestNullRenderDelegate renderDelegate;
    UsdImagingDelegate imagingDelegate(HdRenderIndex::New(&renderDelegate), SdfPath::AbsoluteRootPath());

    if (enableImaging)
        imagingDelegate.Populate(stage->GetPseudoRoot());

    std::string traceDetail = writeDefaults ? "(" + fileExtension + " defaults)" : "(" + fileExtension + " time samples)";

    if (isPerfTest)
        TraceCollector::GetInstance().SetEnabled(true);

    LayerAttrChangeHelper changeGenerator(layer, stage);
    for (int i = 0; i < 10; ++i)
    {
        TRACE_SCOPE_DYNAMIC("Change Attributes " + traceDetail);

        changeGenerator.ExecuteRandomChange(writeDefaults, isBaselinePerfTest, isPerfTest);
        if (enableImaging) {
            if (!isPerfTest && writeDefaults) {
                //TF_AXIOM(imagingDelegate.HasPendingFastUpdates());
            }
            imagingDelegate.ApplyPendingUpdates();
            if (!isPerfTest && writeDefaults) {
                TF_AXIOM(!(imagingDelegate.HasPendingFastUpdates()));
            }
        }
    }

    if (isPerfTest) {
        TraceReporter::GetGlobalReporter()->Report(std::cout);
        TraceCollector::GetInstance().SetEnabled(false);
        TraceReporter::GetGlobalReporter()->ClearTree();
    }
}

int
main(int argc, char **argv)
{
    TfErrorMark errorMark;
    bool isBaselinePerfTest = TfDebug::IsEnabled(TEST_USDIMAGING_FAST_UPDATES_BASELINE_PERF);
    bool isPerfTest = isBaselinePerfTest || TfDebug::IsEnabled(TEST_USDIMAGING_FAST_UPDATES_PERF);
    bool enableImaging = !TfDebug::IsEnabled(TEST_USDIMAGING_FAST_UPDATES_NO_IMAGING);
    BenchmarkFieldUpdate("usda", true /* writeDefaults */, isBaselinePerfTest, isPerfTest, enableImaging);
    BenchmarkFieldUpdate("usda", false /* writeDefaults */, isBaselinePerfTest, isPerfTest, enableImaging);
    BenchmarkFieldUpdate("usdc", true /* writeDefaults */, isBaselinePerfTest, isPerfTest, enableImaging);
    BenchmarkFieldUpdate("usdc", false /* writeDefaults */, isBaselinePerfTest, isPerfTest, enableImaging);

    TF_AXIOM(errorMark.IsClean());

    return 0;
}
// nv end
