#pragma once

#include "Globals.h"

#include "BiomeLoader.h"

class BaseBiome : public BiomeLoader {
public:
	BaseBiome( Client & client );
	~BaseBiome( );
};

