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
#ifndef PXR_USD_IMAGING_USD_IMAGING_VALUE_CACHE_H
#define PXR_USD_IMAGING_USD_IMAGING_VALUE_CACHE_H

/// \file usdImaging/valueCache.h

#include "pxr/pxr.h"
#include "pxr/usdImaging/usdImaging/api.h"
#include "pxr/imaging/hd/enums.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hd/sceneDelegate.h"
#include "pxr/imaging/hd/version.h"

#include "pxr/usd/sdf/path.h"
#include "pxr/base/vt/value.h"

#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/tf/token.h"

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_queue.h>

PXR_NAMESPACE_OPEN_SCOPE

/// \class UsdImagingValueCache
///
/// A heterogeneous value container without type erasure.
///
class UsdImagingValueCache 
{
public:
    UsdImagingValueCache(const UsdImagingValueCache&) = delete;
    UsdImagingValueCache& operator=(const UsdImagingValueCache&) = delete;

    class Key {
        friend class UsdImagingValueCache;
    private:
        SdfPath _path;
        TfToken _attribute;
    public:
		Key() {}
        Key(SdfPath const& path, TfToken const& attr)
            : _path(path)
            , _attribute(attr)
        {}

        inline bool operator==(Key const& rhs) const {
            return _path == rhs._path && _attribute == rhs._attribute;
        }
        inline bool operator!=(Key const& rhs) const {
            return !(*this == rhs);
        }

        struct Hash {
            inline size_t operator()(Key const& key) const {
                size_t hash = key._path.GetHash();
                boost::hash_combine(hash, key._attribute.Hash());
                return hash;
            }
        };

    private:
        static Key InstanceIndices(SdfPath const& path) {
            static TfToken attr("instanceIndices");
            return Key(path, attr);
        }
        static Key Primvars(SdfPath const& path) {
            static TfToken attr("primvars");
            return Key(path, attr);
        }
<<<<<<< HEAD
        static Key Topology(SdfPath const& path) {
            static TfToken attr("topology");
            return Key(path, attr);
        }
        static Key Transform(SdfPath const& path) {
            static TfToken attr("transform");
            return Key(path, attr);
        }
        static Key Visible(SdfPath const& path) {
            static TfToken attr("visible");
            return Key(path, attr);
        }
        static Key Widths(SdfPath const& path) {
            static TfToken attr("widths");
            return Key(path, attr);
        }
        static Key Normals(SdfPath const& path) {
            static TfToken attr("normals");
            return Key(path, attr);
        }
		//+NV_CHANGE FRZHANG : NV GPU Skinning
		//skinned prim keys
		static Key RestPoints(SdfPath const& path) {
			static TfToken attr("restPoints");
			return Key(path, attr);
		}
		static Key GeomBindXform(SdfPath const& path) {
			static TfToken attr("geomBindXform");
			return Key(path, attr);
		}
		static Key JointIndices(SdfPath const& path) {
			static TfToken attr("jointIndices");
			return Key(path, attr);
		}
		static Key JointWeights(SdfPath const& path) {
			static TfToken attr("jointWeights");
			return Key(path, attr);
		}
		static Key NumInfluencesPerPoint(SdfPath const& path) {
			static TfToken attr("numInfluencesPerPoint");
			return Key(path, attr);
		}
		static Key HasConstantInfluences(SdfPath const& path) {
			static TfToken attr("hasConstantInfluences");
			return Key(path, attr);
		}
        static Key SkinningMethod(SdfPath const& path) {
            static TfToken attr("skinningMethod");
            return Key(path, attr);
        }
        static Key SkinningBlendWeights(SdfPath const& path) {
            static TfToken attr("skinningBlendWeights");
            return Key(path, attr);
        }
        static Key HasConstantSkinningBlendWeights(SdfPath const& path) {
            static TfToken attr("hasConstantSkinningBlendWeights");
            return Key(path, attr);
        }
        //skeleton prim keys
		static Key PrimWorldToLocal(SdfPath const& path) {
			static TfToken attr("primWorldToLocal");
			return Key(path, attr);
		}
		static Key SkinningXforms(SdfPath const& path) {
			static TfToken attr("skinningXforms");
			return Key(path, attr);
		}
		static Key SkelLocalToWorld(SdfPath const& path) {
			static TfToken attr("skelLocalToWorld");
			return Key(path, attr);
		}
		//-NV_CHANGE FRZHANG
        static Key MaterialId(SdfPath const& path) {
            static TfToken attr("materialId");
            return Key(path, attr);
        }
        static Key MaterialResource(SdfPath const& path) {
            static TfToken attr("materialResource");
            return Key(path, attr);
        }
        static Key ExtComputationSceneInputNames(SdfPath const& path) {
            static TfToken attr("extComputationSceneInputNames");
            return Key(path, attr);
        }
        static Key ExtComputationInputs(SdfPath const& path) {
            static TfToken attr("extComputationInputs");
            return Key(path, attr);
        }
        static Key ExtComputationOutputs(SdfPath const& path) {
            static TfToken attr("extComputationOutputs");
            return Key(path, attr);
        }
        static Key ExtComputationPrimvars(SdfPath const& path) {
            const TfToken attr("extComputationPrimvars");
            return Key(path, attr);
        }
        static Key ExtComputationKernel(SdfPath const& path) {
            const TfToken attr("extComputationKernel");
            return Key(path, attr);
        }
        static Key CameraParamNames(SdfPath const& path) {
            static TfToken attr("CameraParamNames");
            return Key(path, attr);
        }
=======
>>>>>>> v20.11-rc1
    };

