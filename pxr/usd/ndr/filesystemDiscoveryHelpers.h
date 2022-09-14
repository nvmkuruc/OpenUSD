//
// Copyright 2018 Pixar
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

#ifndef PXR_USD_NDR_FILESYSTEM_DISCOVERY_HELPERS_H
#define PXR_USD_NDR_FILESYSTEM_DISCOVERY_HELPERS_H

/// \file ndr/filesystemDiscoveryHelpers.h

#include "pxr/pxr.h"
#include "pxr/usd/ndr/api.h"
#include "pxr/usd/ndr/declare.h"
#include "pxr/usd/ndr/nodeDiscoveryResult.h"

PXR_NAMESPACE_OPEN_SCOPE

class NdrDiscoveryPluginContext;

/// \file filesystemDiscoveryHelpers.h
///
/// Provides utilities that the default filesystem discovery plugin uses. If
/// a custom filesystem discovery plugin is needed, these can be used to fill
/// in a large chunk of the functionality.
///

/// Walks the specified search paths, optionally following symlinks. Paths
/// are walked recursively, and each directory has `FsHelpersExamineFiles()`
/// called on it. Only files that match one of the provided extensions (case
/// insensitive) are candidates for being turned into `NdrDiscoveryResult`s.
/// Returns a vector of discovery results that have been found while walking
/// the search paths.  In each result the name and identifier will be the
/// same, the version will be invalid and default, and the family will
/// be empty.  The caller is expected to adjust these as appropriate.  A
/// naive client with no versions and no family will work correctly.
NDR_API
NdrNodeDiscoveryResultVec
NdrFsHelpersDiscoverNodes(
    const NdrStringVec& searchPaths,
    const NdrStringVec& allowedExtensions,
    bool followSymlinks = true,
    const NdrDiscoveryPluginContext* context = nullptr
);

/// Struct for holding a URI and its resolved URI for a file discovered
/// by NdrFsHelpersDiscoverFiles
struct NdrDiscoveryUri 
{
    std::string uri;
    std::string resolvedUri;
};

/// A vector of URI/resolved URI structs.
using NdrDiscoveryUriVec = std::vector<NdrDiscoveryUri>;

/// Returns a vector of discovered URIs (as both the unresolved URI and the 
/// resolved URI) that are found while walking  the given search paths.
///
/// Each path in \p searchPaths is walked recursively, optionally following 
/// symlinks if \p followSymlinks is true, looking for files that match one of 
/// the provided \p allowedExtensions. These files' unresolved and resolved URIs
/// are returned in the result vector.
///
/// This is an alternative to NdrFsHelpersDiscoverNodes for discovery plugins 
/// that want to search for files that are not meant to be returned by discovery
/// themselves, but can be parsed to generate the discovery results.
NDR_API
NdrDiscoveryUriVec
NdrFsHelpersDiscoverFiles(
    const NdrStringVec& searchPaths,
    const NdrStringVec& allowedExtensions,
    bool followSymlinks = true
);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_NDR_FILESYSTEM_DISCOVERY_HELPERS_H
