#include "Renderer.hpp"
#include "Utility.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

using namespace GX;

Renderer::Renderer(void)
{
	cubeVertexBufferObject = 0;

	cl_int error = CL_SUCCESS;
	error = clGetPlatformIDs(1, & clplatform, NULL);
	CheckCLError(error);
	
#ifdef GLFW_EXPOSE_NATIVE_GLX
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetGLXContext(glfwGetCurrentContext()),
		CL_GLX_DISPLAY_KHR, (cl_context_properties) glfwGetX11Display(),
		CL_CONTEXT_PLATFORM, (cl_context_properties) clplatform,
		0
	};
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
	CGLContextObj cglcontext = CGLGetCurrentContext();
	CGLShareGroupObj sharegroup = CGLGetShareGroup(cglcontext);
	cl_context_properties properties[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties) sharegroup,
		0
	};
#else
	cl_context_properties* properties = NULL;
#endif

	//This is the correct way of getting a device
	//size_t numberOfSupportingDevices = 0;
	//error = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), & cldevice, & numberOfSupportingDevices);
	//assert(numberOfSupportingDevices > 0);

	//This is the way to get a device that actually links because I don't want to do it 100% correctly
	error = clGetDeviceIDs(clplatform, CL_DEVICE_TYPE_GPU, 1, & clgpudevice, NULL);
	CheckCLError(error);
	error = clGetDeviceIDs(clplatform, CL_DEVICE_TYPE_CPU, 1, & clcpudevice, NULL);
	CheckCLError(error);
	{
		cl_device_id deviceIDs[2] = {clcpudevice, clgpudevice};
		clcontext = clCreateContext(properties, 2, deviceIDs, NULL, NULL, & error);
	}
	CheckCLError(error);
	clgpuqueue = clCreateCommandQueue(clcontext, clgpudevice, 0, &error);
	CheckCLError(error);
	clcpuqueue = clCreateCommandQueue(clcontext, clcpudevice, 0, &error);
	CheckCLError(error);
	
	{
		size_t length = 0;
		clGetPlatformInfo(clplatform, CL_PLATFORM_EXTENSIONS, 0, NULL, & length);
		char* string = new char[length + 1];
		clGetPlatformInfo(clplatform, CL_PLATFORM_EXTENSIONS, length, string, NULL);
		printf("Platform: %s\n", string);
	}
	
	{
		size_t length = 0;
		clGetDeviceInfo(clgpudevice, CL_DEVICE_EXTENSIONS, 0, NULL, & length);
		char* string = new char[length + 1];
		clGetDeviceInfo(clgpudevice, CL_DEVICE_EXTENSIONS, length, string, NULL);
		printf("GPU Device: %s\n", string);
	}
	
	{
		size_t length = 0;
		clGetDeviceInfo(clcpudevice, CL_DEVICE_EXTENSIONS, 0, NULL, & length);
		char* string = new char[length + 1];
		clGetDeviceInfo(clcpudevice, CL_DEVICE_EXTENSIONS, length, string, NULL);
		printf("CPU Device: %s\n", string);
	}
	
	/*{
		currentAutomataStateImageIndex = 0;
		for (size_t i = 0; i < automataStateImageCount; i++)
		{
			automataStateImages[i] = NULL;
			cl_image_format format;
			cl_image_desc desc;
			
			format.image_channel_order = CL_A;
			format.image_channel_data_type = CL_UNSIGNED_INT16;
			
			desc.image_type = CL_MEM_OBJECT_IMAGE3D;
			desc.image_width = automataStateImageResolution;
			desc.image_height = automataStateImageResolution;
			desc.image_depth = automataStateImageResolution;
			desc.image_array_size = 1;
			desc.image_row_pitch = 0;
			desc.image_slice_pitch = 0;
			desc.num_mip_levels = 0;
			desc.num_samples = 0;
			desc.buffer = NULL;
			
			automataStateImages[i] = clCreateImage(clcontext, CL_MEM_READ_WRITE, & format, & desc, NULL, & error);
			CheckCLError(error);
		}
		
		#include "AutomataManagement_cl.h"
		
		automataProgram = CompileCLProgram(clcontext, clcpudevice, AutomataManagement_cl);
		initializeStateKernel = clCreateKernel(automataProgram, "InitializeState", & error);
		CheckCLError(error);
		updateStateKernel = clCreateKernel(automataProgram, "UpdateState", & error);
		CheckCLError(error);
		
		size_t dimensions[3] = {automataStateImageResolution, automataStateImageResolution, automataStateImageResolution};
		clEnqueueNDRangeKernel(clcpuqueue, initializeStateKernel, 3, );
	}*/
	
	glGenVertexArrays(1, & vertexArrayObject);
	
	/*{
		#include "StaticDefault_vs.h"
		#include "Default_fs.h"
		//staticDefaultProgram =
		programs[std::make_pair(vertexBuffer.type, material.presentTextures)] = CompileProgram(StaticDefault_vs, NULL, Default_fs);
	}
	
	{
		#include "SkinnedDefault_vs.h"
		#include "Default_fs.h"
		skinnedDefaultProgram = CompileProgram(SkinnedDefault_vs, NULL, Default_fs);
	}*/
	
	{
		#include "StaticDefault_vs.h"
		#include "SkinnedDefault_vs.h"
		#include "Default_fs.h"
		#include "DiffuseDefault_fs.h"
		#include "NormalDefault_fs.h"
		#include "DiffuseNormalDefault_fs.h"
		
		std::pair<VertexBuffer::Type, InternalMaterial::PresentTextureFlags> key;
		key = std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::None);
		programs[key] = CompileProgram(StaticDefault_vs, NULL, Default_fs);
		//programs.insert(std::make_pair(key, CompileProgram(StaticDefault_vs, NULL, Default_fs)));
		key = std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::None);
		programs[key] = CompileProgram(SkinnedDefault_vs, NULL, Default_fs);
		//programs.insert(std::make_pair(key, CompileProgram(SkinnedDefault_vs, NULL, Default_fs)));
		
		key = std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::Diffuse);
		programs[key] = CompileProgram(StaticDefault_vs, NULL, DiffuseDefault_fs);
		key = std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::Diffuse);
		programs[key] = CompileProgram(SkinnedDefault_vs, NULL, DiffuseDefault_fs);
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::Diffuse), CompileProgram(StaticDefault_vs, NULL, DiffuseDefault_fs)));
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::Diffuse), CompileProgram(SkinnedDefault_vs, NULL, DiffuseDefault_fs)));
		
			
		key = std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::Normal);
		programs[key] = CompileProgram(StaticDefault_vs, NULL, NormalDefault_fs);
		key = std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::Normal);
		programs[key] = CompileProgram(SkinnedDefault_vs, NULL, NormalDefault_fs);
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::Normal), CompileProgram(StaticDefault_vs, NULL, NormalDefault_fs)));
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::Normal), CompileProgram(SkinnedDefault_vs, NULL, NormalDefault_fs)));
		
		key = std::make_pair(VertexBuffer::Type::Static, (InternalMaterial::PresentTextureFlags) (InternalMaterial::PresentTextureFlags::Diffuse | InternalMaterial::PresentTextureFlags::Normal));
		programs[key] = CompileProgram(StaticDefault_vs, NULL, DiffuseNormalDefault_fs);
		key = std::make_pair(VertexBuffer::Type::Skinned, (InternalMaterial::PresentTextureFlags) (InternalMaterial::PresentTextureFlags::Diffuse | InternalMaterial::PresentTextureFlags::Normal));
		programs[key] = CompileProgram(SkinnedDefault_vs, NULL, DiffuseNormalDefault_fs);
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Static, InternalMaterial::PresentTextureFlags::Diffuse | InternalMaterial::PresentTextureFlags::Normal), CompileProgram(StaticDefault_vs, NULL, DiffuseNormalDefault_fs)));
		//programs.insert(std::make_pair(std::make_pair(VertexBuffer::Type::Skinned, InternalMaterial::PresentTextureFlags::Diffuse | InternalMaterial::PresentTextureFlags::Normal), CompileProgram(SkinnedDefault_vs, NULL, DiffuseNormalDefault_fs)));
		
		
		#include "Cursor_vs.h"
		#include "Cursor_fs.h"
		
		cursorProgram = CompileProgram(Cursor_vs, NULL, Cursor_fs);
	}
	
	jointBuffers[0] = std::move(JointBuffer());
	
	{
		//Ripped from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-4-a-colored-cube/
		//Because I'm so lazy I can't even describe a cube.
		static const GLfloat g_vertex_buffer_data[] =
		{
			-1.0f,-1.0f,-1.0f, // triangle 1 : begin
			-1.0f,-1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f, // triangle 1 : end
			1.0f, 1.0f,-1.0f, // triangle 2 : begin
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f, // triangle 2 : end
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f
		};
		
		glGenBuffers(1, & cubeVertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	{
		glGenTextures(1, & paintingData.blendTextureObject);
		glBindTexture(GL_TEXTURE_3D, paintingData.blendTextureObject);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, 256, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, NULL);
		
		void* temp = calloc(1, 256 * 256 * 256 * 4);
		
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, 256, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);
		
		free(temp);
		glBindTexture(GL_TEXTURE_3D, 0);
	}
	
	paintingData.min = glm::vec3(-128, -128, -128);
	paintingData.max = glm::vec3(128, 128, 128);
	
	//paintingData.min += glm::vec3(-16, 0, 0);
	//paintingData.max += glm::vec3(-16, 0, 0);
}