    UsdImagingValueCache()
        : _locked(false)
    { }

private:
    template <typename Element>
    struct _TypedCache
    {
        typedef tbb::concurrent_unordered_map<Key, Element, Key::Hash> _MapType;
        typedef typename _MapType::iterator                            _MapIt;
        typedef typename _MapType::const_iterator                      _MapConstIt;
        typedef tbb::concurrent_queue<Key>                          _QueueType;

        _MapType   _map;
        _QueueType _deferredDeleteQueue;
    };


    /// Locates the requested \p key then populates \p value and returns true if
    /// found.
    template <typename T>
    bool _Find(Key const& key, T* value) const {
        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;

        _GetCache(&cache);
        typename Cache_t::_MapConstIt it = cache->_map.find(key);
        if (it == cache->_map.end()) {
            return false;
        }
        *value = it->second;
        return true;
    }

    /// Locates the requested \p key then populates \p value, swap the value
    /// from the entry and queues the entry up for deletion.
    /// Returns true if found.
    /// This function is thread-safe, but Garbage collection must be called
    /// to perform the actual deletion.
    /// Note: second hit on same key will be sucessful, but return whatever
    /// value was passed into the first _Extract.
    template <typename T>
    bool _Extract(Key const& key, T* value) {
        if (!TF_VERIFY(!_locked)) {
            return false;
        }
      
        typedef _TypedCache<T> Cache_t;
        Cache_t *cache = nullptr;

        _GetCache(&cache);
        typename Cache_t::_MapIt it = cache->_map.find(key);

        if (it == cache->_map.end()) {
            return false;
        }

        // If we're going to erase the old value, swap to avoid a copy.
        //+NV_CHANGE FRZHANG
		//looks like we do need a copy instead of delete this resource. Hope no side effect.
#define PERMANENT_CACHE 0
#if PERMANENT_CACHE
		*value = it->second;
#else
		std::swap(it->second, *value);
		cache->_deferredDeleteQueue.push(key);
#endif
		//-NV_CHANGE FRZHANG
        return true;
    }



    /// Erases the given key from the value cache.
    /// Not thread safe
    template <typename T>
    void _Erase(Key const& key) {
        if (!TF_VERIFY(!_locked)) {
            return;
        }

        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;
        _GetCache(&cache);
        cache->_map.unsafe_erase(key);
    }

    /// Returns a reference to the held value for \p key. Note that the entry
    /// for \p key will created with a default-constructed instance of T if
    /// there was no pre-existing entry.
    template <typename T>
    T& _Get(Key const& key) const {
        typedef _TypedCache<T> Cache_t;

        Cache_t *cache = nullptr;
        _GetCache(&cache);

        // With concurrent_unordered_map, multi-threaded insertion is safe.
        std::pair<typename Cache_t::_MapIt, bool> res =
                                cache->_map.insert(std::make_pair(key, T()));

        return res.first->second;
    }

