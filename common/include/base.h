#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <glm.hpp>
#include <common.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include <gtx/euler_angles.hpp>

#include <FreeImage.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <chrono>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <regex>

#include "project_constants.h"

using uchar = unsigned char;
using uint = unsigned int;