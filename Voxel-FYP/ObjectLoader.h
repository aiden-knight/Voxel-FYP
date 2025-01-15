#pragma once
#include "Mesh.h"
#include <string>

/// <summary>
/// For interfacing with tiny_obj_loader
/// </summary>
namespace ObjectLoader
{
	/// <summary>
	/// Loads mesh's vertices and indices from obj at given path
	/// </summary>
	Mesh LoadMesh(const std::string path);
}