    /// Removes items from the cache that are marked for deletion.
    /// This is not thread-safe and designed to be called after
    /// all the worker threads have been joined.
    template <typename T>
    void _GarbageCollect(_TypedCache<T> &cache) {
        typedef _TypedCache<T> Cache_t;

		Key key;
        while (cache._deferredDeleteQueue.try_pop(key))
        {
            auto itor = cache._map.find(key);
            if (itor != cache._map.end())
            {
                cache._map.unsafe_erase(itor);
            }
        }
    }

public:

    void EnableMutation() { _locked = false; }
    void DisableMutation() { _locked = true; }

    /// Clear all data associated with a specific path.
    void Clear(SdfPath const& path) {
        _Erase<VtValue>(Key::InstanceIndices(path));
<<<<<<< HEAD
        _Erase<TfToken>(Key::Purpose(path));
        _Erase<VtValue>(Key::Topology(path));
        _Erase<GfMatrix4d>(Key::Transform(path));
        _Erase<bool>(Key::Visible(path));
        _Erase<VtValue>(Key::Points(path));
        _Erase<VtValue>(Key::Widths(path));
        _Erase<VtValue>(Key::Normals(path));
		//+NV_CHANGE FRZHANG : NV GPU Skinning
		_Erase<VtValue>(Key::RestPoints(path));
		_Erase<GfMatrix4d>(Key::GeomBindXform(path));
		_Erase<VtValue>(Key::JointIndices(path));
		_Erase<VtValue>(Key::JointWeights(path));
		_Erase<int>(Key::NumInfluencesPerPoint(path));
		_Erase<bool>(Key::HasConstantInfluences(path));
        _Erase<TfToken>(Key::SkinningMethod(path));
        _Erase<VtValue>(Key::SkinningBlendWeights(path));
        _Erase<bool>(Key::HasConstantSkinningBlendWeights(path));
		_Erase<GfMatrix4d>(Key::PrimWorldToLocal(path));
		_Erase<VtValue>(Key::SkinningXforms(path));
		_Erase<GfMatrix4d>(Key::SkelLocalToWorld(path));
		//-NV_CHANGE FRZHANG
        _Erase<VtValue>(Key::MaterialId(path));
        _Erase<VtValue>(Key::MaterialResource(path));
=======
>>>>>>> v20.11-rc1

        // PERFORMANCE: We're copying the primvar vector here, but we could
        // access the map directly, if we need to for performance reasons.
        HdPrimvarDescriptorVector vars;
        if (FindPrimvars(path, &vars)) {
            TF_FOR_ALL(pvIt, vars) {
                _Erase<VtValue>(Key(path, pvIt->name));
            }
            _Erase<HdPrimvarDescriptorVector>(Key::Primvars(path));
        }
    }

