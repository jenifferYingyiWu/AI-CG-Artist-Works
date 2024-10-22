// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MuR/Ptr.h"
#include "MuR/RefCounted.h"
#include "MuT/Node.h"
#include "MuT/NodeMesh.h"
#include "Math/MathFwd.h"

namespace mu
{

	// Forward definitions
    class NodeMeshTransform;
    typedef Ptr<NodeMeshTransform> NodeMeshTransformPtr;
    typedef Ptr<const NodeMeshTransform> NodeMeshTransformPtrConst;


    //! This node applies a geometric transform represented by a 4x4 matrix to a mesh
	//! \ingroup model
    class MUTABLETOOLS_API NodeMeshTransform : public NodeMesh
	{
	public:

		//-----------------------------------------------------------------------------------------
		// Life cycle
		//-----------------------------------------------------------------------------------------

        NodeMeshTransform();

		void SerialiseWrapper(OutputArchive& arch) const override;
		static void Serialise( const NodeMeshTransform* pNode, OutputArchive& arch );
        static NodeMeshTransformPtr StaticUnserialise( InputArchive& arch );

		//-----------------------------------------------------------------------------------------
		// Node Interface
		//-----------------------------------------------------------------------------------------

        const FNodeType* GetType() const override;
		static const FNodeType* GetStaticType();

		//-----------------------------------------------------------------------------------------
		// Own Interface
		//-----------------------------------------------------------------------------------------

		//! Source mesh to be re-formatted
        NodeMeshPtr GetSource() const;
		void SetSource( NodeMesh* );

        //! \param pMat4x4 pointer to 16 floats representing the 4x4 matrix to apply
        void SetTransform(const FMatrix44f&);
		const FMatrix44f& GetTransform() const;

		//-----------------------------------------------------------------------------------------
		// Interface pattern
		//-----------------------------------------------------------------------------------------
		class Private;
		Private* GetPrivate() const;
        Node::Private* GetBasePrivate() const override;

	protected:

		//! Forbidden. Manage with the Ptr<> template.
        ~NodeMeshTransform();

	private:

		Private* m_pD;

	};


}
