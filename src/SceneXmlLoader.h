#pragma once

#include <memory>
#include <string>

#include "Scene.h"

// Loads FortuneRenderer Chapter 08 scenes and legacy attribute-based scenes.
// Throws std::runtime_error when the file or scene data is invalid.
std::unique_ptr<Scene> LoadSceneFromXml(const std::string& path, int width, int height);

// Uses MYRENDER_SCENE, or scenes/scene08.xml when it is unset.
// Reports load errors and exits because main() does not handle them.
Scene* LoadSceneFromEnvironment(int width, int height);
