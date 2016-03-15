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

#include <gtest/gtest.h>

#include <core/cmap/cmap2.h>
#include <core/cmap/cmap2_builder.h>

namespace cgogn
{

#define NB_MAX 1000

/*!
 * \brief The CMap2TopoTest class implements topological tests on CMap2
 * It derives from CMap2 to allow the test of protected methods
 *
 * Note that these tests, check that the topological operators perform as wanted
 * but do neither tests the containers (refs_, used_, etc.) or the iterators.
 * These last tests are implemented in another test suite.
 */
class CMap2TopoTest : public CMap2<DefaultMapTraits>, public ::testing::Test
{

public:

	using Inherit = CMap2<DefaultMapTraits>;
	using MapBuilder = CMap2Builder_T<DefaultMapTraits>;
	using Vertex = CMap2TopoTest::Vertex;
	using Edge   = CMap2TopoTest::Edge;
	using Face   = CMap2TopoTest::Face;
	using Volume   = CMap2TopoTest::Volume;
	using VertexMarker = CMap2TopoTest::CellMarker<Vertex::ORBIT>;

protected:

	/*!
	 * \brief A vector of darts on which the methods are tested.
	 */
	std::vector<Dart> darts_;

	/*!
	 * \brief Generate a random set of faces.
	*/
	CMap2TopoTest()
	{
		darts_.reserve(NB_MAX);
		std::srand(static_cast<unsigned int>(std::time(0)));
	}

	/*!
	 * \brief Tests if the open vertex of d contains a specified dart e.
	 * The method supposes that the given dart d is the first dart
	 * of the open PHI21 orbit (i.e. phi2(d) == d)
	 */
	bool same_open_vertex(Dart d, Dart e)
	{
		cgogn_assert(phi2(d) == d);
		Dart it = d;
		Dart it1 = phi_1(it);

		while (it != e && phi2(it1) != it1)
		{
			it = phi2(it1);
			it1 = phi_1(it);
		}
		if (it == e) return true;
		return false;
	}

	/*!
	 * \brief Tests if the volume of d contains a specified dart e.
	 * The method does not exploit the indexing information
	 */
	bool same_volume(Dart d, Dart e)
	{
		bool result = false;

		foreach_dart_of_orbit_until(Volume(d), [&](Dart vit)
		{
			if (vit == e) result = true;
			return !result;
		});

		return result;
	}

	/*!
	 * \brief Embed an open vertex d on a new attribute.
	 * The method supposes that the given dart d is the first dart
	 * of the open PHI21 orbit (i.e. phi2(d) == d)
	 */
	void new_open_vertex_embedding(Dart d)
	{
		cgogn_assert(phi2(d) == d);
		const unsigned int emb = add_attribute_element<Vertex::ORBIT>();

		Dart it = d;
		Dart it1 = phi_1(it);

		set_embedding<Vertex>(it, emb);
		while (phi2(it1) != it1)
		{
			it = phi2(it1);
			it1 = phi_1(it);
			set_embedding<Vertex>(it, emb);
		}
	}

	/*!
	 * \brief Generate a random set of faces and put them in darts_
	 * \return The total number of added vertices.
	 * The face size ranges from 1 to 10.
	 * A random dart of each face is put in the darts_ array.
	 */
	unsigned int add_faces(unsigned int n)
	{
		darts_.clear();
		unsigned int count = 0u;
		for (unsigned int i = 0u; i < n; ++i)
		{
			unsigned int m = 1u + std::rand() % 10u;
			Dart d = add_face_topo(m);
			count += m;

			m = std::rand() % 10u;
			while (m-- > 0u) d = phi1(d);

			darts_.push_back(d);
		}
		return count;
	}

