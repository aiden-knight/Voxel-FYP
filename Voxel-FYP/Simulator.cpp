#include "Simulator.h"
#include "ImGuiConfig.h"
#include "Structures.h"

Simulator::Simulator(std::vector<Particle>& particles) :
	m_particleRef{particles}
{
}

Simulator::~Simulator()
{
}

void Simulator::Update(float deltaTime)
{
	ImGuiConfig* config = ImGuiConfig::GetInstance();
	if (!config->simulate) return;

	for (auto& particle : m_particleRef)
	{
		particle.position += particle.velocity * deltaTime * config->timeScale;
		particle.velocity.y += (-1 * deltaTime * config->timeScale);

		if (particle.position.y <= -2.0f)
		{
			particle.velocity.y *= -1;
			particle.position.y = -2.0f;
		}
	}
}
