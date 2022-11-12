# Mini-Minecraft Project
## Team O(idk): Benedict Florance Arockiaraj, Evan Grant, Nicholas Gurnard
### Milestone 1:

1. Procedural generation of terrain using noise functions (Evan)
- Created several fragment shaders in ShaderToy.pro to design and visualize various noise functions to be used for terrain
- Wrote customFBM class as a template for the 3 FBM-based noise function I decided upon for mountains, grassland, and biomeMask
- Made 3 post-process functions for each FBM function to further tailor the output to specific terrain and exponentiate/ smoothstep the mask
- Mapped keypress ("P") to a print function which prints the terrain height at player position (useful for debugging terrrain stuff)
- Wrote computeHeight() function which interpolates between biome height maps via the biomeMask FBM and returns final height & biome
- Implemented function to fill a set a given chunk's blocks given terrain parameters (height, biome, snow height, water height, etc.)
- Replaced starting platform create function with a function that generates a given number of terrain chunks around the start posiiton
- Fine-tuned FBM and post-processing parameters to achive desired biome qualities and distribution. I was going for a Jackson, Wyoming look with lakes and grassland surrounded by impossibly-high vertical peaks jutting out of the landscape. I may tweak these moving forward to make the terrain more traversable with some gentler perlin-noise based options.
- Injected noise to break up the snowline such that the snow caps on the peaks look less uniform
- Added sand block option to surround water in the grassland
- Challenges: Finding the proper look and distribution of terrain was exceptionally challenging, and I feel I could still improve the look considerably.

2. Altering the provided Terrain system to efficiently store and render blocks (Benedict)
- Chunk was made inheritable from Drawable
- Wrote the createVBOdata() function according to the pseudocode given in the class lecture
- Setup VertexData (that stores position and uv coord) and BlockFace (that stores direction, offset and vertex) structures based on the classroom suggestions
- Inside the createVBOdata() function, we iterate over each block in the chunk (16 x 256 x 16). If the block is opaque, we iterate over all the adjacent non-opaque blocks and generate faces for such interfaces alone. We append the vertices, colors and normals for these faces. Colors are determined based on a getColor function that returns the color based on a block type.
- We also store the per-vertex data like vertex position, colors, normals, uv coordinates and animatable flag in an interleaved fashion. The vertex indices are also set properly for quadrangulation.
- Made changes in drawable.cpp by adding methods to generate and bind the interleavedList.
- Added a new drawInterleaved function in the ShaderProgram to read the new interleavedBuffer with interleaved data of positions, normals, color, uv coords and animatable job and render a Drawable that has been set up with the interleaved VBOs.
- Terrain function was changed to accomodate terrain expansion to determine whether a new chunk should be added to Terrain based on the player's proximity to the edge of a Chunk without a neighbor. This was also added to MyGL::tick() so that it's being checked in every frame.
- As terrain expands, even though our chunks are designed to create faces only between opaque and empty blocks, it still draws faces between opaque objects as chunks are drawn sequentially. To prevent this, through updateNeigbor function a new Chunk can tell preexisting neighbors to recompute their VBO data.
- Added UNCERTAIN blocks to prevent subterranean blocks getting rendered below the terrain surface.

3. Constructing a controllable Player class with simple physics (Nick)
- Bound key presses to actions to move the player (jumps, fly, move in a certain direction).
- Recreated beleivable physics such that the player accelerated at the same acceleration as Usain bolt up to the maximum speed of Usain Bolt. Also made it such that the player fell at the terminal velocity similar to that of a real human. These quantaties were scaled to better reflect this mini block world.
- Checked which blocks the player on top of to determine whether to jump. Also began implementing next milestone where player moves at different accelerations based on which block they are in (water and lava are slower).
- Limited the camera rotation such that it can not go beyond the global vertical vector by keeping track of net rotation.
- Implmented rotation by a certain degree amount based on user mouse movement. Introduced DPI tunable float parameter in which the user can adjust the sensitivity of rotation (unique to each system).
- When user presses left click, the user can place a remove at a max distance of 3 units away. When user presses right click, the user can place a block at a max distance of 3 units away. Currently only places a grass block.
- Projected the player forward based on speed and acceleration using kinematics equations.
- Reduced the player's velocity accounting for friction and drag in the air.
- If the kinematics equation projects the player such that it collides with a block, the player stops just before the block and reduces the speed normal to the block face to zero.
- Collision detection is done as follows: the player is considered to be a 1x2x1 (x, y, z) rectangular prism conisting of 2 1x1x1 blocks. A ray is cast from each vertex using the forward projection vector based on the kinematics equations. If any of these forward projections collide with a non-empty block, the player moves up to but not through the block. The ray intersections are determined with a gridmarching algirithm, which also keeps track of which face is collided with in order to place blocks in the correct location.
- When the player is in flight mode, translations occur parallel to each of the cardinal planes (xy, xz, yz) intentially such that easy world viewing is achievable. It was tested such that the player moved along its local axes, but that produces unfavorable gameplay.
- The Entity class was edited to hold useful values to any future entities such as button presses and bools to determine whether on ground.  
- Currently cannot jump on water or lava since that doesn't make physical sense.