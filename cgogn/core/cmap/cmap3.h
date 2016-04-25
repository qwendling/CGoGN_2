/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef CGOGN_CORE_CMAP_CMAP3_H_
#define CGOGN_CORE_CMAP_CMAP3_H_

#include <cgogn/core/cmap/cmap2.h>

namespace cgogn
{

// forward declaration of CMap3Builder_T
template <typename MAP_TRAITS> class CMap3Builder_T;

template <typename MAP_TRAITS, typename MAP_TYPE>
class CMap3_T : public CMap2_T<MAP_TRAITS, MAP_TYPE>
{
public:

	static const uint8 DIMENSION = 3;

	static const uint8 PRIM_SIZE = 1;

	using MapTraits = MAP_TRAITS;
	using MapType = MAP_TYPE;
	using Inherit = CMap2_T<MAP_TRAITS, MAP_TYPE>;
	using Self = CMap3_T<MAP_TRAITS, MAP_TYPE>;

	friend class MapBase<MAP_TRAITS, MAP_TYPE>;
	friend class CMap3Builder_T<MapTraits>;
	friend class DartMarker_T<Self>;
	friend class cgogn::DartMarkerStore<Self>;

	using CDart		= typename Inherit::CDart;
	using Vertex2	= typename Inherit::Vertex;
	using Vertex	= Cell<Orbit::PHI21_PHI31>;
	using Edge2		= typename Inherit::Edge;
	using Edge		= Cell<Orbit::PHI2_PHI3>;
	using Face2		= typename Inherit::Face;
	using Face		= Cell<Orbit::PHI1_PHI3>;
	using Volume	= typename Inherit::Volume;

	using Boundary  = Volume;
	using ConnectedComponent = Cell<Orbit::PHI1_PHI2_PHI3>;

	template <typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;
	template <typename T>
	using ChunkArrayContainer = typename Inherit::template ChunkArrayContainer<T>;

	template <typename T, Orbit ORBIT>
	using Attribute = typename Inherit::template Attribute<T, ORBIT>;
	template <typename T>
	using VertexAttribute = Attribute<T, Vertex::ORBIT>;
	template <typename T>
	using EdgeAttribute = Attribute<T, Edge::ORBIT>;
	template <typename T>
	using FaceAttribute = Attribute<T, Face::ORBIT>;
	template <typename T>
	using VolumeAttribute = Attribute<T, Volume::ORBIT>;

	using DartMarker = typename cgogn::DartMarker<Self>;
	using DartMarkerStore = typename cgogn::DartMarkerStore<Self>;

	template <Orbit ORBIT>
	using CellMarker = typename cgogn::CellMarker<Self, ORBIT>;

protected:

	ChunkArray<Dart>* phi3_;

	inline void init()
	{
		phi3_ = this->topology_.template add_attribute<Dart>("phi3");
	}

public:

