// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MuT/NodeMeshGeometryOperation.h"
#include "MuT/NodeScalar.h"
#include "MuT/NodeMeshPrivate.h"
#include "MuT/AST.h"

namespace mu
{


	class NodeMeshGeometryOperation::Private : public NodeMesh::Private
	{
	public:

		static FNodeType s_type;

		NodeMeshPtr m_pMeshA;
		NodeMeshPtr m_pMeshB;
		NodeScalarPtr m_pScalarA;
		NodeScalarPtr m_pScalarB;

        //!
		void Serialise( OutputArchive& arch ) const
		{
            uint32_t ver = 0;
			arch << ver;

			arch << m_pMeshA;
			arch << m_pMeshB;
			arch << m_pScalarA;
            arch << m_pScalarB;
        }

		//!
		void Unserialise( InputArchive& arch )
		{
            uint32_t ver;
			arch >> ver;
			check(ver==0);

			arch >> m_pMeshA;
			arch >> m_pMeshB;
			arch >> m_pScalarA;
			arch >> m_pScalarB;
		}

		// NodeMesh::Private interface
        NodeLayoutPtr GetLayout( int index ) const override;

	};

}
