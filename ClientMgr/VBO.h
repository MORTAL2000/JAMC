#pragma once

#include <vector>

enum TypeGeom { 
	TG_Points,
	TG_Lines,
	TG_Triangles
};

struct IndexSet { 
	TypeGeom type;
	int index;
	int length;
	int last_index;
	int last_length;
};

template< class T >
class VBO {
private:
	std::vector< T > buff_vertex;

public:

private:

public:
	VBO( );
	~VBO( );

	void clear( );
	//void start( 
};

