#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

struct ProfileInfo
{
	int dataCount = 0;
	size_t voxelCount = 0;
	int resolution = 0;
	bool wasSimulated = false;
	std::string model = "";
};

struct ImGuiConfig
{
	bool profile = false;
	ProfileInfo profileInfo{};

	bool resetSimulator = false;
	bool simulate = false;
	bool explode = false;
	float explosionRange = 1.0f;
	float explosionForce = 1.0f;
	float breakingPoint = 5.0f;
	float timeScale = 1.0f;

	glm::vec3 cameraPos = glm::vec3(0.0f, 3.5f, 7.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

	std::string modelString = "teapot";
	float modelHalfExtent = 2;
	int modelResolution = 32;

	static ImGuiConfig* GetInstance();
private:
	static ImGuiConfig* s_Instance;

	ImGuiConfig() = default;
};