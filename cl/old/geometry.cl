/* Helper methods for geometry manipulation. */

/* Represents a mesh object. This is the base geometry of an object in the scene, and simply represents an offset in the BVH node structure (which is really a concatenation of the geometry of each mesh in the scene). */
typedef struct Mesh
{
	int offset;
} Mesh;

/* This is a layer. It contains a reference to a given material, as well as its distance from the previous layer (this is for interference effects). */
typedef struct Layer
{
	int materialIndex;
	float distance;
} Layer;

/* This is a model object, which consists of a mesh, an associated transformation matrix (which indicates how the base geometry is transformed to obtain the final model) and an array of layers, each representing a different material (which is just an index to one slice of an array of 2D textures which contain spectral data). */
typedef struct Model
{
	//float4x4 matrix;
	Layer layer[16];
	int meshOffset;
} Model;
