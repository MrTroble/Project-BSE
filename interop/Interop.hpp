#pragma once

#ifdef WIN32
#ifdef TGE_IMPORT_INTEROP
#define TGE_DLLEXPORT extern "C" __declspec(dllimport)
#else
#define TGE_DLLEXPORT extern "C" __declspec(dllexport)
#endif
#else
#define TGE_DLLEXPORT extern "C"
#endif

typedef unsigned int ReferenceKey;
typedef unsigned int uint;

struct vec3 {
	float x;
	float y;
	float z;
};

struct ReferenceTransform {
	vec3 translation;
	vec3 scale;
	vec3 rotations;
};

struct RefernceLoad {
	char* formKey;
	char* path;
	ReferenceTransform transform;
};

enum class UpdateType {
	TRANSFORM, PATH
};

struct RefernceUpdate {
	ReferenceKey formKey;
	UpdateType update;
	union {
		char* path;
	};
};

TGE_DLLEXPORT ReferenceKey loadReferences(uint count, RefernceLoad* load);

TGE_DLLEXPORT bool updateReferences(uint count, RefernceUpdate* keys);

TGE_DLLEXPORT bool hideReferences(uint count, ReferenceKey* keys);

TGE_DLLEXPORT bool deleteReferences(uint count, ReferenceKey* keys);