    VtValue& GetInstanceIndices(SdfPath const& path) const {
        return _Get<VtValue>(Key::InstanceIndices(path));
    }
    HdPrimvarDescriptorVector& GetPrimvars(SdfPath const& path) const {
        return _Get<HdPrimvarDescriptorVector>(Key::Primvars(path));
    }
<<<<<<< HEAD
    VtValue& GetTopology(SdfPath const& path) const {
        return _Get<VtValue>(Key::Topology(path));
    }
    GfMatrix4d& GetTransform(SdfPath const& path) const {
        return _Get<GfMatrix4d>(Key::Transform(path));
    }
    bool& GetVisible(SdfPath const& path) const {
        return _Get<bool>(Key::Visible(path));
    }
    VtValue& GetWidths(SdfPath const& path) const {
        return _Get<VtValue>(Key::Widths(path));
    }
    VtValue& GetNormals(SdfPath const& path) const {
        return _Get<VtValue>(Key::Normals(path));
    }
	//+NV_CHANGE FRZHANG : NV GPU Skinning
	VtValue& GetRestPoints(SdfPath const& path) const {
		return _Get<VtValue>(Key::RestPoints(path));
	}
	GfMatrix4d& GetGeomBindXform(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::GeomBindXform(path));
	}
	VtValue& GetJointIndices(SdfPath const& path) const {
		return _Get<VtValue>(Key::JointIndices(path));
	}
	VtValue& GetJointWeights(SdfPath const& path) const {
		return _Get<VtValue>(Key::JointWeights(path));
	}
	int& GetNumInfluencesPerPoint(SdfPath const& path) const {
		return _Get<int>(Key::NumInfluencesPerPoint(path));
	}
	bool& GetHasConstantInfluences(SdfPath const& path) const {
		return _Get<bool>(Key::HasConstantInfluences(path));
	}
    TfToken& GetSkinningMethod(SdfPath const& path) const {
        return _Get<TfToken>(Key::SkinningMethod(path));
    }
    VtValue& GetSkinningBlendWeights(SdfPath const& path) const {
        return _Get<VtValue>(Key::SkinningBlendWeights(path));
    }
    bool& GetHasConstantSkinningBlendWeights(SdfPath const& path) const {
        return _Get<bool>(Key::HasConstantSkinningBlendWeights(path));
    }
	GfMatrix4d& GetPrimWorldToLocal(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::PrimWorldToLocal(path));
	}
	VtValue& GetSkinningXforms(SdfPath const& path) const {
		return _Get<VtValue>(Key::SkinningXforms(path));
	}
	GfMatrix4d& GetSkelLocalToWorld(SdfPath const& path) const {
		return _Get<GfMatrix4d>(Key::SkelLocalToWorld(path));
	}
	//-NV_CHANGE FRZHANG
    VtValue& GetPrimvar(SdfPath const& path, TfToken const& name) const {
        return _Get<VtValue>(Key(path, name));
    }
    SdfPath& GetMaterialId(SdfPath const& path) const {
        return _Get<SdfPath>(Key::MaterialId(path));
    }
    VtValue& GetMaterialResource(SdfPath const& path) const {
        return _Get<VtValue>(Key::MaterialResource(path));
    }
    TfTokenVector& GetExtComputationSceneInputNames(SdfPath const& path) const {
        return _Get<TfTokenVector>(Key::ExtComputationSceneInputNames(path));
    }
    HdExtComputationInputDescriptorVector&
    GetExtComputationInputs(SdfPath const& path) const {
        return _Get<HdExtComputationInputDescriptorVector>(
            Key::ExtComputationInputs(path));
    }
    HdExtComputationOutputDescriptorVector&
    GetExtComputationOutputs(SdfPath const& path) const {
        return _Get<HdExtComputationOutputDescriptorVector>(
            Key::ExtComputationOutputs(path));
    }
    HdExtComputationPrimvarDescriptorVector&
    GetExtComputationPrimvars(SdfPath const& path) const {
        return _Get<HdExtComputationPrimvarDescriptorVector>(
            Key::ExtComputationPrimvars(path));
    }
    VtValue& GetExtComputationInput(SdfPath const& path,
                                    TfToken const& name) const {
        return _Get<VtValue>(Key(path, name));
    }
    std::string& GetExtComputationKernel(SdfPath const& path) const {
        return _Get<std::string>(Key::ExtComputationKernel(path));
    }
    VtValue& GetCameraParam(SdfPath const& path, TfToken const& name) const {
        return _Get<VtValue>(Key(path, name));
    }
    TfTokenVector& GetCameraParamNames(SdfPath const& path) const {
        return _Get<TfTokenVector>(Key::CameraParamNames(path));
    }
=======
>>>>>>> v20.11-rc1

    bool FindInstanceIndices(SdfPath const& path, VtValue* value) const {
        return _Find(Key::InstanceIndices(path), value);
    }
    bool FindPrimvars(SdfPath const& path, HdPrimvarDescriptorVector* value) const {
        return _Find(Key::Primvars(path), value);
    }
