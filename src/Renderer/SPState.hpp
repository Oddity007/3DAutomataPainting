#ifndef SPState_hpp
#define SPState_hpp

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

/*#include <CGAL/Epick_d.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/algorithm.h>
#include <CGAL/Timer.h>
#include <CGAL/assertions.h>

#include <Field3D/SparseField.h>*/

#include <vector>
#include <map>
#include <assert.h>
#include <algorithm>

#include "Renderer.hpp"
#include <iostream>

namespace glm
{

	static bool operator<(const glm::ivec3& a, const glm::ivec3& b)
	{
		return a.x < b.x or (a.x == b.x and (a.y < b.y or (a.y == b.y and a.z < b.z)));
	}

}

struct SPState
{
	/*struct SDF
	{
		virtual glm::vec4 sample(const glm::vec3& at) = 0;
		virtual ~SDF(void) = 0;
	};
	
	SDF* root;*/
	
	struct Material
	{
		enum class Type
		{
			None,
			Metal,
			Earth,
			Water,
		};
		
		Type type;
		
		struct Point
		{
			Type type;
			
		};
		
		struct Cell
		{
			union
			{
				struct
				{
					float rustFactor;
				}metal;
				
				struct
				{
					float erosionFactor;
				}earth;
				
				struct
				{
					
				}water;
				
			};
			
			Type type : 16;
			
			Cell(void) :
				type(Type::None)
			{
				
			}
			
			static Cell updated(const Material::Cell* neighbors);
			static void generateBlendWeights(const Material::Cell* neighbors, uint8_t* weights);
		};
	};
	
	/*struct MeshEditHistory
	{
		struct MaterialBrush
		{
			glm::quat rotation;
			glm::vec3 offset;
			float
				radius,
				probability;
		};
		
		struct MaterialVertex
		{
			glm::vec3 offset;
			uint32_t materialIndex;
			float parameters[4];
		};
		
		struct MaterialSimplex
		{
			size_t
				pointIndices[4];
		};
		
		//std::vector<MaterialBrush> brushes;
		//std::vector<MaterialBrush> offset;
		
		//std::vector<MaterialPoint> points;
		
		
	};
	
	struct Mesh
	{
		MeshEditHistory history;
	};*/
	
	//std::vector<Mesh> meshes;
	
	/*struct MaterialMesh
	{
	
		using Triangulation = CGAL::Delaunay_triangulation<CGAL::Epick_d<CGAL::Dimension_tag<3>>, CGAL::Triangulation_data_structure<CGAL::DelaunayTriangulationTraits::Dimension, CGAL::Triangulation_vertex<CGAL::DelaunayTriangulationTraits, Material::Point>, CGAL::Delaunay_full_cell<CGAL::DelaunayTriangulationTraits, Material::Cell>>>;
		Triangulation triangulation;
		
		MaterialMesh(void);
		~MaterialMesh(void);
		
		void insert(const Material::Point& data, glm::vec3 at);
	};
	
	MaterialMesh materialMesh;*/
	
	struct MaterialVolumeCollection;
	
	struct MaterialVolume
	{
		MaterialVolumeCollection* owner;
		glm::ivec3 address;
		Material::Cell* cells;
		static constexpr uint16_t resolution = 16;
		
		//glm::vec3 min, max;
		
		MaterialVolume(void) :
			owner(NULL),
			address(glm::ivec3(0))
		{
			cells = new Material::Cell[resolution * resolution * resolution];
			//min = max = glm::vec3(0);
		}
		
		~MaterialVolume(void)
		{
			delete[] cells;
		}
		
		
		
		Material::Cell* getCellPointer(int32_t x, int32_t y, int32_t z)
		{
			if (x < 0) return NULL;
			if (y < 0) return NULL;
			if (z < 0) return NULL;
			if (((uint16_t) x) >= resolution) return NULL;
			if (((uint16_t) y) >= resolution) return NULL;
			if (((uint16_t) z) >= resolution) return NULL;
			size_t index = 0;
			index += (uint32_t) z;
			index *= resolution;
			index += (uint32_t) y;
			index *= resolution;
			index += (uint32_t) x;
			return & cells[index];
		}
		
