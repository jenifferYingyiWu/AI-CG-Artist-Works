//
// Copyright 2023 Pixar
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
////////////////////////////////////////////////////////////////////////

/* ************************************************************************** */
/* ** This file is generated by a script.  Do not edit directly.  Edit     ** */
/* ** defs.py or the (*)Schema.template.h files to make changes.           ** */
/* ************************************************************************** */

#ifndef PXR_IMAGING_HD_PRIM_ORIGIN_SCHEMA_H
#define PXR_IMAGING_HD_PRIM_ORIGIN_SCHEMA_H

#include "pxr/imaging/hd/api.h"

#include "pxr/imaging/hd/schema.h" 

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HD_PRIM_ORIGIN_SCHEMA_TOKENS \
    (primOrigin) \
    (scenePath) \

TF_DECLARE_PUBLIC_TOKENS(HdPrimOriginSchemaTokens, HD_API,
    HD_PRIM_ORIGIN_SCHEMA_TOKENS);

//-----------------------------------------------------------------------------

class HdPrimOriginSchema : public HdSchema
{
public:
    HdPrimOriginSchema(HdContainerDataSourceHandle container)
    : HdSchema(container) {}

// --(BEGIN CUSTOM CODE: Schema Methods)--
    /// Wraps an SdfPath so that it is not affected by the
    /// prefixing scene index.
    class OriginPath {
    public:
        OriginPath(const SdfPath &path)
         : _path(path)
        {
        }

        const SdfPath &GetPath() const { return _path; }

        bool operator==(const OriginPath &other) const {
            return _path == other._path;
        }

    private:
        HD_API
        friend std::ostream &operator <<(std::ostream &stream,
                                         OriginPath const &p);

        SdfPath _path;
    };

    using OriginPathDataSource = HdTypedSampledDataSource<OriginPath>;

    /// Extracts SdfPath from container that was added via
    /// OriginPathDataSource.
    HD_API SdfPath GetOriginPath(const TfToken &name) const;

// --(END CUSTOM CODE: Schema Methods)--


    /// Retrieves a container data source with the schema's default name token
    /// "primOrigin" from the parent container and constructs a
    /// HdPrimOriginSchema instance.
    /// Because the requested container data source may not exist, the result
    /// should be checked with IsDefined() or a bool comparison before use.
    HD_API
    static HdPrimOriginSchema GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer);

    /// Returns a token where the container representing this schema is found in
    /// a container by default.
    HD_API
    static const TfToken &GetSchemaToken();

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the container representing this schema is found by default.
    HD_API
    static const HdDataSourceLocator &GetDefaultLocator();


    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the scenepath data source can be found.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    HD_API
    static const HdDataSourceLocator &GetScenePathLocator();

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif