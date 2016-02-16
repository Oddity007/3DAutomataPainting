#include <string>
#include "../Renderer/Renderer.hpp"
#include "../Renderer/Stage.hpp"
#include "../Core/G2AssimpScene.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include <iostream>
#include <sstream>

#include <map>
#include <string>

#include <mutex>
#include <thread>

static uint16_t LoadImage(GX::Renderer* renderer, const char* path)
{
	int width, height, components;
	unsigned char* pixels = stbi_load(path, & width, & height, & components, 4);
	//assert(components == 3);
	assert(pixels);
	uint16_t imageIndex = renderer->createImage(GX::Renderer::ImageType::R8G8B8A8, width, height, pixels);
	stbi_image_free(pixels);
	
	return imageIndex;
}

static std::vector<glm::vec3> Points;
static std::mutex PointsMutex;

static void LoadScene(GX::Renderer* renderer, const char* path, const std::map<std::string, uint16_t>& materialIndices)
{
	G2AssimpScene* assimpScene = G2AssimpSceneNewFromFile(path);
	
	struct Mesh
	{
		uint16_t
			vertexBufferIndex,
			indexBufferIndex,
			jointBufferIndex;
	};
	
	std::vector<Mesh> meshes;
	
	for (size_t meshIndex = 0; meshIndex < assimpScene->meshCount; meshIndex++)
	{
		const G2AssimpSceneSkinnedMesh* mesh = assimpScene->meshes + meshIndex;
		
		Mesh outmesh;
		
		size_t vertexCount = mesh->vertexCount;
		auto vertices = new GX::Renderer::StaticVertex[vertexCount];
		for (size_t i = 0; i < vertexCount; i++)
		{
			for (int j = 0; j < 3; j++)
				vertices[i].position[j] = mesh->vertices[i].position[j];
			for (int j = 0; j < 4; j++)
				vertices[i].qtangent[j] = mesh->vertices[i].qtangent[j];
			for (int j = 0; j < 2; j++)
				vertices[i].texcoord[j] = mesh->vertices[i].texcoord[j];
		}
		outmesh.vertexBufferIndex = renderer->createStaticVertexBuffer(vertexCount, vertices);
		delete[] vertices;
	
		size_t indexCount = mesh->indexCount;
		auto* indices = new uint16_t[indexCount];
		for (size_t i = 0; i < indexCount; i++)
		{
			assert(mesh->indices[i] <= UINT16_MAX);
			indices[i] = mesh->indices[i];
		}
		outmesh.indexBufferIndex = renderer->createIndexBuffer(indexCount, indices);
		delete[] indices;

		outmesh.jointBufferIndex = renderer->createJointBuffer(0, NULL);

		meshes.push_back(outmesh);
	}
	assert(assimpScene->meshCount == meshes.size());

	//std::map<std::pair<size_t, std::string>, std::vector<GX::Renderer::Instance>> clusterInstances;
	std::map<std::pair<size_t, std::string>, std::vector<GX::Renderer::StaticMeshRendererInstance>> clusterInstances;

	for (size_t entityIndex = 0; entityIndex < assimpScene->entityCount; entityIndex++)
	{
		const G2AssimpSceneEntity* entity = assimpScene->entities + entityIndex;
		if (not entity->hasMesh) continue;
		assert(meshes.size() > entity->mesh);
		
		//GX::Renderer::Instance instance;
		GX::Renderer::StaticMeshRendererInstance instance;
		for (int i = 0 ; i < 3; i++) instance.position[i] = entity->position[i];
		for (int i = 0 ; i < 4; i++) instance.rotation[i] = entity->rotation[i];
		
		std::string materialName(entity->materialName);
		std::pair<size_t, std::string> key(entity->mesh, std::move(materialName));
		clusterInstances[std::move(key)].push_back(instance);
	}
	
	G2AssimpSceneDelete(assimpScene);
	
	for (auto i = clusterInstances.cbegin(); i not_eq clusterInstances.cend(); i++)
	{
		/*GX::Renderer::Cluster cluster;
		
		cluster.vertexBufferIndex = meshes[i->first.first].vertexBufferIndex;
		cluster.indexBufferIndex = meshes[i->first.first].indexBufferIndex;
		cluster.jointBufferIndex = meshes[i->first.first].jointBufferIndex;
		cluster.materialIndex = renderer->createMaterial(0, NULL);
		cluster.instanceBufferIndex = renderer->createInstanceBuffer(i->second.size(), & i->second[0]);*/
		
		std::cerr << "Using material " << i->first.second << std::endl;
		uint16_t materialIndex = materialIndices.at(i->first.second);
		assert (materialIndices.find(i->first.second) not_eq materialIndices.cend());
		
		renderer->createStaticMeshRendererCluster(meshes[i->first.first].vertexBufferIndex, meshes[i->first.first].indexBufferIndex, materialIndex, i->second.size(), & i->second[0]);
		//renderer->createCluster(cluster);
		//renderer->createClusterBuffer(1, & cluster);
	}
}


static void OnFramebufferResize(GLFWwindow* window, int width, int height)
{
	if (width <= 0 or height <= 0)
		return;
	GX::Renderer* renderer = (GX::Renderer*) glfwGetWindowUserPointer(window);
	//renderer->setMainViewResolution(width, height);
}

static void OnKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*if (key == GLFW_KEY_F)
	{
		Renderer* renderer = (Renderer*) glfwGetWindowUserPointer(window);
		renderer->saveScreenshot();
	}*/
}


int main(int argc, char** argv)
{
	if (argc > 1)
	{
		//std::cerr << "Error: Missing argument string (must provide a scene file)." << std::endl;
		return EXIT_FAILURE;
	}

	if(not glfwInit())
	{
		std::cerr << "Failed to initialize glfw" << std::endl;
		return EXIT_FAILURE;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SRGB_CAPABLE, true);

	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	int width = 800;
	int height = 500;
	
	
	//int width = 1440;
	//int height = 900;
	
	GLFWwindow* window = glfwCreateWindow(width, height, "3D Automata Painting", NULL, NULL);
	//glfwGetPrimaryMonitor()
	//glfwGetPrimaryMonitor()
	
	if(not window)
	{
		glfwTerminate();
		std::cerr << "Failed to create window" << std::endl;
		return 1;
	}
	
	glfwGetFramebufferSize(window, & width, & height);
	std::cout << "width: " << width << ", height: " << height << std::endl;
	
	//glfwSetWindowSize(window, 800, 500);

	glfwMakeContextCurrent(window);
	glViewport(0,0, width, height);

	//glfwSwapInterval(1);

	GX::Stage* stage = new GX::Stage("data");
	stage->acquireBundle(0);
	
	glfwSetWindowUserPointer(window, & stage->renderer);
	glfwSetKeyCallback(window, OnKeyPress);

	stage->renderer.setMainViewResolution(width, height);
	stage->renderer.setMainViewPerspective(1, 100, 45);
	stage->painter.setPaintBoundary(stage->renderer.paintingData.min, stage->renderer.paintingData.max);

	glfwSetFramebufferSizeCallback(window, OnFramebufferResize);
	
	glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
	glm::quat cameraRotation = glm::quat(1, 0, 0, 0);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPos(window, 0, 0);
	
	double then = glfwGetTime();
	
	glfwSwapInterval(0);
	
	float frameCount = 0;
	float timeCount = 0;

	std::thread workerThread
	(
		[&]()
		{
			double then = glfwGetTime();
			while (not glfwWindowShouldClose(window))
			{
				double now = glfwGetTime();
				double secondsElapsed = now - then;
				then = now;
				
				
				stage->painter.update(secondsElapsed);
				stage->painter.updateRenderer(& stage->renderer);
				
				PointsMutex.lock();
				for (size_t i = 0; i < Points.size(); i++)
				{
					SPState::BrushSettings brushSettings;
					brushSettings.cellType = glm::linearRand(0, 1) ? SPState::Material::Type::Earth : SPState::Material::Type::Metal;
					stage->painter.paint(Points[i], brushSettings);
				}
				Points.clear();
				PointsMutex.unlock();
			}
		}
	);

	while(not glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, & width, & height);
		//width = 800;
		//height = 500;
		stage->renderer.setMainViewResolution(width, height);
		std::cout << "width: " << width << ", height: " << height << std::endl;
		
		double now = glfwGetTime();
		double secondsElapsed = now - then;
		then = now;
		
		if (timeCount > 0.25)
		{
			float spf = timeCount / frameCount;
			float fps = frameCount / timeCount;
			std::stringstream titleStream;
			titleStream << "3D Automata Painting (FPS: " << (1 / secondsElapsed) << ", SPF: " << secondsElapsed << ")";
			//glfwSetWindowTitle(window, titleStream.str().c_str());
			
			timeCount = 0;
			frameCount = 0;
		}
		
		frameCount += 1;
		timeCount += secondsElapsed;

		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			//glfwSetCursorPos(window, 0, 0);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			//glfwSetCursorPos(window, 0, 0);
		}

		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			double x, y;
			glfwGetCursorPos(window, & x, & y);
			float angleX = -glm::clamp(float(y * 0.1), -90.0f, 90.0f);
			float angleY = -x * 0.1;
			cameraRotation =
				glm::angleAxis(glm::radians(angleY), glm::vec3(0, 1, 0)) *
				glm::angleAxis(glm::radians(angleX), glm::vec3(1, 0, 0)) *
				glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 0, 1));
			cameraRotation = glm::normalize(cameraRotation);
		}
		
		{
			float dz = glfwGetKey(window, GLFW_KEY_S) - glfwGetKey(window, GLFW_KEY_W);
			float dx = glfwGetKey(window, GLFW_KEY_A) - glfwGetKey(window, GLFW_KEY_D);
			glm::vec3 delta = glm::vec3(-dx, 0, dz);
			cameraPosition += cameraRotation * (delta * float(secondsElapsed * 10));
		}
		
		stage->renderer.setMainViewPosition(cameraPosition);
		stage->renderer.setMainViewRotation(cameraRotation);

		stage->renderer.update(secondsElapsed);
		//stage->painter.update(secondsElapsed);
		//stage->painter.updateRenderer(& stage->renderer);
		stage->renderer.render();
		
		//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
		{
			double x, y;
			glfwGetCursorPos(window, & x, & y);
			glm::vec3 point = stage->renderer.unproject(x, height - y);
			//printf("%f, %f, %f\n", point.x, point.y, point.z);
			stage->renderer.renderCursor(point, 1);
			//SPState::BrushSettings brushSettings;
			//brushSettings.cellType = glm::linearRand(0, 1) ? SPState::Material::Type::Earth : SPState::Material::Type::Metal;
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			{
				PointsMutex.lock();
				Points.push_back(point);
				//stage->painter.paint(point, brushSettings);
				PointsMutex.unlock();
			}
		}

		//glFlush();
		//glFinish();

		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) break;
	}
	
	workerThread.join();
	
	//delete renderer;
	delete stage;

	glfwTerminate();
	return 0;
}

