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

typedef char* FormKey;
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
	FormKey formKey;
	char* path;
	ReferenceTransform transform;
};

enum class UpdateType {
	TRANSFORM, PATH
};

struct RefernceUpdate {
	FormKey formKey;
	UpdateType update;
	union {
		char* path;
		ReferenceTransform transform;
	};
};

typedef void(*LoadCallback)(uint count, RefernceLoad* load);

TGE_DLLEXPORT void loadReferences(uint count, RefernceLoad* load);

TGE_DLLEXPORT bool updateReferences(uint count, RefernceUpdate* keys);

TGE_DLLEXPORT bool hideReferences(uint count, FormKey* keys, bool hide);

TGE_DLLEXPORT bool deleteReferences(uint count, FormKey* keys);

TGE_DLLEXPORT bool addLoadHook(LoadCallback callback);