<<<<<<< HEAD
    bool FindTopology(SdfPath const& path, VtValue* value) const {
        return _Find(Key::Topology(path), value);
    }
    bool FindTransform(SdfPath const& path, GfMatrix4d* value) const {
        return _Find(Key::Transform(path), value);
    }
    bool FindVisible(SdfPath const& path, bool* value) const {
        return _Find(Key::Visible(path), value);
    }
    bool FindWidths(SdfPath const& path, VtValue* value) const {
        return _Find(Key::Widths(path), value);
    }
    bool FindNormals(SdfPath const& path, VtValue* value) const {
        return _Find(Key::Normals(path), value);
    }
	//+NV_CHANGE FRZHANG : NV GPU Skinning
	bool FindRestPoints(SdfPath const& path, VtValue* value) const {
		return _Find(Key::RestPoints(path), value);
	}
	bool FindGeomBindXform(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::GeomBindXform(path), value);
	}
	bool FindJointIndices(SdfPath const& path, VtValue* value) const {
		return _Find(Key::JointIndices(path), value);
	}
	bool FindJointWeights(SdfPath const& path, VtValue* value) const {
		return _Find(Key::JointWeights(path), value);
	}
	bool FindNumInfluencesPerPoint(SdfPath const& path, int* value) const {
		return _Find(Key::NumInfluencesPerPoint(path), value);
	}
	bool FindHasConstantInfluences(SdfPath const& path, bool* value) const {
		return _Find(Key::HasConstantInfluences(path), value);
	}
    bool FindSkinningMethod(SdfPath const& path, TfToken* value) const {
        return _Find(Key::SkinningMethod(path), value);
    }
    bool FindSkinningBlendWeights(SdfPath const& path, VtValue* value) const {
        return _Find(Key::SkinningBlendWeights(path), value);
    }
    bool FindHasConstantSkinningBlendWeights(SdfPath const& path, bool* value) const {
        return _Find(Key::HasConstantSkinningBlendWeights(path), value);
    }
	bool FindPrimWorldToLocal(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::PrimWorldToLocal(path), value);
	}
	bool FindSkinningXforms(SdfPath const& path, VtValue* value) const {
		return _Find(Key::SkinningXforms(path), value);
	}
	bool FindSkelLocalToWorld(SdfPath const& path, GfMatrix4d* value) const {
		return _Find(Key::SkelLocalToWorld(path), value);
	}
	//-NV_CHANGE FRZHANG
    bool FindMaterialId(SdfPath const& path, SdfPath* value) const {
        return _Find(Key::MaterialId(path), value);
    }
    bool FindMaterialResource(SdfPath const& path, VtValue* value) const {
        return _Find(Key::MaterialResource(path), value);
    }
    bool FindExtComputationSceneInputNames(SdfPath const& path,
                                           TfTokenVector* value) const {
        return _Find(Key::ExtComputationSceneInputNames(path), value);
    }
    bool FindExtComputationInputs(
        SdfPath const& path,
        HdExtComputationInputDescriptorVector* value) const {
        return _Find(Key::ExtComputationInputs(path), value);
    }
    bool FindExtComputationOutputs(
        SdfPath const& path,
        HdExtComputationOutputDescriptorVector* value) const {
        return _Find(Key::ExtComputationOutputs(path), value);
    }
    bool FindExtComputationPrimvars(
        SdfPath const& path,
        HdExtComputationPrimvarDescriptorVector* value) const {
        return _Find(Key::ExtComputationPrimvars(path), value);
    }
    bool FindExtComputationInput(
        SdfPath const& path, TfToken const& name, VtValue* value) const {
        return _Find(Key(path, name), value);
    }
    bool FindExtComputationKernel(SdfPath const& path, std::string* value) const {
        return _Find(Key::ExtComputationKernel(path), value);
    }
    bool FindCameraParam(SdfPath const& path, TfToken const& name,
                         VtValue* value) const {
        return _Find(Key(path, name), value);
    }
    bool FindCameraParamNames(SdfPath const& path, TfTokenVector* value) const {
        return _Find(Key::CameraParamNames(path), value);
    }
=======
>>>>>>> v20.11-rc1

    bool ExtractInstanceIndices(SdfPath const& path, VtValue* value) {
        return _Extract(Key::InstanceIndices(path), value);
    }
    bool ExtractPrimvars(SdfPath const& path, HdPrimvarDescriptorVector* value) {
        return _Extract(Key::Primvars(path), value);
    }
