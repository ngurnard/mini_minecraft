Milestone 1:

1. Procedural generation of terrain using noise functions (Evan)


2. Altering the provided Terrain system to efficiently store and render blocks (Benedict)
- Chunk was made inheritable from Drawable
- Wrote the createVBOdata() function according to the pseudocode given in the class lecture
- Setup VertexData (that stores position and uv coord) and BlockFace (that stores direction, offset and vertex) structures based on the classroom suggestions
- Inside the createVBOdata() function, we iterate over each block in the chunk (16 x 256 x 16). If the block is opaque, we iterate over all the adjacent non-opaque blocks and generate faces for such interfaces alone. We append the vertices, colors and normals for these faces. Colors are determined based on a getColor function that returns the color based on a block type.
- We also store the per-vertex data like vertex position, colors, normals, uv coordinates and animatable flag in an interleaved fashion. The vertex indices are also set properly for quadrangulation.
- Made changes in drawable.cpp by adding methods to generate and bind the interleavedList.
- Added a new drawInterleaved function in the ShaderProgram to read the new interleavedBuffer with interleaved data of positions, normals, color, uv coords and animatable job and render a Drawable that has been set up with the interleaved VBOs.
- Terrain function was changed to accomodate terrain expansion to determine whether a new chunk should be added to Terrain based on the player's proximity to the edge of a Chunk without a neighbor. This was also added to MyGL::tick() so that it's being checked in every frame.

3. Constructing a controllable Player class with simple physics (Nick)