Renderer::~Renderer(void)
{
	glDeleteTextures(1, & paintingData.blendTextureObject);
	glDeleteBuffers(1, & cubeVertexBufferObject);
	glDeleteProgram(cursorProgram);

	CheckGLErrors();
	for (auto i = programs.begin(); i not_eq programs.end(); i++)
	{
		glDeleteProgram(i->second);
	}
	//glDeleteProgram(staticDefaultProgram);
	//glDeleteProgram(skinnedDefaultProgram);
	glDeleteVertexArrays(1, & vertexArrayObject);
	CheckGLErrors();
	
	cl_int error = CL_SUCCESS;
	error = clReleaseCommandQueue(clgpuqueue);
	CheckCLError(error);
	error = clReleaseCommandQueue(clcpuqueue);
	CheckCLError(error);
	error = clReleaseContext(clcontext);
	CheckCLError(error);
	CheckGLErrors();
}

void Renderer::setMainViewResolution(GLsizei width, GLsizei height)
{
	mainCamera.resolutionWidth = width;
	mainCamera.resolutionHeight = height;
}

void Renderer::setMainViewPosition(glm::vec3 to)
{
	mainCamera.position = to;
}

void Renderer::setMainViewRotation(glm::quat to)
{
	mainCamera.rotation = to;
}

