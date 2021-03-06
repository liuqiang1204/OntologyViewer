/*
 * $Revision:  $
 * 
 * last checkin:
 *   $Author:  $ 
 *   $Date:  $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of dual graph
 * 
 * \author Michael Schulz
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

#include <ogdf/basic/DualGraph.h>

namespace ogdf{

// Computes combinatorial embedding of dual graph
// Precondition: CE must be combinatorial embedding of connected planar graph
DualGraph::DualGraph(CombinatorialEmbedding &CE)
{
    m_primalEmbedding = &CE;
    Graph &primalGraph = CE.getGraph();
    init(*(new Graph));
    Graph &dualGraph = getGraph();
    
    m_dualNode.init(CE);
    m_dualEdge.init(primalGraph);
    m_dualFace.init(primalGraph);
    m_primalNode.init(*this);
    m_primalFace.init(dualGraph);
    m_primalEdge.init(dualGraph);
    
    // create dual nodes
    face f;
    forall_faces(f, CE)
    {
		node vDual = dualGraph.newNode();
		m_dualNode[f] = vDual;
		m_primalFace[vDual] = f;
    }
    
    // create dual edges 
    edge e;
    forall_edges(e, primalGraph)
    {
		adjEntry aE = e->adjSource();
		node vDualSource = m_dualNode[CE.rightFace(aE)];
		node vDualTarget = m_dualNode[CE.leftFace(aE)];
		edge eDual = dualGraph.newEdge(vDualSource, vDualTarget);
		m_primalEdge[eDual] = e;
		m_dualEdge[e] = eDual;
    }
    
    // sort adjElements of every dual node corresponding to dual embedding
    EdgeArray<bool> visited(dualGraph, false);   // needed for self-loops
    forall_faces(f, CE) 
    {
		node vDual = m_dualNode[f];
		adjEntry aePrimal = f->firstAdj();
		List<adjEntry> aeList;
		do 
		{
			edge eDual = m_dualEdge[aePrimal->theEdge()];
			adjEntry aeDual = eDual->adjSource();
			if((aeDual->theNode()!=vDual) || (eDual->isSelfLoop() && visited[eDual]))
				aeDual = eDual->adjTarget();
			aeList.pushBack( aeDual );
			visited[eDual] = true; // only needed for self-loops
			aePrimal = aePrimal->faceCycleSucc();
		}
		while(aePrimal != f->firstAdj());
		dualGraph.sort(vDual, aeList);
    }
    
    // calculate dual faces and links to corresponding primal nodes
    computeFaces();
    node v;
    forall_nodes(v, primalGraph)
    {
		edge ePrimal = v->firstAdj()->theEdge();
		edge eDual = m_dualEdge[ePrimal];
		face fDual = rightFace(eDual->adjSource());
		if(ePrimal->source()==v)
			fDual = leftFace(eDual->adjSource());
		m_dualFace[v] = fDual;
		m_primalNode[fDual] = v;
    }
}

// Destructor
DualGraph::~DualGraph()
{
    clear();
    delete m_cpGraph;
}
    
}
