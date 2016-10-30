#pragma once

#include "Directional.h"
#include "glm\glm.hpp"

class Shapes {
public:
	struct Verts {
		static glm::vec3 const triangle[ 2 ][ 3 ];
		static glm::vec3 const quad[ 4 ];
		static glm::vec3 const cube[ FaceDirection::FD_Size ][ 4 ];
	};
};

