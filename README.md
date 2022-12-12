# Mini-Minecraft Project
## Team O(idk): Benedict Florance Arockiaraj, Evan Grant, Nicholas Gurnard
### Milestone 3:

**Benedict: L-system Rivers + NPC AI**

**Nick: Inventory + GUI, Water Waves, Post-Process Camera Overlay (Improved), Sound, Back-Face Culling (OpenGL)**
#### Inventory:
- Implemented basic inventory system where the player starts off with 1 of each block type: grass, dirt, stone, water, ice, snow, and sand. The player starts with 32 blocks of lava because... why not!?
- The player is not able to place a block if their block count falls below 1 (ie they have zero)
- The player can collect up to 64 of each block type
- The player can open the gui by pressing the key "i" and can close the gui by pressing the key "i"
- The player can select which block they wish to hold by opening the inventory and then selecting a non-empty block. If the player selects LAVA, then the player will be able to place a lava block
- Once the player runs out of a block type, the block will no longer appear in their inventory
- The player can collect blocks by left clicking (removing) blocks from the world
- The playerinfo widget now displays what block the player is holding, as well as some additional information on whether or not they are on the ground on in liquid
- The inventory widget was implemented with a QWidget and adding an *inventory.ui* inside of the *Forms* diretory 
- Challenges: The hardest part of setting up the inventory was opening a new QWidget window and making sure it can connect to the main program. This took some time for me because I had to refresh my understanding of signals and slots and then implement them thuroughly. Additionally, Getting an image to show in the inventory wasn't trivial to figure out despite the fact it took relatively few lines. Finally, I had to change the player memeber variable to a uniquie pointer in order for player data to be passed through a signal from MyGL -> MainWindow -> Inventory or MyGL -> Inventory -> MainWindow

#### Water Waves:
- If a vertex is animatable and is liquid (water or lava), then the surface of the block will have displaces vertices
- The surface of either water or lava appears to be moving according to a noise function
- The vertex positions were changed inside of the *SkyTerrainUber.vert.glsl* vertex shader file in the *Resources* directory
- Implemented Blinn-Phong lighting in the *SkyTerrainUber.frag.glgl* file in the *Resources* directory since Blinn-Phong is an extension of lambert
- Challenges: It took a long time trying to figure out how to properly displace the normal vectors from within the shader itself. I had the idea of using a tangent vector and bi-tangent vector relative to the surface to then take a cross product to compute the new normal. Additionally, the vertices exit only on the corvers of each cube, and it would have looked nicer to quadrangulate each face to have more vertices but that required a lot of additional work that I did not have the time for.

#### Sound
- Made a additional functions inside of *player.cpp* and *mygl.cpp* that induces sounds based on certain events
- If the player is walking on dirt or grass, footsteps for grass/dirt will play if the player is walking
- If the player is walking on sand or snow, footsteps for snow/sand will play if the player is walking
- If the player's head is underwater, it will sound like the player is underwater
- If the player is flying in flight mode, it will sound windy if the player is moving
- If the player is either in flight more or in non-flight mode, a light wind will sound if they are high enough regardless of player speed
- If the player is in lava, they will scream AHHhhHhHhhH
- If the player switches from non-flight mode to flight-mode, a takeoff sound will play
- Implemented dynamic sound volume, so if the player is going slowly then the volume will be quiter than if they are going faster. This is hard to notice unless turned off, then all of the sounds for walking and flying will sound extremely abrupt

#### Post-Process Camera Overlay
- This was mostly implemented in the last milestone for the sake of this milestone. Please refer to comment there for more information
- Post-process effects are for when the player is under water or lava and are an upgrade from the regular red or blue tinge

#### Back-Face Culling
- Implmented OpenGL solution (simple one-line solution) to backface culling such that the world gets rendered more efficiently since less textures are drawn. The previous solution was not universal accross operating systems