void Renderer::setMainViewPerspective(GLfloat near, GLfloat far, GLfloat fov)
{
	mainCamera.near = near;
	mainCamera.far = far;
	mainCamera.fov = fov;
}

void Renderer::renderCursor(glm::vec3 at, float radius)
{
	//Requires render() have been called first
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	
	assert(vertexArrayObject > 0);
	glBindVertexArray(vertexArrayObject);
	
	GLuint program = cursorProgram;
	
	glUseProgram(program);
	
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBufferObject);
	
	glm::mat4 projectionMatrix = glm::perspective(mainCamera.fov, mainCamera.resolutionWidth / (GLfloat) mainCamera.resolutionHeight, mainCamera.near, mainCamera.far);
	glm::mat4 viewMatrix = glm::mat4_cast(glm::inverse(mainCamera.rotation)) * glm::translate(glm::mat4(1.0f), -mainCamera.position);
	BindUniform(program, "ProjectionMatrix_client", projectionMatrix);
	BindUniform(program, "ViewMatrix_client", viewMatrix);
	
	glm::mat4 rm = glm::mat4(1.0f);//glm::mat4_cast(glm::quat(1, 0, 0, 0));
	glm::mat4 tm = glm::translate(glm::mat4(1.0f), at);
	BindUniform(program, "ModelMatrix_client", tm * rm);
	
	BindUniform(program, "Center_client", at);
	BindUniform(program, "Radius_client", radius);
	
	BindVertexAttribute(program, "VertexPosition_client", 3, GL_FLOAT, false, 0, 0);
	CheckGLErrors();
	
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
	
	UnbindVertexAttribute(program, "VertexPosition_client");
			
	CheckGLErrors();
	
	glUseProgram(0);
}

