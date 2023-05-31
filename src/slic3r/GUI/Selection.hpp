#ifndef slic3r_GUI_Selection_hpp_
#define slic3r_GUI_Selection_hpp_

#include "libslic3r/Geometry.hpp"
#if ENABLE_WORLD_COORDINATE
#include "GUI_Geometry.hpp"
#include "CoordAxes.hpp"
#else
#include "GLModel.hpp"
#endif // ENABLE_WORLD_COORDINATE

#include <set>
#include <optional>

namespace Slic3r {

class Shader;
class Model;
class ModelObject;
class GLVolume;
class GLArrow;
class GLCurvedArrow;
class DynamicPrintConfig;
class GLShaderProgram;
class BuildVolume;

using GLVolumePtrs = std::vector<GLVolume*>;
using ModelObjectPtrs = std::vector<ModelObject*>;


namespace GUI {
#if !ENABLE_WORLD_COORDINATE
class TransformationType
{
public:
    enum Enum {
        // Transforming in a world coordinate system
        World = 0,
        // Transforming in a local coordinate system
        Local = 1,
        // Absolute transformations, allowed in local coordinate system only.
        Absolute = 0,
        // Relative transformations, allowed in both local and world coordinate system.
        Relative = 2,
        // For group selection, the transformation is performed as if the group made a single solid body.
        Joint = 0,
        // For group selection, the transformation is performed on each object independently.
        Independent = 4,

        World_Relative_Joint = World | Relative | Joint,
        World_Relative_Independent = World | Relative | Independent,
        Local_Absolute_Joint = Local | Absolute | Joint,
        Local_Absolute_Independent = Local | Absolute | Independent,
        Local_Relative_Joint = Local | Relative | Joint,
        Local_Relative_Independent = Local | Relative | Independent,
    };

    TransformationType() : m_value(World) {}
    TransformationType(Enum value) : m_value(value) {}
    TransformationType& operator=(Enum value) { m_value = value; return *this; }

    Enum operator()() const { return m_value; }
    bool has(Enum v) const { return ((unsigned int)m_value & (unsigned int)v) != 0; }

    void set_world()        { this->remove(Local); }
    void set_local()        { this->add(Local); }
    void set_absolute()     { this->remove(Relative); }
    void set_relative()     { this->add(Relative); }
    void set_joint()        { this->remove(Independent); }
    void set_independent()  { this->add(Independent); }

    bool world()        const { return !this->has(Local); }
    bool local()        const { return this->has(Local); }
    bool absolute()     const { return !this->has(Relative); }
    bool relative()     const { return this->has(Relative); }
    bool joint()        const { return !this->has(Independent); }
    bool independent()  const { return this->has(Independent); }

private:
    void add(Enum v) { m_value = Enum((unsigned int)m_value | (unsigned int)v); }
    void remove(Enum v) { m_value = Enum((unsigned int)m_value & (~(unsigned int)v)); }

    Enum    m_value;
};
#endif // !ENABLE_WORLD_COORDINATE

class Selection
{
public:
    typedef std::set<unsigned int> IndicesList;

    enum EMode : unsigned char
    {
        Volume,
        Instance
    };

    enum EType : unsigned char
    {
        Invalid,
        Empty,
        WipeTower,
        SingleModifier,
        MultipleModifier,
        SingleVolume,
        MultipleVolume,
        SingleFullObject,
        MultipleFullObject,
        SingleFullInstance,
        MultipleFullInstance,
        Mixed
    };

private:
    struct VolumeCache
    {
    private:
        struct TransformCache
        {
            Vec3d position{ Vec3d::Zero() };
            Vec3d rotation{ Vec3d::Zero() };
            Vec3d scaling_factor{ Vec3d::Ones() };
            Vec3d mirror{ Vec3d::Ones() };
            Transform3d rotation_matrix{ Transform3d::Identity() };
            Transform3d scale_matrix{ Transform3d::Identity() };
            Transform3d mirror_matrix{ Transform3d::Identity() };
            Transform3d full_matrix{ Transform3d::Identity() };
#if ENABLE_WORLD_COORDINATE
            Geometry::Transformation transform;
#endif // ENABLE_WORLD_COORDINATE

            TransformCache() = default;
            explicit TransformCache(const Geometry::Transformation& transform);
        };

        TransformCache m_volume;
        TransformCache m_instance;

    public:
        VolumeCache() = default;
        VolumeCache(const Geometry::Transformation& volume_transform, const Geometry::Transformation& instance_transform);

        const Vec3d& get_volume_position() const { return m_volume.position; }
#if !ENABLE_WORLD_COORDINATE
        const Vec3d& get_volume_rotation() const { return m_volume.rotation; }
        const Vec3d& get_volume_scaling_factor() const { return m_volume.scaling_factor; }
        const Vec3d& get_volume_mirror() const { return m_volume.mirror; }
#endif // !ENABLE_WORLD_COORDINATE
        const Transform3d& get_volume_rotation_matrix() const { return m_volume.rotation_matrix; }
        const Transform3d& get_volume_scale_matrix() const { return m_volume.scale_matrix; }
        const Transform3d& get_volume_mirror_matrix() const { return m_volume.mirror_matrix; }
        const Transform3d& get_volume_full_matrix() const { return m_volume.full_matrix; }
#if ENABLE_WORLD_COORDINATE
        const Geometry::Transformation& get_volume_transform() const { return m_volume.transform; }
#endif // ENABLE_WORLD_COORDINATE

        const Vec3d& get_instance_position() const { return m_instance.position; }
        const Vec3d& get_instance_rotation() const { return m_instance.rotation; }
        const Vec3d& get_instance_scaling_factor() const { return m_instance.scaling_factor; }
        const Vec3d& get_instance_mirror() const { return m_instance.mirror; }
        const Transform3d& get_instance_rotation_matrix() const { return m_instance.rotation_matrix; }
        const Transform3d& get_instance_scale_matrix() const { return m_instance.scale_matrix; }
        const Transform3d& get_instance_mirror_matrix() const { return m_instance.mirror_matrix; }
        const Transform3d& get_instance_full_matrix() const { return m_instance.full_matrix; }
#if ENABLE_WORLD_COORDINATE
        const Geometry::Transformation& get_instance_transform() const { return m_instance.transform; }
#endif // ENABLE_WORLD_COORDINATE
    };

public:
    typedef std::map<unsigned int, VolumeCache> VolumesCache;
    typedef std::set<int> InstanceIdxsList;
    typedef std::map<int, InstanceIdxsList> ObjectIdxsToInstanceIdxsMap;

    class Clipboard
    {
        // Model is stored through a pointer to avoid including heavy Model.hpp.
        // It is created in constructor.
        std::unique_ptr<Model> m_model;

        Selection::EMode m_mode;

    public:
        Clipboard();

        void reset();
        bool is_empty() const;

        bool is_sla_compliant() const;

        ModelObject* add_object();
        ModelObject* get_object(unsigned int id);
        const ModelObjectPtrs& get_objects() const;

        Selection::EMode get_mode() const { return m_mode; }
        void set_mode(Selection::EMode mode) { m_mode = mode; }
    };

private:
    struct Cache
    {
        // Cache of GLVolume derived transformation matrices, valid during mouse dragging.
        VolumesCache volumes_data;
        // Center of the dragged selection, valid during mouse dragging.
        Vec3d dragging_center;
        // Map from indices of ModelObject instances in Model::objects
        // to a set of indices of ModelVolume instances in ModelObject::instances
        // Here the index means a position inside the respective std::vector, not ObjectID.
        ObjectIdxsToInstanceIdxsMap content;
        // List of ids of the volumes which are sinking when starting dragging
        std::vector<unsigned int> sinking_volumes;
    };

    // Volumes owned by GLCanvas3D.
    GLVolumePtrs* m_volumes;
    // Model, not owned.
    Model* m_model;

