# Mini-Minecraft Project
## Team O(idk): Benedict Florance Arockiaraj, Evan Grant, Nicholas Gurnard
### Milestone 2:
**1. Procedural generation of caves using 3D noise and PostProcess Shaders _(Nick)_**
- Created 2 noise function consisting of 3D perlin noise to create cave logic (exists in cave files). One mask is used as the cave generation, the other is a bigger version but rotated such that the first cave function can only exist within this bigger mask. This avoids the extra computation of FBM (although small, more optimal is always better). 
- Refactored shaderprogram.cpp to be a base superclass with 2 subclasses: surfaceshader.h and postprocessshader.h. This way it is easier to keep track of member variables and the memeber functions are more relevant to the shader being implemented.
- Created 2 postprocess shader files: 1 for lava and 1 for water when the players camera position is under water.
- Made the postt process shaders change relative to time such that there is a complex noise effect that more realistically simulated travelling through a fluid. The colors were determined with a cosine palette and the noise function was constructed with Worley Noise and an FBM.
- Made a frame buffer class that stores the output of the 3D terrain as a 2D image. This image is then passed through a "passthrough" vertex shader that does no operation to the terrain, but allows the postprocess shaders to manipulate the appearance of the terrain. If no changes are made, it passes through the "noOp" fragment shader which applies no changes to the scene.
- Made it such that the player swims in liquids such as water and lava. The player's overall speed and acceleration are accurately decreased to simulate the drag which a liquid would apply. 
- The player information log was updated to inform the player what block their camera is within and whether or not they are on the ground or in a liquid.
- Challenges: The postprocess render pipeline was often returning nothing (blue sky) because the frame buffer was improperly being stored after the 3D scene was rendered (was not able to use the screen as input). Additionally, the openGL context was accidentally getting reset in the post process shader class that was causing many errors.


**2. Texturing and texture animation in OpenGL _(Evan)_**

**3. Multithreading of terrain loading _(Benedict)_**
- Used C++ standard library for multithreading functions (`std::thread`) and used standard library mutexes (`std::mutex`) for locking.
- The multithreading parts were designed based on the code structure given in the class. Every tick we call the `multithreadedWork` function that checks if the terrain should expand.
- We make sure we try expanding only once every tick using an expansion timer, that prevents frequent expansion checks which might lead to weird results.
- For every terrain generation zone in this radius that does not yet exist in Terrain's `m_generatedTerrain`, we spawn a thread that calls `BlockTypeWorker` start routine to fill that zone's Chunks with procedural height field BlockType data. 
- For every terrain generation zone in this radius that does exist in `m_generatedTerrain`, we check each Chunk it contains and see if it already has VBO data. If it does not, then we spawn another thread with `VBOWorker` start routine to compute the interleaved buffer and index buffer data for that Chunk. 
- However, VBOWorkers will not pass data to the GPU as threads that are not the main thread do not have an OpenGL context from which to communicate with the GPU. 
- Every tick after attempting to spawn additional threads, we check on the state of these data collections in `checkThreadResults` by sending filled Chunks to new VBOWorkers, and sending completed VBO data to the GPU.
- We use mutexes for any access on `m_chunksThatHaveBlockData` and `m_chunksThatHaveVBOData` to prevent shared memory overwrites at the same instance.
- Also, modified `chunk.cpp`'s `getBlockAt` method to prevent sub-terranean and wall rendering by having access to extra information of what block type the neighboring chunks blocks would have (using the deterministic noise function).
- Challenges: Resolving various segfaults to make the multithreaded portion working was time-consuming (used unique pointer based approach initially with a lot of `std::move`s and later switched to raw pointer based approach). Not detaching the threads properly gave segfaults that were later resolved. Another major bottleneck was to remove sub-terranean and wall rendering that disrupted cave rendering to work properly. Fixed that after resolving a few weird bugs. 

### Milestone 1:

**1. Procedural generation of terrain using noise functions _(Evan)_**
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

**2. Altering the provided Terrain system to efficiently store and render blocks _(Benedict)_**
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

**3. Constructing a controllable Player class with simple physics _(Nick)_**
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
- Challenges: collision with corners