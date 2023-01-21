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

typedef const char* FormKey;
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

constexpr ReferenceTransform TGE_DEFAULT_TRANSFORM = {
    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};

struct ReferenceLoad {
  FormKey formKey;
  const char* path;
  ReferenceTransform transform;
};

enum class UpdateType { TRANSFORM, PATH };

struct ReferenceUpdate {
  FormKey formKey;
  UpdateType update;
  union {
    const char* path;
    ReferenceTransform transform;
  };
};

typedef bool (*LoadCallback)(const uint count, const ReferenceLoad* load);
typedef bool (*UpdateCallback)(const uint count, const ReferenceUpdate* keys);
typedef bool (*HideCallback)(const uint count, const FormKey* keys,
                             const bool hide);
typedef bool (*DeleteCallback)(const uint count, const FormKey* keys);
typedef bool (*LoadFinishedCallback)(void);

/*
 * The given [...]References functions should be called to manipulate the given
 * references with the according event, if any callback function returns false
 * the action returns false.
 *
 * if count > 0 the given pointer must be a valid pointer to a reference array
 * of size count if count = 0 then the pointer should be a nullptr
 */
TGE_DLLEXPORT bool loadReferences(uint count, ReferenceLoad* load);

TGE_DLLEXPORT bool updateReferences(uint count, ReferenceUpdate* keys);

TGE_DLLEXPORT bool hideReferences(uint count, FormKey* keys, bool hide);

TGE_DLLEXPORT bool deleteReferences(uint count, FormKey* keys);

/*
 * With the add[...]Callback functions you can add your callback functions to
 * the system Those are called when the according event occures
 *
 * returns true if and only if the callback was actually added.
 * callbacks cannot be added more then once hence this method returns false if
 * the callback was already added
 *
 * Implicite: callback might not be null
 */
TGE_DLLEXPORT bool addLoadCallback(LoadCallback callback);

TGE_DLLEXPORT bool addUpdateCallback(UpdateCallback callback);

TGE_DLLEXPORT bool addHideCallback(HideCallback callback);

TGE_DLLEXPORT bool addDeleteCallback(DeleteCallback callback);

TGE_DLLEXPORT bool addLoadFinishedCallback(LoadFinishedCallback callback);

void callLoadFinishedCallback();
