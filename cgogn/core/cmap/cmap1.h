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

#ifndef CORE_CMAP_CMAP1_H_
#define CORE_CMAP_CMAP1_H_

#include <core/cmap/cmap0.h>
#include <core/basic/dart.h>
#include <core/utils/assert.h>

namespace cgogn
{

template <typename MAP_TRAITS, typename MAP_TYPE>
class CMap1_T : public CMap0_T<MAP_TRAITS, MAP_TYPE>
{
public:

	static const int PRIM_SIZE = 1;

	typedef MAP_TRAITS MapTraits;
	typedef MAP_TYPE MapType;
	typedef CMap0_T<MAP_TRAITS, MAP_TYPE> Inherit;
	typedef CMap1_T<MAP_TRAITS, MAP_TYPE> Self;

	friend typename Self::Inherit;
	friend typename Inherit::Inherit;
	template<typename T>
	friend class DartMarker_T;
	template<typename T>
	friend class DartMarkerStore;

	static const Orbit DART	  = Orbit::DART;
	static const Orbit VERTEX = Orbit::DART;
	static const Orbit EDGE   = Orbit::DART;
	static const Orbit FACE   = Orbit::PHI1;
	static const Orbit VOLUME = Orbit::PHI1;

	typedef Cell<Self::VERTEX> Vertex;
	typedef Cell<Self::EDGE> Edge;
	typedef Cell<Self::FACE> Face;
	typedef Cell<Self::VOLUME> Volume;

	template <typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;
	template <typename T>
	using ChunkArrayContainer = typename Inherit::template ChunkArrayContainer<T>;

	template <typename T, Orbit ORBIT>
	using AttributeHandler = typename Inherit::template AttributeHandler<T, ORBIT>;
	template <typename T>
	using VertexAttributeHandler = AttributeHandler<T, Self::VERTEX>;
	template <typename T>
	using EdgeAttributeHandler = AttributeHandler<T, Self::EDGE>;
	template <typename T>
	using FaceAttributeHandler = AttributeHandler<T, Self::FACE>;
	template <typename T>
	using VolumeAttributeHandler = AttributeHandler<T, Self::VOLUME>;

	using DartMarker = typename cgogn::DartMarker<Self>;
	using DartMarkerStore = typename cgogn::DartMarkerStore<Self>;

	template <Orbit ORBIT>
	using CellMarker = typename cgogn::CellMarker<Self, ORBIT>;

protected:

	ChunkArray<Dart>* phi1_;
	ChunkArray<Dart>* phi_1_;

	void init()
	{
		phi1_ = this->topology_.template add_attribute<Dart>("phi1");
		phi_1_ = this->topology_.template add_attribute<Dart>("phi_1");
	}

public:

	CMap1_T() : Inherit()
	{
		init();
	}

	~CMap1_T() override
	{}

	CMap1_T(Self const&) = delete;
	CMap1_T(Self &&) = delete;
	Self& operator=(Self const&) = delete;
	Self& operator=(Self &&) = delete;

	/*******************************************************************************
	 * Low-level topological operations
	 *******************************************************************************/

protected:

	/*!
	* \brief Add a Dart in the map (i.e. add a line in the topology container)
	* @return the new Dart (i.e. the index of the added line)
	* The dart is defined as a fixed point for PHI1.
	*/
	inline Dart add_dart_internal()
	{
		Dart d = Inherit::add_dart_internal();
		(*phi1_)[d.index] = d;
		(*phi_1_)[d.index] = d;
		return d;
	}

	/*!
	 * \brief Link two darts with the phi1 permutation what either merge or split their orbit(s).
	 * @param d: the first dart
	 * @param e: the second dart
	 * - Before: d->f and e->g
	 * - After:  d->g and e->f
	 * Join the orbits of dart d and e if they are distinct
	 * - Starting from two cycles : d->f->...->d and e->g->...->e
	 * - It makes one cycle d->g->...->e->f->...->d
	 * If e = g then insert e in the cycle of d : d->e->f->...->d
	 * If d and e are in the same orbit of phi1, this orbit is split in two cycles.
	 * - Starting with d->g->...e->f->...->d
	 * - It makes two cycles : d->f->...->d and e->g->...->e
	 */
	void phi1_sew(Dart d, Dart e)
	{
		Dart f = phi1(d);
		Dart g = phi1(e);
		(*phi1_)[d.index] = g;
		(*phi1_)[e.index] = f;
		(*phi_1_)[g.index] = d;
		(*phi_1_)[f.index] = e;
	}

