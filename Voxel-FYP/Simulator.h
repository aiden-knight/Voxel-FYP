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
	float m_accumulator;

	static bool TestCollision(Particle& lhs, Particle& rhs);
	static std::pair<float, int> TestAABBs(const Particle& lhs, const Particle& rhs);
};

