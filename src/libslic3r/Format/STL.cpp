#include "../libslic3r.h"
#include "../Model.hpp"
#include "../TriangleMesh.hpp"

#include "STL.hpp"

#include <string>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

namespace Slic3r {

bool load_stl(const char *path, Model *model, const char *object_name_in)
{
    TriangleMesh mesh;
    if (! mesh.ReadSTLFile(path)) {
//    die "Failed to open $file\n" if !-e $path;
        return false;
    }
    if (mesh.empty()) {
        // die "This STL file couldn't be read because it's empty.\n"
        return false;
    }

    std::string object_name;
    if (object_name_in == nullptr) {
        const char *last_slash = strrchr(path, DIR_SEPARATOR);
        object_name.assign((last_slash == nullptr) ? path : last_slash + 1);
    } else
       object_name.assign(object_name_in);

    model->add_object(object_name.c_str(), path, std::move(mesh));
    return true;
}

bool load_stl(std::istream &stream, Model *model, const char *object_name_in) {
  TriangleMesh mesh;
  if (!mesh.ReadSTLStream(stream)) {
    //    die "Failed to open $file\n" if !-e $path;
    return false;
  }
  if (mesh.empty()) {
    // die "This STL file couldn't be read because it's empty.\n"
    return false;
  }
  boost::uuids::random_generator rg;
  std::string object_name = boost::uuids::to_string(rg());

  model->add_object(object_name.c_str(), object_name.c_str(), std::move(mesh));
  return true;
}

bool store_stl(const char *path, TriangleMesh *mesh, bool binary)
{
    if (binary)
        mesh->write_binary(path);
    else
        mesh->write_ascii(path);
    //FIXME returning false even if write failed.
    return true;
}

bool store_stl(const char *path, ModelObject *model_object, bool binary)
{
    TriangleMesh mesh = model_object->mesh();
    return store_stl(path, &mesh, binary);
}

bool store_stl(const char *path, Model *model, bool binary)
{
    TriangleMesh mesh = model->mesh();
    return store_stl(path, &mesh, binary);
}

}; // namespace Slic3r
