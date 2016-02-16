#include "SPState.hpp"
#include "glm/gtc/random.hpp"

/*SPState::MaterialMesh::MaterialMesh(void) :
	triangulation(3)
{
	
}

SPState::MaterialMesh::~MaterialMesh(void)
{
	
}

void SPState::MaterialMesh::insert(const Material::Point& data, glm::vec3 at)
{
	Triangulation::Point point(at.x, at.y, at.z);
	Triangulation::Vertex_handle v = triangulation.insert(point);
	v->data() = data;
}*/

void SPState::Material::Cell::generateBlendWeights(const SPState::Material::Cell* neighbors, uint8_t* weights)
{
	SPState::Material::Cell in = neighbors[9 * 1 + 3 * 1 + 1];
	auto neighborAt = [&](int x, int y, int z){return neighbors[(z + 1) * 9 + (y + 1) * 3 + (x + 1)];};
	//glm::vec4 v = glm::linearRand(glm::vec4(0), glm::vec4(255));
	//weights[0] = v.x;
	//weights[1] = v.y;
	//weights[2] = v.z;
	//weights[3] = v.w;
	if (in.type == Material::Type::Water)
	{
		weights[0] = 0;
		weights[1] = 0;
		weights[2] = 255;
	}
	else if (in.type == Material::Type::Earth)
	{
		weights[0] = 0;
		weights[1] = 255;
		weights[2] = 0;
	}
	else if (in.type == Material::Type::Metal)
	{
		weights[0] = 255;
		weights[1] = 0;
		weights[2] = 0;
	}
	else if (in.type == Material::Type::None)
	{
		weights[0] = 0;
		weights[1] = 0;
		weights[2] = 0;
	}
	weights[3] = 255;
}

SPState::Material::Cell SPState::Material::Cell::updated(const SPState::Material::Cell* neighbors)
{
	SPState::Material::Cell in = neighbors[9 * 1 + 3 * 1 + 1];
	SPState::Material::Cell out = in;
	auto neighborAt = [&](int x, int y, int z){return neighbors[(z + 1) * 9 + (y + 1) * 3 + (x + 1)];};
	switch (in.type)
	{
		case Type::None:
		{
			/*uint16_t seenCount = 0;
			for (int z = -1; z < 2; z++)
			for (int y = -1; y < 2; y++)
			for (int x = -1; x < 2; x++)
			{
				if (neighborAt(x, y, z).type not_eq Type::None)
				{
					seenCount++;
					if (seenCount == glm::linearRand(uint16_t(0u), seenCount))
						out = neighborAt(x, y, z);
				}
			}
			if (neighborAt(0, 1, 0).type == Type::Earth)
			{
				out.type = Type::Earth;
				out.earth.erosionFactor = 0;
			}*/
			if (glm::linearRand(0, 12) > 1) break;
			glm::ivec3 offset = glm::linearRand(glm::ivec3(-1), glm::ivec3(1));
			out = neighborAt(offset.x, offset.y, offset.z);
			break;
		}
		case Type::Metal:
		{
			if (glm::linearRand(0, 27) > 1) break;
			glm::ivec3 offset = glm::linearRand(glm::ivec3(-1), glm::ivec3(1));
			out = neighborAt(offset.x, offset.y, offset.z);
			break;
		}
		case Type::Earth:
		{
			if (glm::linearRand(0, 27 * 3 * 3) > 1) break;
			glm::ivec3 offset = glm::linearRand(glm::ivec3(-1), glm::ivec3(1));
			out = neighborAt(offset.x, offset.y, offset.z);
			break;
		}
		case Type::Water:
			//if (neighborAt(0, -1, 0).type == Type::None)
			//	out.type = Type::None;
			break;
	}
	return out;
}

void SPState::MaterialVolume::update(MaterialVolume* destination)
{
	//Material::Cell* newCells = new Material::Cell[resolution * resolution * resolution];
	
	for (int32_t z = 0; z < resolution; z++)
	for (int32_t y = 0; y < resolution; y++)
	for (int32_t x = 0; x < resolution; x++)
	{
		Material::Cell neighbors[3 * 3 * 3];
		for (int32_t w = -1; w < 2; w++)
		for (int32_t v = -1; v < 2; v++)
		for (int32_t u = -1; u < 2; u++)
		{
			glm::ivec3 addressOffset
			(
				(x + 1 == resolution and u == 1) - (x == 0 and u == -1),
				(y + 1 == resolution and v == 1) - (y == 0 and v == -1),
				(z + 1 == resolution and w == 1) - (z == 0 and w == -1)
			);
			
			Material::Cell* neighbor = NULL;
		
			if (owner and (addressOffset not_eq glm::ivec3(0)))
			{
				int32_t
					a = addressOffset.x ? x : (addressOffset.x > 0 ? 0 : resolution - 1),
					b = addressOffset.y ? y : (addressOffset.y > 0 ? 0 : resolution - 1),
					c = addressOffset.z ? z : (addressOffset.z > 0 ? 0 : resolution - 1);
				MaterialVolume* other = owner->getVolume(addressOffset + address);
				if (other)
					neighbor = other->getCellPointer(a, b, c);
			}
			else
				neighbor = getCellPointer(x + u, y + v, z + w);
			if (neighbor)
			{
				uint32_t index = uint32_t(w + 1) * 9u + uint32_t(v + 1) * 3u + uint32_t(u + 1);
				assert(index < 3 * 3 * 3);
				neighbors[index] = *neighbor;
			}
		}
		
		//SPState::Material::Cell
		//newCells[z * resolution * resolution + y * resolution + x] =
		* destination->getCellPointer(x, y, z) = Material::Cell::updated(neighbors);
	}
	
	//delete[] cells;
	//cells = newCells;
}