    bool m_enabled;
    bool m_valid;
    EMode m_mode;
    EType m_type;
    // set of indices to m_volumes
    IndicesList m_list;
    Cache m_cache;
    Clipboard m_clipboard;
    std::optional<BoundingBoxf3> m_bounding_box;
    // Bounding box of a single full instance selection, in world coordinates, with no instance scaling applied.
    // This bounding box is useful for absolute scaling of tilted objects in world coordinate space.
    // Modifiers are NOT taken in account
    std::optional<BoundingBoxf3> m_unscaled_instance_bounding_box;
    // Bounding box of a single full instance selection, in world coordinates.
    // Modifiers are NOT taken in account
    std::optional<BoundingBoxf3> m_scaled_instance_bounding_box;
#if ENABLE_WORLD_COORDINATE
    // Bounding box of a single full instance selection, in world coordinates, with no instance scaling applied.
    // Modifiers are taken in account
    std::optional<BoundingBoxf3> m_full_unscaled_instance_bounding_box;
    // Bounding box of a single full instance selection, in world coordinates.
    // Modifiers are taken in account
    std::optional<BoundingBoxf3> m_full_scaled_instance_bounding_box;
    // Bounding box of a single full instance selection, in local coordinates, with no instance scaling applied.
    // Modifiers are taken in account
    std::optional<BoundingBoxf3> m_full_unscaled_instance_local_bounding_box;
    // Bounding box aligned to the axis of the currently selected reference system (World/Object/Part)
    // and transform to place and orient it in world coordinates
    std::optional<std::pair<BoundingBoxf3, Transform3d>> m_bounding_box_in_current_reference_system;
#endif // ENABLE_WORLD_COORDINATE

#if ENABLE_RENDER_SELECTION_CENTER
    GLModel m_vbo_sphere;
#endif // ENABLE_RENDER_SELECTION_CENTER

#if ENABLE_WORLD_COORDINATE
    CoordAxes m_axes;
#endif // ENABLE_WORLD_COORDINATE
    GLModel m_arrow;
    GLModel m_curved_arrow;
    GLModel m_box;
    struct Planes
    {
        std::array<Vec3f, 2> check_points{ Vec3f::Zero(), Vec3f::Zero() };
        std::array<GLModel, 2> models;
    };
    Planes m_planes;

    float m_scale_factor;
    bool m_dragging;

public:
    Selection();

    void set_volumes(GLVolumePtrs* volumes);
    bool init();

    bool is_enabled() const { return m_enabled; }
    void set_enabled(bool enable) { m_enabled = enable; }

    Model* get_model() const { return m_model; }
    void set_model(Model* model);

    EMode get_mode() const { return m_mode; }
    void set_mode(EMode mode) { m_mode = mode; }

    void add(unsigned int volume_idx, bool as_single_selection = true, bool check_for_already_contained = false);
    void remove(unsigned int volume_idx);

    void add_object(unsigned int object_idx, bool as_single_selection = true);
    void remove_object(unsigned int object_idx);

    void add_instance(unsigned int object_idx, unsigned int instance_idx, bool as_single_selection = true);
    void remove_instance(unsigned int object_idx, unsigned int instance_idx);

    void add_volume(unsigned int object_idx, unsigned int volume_idx, int instance_idx, bool as_single_selection = true);
    void remove_volume(unsigned int object_idx, unsigned int volume_idx);

    void add_volumes(EMode mode, const std::vector<unsigned int>& volume_idxs, bool as_single_selection = true);
    void remove_volumes(EMode mode, const std::vector<unsigned int>& volume_idxs);

    void add_all();
    void remove_all();

    // To be called after Undo or Redo once the volumes are updated.
    void set_deserialized(EMode mode, const std::vector<std::pair<size_t, size_t>> &volumes_and_instances);