<<<<<<< HEAD
    bool ExtractTopology(SdfPath const& path, VtValue* value) {
        return _Extract(Key::Topology(path), value);
    }
    bool ExtractTransform(SdfPath const& path, GfMatrix4d* value) {
        return _Extract(Key::Transform(path), value);
    }
    bool ExtractVisible(SdfPath const& path, bool* value) {
        return _Extract(Key::Visible(path), value);
    }
    bool ExtractWidths(SdfPath const& path, VtValue* value) {
        return _Extract(Key::Widths(path), value);
    }
    bool ExtractNormals(SdfPath const& path, VtValue* value) {
        return _Extract(Key::Normals(path), value);
    }
	//+NV_CHANGE FRZHANG : NV GPU Skinning
	bool ExtractRestPoints(SdfPath const& path, VtValue* value) {
		return _Extract(Key::RestPoints(path), value);
	}
	bool ExtractGeomBindXform(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::GeomBindXform(path), value);
	}
	bool ExtractJointIndices(SdfPath const& path, VtValue* value) {
		return _Extract(Key::JointIndices(path), value);
	}
	bool ExtractJointWeights(SdfPath const& path, VtValue* value) {
		return _Extract(Key::JointWeights(path), value);
	}
	bool ExtractNumInfluencesPerPoint(SdfPath const& path, int* value) {
		return _Extract(Key::NumInfluencesPerPoint(path), value);
	}
	bool ExtractHasConstantInfluences(SdfPath const& path, bool* value) {
		return _Extract(Key::HasConstantInfluences(path), value);
	}
    bool ExtractSkinningMethod(SdfPath const& path, TfToken* value) {
        return _Extract(Key::SkinningMethod(path), value);
    }
    bool ExtractSkinningBlendWeights(SdfPath const& path, VtValue* value) {
        return _Extract(Key::SkinningBlendWeights(path), value);
    }
    bool ExtractHasConstantSkinningBlendWeights(SdfPath const& path, bool* value) {
        return _Extract(Key::HasConstantSkinningBlendWeights(path), value);
    }
	bool ExtractPrimWorldToLocal(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::PrimWorldToLocal(path), value);
	}
	bool ExtractSkinningXforms(SdfPath const& path, VtValue* value) {
		return _Extract(Key::SkinningXforms(path), value);
	}
	bool ExtractSkelLocalToWorld(SdfPath const& path, GfMatrix4d* value) {
		return _Extract(Key::SkelLocalToWorld(path), value);
	}
	//-NV_CHANGE FRZHANG
    bool ExtractMaterialId(SdfPath const& path, SdfPath* value) {
        return _Extract(Key::MaterialId(path), value);
    }
    bool ExtractMaterialResource(SdfPath const& path, VtValue* value) {
        return _Extract(Key::MaterialResource(path), value);
    }
    bool ExtractPrimvar(SdfPath const& path, TfToken const& name, VtValue* value) {
        return _Extract(Key(path, name), value);
    }
    bool ExtractExtComputationSceneInputNames(SdfPath const& path,
                                              TfTokenVector* value) {
        return _Extract(Key::ExtComputationSceneInputNames(path), value);
    }
    bool ExtractExtComputationInputs(
        SdfPath const& path,
        HdExtComputationInputDescriptorVector* value) {
        return _Extract(Key::ExtComputationInputs(path), value);
    }
    bool ExtractExtComputationOutputs(
        SdfPath const& path,
        HdExtComputationOutputDescriptorVector* value) {
        return _Extract(Key::ExtComputationOutputs(path), value);
    }
    bool ExtractExtComputationPrimvars(
        SdfPath const& path,
        HdExtComputationPrimvarDescriptorVector* value) {
        return _Extract(Key::ExtComputationPrimvars(path), value);
    }
    bool ExtractExtComputationInput(SdfPath const& path, TfToken const& name,
                                    VtValue* value) {
        return _Extract(Key(path, name), value);
    }
    bool ExtractExtComputationKernel(SdfPath const& path, std::string* value) {
        return _Extract(Key::ExtComputationKernel(path), value);
    }
    bool ExtractCameraParam(SdfPath const& path, TfToken const& name,
                            VtValue* value) {
        return _Extract(Key(path, name), value);
    }
    // Skip adding ExtractCameraParamNames as we don't expose scene delegate
    // functionality to query all available parameters on a camera.