	/*!
	 * \brief Remove the successor of a given dart from its permutation
	 * @param d a dart
	 * - Before: d->e->f
	 * - After:  d->f and e->e
	 */
	void phi1_unsew(Dart d)
	{
		Dart e = phi1(d);
		Dart f = phi1(e);
		(*phi1_)[d.index] = f;
		(*phi1_)[e.index] = e;
		(*phi_1_)[f.index] = d;
		(*phi_1_)[e.index] = e;
	}

	/*******************************************************************************
	 * Basic topological operations
	 *******************************************************************************/

public:

	/*!
	 * \brief phi1
	 * @param d
	 * @return phi1(d)
	 */
	inline Dart phi1(Dart d) const
	{
		return (*phi1_)[d.index];
	}

	/*!
	 * \brief phi_1
	 * @param d
	 * @return phi_1(d)
	 */
	Dart phi_1(Dart d) const
	{
		return (*phi_1_)[d.index];
	}

	/*******************************************************************************
	 * High-level topological operations
	 *******************************************************************************/

protected:

	/*!
	 * \brief Add a face in the map.
	 * \param size : the number of darts in the built face
	 * \return A dart of the built face
	 */
	inline Dart add_face_topo(unsigned int size)
	{
		cgogn_message_assert(size > 0u, "Cannot create an empty face");

		Dart d = this->add_dart_internal();
		for (unsigned int i = 1u; i < size; ++i)
			cut_edge_topo(d);

		return d;
	}

	/*!
	 * \brief Remove a face from the map.
	 * \param d : a dart of the face to remove
	 */
	inline void remove_face_topo(Dart d)
	{
		Dart e = phi1(d);
		while(e != d)
		{
			Dart f = phi1(e);
			this->remove_dart(e);
			e = f;
		}

		this->remove_dart(d);
	}

	/**
	 * \brief Cut an edge.
	 * \param d : a dart of the edge to cut
	 * \return the inserted new dart
	 * The edge of d is cut by inserting a new dart after d in the PHI orbit.
	 */
	inline Dart cut_edge_topo(Dart d)
	{
		Dart e = this->add_dart_internal();	// Create a new dart e
		phi1_sew(d, e);						// Insert e between d and phi1(d)
		return e;
	}

	/**
	 * \brief Remove edge d from its face and delete it
	 * @param d : the edge to collapse
	 * the edge preceeding d in the face is linked to the successor of d
	 */
	inline void collapse_edge_topo(Dart d)
	{
		Dart e = phi_1(d);
		cgogn_message_assert(e != d,"phi1_unsew: Cannot collapse fixed point");
		phi1_unsew(e);
		this->remove_dart(d);
	}

	inline void reverse_face_topo(Dart d)
	{
		Dart e = phi1(d);			// Dart e is the first edge of the new face

		if (e == d) return;			// Only one edge: nothing to do
		if (phi1(e) == d) return;	// Only two edges: nothing to do

		phi1_unsew(d);				// Detach e from the face of d

		Dart dNext = phi1(d);
		while (dNext != d)			// While the face of d contains more than two edges
		{
			phi1_unsew(d);			// Unsew the edge after d
			phi1_sew(e, dNext);		// Sew it after e (thus in reverse order)
			dNext = phi1(d);
		}
		phi1_sew(e, d);				// Sew the last edge
	}

	/*******************************************************************************
	 * High-level embedded operations
	 *******************************************************************************/

public:

	/*!
	 * \brief Add a face in the map.
	 * \param size : the number of darts in the built face
	 * \return A dart of the built face
	 */
	Face add_face(unsigned int size)
	{
		cgogn_message_assert(size > 0, "Cannot create an empty face");

		return this->to_concrete()->add_face_update_emb(this->add_face_topo(size));
	}

	inline unsigned int degree(Face f) const
	{
		return this->nb_darts(f);
	}

protected:

	Face add_face_update_emb(Face f)
	{
		CGOGN_CHECK_CONCRETE_TYPE;
		if (this->template is_orbit_embedded<DART>())
		{
			foreach_dart_of_orbit(f, [this](Dart d)
			{
				this->template set_orbit_embedding<DART>(d, this->template add_attribute_element<DART>());
			});
		}

		if (this->template is_orbit_embedded<FACE>())
			this->set_orbit_embedding(f, this->template add_attribute_element<FACE>());

		return f;
	}

	/*******************************************************************************
	 * Orbits traversal
	 *******************************************************************************/

	template <typename FUNC>
	inline void foreach_dart_of_PHI1(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			f(it);
			it = phi1(it);
		} while (it != d);
	}

