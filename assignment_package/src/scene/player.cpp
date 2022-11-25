#include "player.h"
#include <QString>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera)
{
    // set up sounds
    walk_grass.setSource(QUrl::fromLocalFile(":/sounds/footsteps_woods_grass.wav"));
    walk_grass.setLoopCount(QSoundEffect::Infinite);

    walk_snow_sand.setSource(QUrl::fromLocalFile(":/sounds/footsteps_snow_sand.wav"));
    walk_snow_sand.setLoopCount(QSoundEffect::Infinite);

    walk_stone.setSource(QUrl::fromLocalFile(":/sounds/footsteps_stone.wav"));
    walk_stone.setLoopCount(QSoundEffect::Infinite);

    swim_water.setSource(QUrl::fromLocalFile(":/sounds/underwater.wav"));
    swim_water.setLoopCount(QSoundEffect::Infinite);
    swim_water.setVolume(0.1f);

    swim_lava.setSource(QUrl::fromLocalFile(":/sounds/lava_scream.wav"));
    swim_lava.setLoopCount(QSoundEffect::Infinite);
    swim_lava.setVolume(1.0f);

    flying.setSource(QUrl::fromLocalFile(":/sounds/flying.wav"));
    flying.setLoopCount(QSoundEffect::Infinite);
    flying.setVolume(0.05f);

    toggle_flying.setSource(QUrl::fromLocalFile(":/sounds/flightmode.wav"));
    toggle_flying.setLoopCount(1);
    toggle_flying.setVolume(0.1f);

}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    // DELETE ME LATER (WHEN DONE WITH POSPROCSSING SHADERS)
//    BlockType TEMP = headSpaceSight();

    this->m_acceleration = glm::vec3(0, 0, 0); // ensure we dont accidentally keep accelerating
    float tune_max_accel = 9.5; // this is acceleration of Usain Bolt
    float accel_scaler;

    // NOTE: it may help to check the note in the input bundle in the entity class to understand this line
    this->flightMode = inputs.flightMode; // make the player the opposite of what is set in the input bundle

    if (this->flightMode) { // if in flight mode

        // Update the players member variables. Go faster, turn off gravity
        this->gravity = 0;
        accel_scaler = 10;
        tune_max_accel *= accel_scaler; // this is acceleration of Usain Bolt scaled

        if (inputs.wPressed) {
            this->m_acceleration += tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
            this->m_acceleration.y = 0; // move parallel to xz plane
        }
        if (inputs.sPressed) {
            this->m_acceleration -= tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
            this->m_acceleration.y = 0; // move parallel to xz plane
        }
        if (inputs.dPressed) {
            this->m_acceleration += tune_max_accel * this->m_right;
        }
        if (inputs.aPressed) {
            this->m_acceleration -= tune_max_accel * this->m_right;
        }
        if (inputs.ePressed) {
            this->m_acceleration += tune_max_accel * glm::vec3(0, 1, 0);
        }
        if (inputs.qPressed) {
            this->m_acceleration -= tune_max_accel * glm::vec3(0, 1, 0);
        }
        if (!inputs.wPressed && !inputs.sPressed && !inputs.dPressed && !inputs.aPressed && !inputs.ePressed && !inputs.qPressed) {
            this->m_acceleration = glm::vec3(0, 0, 0); // can float and do nothing
        }

        // ensure we dont accelerate way too fast
        this->m_acceleration = glm::clamp(this->m_acceleration, -tune_max_accel, tune_max_accel);

    } else { // if not in flight mode

        // Update the players member variables. Ensure gravity is on
        accel_scaler = 8; // ultimately controls speed. Scales the ussain bolt and gravity values the same degree
        this->gravity = 9.81 * accel_scaler;
        tune_max_accel *= accel_scaler;

        checkOnGround(inputs); // check if the player is on the ground
        checkInLiquid(inputs); // check if the player is in LAVA or WATER

        if (inputs.wPressed) {
            this->m_acceleration += tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
        }
        if (inputs.sPressed) {
            this->m_acceleration -= tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
        }
        if (inputs.dPressed) {
            this->m_acceleration += tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
        }
        if (inputs.aPressed) {
            this->m_acceleration -= tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
        }

        // ensure we dont accelerate way too fast
        this->m_acceleration = glm::clamp(this->m_acceleration, -tune_max_accel, tune_max_accel);

        // include the gravity component
        if (!inputs.inLiquid) {
            this->m_acceleration -= glm::vec3(0, gravity, 0);
        } else {
            this->m_acceleration -= glm::vec3(0, gravity, 0);
            this->m_acceleration = glm::vec3(m_acceleration.x * 2/3, m_acceleration.y * 2/3, m_acceleration.z * 2/3);
        }

        if (inputs.spacePressed) {
            // jump if on the ground
            if (inputs.onGround && !inputs.inLiquid) {
                this->m_acceleration.y = this->m_acceleration.y + 20*this->gravity;
                inputs.onGround = false; // no longer on the ground (prevent flying)
            } else if (inputs.inLiquid) {
                this->m_acceleration.y = -m_acceleration.y; // sink and swim at same acceleration
            }
        }
    }

}