void Renderer::render(void)
{
	if (deferedUpdatesMutex.try_lock())
	{
		std::vector<DeferedVolumeUpdate> deferedUpdatesCopy;
		std::swap(deferedUpdatesCopy, deferedUpdates);
		//updates.push_back(std::move(defered));
		deferedUpdatesMutex.unlock();
		
		for (size_t i = 0; i < deferedUpdatesCopy.size(); i++)
		{
			const DeferedVolumeUpdate& update = deferedUpdatesCopy[i];
			glBindTexture(GL_TEXTURE_3D, paintingData.blendTextureObject);
			CheckGLErrors();
			glTexSubImage3D(GL_TEXTURE_3D, 0, update.x, update.y, update.z, update.w, update.h, update.d, GL_RGBA, GL_UNSIGNED_BYTE, update.data);
			CheckGLErrors();
			glBindTexture(GL_TEXTURE_3D, 0);
			CheckGLErrors();
			delete[] update.data;
		}
	}

	CheckGLErrors();
	
	assert(vertexArrayObject > 0);
	glBindVertexArray(vertexArrayObject);
	
	glClearColor(0, 0, 0, 0);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mainCamera.resolutionWidth, mainCamera.resolutionHeight);
	glColorMask(true, true, true, true);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	CheckGLErrors();
	
	//for (auto clusterBufferIterator = clusterBuffers.cbegin(); clusterBufferIterator not_eq clusterBuffers.cend(); clusterBufferIterator++)
	{
		//assert(clusterBufferIterator->second.clusters.size() > 0);
		//for (auto clusterIterator = clusterBufferIterator->second.clusters.cbegin(); clusterIterator not_eq clusterBufferIterator->second.clusters.cend(); clusterIterator++)
		for (auto clusterIterator = staticMeshRendererClusters.cbegin(); clusterIterator not_eq staticMeshRendererClusters.cend(); clusterIterator++)
		{
			//const Cluster& cluster = * clusterIterator;
			const InternalStaticMeshRendererCluster& cluster = * clusterIterator;
			const InternalMaterial& material = materials[cluster.materialIndex];
			//const InstanceBuffer& instanceBuffer = instanceBuffers[cluster.instanceBufferIndex];
			const IndexBuffer& indexBuffer = indexBuffers[cluster.indexBufferIndex];
			const VertexBuffer& vertexBuffer = vertexBuffers[cluster.vertexBufferIndex];
			//const JointBuffer& jointBuffer = jointBuffers[cluster.jointBufferIndex];
			
			const std::vector<StaticMeshRendererInstance>& instances = cluster.instances;
			
			GLuint program = 0;
			size_t vertexSize = 0;
			
			//Program selection
			switch (vertexBuffer.type)
			{
				case VertexBuffer::Type::Static:
					//program = staticDefaultProgram;
					vertexSize = sizeof(StaticVertex);
					break;
				case VertexBuffer::Type::Skinned:
					//program = skinnedDefaultProgram;
					vertexSize = sizeof(SkinnedVertex);
					break;
				default:
					abort();
					break;
			}
			
			assert (programs.end() not_eq programs.find(std::make_pair(vertexBuffer.type, material.presentTextures)));
			program = programs[std::make_pair(vertexBuffer.type, material.presentTextures)];
			
			glUseProgram(program);
			CheckGLErrors();
			
			glm::mat4 projectionMatrix = glm::perspective(mainCamera.fov, mainCamera.resolutionWidth / (GLfloat) mainCamera.resolutionHeight, mainCamera.near, mainCamera.far);
			glm::mat4 viewMatrix = glm::mat4_cast(glm::inverse(mainCamera.rotation)) * glm::translate(glm::mat4(1.0f), -mainCamera.position);
			BindUniform(program, "ProjectionMatrix_client", projectionMatrix);
			BindUniform(program, "ViewMatrix_client", viewMatrix);
			
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.bufferObject);
			CheckGLErrors();
			
			BindVertexAttribute(program, "VertexPosition_client", 3, GL_FLOAT, false, vertexSize, offsetof(StaticVertex, position));
			BindVertexAttribute(program, "VertexTexcoord_client", 2, GL_FLOAT, false, vertexSize, offsetof(StaticVertex, texcoord));
			BindVertexAttribute(program, "VertexQTangent_client", 4, GL_FLOAT, false, vertexSize, offsetof(StaticVertex, qtangent));
			CheckGLErrors();
			
			/*if (vertexBuffer.type == VertexBuffer::Type::Skinned)
			{
				BindVertexAttribute(program, "VertexJointIndices_client", 4, GL_UNSIGNED_BYTE, false, vertexSize, offsetof(SkinnedVertex, jointIndices));
				BindVertexAttribute(program, "VertexJointWeights_client", 4, GL_UNSIGNED_BYTE, true, vertexSize, offsetof(SkinnedVertex, jointWeights));
			}*/
			
			/*{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_BUFFER, jointBuffer.textureObject);
				BindTextureUnitUniform(program, "Joints_client", 0);
			}*/
			
			if (material.presentTextures & InternalMaterial::PresentTextureFlags::Diffuse)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, images[material.diffuseTextureIndex].textureObject);
				BindTextureUnitUniform(program, "BaseColorMap_client", 0);
			}
			
			if (material.presentTextures & InternalMaterial::PresentTextureFlags::Normal)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, images[material.normalTextureIndex].textureObject);
				BindTextureUnitUniform(program, "NormalMap_client", 1);
			}
			
			{
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_3D, paintingData.blendTextureObject);
				BindTextureUnitUniform(program, "BlendMap_client", 2);
				BindUniform(program, "BlendMapMin_client", paintingData.min);
				BindUniform(program, "BlendMapMax_client", paintingData.max);
			}
			
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, images[paintingData.earthUpImageIndex].textureObject);
				BindTextureUnitUniform(program, "EarthUpMap_client", 3);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, images[paintingData.earthSideImageIndex].textureObject);
				BindTextureUnitUniform(program, "EarthSideMap_client", 4);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, images[paintingData.earthDownImageIndex].textureObject);
				BindTextureUnitUniform(program, "EarthDownMap_client", 5);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, images[paintingData.metalImageIndex].textureObject);
				BindTextureUnitUniform(program, "MetalMap_client", 6);
			}
			
			CheckGLErrors();
			
			for (size_t i = 0; i < instances.size(); i++)
			{
				const StaticMeshRendererInstance& instance = instances[i];
				
				{
					glm::mat4 rm = glm::mat4_cast(glm::make_quat(instance.rotation));
					glm::mat4 tm = glm::translate(glm::mat4(1.0f), glm::make_vec3(instance.position));
					BindUniform(program, "ModelMatrix_client", tm * rm);
				}
				
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.bufferObject);
				glDrawElements(GL_TRIANGLES, indexBuffer.indexCount, GL_UNSIGNED_SHORT, NULL);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			
			CheckGLErrors();
			
			
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			{
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_3D, 0);
			}
			
			if (material.presentTextures & InternalMaterial::PresentTextureFlags::Normal)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			if (material.presentTextures & InternalMaterial::PresentTextureFlags::Diffuse)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			/*{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_BUFFER, 0);
			}*/
			
			/*if (vertexBuffer.type == VertexBuffer::Type::Skinned)
			{
				UnbindVertexAttribute(program, "VertexJointIndices_client");
				UnbindVertexAttribute(program, "VertexJointWeights_client");
			}*/
			
			UnbindVertexAttribute(program, "VertexQTangent_client");
			UnbindVertexAttribute(program, "VertexTexcoord_client");
			UnbindVertexAttribute(program, "VertexPosition_client");
			
			CheckGLErrors();
		}
	}
	
	CheckGLErrors();
}