    // Update the selection based on the new instance IDs.
    void instances_changed(const std::vector<size_t> &instance_ids_selected);
    // Update the selection based on the map from old indices to new indices after m_volumes changed.
    // If the current selection is by instance, this call may select newly added volumes, if they belong to already selected instances.
    void volumes_changed(const std::vector<size_t> &map_volume_old_to_new);
    void clear();

    bool is_empty() const { return m_type == Empty; }
    bool is_wipe_tower() const { return m_type == WipeTower; }
    bool is_any_modifier() const { return is_single_modifier() || is_multiple_modifier(); }
    bool is_single_modifier() const { return m_type == SingleModifier; }
    bool is_multiple_modifier() const { return m_type == MultipleModifier; }
    bool is_single_full_instance() const;
    bool is_multiple_full_instance() const { return m_type == MultipleFullInstance; }
    bool is_single_full_object() const { return m_type == SingleFullObject; }
    bool is_multiple_full_object() const { return m_type == MultipleFullObject; }
    bool is_single_volume() const { return m_type == SingleVolume; }
    bool is_multiple_volume() const { return m_type == MultipleVolume; }
    bool is_any_volume() const { return is_single_volume() || is_multiple_volume(); }
    bool is_any_connector() const;
    bool is_any_cut_volume() const;
    bool is_mixed() const { return m_type == Mixed; }
    bool is_from_single_instance() const { return get_instance_idx() != -1; }
    bool is_from_single_object() const;
    bool is_sla_compliant() const;
    bool is_instance_mode() const { return m_mode == Instance; }
#if ENABLE_WORLD_COORDINATE
    bool is_single_volume_or_modifier() const { return is_single_volume() || is_single_modifier(); }
#endif // ENABLE_WORLD_COORDINATE
    bool is_single_volume_instance() const { return is_single_full_instance() && m_list.size() == 1; }
    bool is_single_text() const;

    bool contains_volume(unsigned int volume_idx) const { return m_list.find(volume_idx) != m_list.end(); }
    // returns true if the selection contains all the given indices
    bool contains_all_volumes(const std::vector<unsigned int>& volume_idxs) const;
    // returns true if the selection contains at least one of the given indices
    bool contains_any_volume(const std::vector<unsigned int>& volume_idxs) const;
    // returns true if the selection contains any sinking volume
    bool contains_sinking_volumes(bool ignore_modifiers = true) const;
    // returns true if the selection contains all and only the given indices
    bool matches(const std::vector<unsigned int>& volume_idxs) const;

#if ENABLE_WORLD_COORDINATE
    enum class EUniformScaleRequiredReason : unsigned char
    {
        NotRequired,
        InstanceNotAxisAligned_World,
        VolumeNotAxisAligned_World,
        VolumeNotAxisAligned_Instance,
        MultipleSelection,
    };
#else
    bool requires_uniform_scale() const;
#endif // ENABLE_WORLD_COORDINATE

    // Returns the the object id if the selection is from a single object, otherwise is -1
    int get_object_idx() const;
    // Returns the instance id if the selection is from a single object and from a single instance, otherwise is -1
    int get_instance_idx() const;
    // Returns the indices of selected instances.
    // Can only be called if selection is from a single object.
    const InstanceIdxsList& get_instance_idxs() const;

    const IndicesList& get_volume_idxs() const { return m_list; }
    const GLVolume* get_volume(unsigned int volume_idx) const;
    const GLVolume* get_first_volume() const { return get_volume(*m_list.begin()); }
    GLVolume* get_volume(unsigned int volume_idx);

    const ObjectIdxsToInstanceIdxsMap& get_content() const { return m_cache.content; }

    unsigned int volumes_count() const { return (unsigned int)m_list.size(); }
    const BoundingBoxf3& get_bounding_box() const;
    // Bounding box of a single full instance selection, in world coordinates, with no instance scaling applied.
    // This bounding box is useful for absolute scaling of tilted objects in world coordinate space.
    // Modifiers are NOT taken in account
    const BoundingBoxf3& get_unscaled_instance_bounding_box() const;
    // Bounding box of a single full instance selection, in world coordinates.
    // Modifiers are NOT taken in account
    const BoundingBoxf3& get_scaled_instance_bounding_box() const;