void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    // NOTE: This function is based off of kinematic equations

    float tune_max_speed = 10.f; // approxmately the max speed of usain bolt in 1 direction

    if (this->flightMode) { // if in flight mode

        tune_max_speed = 2 * tune_max_speed; // set the max speed for non-flight mode in the x and z directions

        this->m_velocity *= 0.9f; // reduce velocity for friction and drag (slow down on release)
        this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation

        this->m_velocity = glm::clamp(this->m_velocity, -tune_max_speed, tune_max_speed); // ensure not too fast

        this->moveRightGlobal(this->m_velocity.x * dT);
        this->moveUpGlobal(this->m_velocity.y * dT);
        this->moveForwardGlobal(this->m_velocity.z * dT);

        playSoundsFlight();

    } else { // if not in flight mode

//        std::cout << "acceleration: {" << m_acceleration.x << ", " << m_acceleration.y << ", " << m_acceleration.z << "}" << std::endl;
//        std::cout << "velocity: {" << m_velocity.x << ", " << m_velocity.y << ", " << m_velocity.z << "}" << std::endl;

        this->m_velocity.x *= 0.85f; // reduce velocity for friction and drag (slow down on release)
        this->m_velocity.y *= 0.975f; // reduce velocity for friction and drag (slow down on release) -- allow terminal velocity to be reached
        this->m_velocity.z *= 0.85f; // reduce velocity for friction and drag (slow down on release)
        float terminal_speed = 6.6 * tune_max_speed;

        this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation for all dirs
        BlockType footBlock = checkOnGround(inputs); // check if the player is on the ground
        // Ensure not going too fast
        glm::clamp(this->m_velocity.x, -tune_max_speed, tune_max_speed);
        glm::clamp(this->m_velocity.z, -tune_max_speed, tune_max_speed);
        glm::clamp(this->m_velocity.y, -terminal_speed, terminal_speed);
        if (inputs.inLiquid) {
            m_velocity *= 0.66f; // reduce speed in liquids by 2/3
        }

        // Check if there is a collision
        glm::vec3 posRayDir = this->m_velocity * dT; // position vector (slides say length == speed)
        std::array<bool, 3> collidedAxesMask = checkCollision(posRayDir, terrain, inputs);
        this->moveAlongVector(posRayDir); // move along the position vector
        this->m_velocity = this->m_velocity * glm::vec3(!collidedAxesMask[0], !collidedAxesMask[1], !collidedAxesMask[2]); // zero out the velocity vector is colliding with solid block

//        checkCollision(terrain, inputs, dT); // adams implementation

        // Update player sounds
        playSoundsGround(footBlock);

    }

}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit, float *interfaceAxisPtr) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin)); // cell our ray origin exists within
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f; // distance to nearest axist x,y,z
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes to fing the closest point of intersection with the block grid
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) { // determinging the cell we are intersecting from the point on the axis of intersection
                    min_t = axis_t; // this is now the closest axis intersection
                    interfaceAxis = i; // track axis for which t is smallest
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        if (interfaceAxisPtr != nullptr) {
            *interfaceAxisPtr = interfaceAxis;
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

const static vector<glm::vec3> playerCorners {
    // NOTE: the player is shrunk a little bit to fit into small crevices
    // Bottom
    glm::vec3(-0.45, 0.0, -0.45),
    glm::vec3(0.45, 0.0, -0.45),
    glm::vec3(0.45, 0.0, 0.45),
    glm::vec3(-0.45, 0.0, 0.45),
    // Mid
    glm::vec3(-0.45, 0.95, -0.45),
    glm::vec3(0.45, 0.95, -0.45),
    glm::vec3(0.45, 0.95, 0.45),
    glm::vec3(-0.45, 0.95, 0.45),
    // Top
    glm::vec3(-0.45, 1.9, -0.45),
    glm::vec3(0.45, 1.9, -0.45),
    glm::vec3(0.45, 1.9, 0.45),
    glm::vec3(-0.45, 1.9, 0.45),
};

std::array<bool, 3> Player::checkCollision(glm::vec3 &rayDirection, const Terrain &terrain, InputBundle &inputs)
{
    // NOTE: the ray direction is the position based on the velocity because in gridmarch we check as far as this vector!
        glm::vec3 playerVertOrigin = glm::vec3(this->m_position); // effetively the bottom left vertex of the 2 blocks stack (player)
        float offset = 0.0001; // so the player doesn't fall through the ground, and so it fits in a 2x1x1 space
        float out_dist = 0; // delcare input to grid march (how far away the collision is to the block, if at all)
        glm::ivec3 out_blockHit = glm::ivec3(); // declare the input to grid march (cell that we are colliding with, if any)

        std::array<bool, 3> collidedWithAxis{0, 0, 0}; // check if the axis was collided with. Used as a mask to change the speed

        // make rays for every corner of the 2 block stack and check for collisions
        for (auto &corner : playerCorners) {

            glm::vec3 castedRayOrigin = glm::vec3(playerVertOrigin.x + corner.x,
                                                  playerVertOrigin.y + corner.y,
                                                  playerVertOrigin.z + corner.z);

            // positive direction collisions
            glm::vec3 rayX = rayDirection * glm::vec3(1, 0, 0); // get only the x component
            glm::vec3 rayY = rayDirection * glm::vec3(0, 1, 0); // get only the y component
            glm::vec3 rayZ = rayDirection * glm::vec3(0, 0, 1); // get only the z component

            if (gridMarch(castedRayOrigin, rayX, terrain, &out_dist, &out_blockHit)) { // if there is a collision in x
                if (out_dist < glm::abs(rayDirection.x)) { // colliding with an object
                    if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                        rayDirection.x = glm::sign(rayDirection.x) * (out_dist - offset);
                        collidedWithAxis[0] = true;
    //                    std::cout << "colliding x" << std::endl;
                    }
                }
            }

            if (gridMarch(castedRayOrigin, rayY, terrain, &out_dist, &out_blockHit)) { // if there is a collision in y
                if (out_dist < glm::abs(rayDirection.y)) { // colliding with an object
                    if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                        rayDirection.y = glm::sign(rayDirection.y) * (out_dist - offset);
                        collidedWithAxis[1] = true;
    //                    std::cout << "colliding y" << std::endl;
                    }
                }
            }

            if (gridMarch(castedRayOrigin, rayZ, terrain, &out_dist, &out_blockHit)) { // if there is a collision in z
                if (out_dist < glm::abs(rayDirection.z)) { // colliding with an object
                    if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                        rayDirection.z = glm::sign(rayDirection.z) * (out_dist - offset);
                        collidedWithAxis[2] = true;
    //                    std::cout << "colliding z" << std::endl;
                    }
                }
            }
        }

        return collidedWithAxis;
}

