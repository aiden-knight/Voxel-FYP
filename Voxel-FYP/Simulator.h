#pragma once
#include <vector>

struct Particle;

class Simulator
{
public:
	Simulator(std::vector<Particle>& particles);
	~Simulator();

	void Update(float deltaTime);
private:
	std::vector<Particle>& m_particleRef;
};