    void start_dragging();
    void stop_dragging() { m_dragging = false; }
    bool is_dragging() const { return m_dragging; }

    void translate(const Vec3d& displacement, bool local = false);

#if ENABLE_WORLD_COORDINATE
    // Bounding box of a single full instance selection, in world coordinates, with no instance scaling applied.
    // Modifiers are taken in account
    const BoundingBoxf3& get_full_unscaled_instance_bounding_box() const;
    // Bounding box of a single full instance selection, in world coordinates.
    // Modifiers are taken in account
    const BoundingBoxf3& get_full_scaled_instance_bounding_box() const;
    // Bounding box of a single full instance selection, in local coordinates, with no instance scaling applied.
    // Modifiers are taken in account
    const BoundingBoxf3& get_full_unscaled_instance_local_bounding_box() const;
    // Returns the bounding box aligned to the axes of the currently selected reference system (World/Object/Part)
    // and the transform to place and orient it in world coordinates
    const std::pair<BoundingBoxf3, Transform3d>& get_bounding_box_in_current_reference_system() const;
    // Returns the bounding box aligned to the axes of the given reference system
    // and the transform to place and orient it in world coordinates
    std::pair<BoundingBoxf3, Transform3d> get_bounding_box_in_reference_system(ECoordinatesType type) const;
#endif // ENABLE_WORLD_COORDINATE

    void setup_cache();

#if ENABLE_WORLD_COORDINATE
    void translate(const Vec3d& displacement, TransformationType transformation_type);
#else
    void translate(const Vec3d& displacement, bool local = false);
#endif // ENABLE_WORLD_COORDINATE
    void rotate(const Vec3d& rotation, TransformationType transformation_type);
    void flattening_rotate(const Vec3d& normal);
    void scale(const Vec3d& scale, TransformationType transformation_type);
    void scale_to_fit_print_volume(const BuildVolume& volume);
#if ENABLE_WORLD_COORDINATE
    void scale_and_translate(const Vec3d& scale, const Vec3d& translation, TransformationType transformation_type);
    void mirror(Axis axis, TransformationType transformation_type);
    void reset_skew();
#else
    void mirror(Axis axis);
    void translate(unsigned int object_idx, const Vec3d& displacement);
#endif // ENABLE_WORLD_COORDINATE
    void translate(unsigned int object_idx, unsigned int instance_idx, const Vec3d& displacement);

#if ENABLE_WORLD_COORDINATE
    // returns:
    // -1 if the user refused to proceed with baking when asked
    // 0 if the baking was performed
    // 1 if no baking was needed
    int bake_transform_if_needed() const;
#endif // ENABLE_WORLD_COORDINATE

    void erase();

    void render(float scale_factor = 1.0);
    void render_sidebar_hints(const std::string& sidebar_field);
#if ENABLE_RENDER_SELECTION_CENTER
    void render_center(bool gizmo_is_dragging);
#endif // ENABLE_RENDER_SELECTION_CENTER

    bool requires_local_axes() const;

    void copy_to_clipboard();
    void paste_from_clipboard();

    const Clipboard& get_clipboard() const { return m_clipboard; }