/* Adams implementation */
//bool Player::gridMarchAxis(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, int axis, float *out_dist) {
//    float maxLen = glm::abs(rayDirection[axis]); // Farthest we search
//    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin)); // cell our ray origin exists within

//    glm::vec3 dir(0, 0, 0);
//    for(int k = 0; k < 3; ++k) {
//        if(k == axis) {
//            dir[k] = 1;
//        }
//    }
//    rayDirection = glm::normalize(rayDirection) * dir; // Now all t values represent world dist.

//    float curr_t = 0.f; // distance to nearest axist x,y,z
//    while(curr_t < maxLen) {
//        float min_t = glm::sqrt(3.f);
//        float interfaceAxis = -1; // Track axis for which t is smallest
//        for(int i = 0; i < 3; ++i) { // Iterate over the three axes to fing the closest point of intersection with the block grid
//            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
//                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
//                // If the player is *exactly* on an interface then
//                // they'll never move if they're looking in a negative direction
//                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
//                    offset = -1.f;
//                }
//                int nextIntercept = currCell[i] + offset;
//                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
//                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
//                if(axis_t < min_t) { // determinging the cell we are intersecting from the point on the axis of intersection
//                    min_t = axis_t; // this is now the closest axis intersection
//                    interfaceAxis = i; // track axis for which t is smallest
//                }
//            }
//        }
//        if(interfaceAxis == -1) {
//            std::cout << axis << std::endl;
//            std::cout << maxLen << std::endl;
//            std::cout << "rayDirection: {" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << "}" << std::endl;
//            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarchAxis!");
//        }
//        curr_t += min_t; // min_t is declared in slide 7 algorithm
//        rayOrigin += rayDirection * min_t;
//        glm::ivec3 offset = glm::ivec3(0,0,0);
//        // Sets it to 0 if sign is +, -1 if sign is -
//        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
//        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
//        // If currCell contains something other than EMPTY, return
//        // curr_t
//        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
//        if(cellType != EMPTY) {
//            *out_dist = glm::min(maxLen, curr_t);
//            return true;
//        }
//    }
//    *out_dist = glm::min(maxLen, curr_t);
//    return false;
//}