**Evan: Day and Night Cycle/Procedural Sky, Moving & dynamically dense FBM Clouds, HUD crosshair, Selected Block Wireframe**
#### Day and Night Cycle:
- I was most interested in beautifying our game, so I implmented a day and night cycle starting with the raycast method discussed during class - with many alterations to produce a smooth, natural-looking environment.
- A separate sky shader pipeline was created, but because I wanted to be able to infuse the sky color (possibly off-screen sky color) into the environement, a new "SkyTerrainUber" shader was made to combine lambert and sky shaders. This now handles both draw calls for the sky's screen-spanning quad and the terrain.
- Instead of defining a series of hard-coded color thresholds for sunset and dusk colors and linearly mixing between them, I instead used the tool at http://dev.thi.ng/gradients/ to carefully design 3 separate cosine color palettes to represent, day, sunrise/sunset, and night sky gradients.
- The day and night sky gradients are static - the dot product (re-mapped to [0,1] range) between ray direction and the world up vector sets the cosine color value at each fragment.
- The sunset palette, however, is dynamic - the value to retrieve its color is the dot between ray direction and the sun direction, such that the gradient enveloping the sky is always rotating with the sun.
- To blend between these palettes, I use a custom non-linear blending strategy such that if the y-component of sun direction is positive, the blending occurs between sunset and day palettes, and otherwise between sunset and night. The nonlinearity of this blending essentially compresses the region (angle) at which sunset coloring occurs. This effectively holds off the sunset until the sun is lower in the sky, and there is more time spent in unaffected day/night coloring before the vibrant sunset/sunrise happens.
- Fixed one issue with the sun/corona method mentioned in class. Instead of having linear falloff in the glow around the sun, exponential falloff is used for a more natural and seamless look.
- Added a moon opposite the sun with a different color and smaller corona.
- Implmented stars using 3D worley noise which fade into the sky as the brightness falls off toward nighttime. The vec3 ray direction is used and a threshold is used to make the sky color white within close 3D proximity to the worley points. The extra dimension in this noise allows for varying star sizes - some of which twinkle or wink out according to look direction.

#### Distance Fog:
- Implemented distance fog based on vertex depth with a fine-tuned exponential falloff function which starts a given distance from the player.
- Initially mixed terrain terrain color with arbitrary fog color, but was dissatisfied with the disconnect between ambient sky color, especially at night. then, the falloff function was applied to the terrain alpha channel, but I encountered the same tranparency render order issues encountered previously with OpenGL.
- Finally, after implmenting the "SkyTerrainUber" shader, The distance fog now blends a slightly darker sky color into the terrian with the exponential falloff. This makes it feel like the sky has much greater influence over terrain color and brings the environement together.

#### Dynamic FBM Clouds:
- Created new Cloud drawable class with separate VBO comprising 16x16 grid of opaque white squares to fit over exaclty one chunk at a given height. I then draw this instance over every chunk that gets loaded into the terrain class.
- Created new cloud fragment shader. This shader is inspired by the CPU-side terrain height generation - it uses 3 FBM noise maps where two act as height maps and the third acts as a mask. these noise functions are identical the ones which generate the terrain/biomes except the position is offset by 1000 in x and z, the "height" value is instead applied to the alpha channel of the white cloud color to get a nice transparency, and the position offset is animated such that the clouds appear to move smoothly over time.
- Additionally, a sine function is applied to the masking threshold for the 3rd FBM such that the sky oscillates slowly between very sparse cloud coverage and heavily overcast

#### HUD Crosshair and Selected Block Wireframe:
- Created HUD post process to draw a plus-shaped crosshair in the middle of the screen which subtly inverts the colors of the rendered texture behind it for better visibility.
- Created Block wireframe drawable and drew it with flat shading over the terrain when the Player's look vector intersects with an opaque block in a 4-block range

#### Misc...
- Implemented naive backface culling before Nick found a much simpler and better way!
- Fixed an issue with liquid blocks' vertex deformation causing them to move into other blocks causing Z-fighting.

