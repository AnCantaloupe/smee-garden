#ifndef SEEDS_ENTITY_H

#define MAX_BITMAPS    (100)
#define MAX_MATERIALS  (100)
#define MAX_COMPONENTS (100)
#define MAX_MESHES     (100)
#define MAX_ENTITIES   (100)

enum entity_type : int {
    
    ENT_Error = 0,
    
    ENT_Camera,
    ENT_Cube,
    ENT_Windmill,
    
    ENT_Count
};

struct rot {
    
    union {
        v3 AngDis;
        struct {
            f32 Theta;
            f32 Phi;
            f32 Zeta;
        };
    };
    
    union {
        v3 AngVel;
        struct {
            f32 dTheta;
            f32 dPhi;
            f32 dZeta;
        };
    };
    
    union {
        v3 AngAcc;
        struct {
            f32 ddTheta;
            f32 ddPhi;
            f32 ddZeta;
        };
    };
};

struct kino {
    
    v3 Pos;
    v3 Vel;
    v3 Acc;
};

struct entity {
    
    v3 Origin;
    
    union {
        struct {
            v3 UnitX;
            v3 UnitY;
            v3 UnitZ;
        };
        mat3x3 UnitAxes;
    };
    
    v3 Scale;
};

#ifdef SEEDS_SIMD_H
struct entity_wide {
    
    v3_wide Origin;
    
    v3_wide UnitX;
    v3_wide UnitY;
    v3_wide UnitZ;
    
    v3_wide Scale;
};
#endif

// TODO(canta): Temporary
struct entity_colour_info {
    f32 NormDist;
    colour High;
    colour Low;
};

#ifdef SEEDS_ASSETS_H
struct entity_prototype {
    
    entity_type EntityType;
    
    v3 Scale;
    entity_colour_info ColourInfo;
    
    int   MeshIndex;
    mesh *Mesh;
};

struct entity_library {
    
    string           Names      [MAX_ENTITIES]; // TODO(canta): Use this.
    entity_prototype Prototypes [MAX_ENTITIES];
};

struct entity_manager {
    
    b32 Exists                    [MAX_ENTITIES];
    
    string       Names            [MAX_ENTITIES];
    u32          EntityIds        [MAX_ENTITIES];
    entity_type  Types            [MAX_ENTITIES];
    entity       Entities         [MAX_ENTITIES]; // TODO(canta): Better name
    
    f32          Thetas           [MAX_ENTITIES];
    f32          Phis             [MAX_ENTITIES];
    
    entity_colour_info ColourInfo [MAX_ENTITIES];
    
    int       MeshIndices         [MAX_ENTITIES];
    v3       *Verticeses          [MAX_ENTITIES];
    int       VertexCounts        [MAX_ENTITIES];
    v2       *UVses               [MAX_ENTITIES];
    triangle *Triangleses         [MAX_ENTITIES];
    int       TriangleCounts      [MAX_ENTITIES];
    int       MaterialCounts      [MAX_ENTITIES];
    material *Materialses         [MAX_ENTITIES];
    
    trap_prism BoundingBoxes      [MAX_ENTITIES];
};
#endif

#define SEEDS_ENTITY_H
#endif