//void Player::checkCollision(const Terrain &terrain, InputBundle &inputs, float dT)
//{
//    float offset = 0.0001f; // so the player doesn't fall through the ground, and so it fits in a 2x1x1 space
//    std::map<int, char> my_map = {
//        { 0, 'x' },
//        { 1, 'y' },
//        { 2, 'z' }
//    };

//    for(int i = 0; i < 3; ++i) { // for each x, y, z axis
//        float distToMove = glm::abs(m_velocity[i] * dT); // how far the player should move along axis without collision (magnitude)
//        for(glm::vec3 corner : playerCorners) {
//            glm::vec3 projectedCorner = corner + m_position + m_velocity * dT; // project the corner along the chosen axis
//            float collisionDist = distToMove;
//            bool collision = gridMarchAxis(corner + m_position, projectedCorner - (corner + m_position), terrain, i, &collisionDist);
////            if (collision && i != 1) {
////                std::cout << "Collision in " << my_map[i] << std::endl;
////            }
//            collisionDist = collisionDist - offset;
//            if (collisionDist < distToMove) {
//            }
//            distToMove = glm::min(distToMove, collisionDist);
//        }
//        // 2. Only move along the ith axis of our velocity
//        glm::vec3 dir = glm::normalize(m_velocity);
////        glm::vec3 dir = glm::sign(m_velocity);ds
//        for(int k = 0; k < 3; ++k) {
//            if(k != i) {
//                dir[k] = 0.f;
//            }
//        }
//        moveAlongVector(dir * distToMove); // move along the vector i
//    }
//}