	/*!
	 * \brief Generate a set of closed surfaces with arbitrary genus.
	 */
	void add_closed_surfaces()
	{
		darts_.clear();
		unsigned int n;

		// Generate NB_MAX random 1-faces (without boundary)
		for (unsigned int i = 0u; i < NB_MAX; ++i)
		{
			n = 1u + std::rand() % 10;
			Dart d = Inherit::Inherit::add_face_topo(n);
			darts_.push_back(d);
		}
		// Sew some pairs off 1-edges
		for (unsigned int i = 0u; i < 3u * NB_MAX; ++i)
		{
			Dart e1 = darts_[std::rand() % NB_MAX];
			n = std::rand() % 10u;
			while (n-- > 0u) e1 = phi1(e1);

			Dart e2 = darts_[std::rand() % NB_MAX];
			n = std::rand() % 10u;
			while (n-- > 0u) e2 = phi1(e2);

			n = 1 + std::rand() % 3u;
			while (n-- > 0u && phi2(e1) == e1 && phi2(e2) == e2 && e2 != e1)
			{
				phi2_sew(e2, e1);
				e1 = phi1(e1);
				e2 = phi_1(e2);
			}
		}
		// Close de map
		MapBuilder mbuild(*this);
		mbuild.close_map();
	}
};

/*!
 * \brief The random generated maps used in the tests are sound.
 */
TEST_F(CMap2TopoTest, random_map_generators)
{
	EXPECT_EQ(nb_darts(), 0u);

	add_faces(NB_MAX);
	EXPECT_TRUE(check_map_integrity());

	add_closed_surfaces();
	EXPECT_TRUE(check_map_integrity());
}

/*!
 * \brief Sewing and unsewing darts correctly changes the topological relations.
 * The test performs NB_MAX sewing and unsewing on randomly chosen dart of darts_.
 * The map integrity is not preserved (this test creates fixed points for PHI2).
 */
TEST_F(CMap2TopoTest, phi2_sew_unsew)
{
	add_faces(NB_MAX);

	for (unsigned int i = 0u; i < NB_MAX; ++i)
	{
		Dart d0 = darts_[std::rand() % NB_MAX];
		Dart d2 = phi2(d0);
		phi2_unsew(d0);
		EXPECT_TRUE(phi2(d0) == d0);
		EXPECT_TRUE(phi2(d2) == d2);
		Dart e0 = d0;
		while (e0 == d0) e0 = darts_[std::rand() % NB_MAX];
		phi2_unsew(e0);

		phi2_sew(d0, e0);
		EXPECT_TRUE(phi2(d0) == e0);
		EXPECT_TRUE(phi2(e0) == d0);
	}
}

/*!
 * \brief Adding a 2-face of size n adds 2*n darts, n vertices and edges, 2 1-faces and 1 volume.
 * The test adds some faces and check that the number of generated cells is correct
 * and that the map integrity is preserved.
 */
TEST_F(CMap2TopoTest, add_face_topo)
{
	add_face_topo(1u);
	EXPECT_EQ(nb_darts(), 2u);
	EXPECT_EQ(nb_cells<Vertex::ORBIT>(), 1u);
	EXPECT_EQ(nb_cells<Edge::ORBIT>(), 1u);
	EXPECT_EQ(nb_cells<Face::ORBIT>(), 2u);
	EXPECT_EQ(nb_cells<Volume::ORBIT>(), 1u);

	add_face_topo(10u);
	EXPECT_EQ(nb_darts(), 22u);
	EXPECT_EQ(nb_cells<Vertex::ORBIT>(), 11u);
	EXPECT_EQ(nb_cells<Edge::ORBIT>(), 11u);
	EXPECT_EQ(nb_cells<Face::ORBIT>(), 4u);
	EXPECT_EQ(nb_cells<Volume::ORBIT>(), 2u);

	unsigned int count_vertices = 11u + add_faces(NB_MAX);

	EXPECT_EQ(nb_darts(), 2u * count_vertices);
	EXPECT_EQ(nb_cells<Vertex::ORBIT>(), count_vertices);
	EXPECT_EQ(nb_cells<Edge::ORBIT>(), count_vertices);
	EXPECT_EQ(nb_cells<Face::ORBIT>(), 2u * (NB_MAX + 2u));
	EXPECT_EQ(nb_cells<Volume::ORBIT>(), NB_MAX + 2u);
	EXPECT_TRUE(check_map_integrity());
}

/*! \brief Cutting an edge increases the size of both incident faces and add a vertex of degree 2.
 * The test performs NB_MAX edge cutting on edges of randomly generated faces.
 * The number of generated cells is correct and the map integrity is preserved.
 */
TEST_F(CMap2TopoTest, cut_edge_topo)
{
	add_closed_surfaces();

	unsigned int count_vertices = nb_cells<Vertex::ORBIT>();
	unsigned int count_edges = nb_cells<Edge::ORBIT>();
	unsigned int count_faces = nb_cells<Face::ORBIT>();
	unsigned int count_volumes = nb_cells<Volume::ORBIT>();

	for (Dart d : darts_)
	{
		unsigned int k1 = degree(Face(d));
		unsigned int k2 = degree(Face(phi2(d)));
		cut_edge_topo(d);
		if (same_cell(Face(d), Face(phi2(d))))
		{
			EXPECT_EQ(degree(Face(d)), k1 + 2u);
		}
		else
		{
			EXPECT_EQ(degree(Face(d)), k1 + 1u);
			EXPECT_EQ(degree(Face(phi2(d))), k2 + 1u);
		}
	}
	EXPECT_EQ(nb_cells<Vertex::ORBIT>(), count_vertices + NB_MAX);
	EXPECT_EQ(nb_cells<Edge::ORBIT>(), count_edges + NB_MAX);
	EXPECT_EQ(nb_cells<Face::ORBIT>(), count_faces);
	EXPECT_EQ(nb_cells<Volume::ORBIT>(), count_volumes);
	EXPECT_TRUE(check_map_integrity());
}

/*! \brief Cutting a face add an edge and replace a face of degree K,
 * with two subfaces whose degrees K1 and K2 verify K1+K2 = K+2.
 * The test performs NB_MAX face cuts between vertices of a randomly generated surface.
 * The number of generated cells is correct and the map integrity is preserved.
 */
TEST_F(CMap2TopoTest, cut_face_topo)
{
	add_closed_surfaces();

	unsigned int count_vertices = nb_cells<Vertex::ORBIT>();
	unsigned int count_edges = nb_cells<Edge::ORBIT>();
	unsigned int count_faces = nb_cells<Face::ORBIT>();
	unsigned int count_volumes = nb_cells<Volume::ORBIT>();

	for (Dart d : darts_)
	{
		unsigned int k = degree(Face(d));
		if (k > 1u)
		{
			Dart e = d; // find a second dart in the face of d (distinct from d)
			unsigned int i = std::rand() % 10u;
			while (i-- > 0u) e = phi1(e);
			if (e == d) e = phi1(e);

			cut_face_topo(d, e);
			++count_edges;
			++count_faces;
			EXPECT_EQ(degree(Face(d)) + degree(Face(e)), k + 2);
		}
	}
	EXPECT_EQ(nb_cells<Vertex::ORBIT>(), count_vertices);
	EXPECT_EQ(nb_cells<Edge::ORBIT>(), count_edges);
	EXPECT_EQ(nb_cells<Face::ORBIT>(), count_faces);
	EXPECT_EQ(nb_cells<Volume::ORBIT>(), count_volumes);
	EXPECT_TRUE(check_map_integrity());
}

/*! \brief Closing a map add one face per holes.
 * The test closes the holes of a randomly generated open surface.
 * The map integrity is preserved and the cell indexation is soundly completed
 */
TEST_F(CMap2TopoTest, close_map)
{
	add_closed_surfaces();

	// add attributes to initialize the indexation
	add_attribute<int, CDart::ORBIT>("darts");
	add_attribute<int, Vertex::ORBIT>("vertices");
	add_attribute<int, Edge::ORBIT>("edges");
	add_attribute<int, Face::ORBIT>("faces");
	add_attribute<int, Volume::ORBIT>("volumes");
	EXPECT_TRUE(check_map_integrity());

	// create some random holes (full removal or partial unsewing of faces)
	for (Dart d : darts_)
	{
		if (std::rand() % 2 == 1)
		{
			unsigned int n = std::rand() % 10u;
			unsigned int k = degree(Face(d));

			foreach_dart_of_orbit_until(Face(d), [&] (Dart e)
			{
				Dart e2 = phi2(e);
				phi2_unsew(e);
				// correct indexation of vertices
				if (!same_open_vertex(e2, phi1(e))) new_open_vertex_embedding(e2);
				if (!same_open_vertex(e, phi1(e2))) new_open_vertex_embedding(e);
				// correct indexation of edges
				new_orbit_embedding(Edge(e2));
				// correct indexation of volumes
				if (!same_volume(e2, e)) new_orbit_embedding(Volume(e));
				// interrupt the face unsewing after n steps
				if (n-- <= 0) return false;
				// control if a partial or full face unsewing has been done
				--k;
				return true;
			});
			// if the face is completely unsewn randomly removes it
			if (k == 0u && std::rand() % 2 == 1)
			{
				Dart e = d;
				Dart it = phi1(e);
				while (it != e)
				{
					Dart next = phi1(it);
					this->remove_dart(it);
					it = next;
				}
				this->remove_dart(e);
			}
		}
	}

	MapBuilder mbuild(*this);
	mbuild.close_map();
	EXPECT_TRUE(check_map_integrity());
}

TEST_F(CMap2TopoTest, degree)
{
	Face f(this->add_face_topo(10u));

	EXPECT_EQ(degree(f), 10u);
}

#undef NB_MAX

} // namespace cgogn