void Renderer::update(double seconds)
{
	
}

/*uint16_t Renderer::createVertexBuffer(size_t vertexAttributeCount, const VertexAttribute* vertexAttributes, size_t elementCount, const void* elements)
{
	assert (vertexBuffers.size() < UINT16_MAX);
	
	uint16_t bufferIndex = vertexBuffers.size();
	while (vertexBuffers.cend() not_eq vertexBuffers.find(bufferIndex))
		bufferIndex++;
	vertexBuffers[bufferIndex] = std::move(VertexBuffer());
	
	return bufferIndex;
}*/

void Renderer::destroyVertexBuffer(uint16_t bufferIndex)
{
	vertexBuffers.erase(bufferIndex);
}

uint16_t Renderer::createStaticVertexBuffer(size_t elementCount, const StaticVertex* elements)
{
	assert (vertexBuffers.size() < UINT16_MAX);
	
	uint16_t bufferIndex = vertexBuffers.size();
	while (vertexBuffers.cend() not_eq vertexBuffers.find(bufferIndex))
		bufferIndex++;
	vertexBuffers[bufferIndex] = std::move(VertexBuffer());
	
	vertexBuffers[bufferIndex].type = VertexBuffer::Type::Static;
	
	glGenBuffers(1, & vertexBuffers[bufferIndex].bufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[bufferIndex].bufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(StaticVertex) * elementCount, elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vertexBuffers[bufferIndex].vertexCount = elementCount;
	CheckGLErrors();
	
	return bufferIndex;
}

/*void Renderer::destroyStaticVertexBuffer(uint16_t bufferIndex)
{
	vertexBuffers.erase(bufferIndex);
}*/
	
uint16_t Renderer::createSkinnedVertexBuffer(size_t elementCount, const SkinnedVertex* elements)
{
	assert (vertexBuffers.size() < UINT16_MAX);
	
	uint16_t bufferIndex = vertexBuffers.size();
	while (vertexBuffers.cend() not_eq vertexBuffers.find(bufferIndex))
		bufferIndex++;
	vertexBuffers[bufferIndex] = std::move(VertexBuffer());
	
	vertexBuffers[bufferIndex].type = VertexBuffer::Type::Skinned;
	
	glGenBuffers(1, & vertexBuffers[bufferIndex].bufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[bufferIndex].bufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkinnedVertex) * elementCount, elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vertexBuffers[bufferIndex].vertexCount = elementCount;
	CheckGLErrors();
	
	return bufferIndex;
}

/*void Renderer::destroySkinnedVertexBuffer(uint16_t bufferIndex)
{
	vertexBuffers.erase(bufferIndex);
}*/
	
uint16_t Renderer::createIndexBuffer(size_t elementCount, const Index* elements)
{
	assert (indexBuffers.size() < UINT16_MAX);
	
	uint16_t bufferIndex = indexBuffers.size();
	while (indexBuffers.cend() not_eq indexBuffers.find(bufferIndex))
		bufferIndex++;
	indexBuffers[bufferIndex] = std::move(IndexBuffer());
	
	glGenBuffers(1, & indexBuffers[bufferIndex].bufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffers[bufferIndex].bufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * elementCount, elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	indexBuffers[bufferIndex].indexCount = elementCount;
	CheckGLErrors();
	
	return bufferIndex;
}

void Renderer::destroyIndexBuffer(uint16_t bufferIndex)
{
	indexBuffers.erase(bufferIndex);
}

uint16_t Renderer::createJointBuffer(size_t elementCount, const Joint* elements)
{
	assert (jointBuffers.size() < UINT16_MAX);
	
	if (elementCount == 0)
		return 0;
	
	uint16_t bufferIndex = jointBuffers.size();
	while (jointBuffers.cend() not_eq jointBuffers.find(bufferIndex))
		bufferIndex++;
	jointBuffers[bufferIndex] = std::move(JointBuffer());
	
	glGenBuffers(1, & jointBuffers[bufferIndex].bufferObject);
	glBindBuffer(GL_TEXTURE_BUFFER, jointBuffers[bufferIndex].bufferObject);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(Joint) * elementCount, elements, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	CheckGLErrors();
	glGenTextures(1, & jointBuffers[bufferIndex].textureObject);
	glBindTexture(GL_TEXTURE_BUFFER, jointBuffers[bufferIndex].textureObject);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, jointBuffers[bufferIndex].bufferObject);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	CheckGLErrors();
	
	return bufferIndex;
}

void Renderer::destroyJointBuffer(uint16_t bufferIndex)
{
	jointBuffers.erase(bufferIndex);
}

void Renderer::setJointBufferData(uint16_t bufferIndex, size_t elementStart, size_t elementCount, const Joint* elements)
{
	glBindBuffer(GL_TEXTURE_BUFFER, jointBuffers[bufferIndex].bufferObject);
	glBufferSubData(GL_TEXTURE_BUFFER, elementStart * sizeof(Joint), sizeof(Joint) * elementCount, elements);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	CheckGLErrors();
}

/*uint16_t Renderer::createInstanceBuffer(size_t elementCount, const Instance* elements)
{
	assert (instanceBuffers.size() < UINT16_MAX);
	
	uint16_t bufferIndex = instanceBuffers.size();
	while (instanceBuffers.cend() not_eq instanceBuffers.find(bufferIndex))
		bufferIndex++;
	instanceBuffers[bufferIndex] = std::move(InstanceBuffer());
	
	instanceBuffers[bufferIndex].instances.resize(elementCount);
	for (size_t i = 0; i < elementCount; i++)
		instanceBuffers[bufferIndex].instances[i] = elements[i];
	
	return bufferIndex;
}

void Renderer::destroyInstanceBuffer(uint16_t bufferIndex)
{
	instanceBuffers.erase(bufferIndex);
}

void Renderer::setInstanceBufferData(uint16_t bufferIndex, size_t elementStart, size_t elementCount, const Instance* elements)
{
	for (size_t i = 0; i < elementCount; i++)
		instanceBuffers[bufferIndex].instances[elementStart + i] = elements[i];
}*/

uint16_t Renderer::createStaticMeshRendererCluster(uint16_t vertexBufferIndex, uint16_t indexBufferIndex, uint16_t materialIndex, size_t instanceCount, const StaticMeshRendererInstance* instances)
{
	assert (clusters.size() < UINT16_MAX);
	
	uint16_t clusterIndex = clusters.size();
	while (clusters.cend() not_eq clusters.find(clusterIndex))
		clusterIndex++;
	
	InternalClusterReference reference;
	reference.type = InternalClusterReference::Type::Static;
	reference.index = staticMeshRendererClusters.size();
	clusters[clusterIndex] = reference;
	
	InternalStaticMeshRendererCluster cluster;
	cluster.vertexBufferIndex = vertexBufferIndex;
	cluster.indexBufferIndex = indexBufferIndex;
	cluster.materialIndex = materialIndex;
	cluster.instances.assign(instances, instances + instanceCount);
	
	staticMeshRendererClusters.push_back(std::move(cluster));
	
	return clusterIndex;
}

void Renderer::setStaticMeshRendererClusterInstanceData(uint16_t clusterIndex, size_t elementStart, size_t elementCount, const StaticMeshRendererInstance* elements)
{

}

/*uint16_t Renderer::createCluster(const Cluster& cluster)
{
	assert (clusters.size() < UINT16_MAX);
	
	uint16_t clusterIndex = clusters.size();
	while (clusters.cend() not_eq clusters.find(clusterIndex))
		clusterIndex++;
	
	clusters[clusterIndex] = cluster;
	
	return clusterIndex;
}*/

void Renderer::destroyCluster(uint16_t clusterIndex)
{
	const InternalClusterReference reference = clusters[clusterIndex];
	if (staticMeshRendererClusters.size() == 1)
	{
		clusters.erase(clusterIndex);
		return;
	}
	
	std::swap(staticMeshRendererClusters[reference.index], staticMeshRendererClusters.back());
	staticMeshRendererClusters.pop_back();
	
	for (auto i = clusters.begin(); i not_eq clusters.end(); i++)
	{
		if (i->second.type == InternalClusterReference::Type::Static and i->second.index == staticMeshRendererClusters.size())
		{
			i->second.index = reference.index;
			break;
		}
	}
	clusters.erase(clusterIndex);
}

uint16_t Renderer::createMaterial(size_t nodeCount, const MaterialNode* nodes)
{
	assert (materials.size() < UINT16_MAX);
	
	uint16_t materialIndex = materials.size();
	while (materials.cend() not_eq materials.find(materialIndex))
		materialIndex++;
	materials[materialIndex] = std::move(InternalMaterial());
	
	bool hasBaseColor = false;
	bool hasNormal = false;
	
	for (size_t i = 0; i < nodeCount; i++)
	{
		switch (nodes[i].type)
		{
			case MaterialNodeType::OutputBaseColor:
			{
				MaterialNode outputNode = nodes[i];
				MaterialNode samplerNode = nodes[outputNode.outputBaseColor.nodeIndex];
				assert (samplerNode.type == MaterialNodeType::SampleTexture);
				MaterialNode texcoordNode = nodes[samplerNode.sampleTexture.texcoordNodeIndex];
				assert (texcoordNode.type == MaterialNodeType::InputTexcoord);
				materials[materialIndex].diffuseTextureIndex = samplerNode.sampleTexture.imageIndex;
				hasBaseColor = true;
				break;
			}
			case MaterialNodeType::OutputTangentSpaceNormal:
			{
				MaterialNode outputNode = nodes[i];
				MaterialNode samplerNode = nodes[outputNode.outputTangentSpaceNormal.nodeIndex];
				assert (samplerNode.type == MaterialNodeType::SampleTexture);
				MaterialNode texcoordNode = nodes[samplerNode.sampleTexture.texcoordNodeIndex];
				assert (texcoordNode.type == MaterialNodeType::InputTexcoord);
				materials[materialIndex].normalTextureIndex = samplerNode.sampleTexture.imageIndex;
				hasNormal = true;
				break;
			}
			case MaterialNodeType::VolumeBlendedMaterial:
			{
				break;
			}
			default:
				break;
		}
	}
	
	InternalMaterial::PresentTextureFlags presentTextures = (InternalMaterial::PresentTextureFlags) 0;
	if (hasBaseColor)
		presentTextures = InternalMaterial::PresentTextureFlags(presentTextures | InternalMaterial::PresentTextureFlags::Diffuse);
	if (hasNormal)
		presentTextures = InternalMaterial::PresentTextureFlags(presentTextures | InternalMaterial::PresentTextureFlags::Normal);
	materials[materialIndex].presentTextures = presentTextures;
	
	//assert (hasBaseColor and hasNormal);
	
	return materialIndex;
}

void Renderer::destroyMaterial(uint16_t materialIndex)
{
	materials.erase(materialIndex);
}
	
uint16_t Renderer::createImage(ImageType imageType, size_t width, size_t height, const void* data)
{
	assert (images.size() < UINT16_MAX);
	
	uint16_t imageIndex = images.size();
	while (images.cend() not_eq images.find(imageIndex))
		imageIndex++;
	images[imageIndex] = std::move(Image());
	
	images[imageIndex].width = width;
	images[imageIndex].height = height;
	glGenTextures(1, & images[imageIndex].textureObject);
	glBindTexture(GL_TEXTURE_2D, images[imageIndex].textureObject);
	CheckGLErrors();
	images[imageIndex].type = imageType;
	switch (imageType)
	{
		case ImageType::R8G8B8A8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
	}
	
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLErrors();
	
	return imageIndex;
}

void Renderer::destroyImage(uint16_t imageIndex)
{
	images.erase(imageIndex);
}

glm::vec3 Renderer::unproject(GLsizei x, GLsizei y)
{
	x = glm::clamp(x, GLsizei(0), mainCamera.resolutionWidth);
	y = glm::clamp(y, GLsizei(0), mainCamera.resolutionHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	float distance = 0;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, & distance);
	glm::vec3 windowCoord(x, y, distance);
	//printf("raw: %f, %f, %f\n", windowCoord.x, windowCoord.y, windowCoord.z);
	return glm::unProject(windowCoord, mainCamera.calculateViewMatrix(), mainCamera.calculateProjectionMatrix(), glm::vec4(0, 0, mainCamera.resolutionWidth, mainCamera.resolutionHeight));
}

void Renderer::updateBlendVolumeRegion(GLsizei x, GLsizei y, GLsizei z, GLsizei w, GLsizei h, GLsizei d, const uint8_t* data)
{
	assert(x + w < 256u);
	assert(y + h < 256u);
	assert(z + d < 256u);
	/*glBindTexture(GL_TEXTURE_3D, paintingData.blendTextureObject);
	CheckGLErrors();
	glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, w, h, d, GL_RGBA, GL_UNSIGNED_BYTE, data);
	CheckGLErrors();
	glBindTexture(GL_TEXTURE_3D, 0);
	CheckGLErrors();*/
	
	DeferedVolumeUpdate update;
	update.x = x;
	update.y = y;
	update.z = z;
	update.w = w;
	update.h = h;
	update.d = d;
	
	size_t size = w * h * d * 4;
	update.data = (uint8_t*) memcpy(new uint8_t[size], data, size);
	
	deferedUpdatesMutex.lock();
	//deferedUpdates.push_back(std::move(defered));
	deferedUpdates.push_back(update);
	deferedUpdatesMutex.unlock();
}