		/*Material::Cell getCell(glm::vec3 at)
		{
			at -= min;
			at /= max - min;
			Material::Cell* cell = getCellPointer(at.x, at.y, at.z);
			return cell ? *cell : Material::Cell();
		}
		
		void setCell(glm::vec3 at, Material::Cell to)
		{
			at -= min;
			at /= max - min;
			Material::Cell* cell = getCellPointer(at.x, at.y, at.z);
			if (not cell) return;
			*cell = to;
		}*/
		
		void update(MaterialVolume* destination);
		
		void generateBlendWeights(uint8_t* weights);
	};
	
	struct MaterialVolumeCollection
	{
		std::map<glm::ivec3, MaterialVolume> volumes;
		glm::vec3 center, volumeExtents;
		bool isDirty;
		
		MaterialVolumeCollection(void) :
			center(0),
			volumeExtents(1),
			isDirty(true)
		{}
		
		MaterialVolume* getVolume(glm::ivec3 address)
		{
			std::map<glm::ivec3, MaterialVolume>::iterator iterator = volumes.find(address);
			if (iterator == volumes.end())
				return NULL;
			return & volumes[address];
			//return & iterator->second;
		}
		
		MaterialVolume* createVolume(glm::ivec3 address)
		{
			assert(getVolume(address) == NULL);
			//volumes.insert(std::make_pair<glm::ivec3, MaterialVolume>(address, MaterialVolume()));
			//volumes[address] = MaterialVolume();
			MaterialVolume* volume = & volumes[address];//getVolume(address);
			volume->owner = this;
			volume->address = address;
			//std::cout << glm::to_string(address) << std::endl;
			return volume;
		}
		
		void setCell(glm::vec3 at, const Material::Cell& to)
		{
			//std::cout << glm::to_string(at) << std::endl;
			at -= center;
			at /= volumeExtents;
			glm::ivec3 address = glm::ivec3(glm::floor(at / float(16)));
			//MaterialVolume::resolution
			at -= glm::vec3(address * 16);
			assert(0 <= at.x);
			assert(0 <= at.y);
			assert(0 <= at.z);
			assert(at.x < 256);
			assert(at.y < 256);
			assert(at.z < 256);
			//std::cout << "address: " << glm::to_string(address) << " : " << glm::to_string(glm::ivec3(at)) << std::endl;
			MaterialVolume* volume = getVolume(address);
			if (not volume) volume = createVolume(address);
			Material::Cell* cell = volume ? volume->getCellPointer(at.x, at.y, at.z) : NULL;
			if (cell) * cell = to;
		}
		
		void update(void)
		{
			MaterialVolume scratch;
			for (auto iterator = volumes.begin(); iterator not_eq volumes.end(); iterator++)
			{
				iterator->second.update(& scratch);
				std::swap(iterator->second.cells, scratch.cells);
			}
		}
	};
	
	/*struct OctreeNode
	{
		OctreeNode* children;
		uint8_t* voxels;
		int16_t level;
		uint16_t resolution;
		
		OctreeNode(OctreeNode* parent)
		{
			
		}
	};*/
	
	//Field3D::SparseField<Material::Cell> field;
	/*static const uint16_t volumeCount = 2;
	MaterialVolume volumes[volumeCount];
	uint16_t currentVolumeIndex;*/
	MaterialVolumeCollection volumeCollection;
	double timeUntilNextUpdate;
	
	SPState(void);
	~SPState(void);
	
	void update(double seconds);
	void updateRenderer(GX::Renderer* renderer);
	
	struct BrushSettings
	{
		Material::Type cellType;
		float radius;
		uint16_t sampleCount;
		
		BrushSettings(void) :
			cellType(Material::Type::None),
			sampleCount(16),
			radius(1)
		{
			
		}
	};
	
	void paint(const glm::vec3& at, const BrushSettings& settings);
	void setPaintBoundary(glm::vec3 min, glm::vec3 max);
};

#endif