    // returns the list of idxs of the volumes contained into the object with the given idx
    std::vector<unsigned int> get_volume_idxs_from_object(unsigned int object_idx) const;
    // returns the list of idxs of the volumes contained into the instance with the given idxs
    std::vector<unsigned int> get_volume_idxs_from_instance(unsigned int object_idx, unsigned int instance_idx) const;
    // returns the idx of the volume corresponding to the volume with the given idxs
    std::vector<unsigned int> get_volume_idxs_from_volume(unsigned int object_idx, unsigned int instance_idx, unsigned int volume_idx) const;
    // returns the list of idxs of the volumes contained in the selection but not in the given list
    std::vector<unsigned int> get_missing_volume_idxs_from(const std::vector<unsigned int>& volume_idxs) const;
    // returns the list of idxs of the volumes contained in the given list but not in the selection
    std::vector<unsigned int> get_unselected_volume_idxs_from(const std::vector<unsigned int>& volume_idxs) const;
    // returns the list of idxs of the objects which are in the selection
    std::set<unsigned int> get_object_idxs() const;

#if ENABLE_WORLD_COORDINATE_DEBUG
    void render_debug_window() const;
#endif // ENABLE_WORLD_COORDINATE_DEBUG

private:
    void update_valid();
    void update_type();
    void set_caches();
    void do_add_volume(unsigned int volume_idx);
    void do_add_volumes(const std::vector<unsigned int>& volume_idxs);
    void do_remove_volume(unsigned int volume_idx);
    void do_remove_instance(unsigned int object_idx, unsigned int instance_idx);
    void do_remove_object(unsigned int object_idx);
#if ENABLE_WORLD_COORDINATE
    void set_bounding_boxes_dirty() {
        m_bounding_box.reset();
        m_unscaled_instance_bounding_box.reset(); m_scaled_instance_bounding_box.reset();
        m_full_unscaled_instance_bounding_box.reset(); m_full_scaled_instance_bounding_box.reset();
        m_full_unscaled_instance_local_bounding_box.reset();
        m_bounding_box_in_current_reference_system.reset();
    }
#else
    void set_bounding_boxes_dirty() { m_bounding_box.reset(); m_unscaled_instance_bounding_box.reset(); m_scaled_instance_bounding_box.reset(); }
#endif // ENABLE_WORLD_COORDINATE
    void render_synchronized_volumes();
#if ENABLE_WORLD_COORDINATE
    void render_bounding_box(const BoundingBoxf3& box, const Transform3d& trafo, const ColorRGB& color);
#else
    void render_bounding_box(const BoundingBoxf3& box, const ColorRGB& color);
#endif // ENABLE_WORLD_COORDINATE
    void render_selected_volumes() const;
    void render_bounding_box(const BoundingBoxf3& box, float* color) const;
    void render_sidebar_position_hints(const std::string& sidebar_field, GLShaderProgram& shader, const Transform3d& matrix);
    void render_sidebar_rotation_hints(const std::string& sidebar_field, GLShaderProgram& shader, const Transform3d& matrix);
    void render_sidebar_scale_hints(const std::string& sidebar_field, GLShaderProgram& shader, const Transform3d& matrix);
    void render_sidebar_layers_hints(const std::string& sidebar_field, GLShaderProgram& shader);

public:
    enum class SyncRotationType {
        // Do not synchronize rotation. Either not rotating at all, or rotating by world Z axis.
        NONE = 0,
        // Synchronize after rotation by an axis not parallel with Z.
        GENERAL = 1,
#if ENABLE_WORLD_COORDINATE
        // Synchronize after rotation reset.
        RESET = 2
#endif // ENABLE_WORLD_COORDINATE
    };
    void synchronize_unselected_instances(SyncRotationType sync_rotation_type);
    void synchronize_unselected_volumes();

private:
    void ensure_on_bed();
    void ensure_not_below_bed();
    bool is_from_fully_selected_instance(unsigned int volume_idx) const;

    void paste_volumes_from_clipboard();
    void paste_objects_from_clipboard();

#if ENABLE_WORLD_COORDINATE
    void transform_instance_relative(GLVolume& volume, const VolumeCache& volume_data, TransformationType transformation_type,
        const Transform3d& transform, const Vec3d& world_pivot);
    void transform_volume_relative(GLVolume& volume, const VolumeCache& volume_data, TransformationType transformation_type,
        const Transform3d& transform, const Vec3d& world_pivot);
#endif // ENABLE_WORLD_COORDINATE
};

} // namespace GUI
} // namespace Slic3r

#endif // slic3r_GUI_Selection_hpp_