#pragma once

// Forward Decls
class Runnable;
class Manager;

class ResourceMgr;
class TextureMgr;
class TimeMgr;
class ThreadMgr;
class DisplayMgr;
class InputMgr;
class GuiMgr;
class BlockMgr;
class BiomeMgr;
class ChunkMgr;
class EntityMgr;

class Client;
class ClientMgr;

class AbstractHandle;
template< class T > class Handle;
class Pool;
template< class T > class ResPool;

class Block;

class Chunk;
struct ChunkNoise;
struct ChunkFile;

class Entity;

struct quad_uvs;

class Page;

// Static Variables
#define PI 3.14159265f
#define BUFFER_OFFSET(i) ((void*)(i))

constexpr float UPDATE_RATE = 60;
constexpr float TIME_MILLISEC = 1000.0f;
constexpr float DELTA_CORRECT = 1.0f / UPDATE_RATE;
constexpr float TIME_FRAME_MILLI = TIME_MILLISEC / UPDATE_RATE;