	template <Orbit ORBIT, typename FUNC>
	inline void foreach_dart_of_orbit(Cell<ORBIT> c, const FUNC& f) const
	{
		static_assert(ORBIT == Orbit::DART || ORBIT == Orbit::PHI1,
					  "Orbit not supported in a CMap1");

		switch (ORBIT)
		{
			case Orbit::DART: this->foreach_dart_of_DART(c, f); break;
			case Orbit::PHI1: foreach_dart_of_PHI1(c, f); break;
			case Orbit::PHI2:
			case Orbit::PHI1_PHI2:
			case Orbit::PHI1_PHI3:
			case Orbit::PHI2_PHI3:
			case Orbit::PHI21:
			case Orbit::PHI21_PHI31:
			default: cgogn_assert_not_reached("This orbit is not handled"); break;
		}
	}

	template <typename FUNC>
	inline void foreach_dart_of_PHI1_until(Dart d, const FUNC& f) const
	{
		Dart it = d;
		do
		{
			if (!f(it))
				break;
			it = phi1(it);
		} while (it != d);
	}

	template <Orbit ORBIT, typename FUNC>
	inline void foreach_dart_of_orbit_until(Cell<ORBIT> c, const FUNC& f) const
	{
		static_assert(check_func_return_type(FUNC, bool),
					  "Wrong function return type");

		static_assert(ORBIT == Orbit::DART || ORBIT == Orbit::PHI1,
					  "Orbit not supported in a CMap1");

		switch (ORBIT)
		{
			case Orbit::DART: this->foreach_dart_of_DART(c, f); break;
			case Orbit::PHI1: foreach_dart_of_PHI1_until(c, f); break;
			case Orbit::PHI2:
			case Orbit::PHI1_PHI2:
			case Orbit::PHI1_PHI3:
			case Orbit::PHI2_PHI3:
			case Orbit::PHI21:
			case Orbit::PHI21_PHI31:
			default: cgogn_assert_not_reached("This orbit is not handled"); break;
		}
	}

public:

	/*******************************************************************************
	 * Incidence traversal
	 *******************************************************************************/

	template <typename FUNC>
	inline void foreach_incident_vertex(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
		foreach_dart_of_orbit<Orbit::PHI1>(f, func);
	}

	template <typename FUNC>
	inline void foreach_incident_edge(Face f, const FUNC& func) const
	{
		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
		foreach_dart_of_orbit<Orbit::PHI1>(f, func);
	}

	/*******************************************************************************
	 * Adjacence traversal
	 *******************************************************************************/

// To remove

//	template <typename FUNC>
//	inline void foreach_adjacent_vertex_through_edge(Vertex v, const FUNC& f) const
//	{
//		static_assert(check_func_parameter_type(FUNC, Vertex), "Wrong function cell parameter type");
//		f(Vertex(phi1(v.dart)));
//		f(Vertex(phi_1(v.dart)));
//	}

// To remove

//	template <typename FUNC>
//	inline void foreach_adjacent_edge_through_vertex(Edge e, const FUNC& f) const
//	{
//		static_assert(check_func_parameter_type(FUNC, Edge), "Wrong function cell parameter type");
//		f(Edge(phi1(e.dart)));
//		f(Edge(phi_1(e.dart)));
//	}
};

template <typename MAP_TRAITS>
struct CMap1Type
{
	typedef CMap1_T<MAP_TRAITS, CMap1Type<MAP_TRAITS>> TYPE;
};

template <typename MAP_TRAITS>
using CMap1 = CMap1_T<MAP_TRAITS, CMap1Type<MAP_TRAITS>>;

#if defined(CGOGN_USE_EXTERNAL_TEMPLATES) && (!defined(CORE_MAP_MAP1_CPP_))
extern template class CGOGN_CORE_API CMap1_T<DefaultMapTraits, CMap1Type<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarker<CMap1<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarkerStore<CMap1<DefaultMapTraits>>;
extern template class CGOGN_CORE_API DartMarkerNoUnmark<CMap1<DefaultMapTraits>>;
extern template class CGOGN_CORE_API CellMarker<CMap1<DefaultMapTraits>, Orbit::DART>;
extern template class CGOGN_CORE_API CellMarker<CMap1<DefaultMapTraits>, Orbit::PHI1>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap1<DefaultMapTraits>, Orbit::DART>;
extern template class CGOGN_CORE_API CellMarkerStore<CMap1<DefaultMapTraits>, Orbit::PHI1>;
#endif // defined(CGOGN_USE_EXTERNAL_TEMPLATES) && (!defined(CORE_MAP_MAP1_CPP_))



} // namespace cgogn

#endif // CORE_CMAP_CMAP1_H_
