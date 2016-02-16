#ifndef Renderer_Renderer_hpp
#define Renderer_Renderer_hpp

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GL_GLEXT_PROTOTYPES
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include "CL/cl.h"
#include "CL/cl_gl.h"
#include "CL/cl_ext.h"
#endif

#include <vector>
#include <map>
#include <queue>
#include <memory>

#include "ViewFrustum.hpp"

#include <mutex>

namespace GX
{

struct Renderer
{
	enum class ImageType
	{
		R8G8B8A8,
	};
	
	struct StaticVertex
	{
		float
			position[3],
			unused,
			qtangent[4];
		float
			texcoord[2];
	};
	
	struct SkinnedVertex : StaticVertex
	{
		uint8_t
			jointIndices[4],
			jointWeights[4];
	};
	
	using Index = uint16_t;
	
	struct Joint
	{
		float
			position[3],
			unused,
			qtangent[4];
	};
	
	struct StaticMeshRendererInstance
	{
		float
			position[3],
			rotation[4];
	};
	
	struct SkinnedMeshRendererInstance
	{
		float
			position[3],
			rotation[4];
	};
	
	enum class MaterialNodeType : uint8_t
	{
		OutputBaseColor,
		OutputSmoothness,
		OutputMetalicity,
		OutputTangentSpaceNormal,
		
		InputTexcoord,
		//InputNormal,
		SampleTexture,
		InputWorldPosition,
		InputLocalPosition,
		
		VolumeBlendedMaterial,
		
		/*Add,
		Subtract,
		Multiply,
		Divide,
		Maximum,
		Minimum,
		
		ConstantFloat32,
		ConstantFloat32x2,
		ConstantFloat32x3,
		ConstantFloat32x4,*/
	};
	
	struct MaterialNode
	{
		MaterialNodeType type;
		
		union
		{
			struct
			{
				uint16_t nodeIndex;
			}outputBaseColor;
			struct
			{
				uint16_t nodeIndex;
			}outputSmoothness;
			struct
			{
				uint16_t nodeIndex;
			}outputMetalicity;
			struct
			{
				uint16_t nodeIndex;
			}outputTangentSpaceNormal;
			/*struct
			{
				
			}inputNormal;*/
			struct
			{
				
			}inputTexcoord;
			struct
			{
				uint16_t imageIndex;
				uint16_t texcoordNodeIndex;
			}sampleTexture;
			struct
			{
				
			}inputWorldPosition;
			struct
			{
				
			}inputLocalPosition;
			/*struct
			{
				uint16_t operands[2];
			}add;
			struct
			{
				uint16_t operands[2];
			}subtract;
			struct
			{
				uint16_t operands[2];
			}multiply;
			struct
			{
				uint16_t operands[2];
			}divide;
			struct
			{
				uint16_t operands[2];
			}maximum;
			struct
			{
				uint16_t operands[2];
			}minimum;
			struct
			{
				float values[1];
			}constantFloat32;
			struct
			{
				float values[2];
			}constantFloat32x2;
			struct
			{
				float values[3];
			}constantFloat32x3;
			struct
			{
				float values[4];
			}constantFloat32x4;*/
			struct
			{
				uint16_t
					diffuseImageIndices[4],
					normalImageIndices[4];
			}volumeBlendedMaterial;
		};
	};

	Renderer(void);
	~Renderer(void);

	void setMainViewResolution(GLsizei width, GLsizei height);
	void setMainViewPosition(glm::vec3 to);
	void setMainViewRotation(glm::quat to);
	void setMainViewPerspective(GLfloat near, GLfloat far, GLfloat fov);

	void renderCursor(glm::vec3 at, float radius);
	void render(void);
	void update(double seconds);
	
	uint16_t createStaticVertexBuffer(size_t elementCount, const StaticVertex* elements);
	uint16_t createSkinnedVertexBuffer(size_t elementCount, const SkinnedVertex* elements);
	void destroyVertexBuffer(uint16_t bufferIndex);
	
	uint16_t createIndexBuffer(size_t elementCount, const Index* elements);
	void destroyIndexBuffer(uint16_t bufferIndex);
	