void SPState::MaterialVolume::generateBlendWeights(uint8_t* weights)
{
	for (int32_t z = 0; z < resolution; z++)
	for (int32_t y = 0; y < resolution; y++)
	for (int32_t x = 0; x < resolution; x++)
	{
		Material::Cell neighbors[3 * 3 * 3];
		for (int32_t w = -1; w < 2; w++)
		for (int32_t v = -1; v < 2; v++)
		for (int32_t u = -1; u < 2; u++)
		{
			glm::ivec3 addressOffset
			(
				(x + 1 == resolution and u == 1) - (x == 0 and u == -1),
				(y + 1 == resolution and v == 1) - (y == 0 and v == -1),
				(z + 1 == resolution and w == 1) - (z == 0 and w == -1)
			);
			
			Material::Cell* neighbor = NULL;
		
			if (owner and (addressOffset not_eq glm::ivec3(0)))
			{
				int32_t
					a = addressOffset.x ? x : (addressOffset.x > 0 ? 0 : resolution - 1),
					b = addressOffset.y ? y : (addressOffset.y > 0 ? 0 : resolution - 1),
					c = addressOffset.z ? z : (addressOffset.z > 0 ? 0 : resolution - 1);
				MaterialVolume* other = owner->getVolume(addressOffset + address);
				if (other)
					neighbor = other->getCellPointer(a, b, c);
			}
			else
				neighbor = getCellPointer(x + u, y + v, z + w);
			if (neighbor)
			{
				uint32_t index = uint32_t(w + 1) * 9u + uint32_t(v + 1) * 3u + uint32_t(u + 1);
				assert(index < 3 * 3 * 3);
				neighbors[index] = *neighbor;
			}
		}
		
		size_t index = 0;
		index += (uint16_t) z;
		index *= resolution;
		index += (uint16_t) y;
		index *= resolution;
		index += (uint16_t) x;
		index *= 4;
		//weights[index + 0] = 255;
		//weights[index + 1] = 0;
		//weights[index + 2] = 0;
		//weights[index + 3] = 255;
		SPState::Material::Cell::generateBlendWeights(neighbors, weights + index);
	}
}

SPState::SPState(void) :
//	currentVolumeIndex(0),
	timeUntilNextUpdate(0)
{
	//field.clear();
	volumeCollection.createVolume(glm::ivec3(0));
}

SPState::~SPState(void)
{
	
}

void SPState::update(double seconds)
{
	timeUntilNextUpdate -= seconds;
	if (timeUntilNextUpdate < 0)
	{
		volumeCollection.isDirty = true;
		timeUntilNextUpdate = 0.125;
		/*uint16_t sourceIndex = currentVolumeIndex;
		currentVolumeIndex++;
		currentVolumeIndex %= volumeCount;
		volumes[currentVolumeIndex].update(volumes + currentVolumeIndex);*/
		volumeCollection.update();
	}
}

void SPState::paint(const glm::vec3& at, const BrushSettings& settings)
{
	for (uint16_t i = 0; i < settings.sampleCount; i++)
	{
		Material::Cell cell;
		cell.type = settings.cellType;
		switch (cell.type)
		{
			case Material::Type::None:
				break;
			case Material::Type::Metal:
				cell.metal.rustFactor = 0;
				break;
			case Material::Type::Earth:
				cell.earth.erosionFactor = 0;
				break;
			case Material::Type::Water:
				break;
		}
		volumeCollection.setCell(at + glm::gaussRand(0.0f, settings.radius), cell);
		//volumes[currentVolumeIndex].setCell(at + glm::gaussRand(0.0f, settings.radius), cell);
	}
	
	//glm::vec3 percentage = (at - min) / (max - min);
	//field.lvalue();
}

void SPState::setPaintBoundary(glm::vec3 min, glm::vec3 max)
{
	volumeCollection.center = (max - min) * 0.5f + min;
	volumeCollection.volumeExtents = (max - min) / 256.0f;
	/*for (uint16_t i = 0; i < volumeCount; i++)
	{
		volumes[i].min = min;
		volumes[i].max = max;
	}*/
}

void SPState::updateRenderer(GX::Renderer* renderer)
{
	if (not volumeCollection.isDirty) return;
	volumeCollection.isDirty = false;
	uint8_t weights[MaterialVolume::resolution * MaterialVolume::resolution * MaterialVolume::resolution * 4];
	for (auto iterator = volumeCollection.volumes.begin(); iterator not_eq volumeCollection.volumes.end(); iterator++)
	{
		glm::ivec3 start = glm::ivec3(MaterialVolume::resolution) * iterator->second.address + glm::ivec3(128);
		iterator->second.generateBlendWeights(weights);
		//std::cout << glm::to_string(start) << std::endl;
		if
		(
			0 <= start.x and start.x < 256 and
			0 <= start.y and start.y < 256 and
			0 <= start.z and start.z < 256
		)
		{
			renderer->updateBlendVolumeRegion(start.x, start.y, start.z, MaterialVolume::resolution, MaterialVolume::resolution, MaterialVolume::resolution, weights);
		}
	}
}