BlockType Player::checkOnGround(InputBundle &inputs)
{
    // Get the block type below the player (just barely below the foot of the player) to see if on the ground
    BlockType blockBelow = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z));
//    BlockType blockBelow = this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y - 0.05, this->m_position.z);

    if (blockBelow != EMPTY && blockBelow != WATER && blockBelow != LAVA) {
        inputs.onGround = true;
        //        m_velocity.y = 0;
        playerOnGround = true;
    } else {
        inputs.onGround = false;
        playerOnGround = false;
    }

//    // Alternative way, but slower
//    // Get the 4 corners of the players foot to check if any are on the ground
//    BlockType foot1 = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z));
//    BlockType foot2 = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x) + 1, this->m_position.y - 0.05, glm::floor(this->m_position.z));
//    BlockType foot3 = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z) + 1);
//    BlockType foot4 = this->mcr_terrain.getBlockAt(glm::floor(this->m_position.x) + 1, this->m_position.y - 0.05, glm::floor(this->m_position.z) + 1);
//    BlockType blockBelow;
//    std::array<BlockType, 4> majority_vote = {foot1, foot2, foot3, foot4}; // then find the most occuring. Too much work

//    bool condition1 = foot1 != EMPTY && foot2 != EMPTY && foot3 != EMPTY && foot4 != EMPTY;
//    bool condition2 = foot1 != WATER && foot2 != WATER && foot3 != WATER && foot4 != WATER;
//    bool condition3 = foot1 != LAVA && foot2 != LAVA && foot3 != LAVA && foot4 != LAVA;

//    if (condition1 && condition2 && condition3) {
//        inputs.onGround = true;
//        //        m_velocity.y = 0;
//        playerOnGround = true;
//    } else {
//        inputs.onGround = false;
//        playerOnGround = false;
//    }

    return blockBelow;
}

BlockType Player::checkInLiquid(InputBundle &inputs) {
    // Get the block type where the player is
    BlockType currBlock = this->mcr_terrain.getBlockAt(glm::floor(this->m_position));

    if (currBlock == WATER || currBlock == LAVA) {
        inputs.inLiquid = true;
        playerInLiquid = true;
    } else {
        inputs.inLiquid = false;
        playerInLiquid = false;
    }

    return currBlock;
}

//QString Player::camSight() {


//    // For Debugging in the player info panel
//    // Get the block type where the player's camera is and check if it is lava or water
//    int block = int(this->mcr_terrain.getBlockAt(this->m_position.x, this->m_position.y + 1.5f, this->m_position.z));
//    this->camSight
//    return type_enum_to_string[block];
//}

std::unordered_map<int, QString> type_enum_to_string = {
    { 0, "EMPTY" },
    { 1, "GRASS" },
    { 2, "DIRT" },
    { 3, "STONE" },
    { 4, "WATER" },
    { 5, "LAVA" },
    { 6, "ICE" },
    { 7, "SNOW" },
    { 8, "SAND" },
    { 9, "UNCERTAIN" },
    { 10, "BEDROCK" },
};

BlockType Player::headSpaceSight()
{
    // Get the block type where the player's camera is and check if it is lava or water
    //    BlockType block = this->mcr_terrain.getBlockAt(this->mcr_camera.mcr_position);
    BlockType block;
    try {
        block = this->mcr_terrain.getBlockAt(glm::floor(this->mcr_camera.mcr_position));

        this->camBlock = type_enum_to_string[block];
    } catch(const std::out_of_range &exception) {
        this->camBlock = "NOCHUNK";
        block = UNCERTAIN;
    }

    return block;
}

bool Player::checkIsLiquid(float x, float y, float z) {
    // Get the block type at coords to see if it is liquid
    BlockType currBlock = this->mcr_terrain.getBlockAt(x, y, z);

    if (currBlock == WATER || currBlock == LAVA) {
        return true;
    } else {
        return false;
    }
}