	uint16_t createJointBuffer(size_t elementCount, const Joint* elements);
	void destroyJointBuffer(uint16_t bufferIndex);
	void setJointBufferData(uint16_t bufferIndex, size_t elementStart, size_t elementCount, const Joint* elements);
	
	uint16_t createStaticMeshRendererCluster(uint16_t vertexBufferIndex, uint16_t indexBufferIndex, uint16_t materialIndex, size_t instanceCount, const StaticMeshRendererInstance* instances);
	void setStaticMeshRendererClusterInstanceData(uint16_t clusterIndex, size_t elementStart, size_t elementCount, const StaticMeshRendererInstance* elements);
	
	uint16_t createSkinnedMeshRendererCluster(uint16_t vertexBufferIndex, uint16_t indexBufferIndex, uint16_t jointBufferIndex, uint16_t materialIndex, size_t instanceCount, const SkinnedMeshRendererInstance* instances);
	void setSkinnedMeshRendererClusterInstanceData(uint16_t clusterIndex, size_t elementStart, size_t elementCount, const SkinnedMeshRendererInstance* elements);
	
	void destroyCluster(uint16_t clusterIndex);
	
	uint16_t createMaterial(size_t nodeCount, const MaterialNode* nodes);
	void destroyMaterial(uint16_t materialIndex);
	
	uint16_t createImage(ImageType imageType, size_t width, size_t height, const void* data);
	void destroyImage(uint16_t imageIndex);
	
	glm::vec3 unproject(GLsizei x, GLsizei y);
	void updateBlendVolumeRegion(GLsizei x, GLsizei y, GLsizei z, GLsizei w, GLsizei h, GLsizei d, const uint8_t* data);
	
	//uint16_t createShader(size_t nodeCount, const ShaderNode* nodes);
	//void destroyShader(uint16_t shaderIndex);
	
	/*enum class ShaderNodeType : uint8_t
	{
		OutputBaseColor,
		OutputSmoothness,
		OutputMetalicity,
		OutputNormal,
				
		InputBasis,
		InputTexture,
		InputWorldPosition,
		InputLocalPosition,
		
		Add,
		Subtract,
		Multiply,
		Divide,
		Maximum,
		Minimum,
		
		ConstantFloat32,
		ConstantFloat32x3,
	};
	
	struct ShaderNode
	{
		ShaderNodeType type;
		
		union
		{
			struct
			{
				uint16_t nodeIndex;
			}outputBaseColor;
			struct
			{
				uint16_t nodeIndex;
			}outputSmoothness;
			struct
			{
				uint16_t nodeIndex;
			}outputSmoothness;
			struct
			{
				uint16_t nodeIndex;
			}outputNormal;
			struct
			{
				
			}inputBasis;
			struct
			{
				
			}inputTexture;
			struct
			{
				
			}inputWorldPosition;
			struct
			{
				
			}inputLocalPosition;
			struct
			{
				
			}add;
			struct
			{
				
			}subtract;
			struct
			{
			
			}multiply;
			struct
			{
			
			}divide;
			struct
			{
			
			}maximum;
			struct
			{
			
			}minimum;
			
			struct
			{
			
			}multiply;
		};
	};
	
	uint16_t createShader(size_t nodeCount, const ShaderNode* nodes);
	void destroyShader(uint16_t shaderIndex);*/
	
	/*struct Shader
	{
		struct Statement
		{
			enum class Type : uint8_t
			{
				OutputBaseColor,
				OutputSmoothness,
				OutputMetalicity,
				OutputNormal,
				
				InputBasis,
				InputTexture,
				InputWorldPosition,
				InputLocalPosition,
				
				Add,
				Subtract,
				Multiply,
				Divide,
				Maximum,
				Minimum,
				Clamp,
				Mix,
			};
			
			Type type;
			union
			{
				struct
				{
					
				}outputBaseColor;
				struct
				{
					
				}outputSmoothness;
				struct
				{
					
				}outputSmoothness;
				struct
				{
					
				}outputNormal;
			} data;
		};
	};*/
	