=======
>>>>>>> v20.11-rc1

    /// Remove any items from the cache that are marked for defered deletion.
    void GarbageCollect()
    {
<<<<<<< HEAD
        _GarbageCollect(_boolCache);
		//+NV_CHANGE FRZHANG
		_GarbageCollect(_intCache);
		//-NV_CHANGE FRZHANG
        _GarbageCollect(_tokenCache);
        _GarbageCollect(_tokenVectorCache);
        _GarbageCollect(_rangeCache);
        _GarbageCollect(_cullStyleCache);
        _GarbageCollect(_matrixCache);
        _GarbageCollect(_vec4Cache);
=======
>>>>>>> v20.11-rc1
        _GarbageCollect(_valueCache);
        _GarbageCollect(_pviCache);
    }

private:
    bool _locked;

<<<<<<< HEAD
    // visible, doubleSided
    typedef _TypedCache<bool> _BoolCache;
    mutable _BoolCache _boolCache;

	//+NV_CHANGE FRZHANG
	typedef _TypedCache<int> _IntCache;
	mutable _IntCache _intCache;
	//-NV_CHANGE FRZHANG

    // purpose
    typedef _TypedCache<TfToken> _TokenCache;
    mutable _TokenCache _tokenCache;

    // extComputationSceneInputNames
    typedef _TypedCache<TfTokenVector> _TokenVectorCache;
    mutable _TokenVectorCache _tokenVectorCache;

    // extent
    typedef _TypedCache<GfRange3d> _RangeCache;
    mutable _RangeCache _rangeCache;

    // cullstyle
    typedef _TypedCache<HdCullStyle> _CullStyleCache;
    mutable _CullStyleCache _cullStyleCache;

    // transform
    typedef _TypedCache<GfMatrix4d> _MatrixCache;
    mutable _MatrixCache _matrixCache;

    // color (will be VtValue)
    typedef _TypedCache<GfVec4f> _Vec4Cache;
    mutable _Vec4Cache _vec4Cache;

    // sdfPath
    typedef _TypedCache<SdfPath> _SdfPathCache;
    mutable _SdfPathCache _sdfPathCache;

    // primvars, topology, materialResources, extCompInputs
=======
>>>>>>> v20.11-rc1
    typedef _TypedCache<VtValue> _ValueCache;
    mutable _ValueCache _valueCache;

    typedef _TypedCache<HdPrimvarDescriptorVector> _PviCache;
    mutable _PviCache _pviCache;

<<<<<<< HEAD
    typedef _TypedCache<std::string> _StringCache;
    mutable _StringCache _stringCache;

    typedef _TypedCache<HdExtComputationInputDescriptorVector>
        _ExtComputationInputsCache;
    mutable _ExtComputationInputsCache _extComputationInputsCache;

    typedef _TypedCache<HdExtComputationOutputDescriptorVector>
        _ExtComputationOutputsCache;
    mutable _ExtComputationOutputsCache _extComputationOutputsCache;

    typedef _TypedCache<HdExtComputationPrimvarDescriptorVector>
        _ExtComputationPrimvarsCache;
    mutable _ExtComputationPrimvarsCache _extComputationPrimvarsCache;

    void _GetCache(_BoolCache **cache) const {
        *cache = &_boolCache;
    }
	//+NV_CHANGE FRZHANG
	void _GetCache(_IntCache **cache) const {
		*cache = &_intCache;
	}
	//_NV_CHANGE
    void _GetCache(_TokenCache **cache) const {
        *cache = &_tokenCache;
    }
    void _GetCache(_TokenVectorCache **cache) const {
        *cache = &_tokenVectorCache;
    }
    void _GetCache(_RangeCache **cache) const {
        *cache = &_rangeCache;
    }
    void _GetCache(_CullStyleCache **cache) const {
        *cache = &_cullStyleCache;
    }
    void _GetCache(_MatrixCache **cache) const {
        *cache = &_matrixCache;
    }
    void _GetCache(_Vec4Cache **cache) const {
        *cache = &_vec4Cache;
    }
=======
>>>>>>> v20.11-rc1
    void _GetCache(_ValueCache **cache) const {
        *cache = &_valueCache;
    }
    void _GetCache(_PviCache **cache) const {
        *cache = &_pviCache;
    }
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_IMAGING_USD_IMAGING_VALUE_CACHE_H
