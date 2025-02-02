#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

struct ImGuiConfig
{
	bool simulate = false;
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