	/*struct VertexBuffer
	{
		GLuint bufferObject;
	};
	
	struct IndexBuffer
	{
		
	};

	struct Material
	{
		
	};
	
	struct Mesh
	{
		enum class Type
		{
			Static,
			Skinned,
		};
		
		Type type;
		uint16_t
			vertexBufferIndex,
			vertexStart,
			vertexCount,
			
			indexBufferIndex,
			indexStart,
			indexCount;
		GLuint
			indexBufferObject,
			vertexBufferObject;
		
		struct StaticVertex
		{
			glm::half
				position[3],
				unused,
				qtangent[4];
			uint16_t
				texcoord[2];
		};
		
		struct SkinnedVertex : StaticVertex
		{
			uint8_t
				jointIndices[4],
				jointWeights[4];
		};
	};
	
	struct Cluster
	{
		enum class Type
		{
			StaticMeshRenderer
		};
		
		struct StaticMeshRenderer
		{
			struct Instance
			{
				float
					position[3],
					unused,
					rotation[4];
			};
		
			uint16_t
				materialIndex,
				meshIndex,
				instanceCount;
			GLuint instanceBufferObject;
		};
		
		struct SkinnedMeshRenderer
		{
			struct Instance
			{
				float
					position[3],
					jointSetIndex,
					rotation[4];
			};
		
			uint16_t
				materialIndex,
				meshIndex,
				instanceCount;
			GLuint instanceBufferObject;
		};
		
		Type type;
	};*/
	
	struct Camera
	{
		glm::vec3 position;
		GLfloat
			near,
			far,
			fov;
		glm::quat rotation;
		GLsizei
			resolutionWidth,
			resolutionHeight;

		enum class Type
		{
			Main,
		};

		Type type;
		
		Camera(void) :
			position(0, 0, 0), 
			rotation(1, 0, 0, 0), 
			near(1), 
			far(10000), 
			fov(45), 
			resolutionWidth(256),
			resolutionHeight(256),
			type(Type::Main)
		{
			
		}

		glm::mat4 calculateViewMatrix(void) const
		{
			return 
				glm::mat4_cast(glm::inverse(rotation)) * 
				glm::translate(glm::mat4(1.0f), -position);
		}

		glm::mat4 calculateProjectionMatrix(void) const
		{
			float aspect = resolutionWidth / (GLfloat) resolutionHeight;
			return glm::perspective(fov, aspect, near, far);
		}

		ViewFrustum calculateViewFrustum(void) const
		{
			return ViewFrustum(position, rotation, fov, resolutionWidth / float(resolutionHeight), near, far);
		}
	};
	
	struct Image
	{
		uint16_t
			width,
			height;
		ImageType type;
		GLuint textureObject;
		
		Image(void)
		{
			type = ImageType::R8G8B8A8;
			textureObject = 0;
			width = 0;
			height = 0;
		}
		
		Image(const Image&) = delete;
		//Image(Image&&) = default;
		
		~Image(void)
		{
			glDeleteTextures(1, & textureObject);
		}
	};
	
	struct VertexBuffer
	{
		enum class Type
		{
			None,
			Static,
			Skinned,
		};
		
		GLuint
			bufferObject,
			vertexCount;
		Type type;
		
		VertexBuffer(void)
		{
			type = Type::None;
			bufferObject = 0;
			vertexCount = 0;
		}
		
		VertexBuffer(const VertexBuffer&) = delete;
		//VertexBuffer(VertexBuffer&&) = default;
		
		~VertexBuffer(void)
		{
			glDeleteBuffers(1, & bufferObject);
		}
	};
	
	struct IndexBuffer
	{
		GLuint
			bufferObject,
			indexCount;
			
		IndexBuffer(void)
		{
			bufferObject = 0;
			indexCount = 0;
		}
		
		IndexBuffer(const IndexBuffer&) = delete;
		//IndexBuffer(IndexBuffer&&) = default;
		
		~IndexBuffer(void)
		{
			glDeleteBuffers(1, & bufferObject);
		}
	};
	
	struct JointBuffer
	{
		GLuint
			bufferObject,
			textureObject;
		