	CMap3_T() : Inherit()
	{
		init();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(CMap3_T);

	~CMap3_T() override
	{}

	/*!
	 * \brief Check the integrity of embedding information
	 */
	inline bool check_embedding_integrity()
	{
		bool result = true;

		if (this->template is_embedded<CDart>())
			result = result && this->template is_well_embedded<CDart>();

		if (this->template is_embedded<Vertex2>())
			result = result && this->template is_well_embedded<Vertex2>();

		if (this->template is_embedded<Vertex>())
			result = result && this->template is_well_embedded<Vertex>();

		if (this->template is_embedded<Edge2>())
			result = result && this->template is_well_embedded<Edge2>();

		if (this->template is_embedded<Edge>())
			result = result && this->template is_well_embedded<Edge>();

		if (this->template is_embedded<Face2>())
			result = result && this->template is_well_embedded<Face2>();

		if (this->template is_embedded<Face>())
			result = result && this->template is_well_embedded<Face>();

		if (this->template is_embedded<Volume>())
			result = result && this->template is_well_embedded<Volume>();

		return result;
	}

	/*******************************************************************************
	 * Low-level topological operations
	 *******************************************************************************/

protected:

	/**
	* \brief Init an newly added dart.
	* The dart is defined as a fixed point for PHI3.
	*/
	inline void init_dart(Dart d)
	{
		Inherit::init_dart(d);
		(*phi3_)[d.index] = d;
	}

	/**
	 * @brief Check the integrity of a dart
	 * @param d the dart to check
	 * @return true if the integrity constraints are locally statisfied
	 * PHI3_PHI1 should be an involution without fixed point and
	 */
	inline bool check_integrity(Dart d) const
	{
		return (Inherit::check_integrity(d) &&
				phi3(phi3(d)) == d &&
				phi3(d) != d &&
				phi3(this->phi1(phi3(this->phi1(d)))) == d &&
				( this->is_boundary(d) == this->is_boundary(this->phi2(d)) ));
	}

	/**
	 * @brief Check the integrity of a boundary dart
	 * @param d the dart to check
	 * @return true if the bondary constraints are locally statisfied
	 * The boundary is a 2-manyfold: the boundary marker is the same
	 * for all darts of a face and for two adjacent faces.
	 */
	inline bool check_boundary_integrity(Dart d) const
	{
		return (( this->is_boundary(d) == this->is_boundary(this->phi1(d))  ) &&
				( this->is_boundary(d) == this->is_boundary(this->phi2(d)) ));
	}

	/**
	 * \brief Link dart d with dart e by an involution
	 * @param d,e the darts to link
	 *	- Before: d->d and e->e
	 *	- After:  d->e and e->d
	 */
	inline void phi3_sew(Dart d, Dart e)
	{
		cgogn_assert(phi3(d) == d);
		cgogn_assert(phi3(e) == e);
		(*phi3_)[d.index] = e;
		(*phi3_)[e.index] = d;
	}

	/**
	 * \brief Remove the phi3 link between the current dart and its linked dart
	 * @param d the dart to unlink
	 * - Before: d->e and e->d
	 * - After:  d->d and e->e
	 */
	inline void phi3_unsew(Dart d)
	{
		Dart e = phi3(d);
		(*phi3_)[d.index] = d;
		(*phi3_)[e.index] = e;
	}

	/*******************************************************************************
	 * Basic topological operations
	 *******************************************************************************/

public:

	/**
	 * \brief phi3
	 * @param d
	 * @return phi3(d)
	 */
	inline Dart phi3(Dart d) const
	{
		return (*phi3_)[d.index];
	}

	/**
	 * \brief phi composition
	 * @param d
	 * @return applied composition of phi in order of declaration
	 */
	template <uint64 N>
	inline Dart phi(Dart d) const
	{
		static_assert((N%10)<=3,"composition on phi1/phi2/only");
		switch(N%10)
		{
			case 1 : return this->phi1(phi<N/10>(d)) ;
			case 2 : return this->phi2(phi<N/10>(d)) ;
			case 3 : return this->phi3(phi<N/10>(d)) ;
			default : return d ;
		}
	}

	/*******************************************************************************
	 * High-level embedded and topological operations
	 *******************************************************************************/

protected:

	/**
	 * @brief add_stamp_volume_topo : a flat volume with one face composed of two triangles and another compose of one quad
	 * @return a dart of the quad
	 */
	Dart add_stamp_volume_topo()
	{
		const Dart d_quad = Inherit::Inherit::add_face_topo(4u);
		const Dart d_tri1 = Inherit::Inherit::add_face_topo(3u);
		const Dart d_tri2 = Inherit::Inherit::add_face_topo(3u);

		this->phi2_sew(d_tri1, d_tri2);
		this->phi2_sew(d_quad, this->phi1(d_tri1));
		this->phi2_sew(this->phi1(d_quad), this->phi_1(d_tri2));
		this->phi2_sew(this->phi1(this->phi1(d_quad)), this->phi1(d_tri2));
		this->phi2_sew(this->phi_1(d_quad), this->phi_1(d_tri1));

		return d_quad;
	}

	/*******************************************************************************
	 * Connectivity information
	 *******************************************************************************/

public:

	inline uint32 degree(Vertex2 v) const
	{
		return Inherit::degree(v);
	}

	inline uint32 degree(Vertex v) const
	{
		uint32 result = 0;
		foreach_incident_edge(v, [&result] (Edge) { ++result; });
		return result;
	}

	inline uint32 codegree(Edge2 e) const
	{
		return Inherit::codegree(e);
	}

	inline uint32 degree(Edge2 e) const
	{
		return Inherit::degree(e);
	}

	inline uint32 codegree(Edge) const
	{
		return 2;
	}

	inline uint32 degree(Edge e) const
	{
		uint32 result = 0;
		foreach_incident_face(e, [&result] (Face) { ++result; });
		return result;
	}

	inline uint32 codegree(Face2 f) const
	{
		return Inherit::codegree(f);
	}

	inline uint32 degree(Face2 f) const
	{
		return Inherit::degree(f);
	}

	inline uint32 codegree(Face f) const
	{
		return codegree(Face2(f.dart));
	}

	inline uint32 degree(Face f) const
	{
		if (this->is_boundary(f.dart) || this->is_boundary(phi3(f.dart)))
			return 1;
		else
			return 2;
	}

	inline uint32 codegree(Volume v) const
	{
		uint32 result = 0;
		foreach_incident_face(v, [&result] (Face) { ++result; });
		return result;
	}

	inline bool has_codegree(Face2 f, uint32 codegree) const
	{
		return Inherit::has_codegree(f, codegree);
	}

	inline bool has_codegree(Face f, uint32 codegree) const
	{
		return Inherit::has_codegree(Face2(f.dart), codegree);
	}

	/*******************************************************************************
	 * Boundary information
	 *******************************************************************************/

	bool is_adjacent_to_boundary(Boundary c)
	{
		CGOGN_CHECK_CONCRETE_TYPE;

		bool result = false;
		foreach_dart_of_orbit_until(c, [this, &result] (Dart d)
		{
			if (this->is_boundary(phi3(d))) { result = true; return false; }
			return true;
		});
		return result;
	}

protected:

	/*******************************************************************************
	 * Orbits traversal
	 *******************************************************************************/

	template <typename FUNC>
	inline void foreach_dart_of_PHI21_PHI31(Dart d, const FUNC& f) const
	{
		DartMarkerStore marker(*this);
		const std::vector<Dart>* marked_darts = marker.get_marked_darts();

		marker.mark(d);
		for(uint32 i = 0; i < marked_darts->size(); ++i)
		{
			const Dart curr_dart = marked_darts->operator [](i);
			f(curr_dart);

			const Dart d_1 = this->phi_1(curr_dart);
			const Dart d2_1 = this->phi2(d_1); // turn in volume
			const Dart d3_1 = phi3(d_1); // change volume

			if(!marker.is_marked(d2_1))
				marker.mark(d2_1);
			if(!marker.is_marked(d3_1))
				marker.mark(d3_1);
		}
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI2_PHI3(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			f(it);
			it = this->phi2(it);
			f(it);
			it = phi3(it);
		} while (it != d);
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI23(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			f(it);
			it = phi3(this->phi2(it));
		} while (it != d);
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI1_PHI3(Dart d, const FUNC& f) const
	{
		this->foreach_dart_of_PHI1(d, [&] (Dart fd)
		{
			f(fd);
			f(phi3(fd));
		});
	}

	template <typename FUNC>
	void foreach_dart_of_PHI1_PHI2_PHI3(Dart d, const FUNC& f) const
	{
		DartMarkerStore marker(*this);

		std::vector<Dart>* visited_face2 = cgogn::get_dart_buffers()->get_buffer();
		visited_face2->push_back(d); // Start with the face of d

		// For every face added to the list
		for(uint32 i = 0; i < visited_face2->size(); ++i)
		{
			Dart e = (*visited_face2)[i];
			if (!marker.is_marked(e))	// Face2 has not been visited yet
			{
				// mark visited darts (current face2)
				// and add non visited phi2-adjacent face2 to the list of face2
				Dart it = e;
				do
				{
					f(it); // apply the function to the darts of the face2
					marker.mark(it);				// Mark
					Dart adj2 = this->phi2(it);		// Get phi2-adjacent face2
					if (!marker.is_marked(adj2))
						visited_face2->push_back(adj2);	// Add it
					it = this->phi1(it);
				} while (it != e);
				// add phi3-adjacent face2 to the list
				visited_face2->push_back(phi3(it));
			}
		}
		cgogn::get_dart_buffers()->release_buffer(visited_face2);
	}

	template <Orbit ORBIT, typename FUNC>
	inline void foreach_dart_of_orbit(Cell<ORBIT> c, const FUNC& f) const
	{
		static_assert(check_func_parameter_type(FUNC, Dart), "Wrong function parameter type");
		static_assert(ORBIT == Orbit::DART || ORBIT == Orbit::PHI1 || ORBIT == Orbit::PHI2 ||
					  ORBIT == Orbit::PHI1_PHI2 || ORBIT == Orbit::PHI21 ||
					  ORBIT == Orbit::PHI1_PHI3 || ORBIT == Orbit::PHI2_PHI3 ||
					  ORBIT == Orbit::PHI21_PHI31 || ORBIT == Orbit::PHI1_PHI2_PHI3,
					  "Orbit not supported in a CMap3");

		switch (ORBIT)
		{
			case Orbit::DART: f(c.dart); break;
			case Orbit::PHI1: this->foreach_dart_of_PHI1(c.dart, f); break;
			case Orbit::PHI2: this->foreach_dart_of_PHI2(c.dart, f); break;
			case Orbit::PHI1_PHI2: this->foreach_dart_of_PHI1_PHI2(c.dart, f); break;
			case Orbit::PHI1_PHI3: foreach_dart_of_PHI1_PHI3(c.dart, f); break;
			case Orbit::PHI2_PHI3: foreach_dart_of_PHI2_PHI3(c.dart, f); break;
			case Orbit::PHI21: this->foreach_dart_of_PHI21(c.dart, f); break;
			case Orbit::PHI21_PHI31: foreach_dart_of_PHI21_PHI31(c.dart, f); break;
			case Orbit::PHI1_PHI2_PHI3: foreach_dart_of_PHI1_PHI2_PHI3(c.dart, f); break;
			default: cgogn_assert_not_reached("This orbit is not handled"); break;
		}
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI21_PHI31_until(Dart d, const FUNC& f) const
	{
		DartMarkerStore marker(*this);
		const std::vector<Dart>* marked_darts = marker.get_marked_darts();

		marker.mark(d);
		for(uint32 i = 0; i < marked_darts->size(); ++i)
		{
			const Dart curr_dart = marked_darts->operator [](i);
			if (!f(curr_dart))
				break;

			const Dart d_1 = this->phi_1(curr_dart);
			const Dart d2_1 = this->phi2(d_1); // turn in volume
			const Dart d3_1 = phi3(d_1); // change volume

			if(!marker.is_marked(d2_1))
				marker.mark(d2_1);
			if(!marker.is_marked(d3_1))
				marker.mark(d3_1);
		}
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI2_PHI3_until(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			if (!f(it))
				break;
			it = this->phi2(it);
			if (!f(it))
				break;
			it = phi3(it);
		} while (it != d);
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI23_until(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			if (!f(it))
				break;
			it = phi3(this->phi2(it));
		} while (it != d);
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI1_PHI3_until(Dart d, const FUNC& f) const
	{
		this->foreach_dart_of_PHI1_until(d, [&] (Dart fd) -> bool
		{
			if (f(fd))
				return f(phi3(fd));
			return false;
		});
	}

	template <typename FUNC>
	void foreach_dart_of_PHI1_PHI2_PHI3_until(Dart d, const FUNC& f) const
	{
		DartMarkerStore marker(*this);

		std::vector<Dart>* visited_face2 = cgogn::get_dart_buffers()->get_buffer();
		visited_face2->push_back(d); // Start with the face of d

		// For every face added to the list
		for(uint32 i = 0; i < visited_face2->size(); ++i)
		{
			Dart e = (*visited_face2)[i];
			if (!marker.is_marked(e))	// Face2 has not been visited yet
			{
				// mark visited darts (current face2)
				// and add non visited phi2-adjacent face2 to the list of face2
				Dart it = e;
				do
				{
					if (!f(it)) // apply the function to the darts of the face2
					{
						cgogn::get_dart_buffers()->release_buffer(visited_face2);
						return;
					}
					marker.mark(it);				// Mark
					Dart adj2 = this->phi2(it);		// Get phi2-adjacent face2
					if (!marker.is_marked(adj2))
						visited_face2->push_back(adj2);	// Add it
					it = this->phi1(it);
				} while (it != e);
				// add phi3-adjacent face2 to the list
				visited_face2->push_back(phi3(it));
			}
		}
		cgogn::get_dart_buffers()->release_buffer(visited_face2);
	}

	template <Orbit ORBIT, typename FUNC>
	inline void foreach_dart_of_orbit_until(Cell<ORBIT> c, const FUNC& f) const
	{
		static_assert(check_func_parameter_type(FUNC, Dart), "Wrong function parameter type");
		static_assert(check_func_return_type(FUNC, bool), "Wrong function return type");
		static_assert(ORBIT == Orbit::DART || ORBIT == Orbit::PHI1 || ORBIT == Orbit::PHI2 ||
					  ORBIT == Orbit::PHI1_PHI2 || ORBIT == Orbit::PHI21 ||
					  ORBIT == Orbit::PHI1_PHI3 || ORBIT == Orbit::PHI2_PHI3 ||
					  ORBIT == Orbit::PHI21_PHI31 || ORBIT == Orbit::PHI1_PHI2_PHI3,
					  "Orbit not supported in a CMap3");

		switch (ORBIT)
		{
			case Orbit::DART: f(c.dart); break;
			case Orbit::PHI1: this->foreach_dart_of_PHI1_until(c.dart, f); break;
			case Orbit::PHI2: this->foreach_dart_of_PHI2_until(c.dart, f); break;
			case Orbit::PHI1_PHI2: this->foreach_dart_of_PHI1_PHI2_until(c.dart, f); break;
			case Orbit::PHI1_PHI3: foreach_dart_of_PHI1_PHI3_until(c.dart, f); break;
			case Orbit::PHI2_PHI3: foreach_dart_of_PHI2_PHI3_until(c.dart, f); break;
			case Orbit::PHI21: this->foreach_dart_of_PHI21_until(c.dart, f); break;
			case Orbit::PHI21_PHI31: foreach_dart_of_PHI21_PHI31_until(c.dart, f); break;
			case Orbit::PHI1_PHI2_PHI3: foreach_dart_of_PHI1_PHI2_PHI3_until(c.dart, f); break;
			default: cgogn_assert_not_reached("This orbit is not handled"); break;
		}
	}

public:

	/*******************************************************************************
	 * Incidence traversal
	 *******************************************************************************/

	template <typename FUNC>
	inline void foreach_incident_edge(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		DartMarkerStore marker(*this);
		foreach_dart_of_orbit(v, [&] (Dart d)
		{
			if (!marker.is_marked(d))
			{
				foreach_dart_of_PHI23(d, [&marker] (Dart dd) { marker.mark(dd); });
				func(Edge(d));
			}
		});
	}

	template <typename FUNC>
	inline void foreach_incident_face(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		DartMarkerStore marker(*this);
		foreach_dart_of_orbit(v, [&] (Dart d)
		{
			if (!marker.is_marked(d))
			{
				marker.mark(d);
				marker.mark(this->phi1(phi3(d)));
				func(Face(d));
			}
		});
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		DartMarkerStore marker(*this);
		foreach_dart_of_orbit(v, [&] (Dart d)
		{
			if (!marker.is_marked(d) && !this->is_boundary(d))
			{
				marker.mark_orbit(Vertex2(d));
				func(Volume(d));
			}
		});
	}

	template <typename FUNC>
	inline void foreach_incident_vertex(Edge e, const FUNC& f) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		f(Vertex(e.dart));
		f(Vertex(this->phi2(e.dart)));
	}

	template <typename FUNC>
	inline void foreach_incident_face(Edge e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		foreach_dart_of_PHI23(e.dart, [&func] (Dart d) { func(Face(d)); });
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Edge e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		foreach_dart_of_PHI23(e.dart, [this, &func] (Dart d)
		{
			if (!this->is_boundary(d))
				func(Volume(d));
		});
	}

	template <typename FUNC>
	inline void foreach_incident_vertex(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		foreach_dart_of_orbit(Face2(f.dart), [&func] (Dart v) { func(Vertex(v)); });
	}


	template <typename FUNC>
	inline void foreach_incident_edge(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		foreach_dart_of_orbit(Face2(f.dart), [&func] (Dart e) { func(Edge(e)); });
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		if (!this->is_boundary(f.dart))
			func(Volume(f.dart));
		const Dart d3 = phi3(f.dart);
		if (!this->is_boundary(d3))
			func(Volume(d3));
	}

	template <typename FUNC>
	inline void foreach_incident_vertex(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		Inherit::foreach_incident_vertex(v, [&func] (Vertex2 ve)
		{
			func(Vertex(ve.dart));
		});
	}

	template <typename FUNC>
	inline void foreach_incident_edge(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		Inherit::foreach_incident_edge(v, [&func] (Edge2 e)
		{
			func(Edge(e.dart));
		});
	}

	template <typename FUNC>
	inline void foreach_incident_face(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		DartMarkerStore marker(*this);
		foreach_dart_of_orbit(v, [&] (Dart d)
		{
			if (!marker.is_marked(d))
			{
				marker.mark_orbit(Face2(d));
				func(Face(d));
			}
		});
	}

	// redeclare CMap2 hidden functions

	template <typename FUNC>
	inline void foreach_incident_edge(Vertex2 v, const FUNC& func) const
	{
		Inherit::foreach_incident_edge(v, func);
	}

	template <typename FUNC>
	inline void foreach_incident_face(Vertex2 v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(v, [this, &func] (Dart d)
		{
			func(Face2(d));
		});
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Vertex2 v, const FUNC& func) const
	{
		Inherit::foreach_incident_volume(v, func);
	}

	template <typename FUNC>
	inline void foreach_incident_vertex(Edge2 e, const FUNC& func) const
	{
		Inherit::foreach_incident_vertex(e, func);
	}

	template <typename FUNC>
	inline void foreach_incident_face(Edge2 e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(e, [this, &func] (Dart d)
		{
			func(Face2(d));
		});
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Edge2 e, const FUNC& func) const
	{
		Inherit::foreach_incident_volume(e, func);
	}

	template <typename FUNC>
	inline void foreach_incident_vertex(Face2 f, const FUNC& func) const
	{
		Inherit::foreach_incident_vertex(f, func);
	}

	template <typename FUNC>
	inline void foreach_incident_edge(Face2 f, const FUNC& func) const
	{
		Inherit::foreach_incident_edge(f, func);
	}

	template <typename FUNC>
	inline void foreach_incident_volume(Face2 f, const FUNC& func) const
	{
		Inherit::foreach_incident_volume(f, func);
	}

	/*******************************************************************************
	 * Adjacence traversal
	 *******************************************************************************/

	template <typename FUNC>
	inline void foreach_adjacent_vertex_through_edge(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		foreach_incident_edge(v, [&] (Edge e)
		{
			func(Vertex(this->phi2(e.dart)));
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_vertex_through_face(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		DartMarker marker_vertex(*this);
		marker_vertex.mark_orbit(v);
		foreach_incident_face(v, [&] (Face inc_face)
		{
			foreach_incident_vertex(inc_face, [&] (Vertex vertex_of_face)
			{
				if (!marker_vertex.is_marked(vertex_of_face.dart))
				{
					marker_vertex.mark_orbit(vertex_of_face);
					func(vertex_of_face);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_vertex_through_volume(Vertex v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		DartMarker marker_vertex(*this);
		marker_vertex.mark_orbit(v);
		foreach_incident_volume(v, [&] (Volume inc_vol)
		{
			foreach_incident_vertex(inc_vol, [&] (Vertex inc_vert)
			{
				if (!marker_vertex.is_marked(inc_vert.dart))
				{
					marker_vertex.mark_orbit(inc_vert);
					func(inc_vert);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_edge_through_vertex(Edge e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		foreach_incident_vertex(e, [&] (Vertex iv)
		{
			foreach_incident_edge(iv, [&] (Edge ie)
			{
				if (ie.dart != iv.dart)
					func(ie);
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_edge_through_face(Edge e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		DartMarker marker_edge(*this);
		marker_edge.mark_orbit(e);
		foreach_incident_face(e, [&] (Face inc_face)
		{
			foreach_incident_edge(inc_face, [&] (Edge inc_edge)
			{
				if (!marker_edge.is_marked(inc_edge.dart))
				{
					marker_edge.mark_orbit(inc_edge);
					func(inc_edge);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_edge_through_volume(Edge e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		DartMarker marker_edge(*this);
		marker_edge.mark_orbit(e);
		foreach_incident_volume(e, [&] (Volume inc_vol)
		{
			foreach_incident_edge(inc_vol, [&] (Edge inc_edge)
			{
				if (!marker_edge.is_marked(inc_edge.dart))
				{
					marker_edge.mark_orbit(inc_edge);
					func(inc_edge);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_face_through_vertex(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		DartMarker marker_face(*this);
		marker_face.mark_orbit(f);
		foreach_incident_vertex(f, [&] (Vertex v)
		{
			foreach_incident_face(f, [&](Face inc_fac)
			{
				if (!marker_face.is_marked(inc_fac.dart))
				{
					marker_face.mark_orbit(inc_fac);
					func(inc_fac);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_face_through_edge(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		foreach_incident_edge(f, [&] (Edge ie)
		{
			foreach_incident_face(ie, [&] (Face iface)
			{
				if (iface.dart != ie.dart)
					func(iface);
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_face_through_volume(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face), "Wrong function cell parameter type");
		DartMarker marker_face(*this);
		marker_face.mark_orbit(f);
		if (!this->is_boundary(f.dart))
		{
			foreach_incident_face(Volume(f.dart), [&] (Face inc_face)
			{
				if (!marker_face.is_marked(inc_face.dart))
				{
					marker_face.mark_orbit((inc_face));
					func(inc_face);
				}
			});
		}
		const Dart d3 = phi3(f.dart);
		if (!this->is_boundary(d3))
		{
			foreach_incident_face(Volume(d3), [&] (Face inc_face)
			{
				if (!marker_face.is_marked(inc_face.dart))
				{
					marker_face.mark_orbit((inc_face));
					func(inc_face);
				}
			});
		}
	}

	template <typename FUNC>
	inline void foreach_adjacent_volume_through_vertex(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		DartMarker marker_volume(*this);
		marker_volume.mark_orbit(v);
		foreach_incident_vertex(v, [&] (Vertex inc_vert)
		{
			foreach_incident_volume(inc_vert, [&](Volume inc_vol)
			{
				if (!marker_volume.is_marked(inc_vol.dart) && !this->is_boundary(inc_vol.dart))
				{
					marker_volume.mark_orbit(inc_vol);
					func(inc_vol);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_volume_through_edge(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		DartMarker marker_volume(*this);
		marker_volume.mark_orbit(v);
		foreach_incident_edge(v, [&] (Edge inc_edge)
		{
			foreach_incident_volume(inc_edge, [&] (Volume inc_vol)
			{
				if (!marker_volume.is_marked(inc_vol.dart) && !this->is_boundary(inc_vol.dart))
				{
					marker_volume.mark_orbit(inc_vol);
					func(inc_vol);
				}
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_volume_through_face(Volume v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Volume), "Wrong function cell parameter type");
		DartMarker marker_volume(*this);
		marker_volume.mark_orbit(v);
		foreach_incident_face(v, [&] (Edge inc_face)
		{
			foreach_incident_volume(inc_face, [&] (Volume inc_vol)
			{
				if (!marker_volume.is_marked(inc_vol.dart) && !this->is_boundary(inc_vol.dart))
				{
					marker_volume.mark_orbit(inc_vol);
					func(inc_vol);
				}
			});
		});
	}

	// redeclare CMap2 hidden functions

	template <typename FUNC>
	inline void foreach_adjacent_vertex_through_edge(Vertex2 v, const FUNC& func) const
	{
		Inherit::foreach_adjacent_vertex_through_edge(v, func);
	}

	template <typename FUNC>
	inline void foreach_adjacent_vertex_through_face(Vertex2 v, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(v, [this, &func] (Dart vd)
		{
			Dart vd1 = this->phi1(vd);
			this->foreach_dart_of_orbit(Face2(vd), [&func, vd, vd1] (Dart fd)
			{
				// skip Vertex2 v itself and its first successor around current face
				if (fd != vd && fd != vd1)
					func(Vertex2(fd));
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_edge_through_vertex(Edge2 e, const FUNC& func) const
	{
		Inherit::foreach_adjacent_edge_through_vertex(e, func);
	}

	template <typename FUNC>
	inline void foreach_adjacent_edge_through_face(Edge2 e, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(e, [this, &func] (Dart ed)
		{
			this->foreach_dart_of_orbit(Face2(ed), [&func, ed] (Dart fd)
			{
				// skip Edge2 e itself
				if (fd != ed)
					func(Edge2(fd));
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_face_through_vertex(Face2 f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(f, [this, &func] (Dart fd)
		{
			Dart fd1 = this->phi2(this->phi_1(fd));
			this->foreach_dart_of_orbit(Vertex2(fd), [this, &func, fd, fd1] (Dart vd)
			{
				// skip Face2 f itself and its first successor around current vertex
				if (vd != fd && vd != fd1)
					func(Face2(vd));
			});
		});
	}

	template <typename FUNC>
	inline void foreach_adjacent_face_through_edge(Face2 f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Face2), "Wrong function cell parameter type");
		foreach_dart_of_orbit(f, [this, &func] (Dart d)
		{
			const Dart d2 = this->phi2(d);
				func(Face2(d2));
		});
	}

	inline std::pair<Vertex, Vertex> vertices(Edge e)
	{
		return std::pair<Vertex, Vertex>(Vertex(e.dart), Vertex(this->phi1(e.dart)));
	}
};

template <typename MAP_TRAITS>
struct CMap3Type
{
	using TYPE = CMap3_T<MAP_TRAITS, CMap3Type<MAP_TRAITS>>;
};

template <typename MAP_TRAITS>
using CMap3 = CMap3_T<MAP_TRAITS, CMap3Type<MAP_TRAITS>>;

#if defined(CGOGN_USE_EXTERNAL_TEMPLATES) && (!defined(CORE_MAP_MAP3_CPP_))
extern template class CGOGN_CORE_API CMap3_T<DefaultMapTraits, CMap3Type<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarker<CMap3<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarkerStore<CMap3<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarkerNoUnmark<CMap3<DefaultMapTraits>>;
extern template class CGOGN_CORE_API CellMarker<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Vertex::ORBIT>;
extern template class CGOGN_CORE_API CellMarker<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Edge::ORBIT>;
extern template class CGOGN_CORE_API CellMarker<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Face::ORBIT>;
extern template class CGOGN_CORE_API CellMarker<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Volume::ORBIT>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Vertex::ORBIT>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Edge::ORBIT>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Face::ORBIT>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap3<DefaultMapTraits>, CMap3<DefaultMapTraits>::Volume::ORBIT>;
#endif // defined(CGOGN_USE_EXTERNAL_TEMPLATES) && (!defined(CORE_MAP_MAP3_CPP_))

} // namespace cgogn

#endif // CGOGN_CORE_CMAP_CMAP3_H_