glm::ivec3 Player::getViewedBlockCoord(Terrain &terrain) {

    glm::vec3 cameraOrigin = this->m_camera.mcr_position;
    glm::vec3 rayCamera = 4.f * glm::normalize(this->m_forward); // cast the camera ray in the forward direction 3 blocks
    float out_dist = 0; // delcare input to grid march (how far away the collision is to the block, if at all)
    glm::ivec3 out_blockHit = glm::ivec3(); // declare the input to grid march (cell that we are colliding with, if any)

    if (terrain.hasChunkAt(cameraOrigin.x + rayCamera.x, cameraOrigin.z + rayCamera.z)) {
        // TODO: consider passing chunk VBO regeneration to a VBO worker thread
        if (gridMarch(cameraOrigin, rayCamera, terrain, &out_dist, &out_blockHit)) {
            return out_blockHit;
        }
    }
    return glm::ivec3(NULL);
}


BlockType Player::removeBlock(Terrain &terrain) {
    glm::vec3 cameraOrigin = this->m_camera.mcr_position; // the camera in position
    glm::vec3 rayCamera = 4.f * glm::normalize(this->m_forward); // cast the camera ray in the forward direction 3 blocks
    float out_dist = 0; // delcare input to grid march (how far away the collision is to the block, if at all)
    glm::ivec3 out_blockHit = glm::ivec3(); // declare the input to grid march (cell that we are colliding with, if any)

    // TODO: consider passing chunk VBO regeneration to a VBO worker thread
    if (gridMarch(cameraOrigin, rayCamera, terrain, &out_dist, &out_blockHit)) { // if there is a detected block
        BlockType block = terrain.getBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z); // get the block type that we clicked
        if (block != BEDROCK) {
            terrain.setBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z, EMPTY); // set the clicked blocktype to empty
            block = EMPTY; // reset the block to EMPTY so the player can't hold bedrock
        }
        const uPtr<Chunk> &chunk = terrain.getChunkAt(out_blockHit.x, out_blockHit.z);
        chunk->recreateVBOdata();

        for(auto &neighbor : terrain.getChunkAt(out_blockHit.x, out_blockHit.z)->m_neighbors) {
            neighbor.second->recreateVBOdata();
        }
        return block; // return the block type that was hit
    }
    return EMPTY; // if no block hit, return empty block
}

void Player::placeBlock(Terrain &terrain, BlockType &blockToPlace) {
    glm::vec3 cameraOrigin = this->m_camera.mcr_position; // the camera in position
    glm::vec3 rayCamera = 4.f * glm::normalize(this->m_forward); // cast the camera ray in the forward direction 3 blocks
    float out_dist = 0; // delcare input to grid march (how far away the collision is to the block, if at all)
    glm::ivec3 out_blockHit = glm::ivec3(); // declare the input to grid march (cell that we are colliding with, if any)
    float interfaceAxis; // to keep track of the face that we hit

    if (gridMarch(cameraOrigin, rayCamera, terrain, &out_dist, &out_blockHit, &interfaceAxis)) { // if there is a detected block
//      if (out_blockHit != glm::ivec3(glm::floor(this->m_position).x, glm::floor(this->m_position).y, glm::floor(this->m_position).z)) { // if not on the block
            if (interfaceAxis == 2) { // z-axis
                terrain.setBlockAt(out_blockHit.x, out_blockHit.y, out_blockHit.z - glm::sign(rayCamera.z), blockToPlace); // place block a small distance in front of the interface axis (otherwise replaces block)
            } else if (interfaceAxis == 1) { // y-axis
                terrain.setBlockAt(out_blockHit.x, out_blockHit.y - glm::sign(rayCamera.y), out_blockHit.z, blockToPlace); // place block a small distance in front of the interface axis (otherwise replaces block)
            } else if (interfaceAxis == 0) { // x-axis
                terrain.setBlockAt(out_blockHit.x - glm::sign(rayCamera.x), out_blockHit.y, out_blockHit.z, blockToPlace); // place block a small distance in front of the interface axis (otherwise replaces block)
            }
            // TODO: consider passing chunk VBO regeneration to a VBO worker thread
            const uPtr<Chunk> &chunk = terrain.getChunkAt(out_blockHit.x, out_blockHit.z);
            chunk->recreateVBOdata();

            for(auto &neighbor : terrain.getChunkAt(out_blockHit.x, out_blockHit.z)->m_neighbors) {
                neighbor.second->recreateVBOdata();
            }
//      }
    }
}