		JointBuffer(void)
		{
			bufferObject = 0;
			textureObject = 0;
		}
		
		JointBuffer(const JointBuffer&) = delete;
		//JointBuffer(JointBuffer&&) = default;
		
		~JointBuffer(void)
		{
			glDeleteTextures(1, & textureObject);
			glDeleteBuffers(1, & bufferObject);
		}
	};
	
	struct InternalClusterReference
	{
		enum class Type
		{
			Static,
			Skinned
		};
		
		Type type;
		uint16_t index;
	};
	
	struct InternalStaticMeshRendererCluster
	{
		uint16_t
			vertexBufferIndex,
			indexBufferIndex,
			materialIndex;
		std::vector<StaticMeshRendererInstance> instances;
	};
	
	struct InternalMaterial
	{
		enum PresentTextureFlags : uint32_t
		{
			None = 0,
			Diffuse = 1 << 0,
			Normal = 1 << 1,
		};
		
		uint16_t
			diffuseTextureIndex,
			normalTextureIndex;
		
		PresentTextureFlags presentTextures;
		
		InternalMaterial(void)
		{
			presentTextures = PresentTextureFlags::None;
		}
	};
	
	std::map<uint16_t, VertexBuffer> vertexBuffers;
	std::map<uint16_t, IndexBuffer> indexBuffers;
	std::map<uint16_t, JointBuffer> jointBuffers;
	//std::map<uint16_t, ClusterBuffer> clusterBuffers;
	std::map<uint16_t, InternalClusterReference> clusters;
	std::map<uint16_t, Image> images;
	std::map<uint16_t, InternalMaterial> materials;
	
	std::map<std::pair<VertexBuffer::Type, InternalMaterial::PresentTextureFlags>, GLuint> programs;
	
	std::vector<InternalStaticMeshRendererCluster> staticMeshRendererClusters;
	
	GLuint
		vertexArrayObject,
		cubeVertexBufferObject,
		cursorProgram;
		//staticDefaultProgram,
		//skinnedDefaultProgram;
	
	cl_platform_id clplatform;
	cl_context clcontext;
	cl_command_queue clgpuqueue, clcpuqueue;
	cl_device_id clgpudevice, clcpudevice;
	
	
	Camera mainCamera;
	
	struct
	{
		/*size_t automataStateImageResolution = 256;
		static const size_t automataStateImageCount = 2;
		cl_mem automataStateImages[automataStateImageCount];
		
		cl_program automataProgram;
		cl_kernel
			initializeStateKernel,
			updateStateKernel;
		
		size_t currentAutomataStateImageIndex;*/
		GLuint blendTextureObject;
		glm::vec3 min, max;
		/*uint16_t
			diffuseImageIndices[8];
			normalImageIndices[8];*/
		uint16_t
			earthUpImageIndex,
			earthSideImageIndex,
			earthDownImageIndex,
			metalImageIndex;
	} paintingData;
	
	struct DeferedVolumeUpdate
	{
		GLsizei x, y, z, w, h, d;
		uint8_t* data;
		
		/*DeferedVolumeUpdate(GLsizei x, GLsizei y, GLsizei z, GLsizei w, GLsizei h, GLsizei d, const uint8_t* data) :
			x(x), y(y), z(z), w(w), h(h), d(d)
		{
			size_t size = w * h * d * 4;
			this->data = (uint8_t*) memcpy(new uint8_t[size], data, size);
		}*/
		
		/*~DeferedVolumeUpdate(void)
		{
			delete[] data;
		}
		
		DeferedVolumeUpdate(const DeferedVolumeUpdate& from)
		{
			assert(false);
		}
		
		DeferedVolumeUpdate(DeferedVolumeUpdate&& from)
		{
			x = from.x;
			y = from.y;
			z = from.z;
			w = from.w;
			h = from.h;
			d = from.d;
			data = from.data;
			from.data = NULL;
		}*/
	};
	
	std::mutex deferedUpdatesMutex;
	std::vector<DeferedVolumeUpdate> deferedUpdates;
};

}//namespace GX

#endif
