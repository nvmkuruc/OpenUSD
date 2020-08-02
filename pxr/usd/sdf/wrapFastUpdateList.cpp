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

// #nv begin #fast-updates
#include "pxr/pxr.h"

#include <vector>

#include "pxr/usd/sdf/changeList.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/vt/valueFromPython.h"

#include <boost/python.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

static VtValue
_GetValue(const SdfFastUpdateList::FastUpdate &fastUpdate) {
    return fastUpdate.value;
}

static const std::vector<SdfFastUpdateList::FastUpdate> &
_GetFastUpdates(const SdfFastUpdateList &fastUpdateList) {
    return fastUpdateList.fastUpdates;
}

} // anonymous namespace

void wrapFastUpdateList() {
    scope s = class_<SdfFastUpdateList> ( "FastUpdateList", no_init )
        .def_readonly("hasCompositionDependents", &SdfFastUpdateList::hasCompositionDependents)
        .add_property("fastUpdates", make_function(&_GetFastUpdates,
            return_value_policy<TfPySequenceToList>()))
        ;

    class_<SdfFastUpdateList::FastUpdate>("FastUpdate", no_init)
        .def_readonly("path", &SdfFastUpdateList::FastUpdate::path)
        .add_property("value", &_GetValue)
        ;
}
// nv end
