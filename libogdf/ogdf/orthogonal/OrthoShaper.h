/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-26 15:44:08 +0100 (Sat, 26 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Computes the orthogonal representation of a planar 
 *        representation of a UML graph using the simple flow
 *        approach.
 * 
 * \author Karsten Klein
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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_ORTHO_FORMER_GENERIC_H
#define OGDF_ORTHO_FORMER_GENERIC_H


#include <ogdf/orthogonal/OrthoRep.h>
#include <ogdf/planarity/PlanRepUML.h>


namespace ogdf {

const bool angleMaxBound = true;
const bool angleMinBound = false;

class OGDF_EXPORT OrthoShaper
{
public:

    enum n_type {low, high, inner, outer}; //types of network nodes: 
	                                       //nodes and faces

	OrthoShaper() {
		setDefaultSettings();
    };

	~OrthoShaper() {};

	// Given a planar representation for a UML graph and its planar 
	// combinatorial embedding, call() produces an orthogonal 
	// representation using Tamassias bend minimization algorithm
    // with a flow network where every flow unit defines 90 degree angle
	// in traditional mode.
	// A maximum number of bends per edge can be specified in 
	// startBoundBendsPerEdge. If the algorithm is not successful in 
	// producing a bend minimal representation subject to 
	// startBoundBendsPerEdge, it successively enhances the bound by 
	// one trying to compute an orthogonal representation.
	//
	// Using startBoundBendsPerEdge may not produce a bend minimal
	// representation in general.
	void call(PlanRepUML &PG,
		CombinatorialEmbedding &E,
		OrthoRep &OR,
		int startBoundBendsPerEdge = 0,
		bool fourPlanar = true) throw(AlgorithmFailureException);

	//sets the default settings used in the standard constructor
    void setDefaultSettings()
	{
		m_distributeEdges = true; // true;  //try to distribute edges to all node sides
        m_fourPlanar      = true;  //do not allow zero degree angles at high degree
        m_allowLowZero    = false; //do allow zero degree at low degree nodes
        m_multiAlign      = true;//true;  //start/end side of multi edges match
        m_traditional     = true;//true;  //prefer 3/1 flow at degree 2 (false: 2/2)
		m_deg4free        = false; //allow free angle assignment at degree four
		m_align           = false; //align nodes on same hierarchy level
	}

	// returns option distributeEdges
    bool distributeEdges() { return m_distributeEdges; }
	// sets option distributeEdges to b
	void distributeEdges(bool b) { m_distributeEdges = b; }

	// returns option multiAlign
    bool multiAlign() { return m_multiAlign; }
	// sets option multiAlign to b
	void multiAlign(bool b) { m_multiAlign = b; }

	// returns option traditional
    bool traditional() { return m_traditional; }
	// sets option traditional to b
	void traditional(bool b) { m_traditional = b; }

	//returns option deg4free
    bool fixDegreeFourAngles() { return m_deg4free; }
	//sets option deg4free
	void fixDegreeFourAngles(bool b) { m_deg4free = b; }

	//alignment of brothers in hierarchies
	void align(bool al) {m_align = al;}
	bool align() {return m_align;}



private:
	bool m_distributeEdges; // distribute edges among all sides if degree > 4
    bool m_fourPlanar;      // should the input graph be four planar 
	                        // (no zero degree)
    bool m_allowLowZero;    // allow low degree nodes zero degree
	                        // (to low for zero...)
    bool m_multiAlign;      // multi edges aligned on the same side
	bool m_deg4free;        // allow degree four nodes free angle assignment
    bool m_traditional;     // do not prefer 180 degree angles,
	                        // traditional is not tamassia,
    // traditional is a kandinsky - ILP - like network with node supply 4,
    // not traditional interprets angle flow zero as 180 degree, "flow
	// through the node"
	bool m_align;           //try to achieve an alignment in hierarchy levels

	//set angle boundary
	//warning: sets upper AND lower bounds, therefore may interfere with existing bounds
	void setAngleBound(edge netArc, int angle, EdgeArray<int>& lowB,
				  EdgeArray<int>& upB, EdgeArray<edge>& aTwin, bool maxBound = true)
	{
		//vorlaeufig
		OGDF_ASSERT(!m_traditional);
		if (m_traditional)
		{
			switch (angle)
			{
				case 0:
				case 90:
				case 180:
						break;
				OGDF_NODEFAULT
			}//switch
		}//trad
		else
		{
			switch (angle)
			{
				case 0: if (maxBound)
						{
							upB[netArc] = lowB[netArc] = 2;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = lowB[e2] = 0;
							}
						}
						else
						{
							upB[netArc] = 2; lowB[netArc] = 0;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = 2;
								lowB[e2] = 0;
							}

						}
					   break;
				case 90:
						if (maxBound)
						{
							lowB[netArc] = 1;
							upB[netArc] = 2;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = lowB[e2] = 0;
							}
						}
						else
						{
							upB[netArc] = 1; 
							lowB[netArc] = 0;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = 2;
								lowB[e2] = 0;
							}

						}
						break;
				case 180:
						if (maxBound)
						{
							lowB[netArc] = 0;
							upB[netArc] = 2;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = lowB[e2] = 0;
							}
						}
						else
						{
							upB[netArc] = 0; 
							lowB[netArc] = 0;
							edge e2 = aTwin[netArc];
							if (e2) 
							{
								upB[e2] = 2;
								lowB[e2] = 0;
							}

						}
						break;
				OGDF_NODEFAULT // wrong bound
			}//switch		
		}//progressive

	}//setAngle
};


} // end namespace ogdf


#endif