- Challenges: arriving at the proper shader pipeline for all these effects proved difficult - I spent an entire day setting up a secondary frame buffer, texture loading, and compositing step before realizing it would be best and more efficient to implement a larger, more flexible shader for both sky and terrain. Additionally, my work was largely aesthetic in nature, and required a great deal of tuning to perfect the look (e.g. star distribution, cloud density/speed, color gradients and the unexpected blending of their values, etc...)

### Milestone 2:

**1. Procedural generation of caves using 3D noise and PostProcess Shaders _(Nick)_**
- Created 2 noise function consisting of 3D perlin noise to create cave logic (exists in cave files). One mask is used as the cave generation, the other is a bigger version but rotated such that the first cave function can only exist within this bigger mask. This avoids the extra computation of FBM (although small, more optimal is always better). 
- Refactored shaderprogram.cpp to be a base superclass with 2 subclasses: surfaceshader.h and postprocessshader.h. This way it is easier to keep track of member variables and the memeber functions are more relevant to the shader being implemented.
- Created 2 postprocess shader files: 1 for lava and 1 for water when the players camera position is under water.
- Made the post process shaders change relative to time such that there is a complex noise effect that more realistically simulated travelling through a fluid. The colors were determined with a cosine palette and the noise function was constructed with Worley Noise and an FBM.
- Made a frame buffer class that stores the output of the 3D terrain as a 2D image. This image is then passed through a "passthrough" vertex shader that does no operation to the terrain, but allows the postprocess shaders to manipulate the appearance of the terrain. If no changes are made, it passes through the "noOp" fragment shader which applies no changes to the scene.
- Made it such that the player swims in liquids such as water and lava. The player's overall speed and acceleration are accurately decreased to simulate the drag which a liquid would apply. 
- The player information log was updated to inform the player what block their camera is within and whether or not they are on the ground or in a liquid.
- Challenges: The postprocess render pipeline was often returning nothing (blue sky) because the frame buffer was improperly being stored after the 3D scene was rendered (was not able to use the screen as input). Additionally, the openGL context was accidentally getting reset in the post process shader class that was causing many errors.


**2. Texturing and texture animation in OpenGL _(Evan)_**
- Created Texture class as a way of loading images into OpenGL
- Troubleshooted and developed pipeline for creating, loading, and binding the texture atlas to slot0
- Implemented time variable and animatible flag such that lava and water textures could be animated
- Created custom (intentionally subtle, so look closely!) animations for lava and water blocks (and decoupled lava shading from lambert to give it a uniform, unshaded look which resembles illumination/glow)
- Enabled alpha blending in myGL and extensively updated drawable/chunk/terrain/multithreading code to split the vbo construction and drawing for the opaque and transparent block passes.
- Created new shader variable to handle passing per-vertex random noise to the fragment shader for animatable blocks
- Added new VBO element (utilizing the unused float in vec4 already being sent) as a flag to identify if a face of a transparent block is in contact with another transparent block of a different type. I then set that block's alpha channel to 1 if it has a higher display priority (e.g. ice should be more opaque and show through water), and have some complicated logic in generateVBOdata() for the transparent pass to instead push back that tranparent block onto the opaque VBO. This is a complicated method to get around the OpenGL arbitrary transparency rendering order for blocks where discarding a fragment is not possible. Essentially, looking at Ice underneath water will now make the ice opaque so that it is guarnteed to be rendered instead of invisible (I'm hoping to make an icy river biome in the future)
- Challenges: understanding the requisite ordering of OpenGL functions to properly create and bind a texture was time consuming and led to no visible output and no errors with which to debug. Also, in order to properly split VBOs, I felt I needed to fully understand Benedict's multithreading code and this was challenging as an additional task. Otherwise, I'm enjoying the satisfying texturing results.
- Also worked with Nick to re-factor ShaderProgram to be a generic parent class from which SurfaceShader and PostProcessShader could inherit for our two respective parts.

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
- Challenges: collision with corners. The ray casting method is geometricxally limiting, and this solution would be more easily solved with volume projection.