void Player::playSoundsGround(BlockType footBlock) {

    // Cut flying sounds
//    flying.stop();

    // Grass sounds
    if (glm::length(this->m_velocity) > 0.01 && (footBlock == GRASS || footBlock == DIRT)) { // if moving at a threshold of epsilon = 0.1
        walk_grass.setMuted(false);
        // dynamically set the volume based on player speed
        float volume = glm::length(this->m_velocity)/50;
        walk_grass.setVolume(volume);
        if (!walk_grass.isPlaying()) { // do not reloop if already playing
            walk_grass.play();
        }
    } else {
        walk_grass.setMuted(true); // instead of replaying the walking just mute it or "pause", sounds more natural
    }

    // Snow/Sand sounds
    if (glm::length(this->m_velocity) > 0.01 && (footBlock == SNOW || footBlock ==SAND)) { // if moving at a threshold of epsilon = 0.1
        walk_snow_sand.setMuted(false);
        // dynamically set the volume based on player speed
        walk_snow_sand.setVolume(0.75f);
        if (!walk_snow_sand.isPlaying()) { // do not reloop if already playing
            walk_snow_sand.play();
        }
    } else {
        walk_snow_sand.setMuted(true); // instead of replaying the walking just mute it or "pause", sounds more natural
    }

    // Stone sounds
    if (glm::length(this->m_velocity) > 0.01 && footBlock == STONE) { // if moving at a threshold of epsilon = 0.1
        walk_stone.setMuted(false);
        // dynamically set the volume based on player speed
        walk_stone.setVolume(0.01f);
        if (!walk_stone.isPlaying()) { // do not reloop if already playing
            walk_stone.play();
        }
    } else {
        walk_stone.setMuted(true); // instead of replaying the walking just mute it or "pause", sounds more natural
    }

    // Check headpace
    BlockType headSpace = headSpaceSight();
    BlockType currBlock = this->mcr_terrain.getBlockAt(glm::floor(this->m_position));

    // If in water or lava
    if (headSpace == WATER) {
        if (!swim_water.isPlaying()) {
            swim_water.play();
        }
    } else if (headSpace == LAVA || currBlock == LAVA) {
        if (!swim_lava.isPlaying()) {
            swim_lava.play();
        }
    } else {
        swim_water.stop();
        swim_lava.stop();
    }

    // If very high up
    if (this->mcr_camera.mcr_position.y >= 200) {
        if (!flying.isPlaying()) {
            flying.play();
        }
    } else {
        flying.stop();
    }
}

void Player::playSoundsFlight() {
    // Cut ground sounds
    walk_grass.stop();
    walk_snow_sand.stop();
    walk_stone.stop();
    swim_water.stop();
    swim_lava.stop();

    if (glm::length(this->m_velocity) > 0.01) {
        if (!flying.isPlaying()) {
            flying.play();
        }
    } else {
        flying.stop();
    }

}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    // The following logic ensures the player cant rotate past the global up vector
    if (glm::abs(this->lockDegree + degrees) < 90.f) {
        this->lockDegree += degrees;
        Entity::rotateOnRightLocal(degrees);
        m_camera.rotateOnRightLocal(degrees);
    } else {
        degrees = glm::sign(this->lockDegree) * 90 - this->lockDegree;
        this->lockDegree += degrees;
        Entity::rotateOnRightLocal(degrees);
        m_camera.rotateOnRightLocal(degrees);
    }
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}


