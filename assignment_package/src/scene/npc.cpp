#include "npc.h"
#include "glm/gtx/vector_angle.hpp"
#include <queue>

NPC::NPC(glm::vec3 m_position, const Terrain &m_terrain, const Player &m_player, OpenGLContext* context) :
    Entity(m_position), m_player(m_player), m_terrain(m_terrain), context(context), m_velocity(0,0,0),
    m_acceleration(0,0,0), m_geomCube(context), root(nullptr),
    leftHandRotateRaw(nullptr), rightHandRotateRaw(nullptr), leftLegRotateRaw(nullptr),
    rightLegRotateRaw(nullptr), headRotateRaw(nullptr), torsoRotateRaw(nullptr), torsoScaleRaw(nullptr), leftHandScaleRaw(nullptr),
    rightHandScaleRaw(nullptr), leftLegScaleRaw(nullptr), rightLegScaleRaw(nullptr), headScaleRaw(nullptr),
    m_tryMotionTimer(0), is_npc_moving(false), doBFS(false), m_prev_position(m_position), prev_dir(FORWARD)
{}
bool NPC::checkOnGround()
{
    // Get the block type below the player (just barely below the foot of the player) to see if on the ground
    BlockType blockBelow = m_terrain.getBlockAt(glm::floor(this->m_position.x), this->m_position.y - 0.05, glm::floor(this->m_position.z));
    if (blockBelow != EMPTY && blockBelow != WATER && blockBelow != LAVA) {
        return true;
    }
    return false;
}
bool NPC::checkOnGround(int x, int y, int z)
{
    BlockType block = m_terrain.getBlockAt(x, y, z);
    BlockType blockBelow = m_terrain.getBlockAt(x, y-1, z);
    if (block == EMPTY && blockBelow != EMPTY) {
        return true;
    }
    return false;
}
bool NPC::checkPlayerOnGround()
{
    // Get the block type below the player (just barely below the foot of the player) to see if on the ground
    BlockType blockBelow = m_terrain.getBlockAt(glm::floor(m_player.m_position.x), m_player.m_position.y - 0.05, glm::floor(m_player.m_position.z));
    if (blockBelow != EMPTY && blockBelow != WATER && blockBelow != LAVA) {
        return true;
    }
    return false;
}
bool NPC::checkInLiquid() {
    // Get the block type where the player is
    BlockType currBlock = m_terrain.getBlockAt(glm::floor(this->m_position));
    if (currBlock == WATER || currBlock == LAVA) {
        return true;
    }
    return false;
}
bool NPC::checkIsLiquid(float x, float y, float z) {
    // Get the block type at coords to see if it is liquid
    BlockType currBlock = m_terrain.getBlockAt(x, y, z);

    if (currBlock == WATER || currBlock == LAVA) {
        return true;
    } else {
        return false;
    }
}
void NPC::moveNPC(float dT)
{
    m_tryMotionTimer += dT;
    float rateMultiplier = 6.f;
    float max_angle = 65.f;
    float zero_out_threshold = 3.f;
    if(is_npc_moving)
    {
        if((static_cast<RotateNode*>(leftHandRotateRaw))->axis != glm::vec3(1, 0, 0))
            (static_cast<RotateNode*>(leftHandRotateRaw))->axis = glm::vec3(1, 0, 0);
        if((static_cast<RotateNode*>(rightHandRotateRaw))->axis != glm::vec3(1, 0, 0))
            (static_cast<RotateNode*>(rightHandRotateRaw))->axis = glm::vec3(1, 0, 0);
        (static_cast<RotateNode*>(leftHandRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier + M_PI) * max_angle ;
        (static_cast<RotateNode*>(rightHandRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier) * max_angle; // Opposite sign as right leg
        (static_cast<RotateNode*>(leftLegRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier) * max_angle;
        (static_cast<RotateNode*>(rightLegRotateRaw))->angle = glm::sin(M_PI + m_tryMotionTimer * rateMultiplier) * max_angle; // Opposite sign as left leg
    }
    else
    {
        // Slowly move to default position and rotate with a very small angle in a different axis
        if((static_cast<RotateNode*>(leftHandRotateRaw)->axis == glm::vec3(1, 0, 0)))
        {
            if((static_cast<RotateNode*>(leftHandRotateRaw))->angle > zero_out_threshold)
            {
                (static_cast<RotateNode*>(leftHandRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier + M_PI) * max_angle ;
            }
            else
            {
                (static_cast<RotateNode*>(leftHandRotateRaw))->angle = 0;
                (static_cast<RotateNode*>(leftHandRotateRaw))->axis = glm::vec3(0, 0, 1);
            }
        }
        else
        {
            float angle = glm::sin(m_tryMotionTimer * rateMultiplier * 0.8 + M_PI) * max_angle/6;
            (static_cast<RotateNode*>(leftHandRotateRaw))->angle = angle > 0?-1*angle:angle;
        }
        if((static_cast<RotateNode*>(rightHandRotateRaw)->axis == glm::vec3(1, 0, 0)))
        {
            if((static_cast<RotateNode*>(rightHandRotateRaw))->angle > zero_out_threshold)
            {
                (static_cast<RotateNode*>(rightHandRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier) * max_angle; // Opposite sign as right leg
            }
            else
            {
                (static_cast<RotateNode*>(rightHandRotateRaw))->angle = 0;
                (static_cast<RotateNode*>(rightHandRotateRaw))->axis = glm::vec3(0, 0, 1);
            }
        }
        else
        {
            float angle = glm::sin(m_tryMotionTimer * rateMultiplier * 0.8 + M_PI) * max_angle/6;
            (static_cast<RotateNode*>(rightHandRotateRaw))->angle = angle < 0?-1*angle:angle;
        }
        if((static_cast<RotateNode*>(leftLegRotateRaw))->angle > zero_out_threshold)
        {
            (static_cast<RotateNode*>(leftLegRotateRaw))->angle = glm::sin(m_tryMotionTimer * rateMultiplier) * max_angle;
        }
        else
        {
            (static_cast<RotateNode*>(leftLegRotateRaw))->angle = 0;
        }
        if((static_cast<RotateNode*>(rightLegRotateRaw))->angle > zero_out_threshold)
        {
            (static_cast<RotateNode*>(rightLegRotateRaw))->angle = glm::sin(M_PI + m_tryMotionTimer * rateMultiplier) * max_angle; // Opposite sign as left leg
        }
        else
        {
            (static_cast<RotateNode*>(rightLegRotateRaw))->angle = 0;
        }
    }
    // Prevent large values inside sin
    if(m_tryMotionTimer > 100.f)
        m_tryMotionTimer = 0.f;
}
const static vector<glm::vec3> playerCorners {
    // NOTE: the player is shrunk a little bit to fit into small crevices
    // Bottom
    glm::vec3(-0.25, 0.0, -0.25),
    glm::vec3(0.25, 0.0, -0.25),
    glm::vec3(0.25, 0.0, 0.25),
    glm::vec3(-0.25, 0.0, 0.25),
    // Mid
    glm::vec3(-0.25, 0.95, -0.25),
    glm::vec3(0.25, 0.95, -0.25),
    glm::vec3(0.25, 0.95, 0.25),
    glm::vec3(-0.25, 0.95, 0.25),
    // Top
    glm::vec3(-0.25, 1.9, -0.25),
    glm::vec3(0.25, 1.9, -0.25),
    glm::vec3(0.25, 1.9, 0.25),
    glm::vec3(-0.25, 1.9, 0.25),
};

bool NPC::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, float *out_dist, glm::ivec3 *out_blockHit, float *interfaceAxisPtr) {
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
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch (npc.cpp)!");
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
        BlockType cellType = m_terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

std::array<bool, 3> NPC::checkCollision(glm::vec3 &rayDirection)
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

            if (gridMarch(castedRayOrigin, rayX, &out_dist, &out_blockHit)) { // if there is a collision in x
                if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                    rayDirection.x = glm::sign(rayDirection.x) * glm::max((out_dist - offset), 0.f);
                    if (out_dist <= offset) {
                        collidedWithAxis[0] = true;
                    }
                }
            }
            if (gridMarch(castedRayOrigin, rayY, &out_dist, &out_blockHit)) { // if there is a collision in y
                if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                    rayDirection.y = glm::sign(rayDirection.y) * glm::max((out_dist - offset), 0.f);
                    //cout<<"Out distance is "<<out_dist<<endl;
                    if (out_dist <= offset) {
                        collidedWithAxis[1] = true;
                    }
                }
            }

            if (gridMarch(castedRayOrigin, rayZ, &out_dist, &out_blockHit)) { // if there is a collision in z
                if (!checkIsLiquid(out_blockHit.x, out_blockHit.y, out_blockHit.z)) { // dont collide with liquid
                    rayDirection.z = glm::sign(rayDirection.z) * glm::max((out_dist - offset), 0.f);
                    if (out_dist <= offset) {
                        collidedWithAxis[2] = true;
                    }
                }
            }
        }
        return collidedWithAxis;
}
void NPC::computePhysics(float dT)
{
    float tune_max_speed = 10.f; // approxmately the max speed of usain bolt in 1 direction
    this->m_velocity.x *= 0.85f; // reduce velocity for friction and drag (slow down on release)
    this->m_velocity.y *= 0.975f; // reduce velocity for friction and drag (slow down on release) -- allow terminal velocity to be reached
    this->m_velocity.z *= 0.85f; // reduce velocity for friction and drag (slow down on release)
    float terminal_speed = 6.6 * tune_max_speed;

    this->m_velocity = this->m_velocity + this->m_acceleration * dT; // kinematics equation for all dirs
    // Ensure not going too fast
    glm::clamp(this->m_velocity.x, -tune_max_speed, tune_max_speed);
    glm::clamp(this->m_velocity.z, -tune_max_speed, tune_max_speed);
    glm::clamp(this->m_velocity.y, -terminal_speed, terminal_speed);
    if (checkInLiquid()) {
        m_velocity *= 0.66f; // reduce speed in liquids by 2/3
    }

    // Check if there is a collision
    glm::vec3 posRayDir = this->m_velocity * dT; // position vector (slides say length == speed)
    std::array<bool, 3> collidedAxesMask = checkCollision(posRayDir);
    if(glm::abs(posRayDir.x / dT) >= 0.1f || glm::abs(posRayDir.z/ dT) >= 0.1f)
    {
        is_npc_moving = true;
    }
    else
    {
        is_npc_moving = false;
    }
    this->moveAlongVector(posRayDir); // move along the position vector
    this->m_velocity = this->m_velocity * glm::vec3(!collidedAxesMask[0], !collidedAxesMask[1], !collidedAxesMask[2]); // zero out the velocity vector is colliding with solid block
}
void NPC::moveInGivenDirection(NPCDirection direction)
{
    this->m_acceleration = glm::vec3(0, 0, 0); // ensure we dont accidentally keep accelerating
    // Update the players member variables. Ensure gravity is on
    float accel_scaler = 8; // ultimately controls speed. Scales the ussain bolt and gravity values the same degree
    float gravity = 9.81 * accel_scaler;
    float tune_max_accel =  9.5;
    tune_max_accel *= accel_scaler;
    if (direction == FORWARD) {
        this->m_acceleration += tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
    }
    if (direction == BACKWARD) {
        this->m_acceleration -= tune_max_accel * glm::cross(glm::vec3(0, 1, 0), this->m_right);
    }
    if (direction == RIGHT) {
        this->m_acceleration += tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
    }
    if (direction == LEFT) {
        this->m_acceleration -= tune_max_accel * glm::vec3(this->m_right.x, 0.f, this->m_right.z); // zero out the y component of the acceleration
    }

    // ensure we dont accelerate way too fast
    this->m_acceleration = glm::clamp(this->m_acceleration, -tune_max_accel, tune_max_accel);

    // include the gravity component
    if(!checkInLiquid()) {
        this->m_acceleration -= glm::vec3(0, gravity, 0);
    } else {
        this->m_acceleration -= glm::vec3(0, gravity, 0);
        this->m_acceleration = glm::vec3(m_acceleration.x * 4/5, m_acceleration.y * 4/5, m_acceleration.z * 4/5);
    }

    if (direction == JUMP) {
        // jump if on the ground
        if (checkOnGround() && !checkInLiquid()) {
            this->m_acceleration.y = this->m_acceleration.y + 15*gravity;
        } else if (checkInLiquid()) {
            this->m_acceleration.y = -m_acceleration.y; // sink and swim at same acceleration
        }
    }
}
void NPC::rotatePlayer(NPCDirection curr_dir)
{
    if((prev_dir == FORWARD && curr_dir == LEFT) || (prev_dir == LEFT && curr_dir == BACKWARD)
            || (prev_dir == BACKWARD && curr_dir == RIGHT) || (prev_dir == RIGHT && curr_dir == FORWARD))
        (static_cast<RotateNode*>(torsoRotateRaw))->angle += glm::degrees(90.f);
    else if((prev_dir == FORWARD && curr_dir == RIGHT) || (prev_dir == RIGHT && curr_dir == BACKWARD)
                || (prev_dir == BACKWARD && curr_dir == LEFT) || (prev_dir == LEFT && curr_dir == FORWARD))
        (static_cast<RotateNode*>(torsoRotateRaw))->angle += glm::degrees(-90.f);
    else if((prev_dir == FORWARD && curr_dir == BACKWARD) || (prev_dir == LEFT && curr_dir == RIGHT)
         || (prev_dir == BACKWARD && curr_dir == FORWARD) || (prev_dir == RIGHT && curr_dir == LEFT))
        (static_cast<RotateNode*>(torsoRotateRaw))->angle += glm::degrees(180.f);
}
void NPC::processInputs(InputBundle &inputs)
{
    glm::vec3 player_to_npc_dir = m_player.m_position - m_position;
    float player_to_npc_dist = glm::length(player_to_npc_dir);
    float min_player_to_npc_dist = 1.f, max_player_to_npc_dist = 20.f;
    glm::vec3 y_axis = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 source = glm::vec3(m_position);
    glm::ivec3 source_int = glm::ivec3(m_position);
    if(player_to_npc_dist >= min_player_to_npc_dist && player_to_npc_dist <= max_player_to_npc_dist)
    {
        vector<glm::ivec3> directions = {glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)};
        if(pathToPlayer.empty() && !m_player.flightMode && checkPlayerOnGround() && doBFS)
        {
            // BFS: https://www.geeksforgeeks.org/check-possible-path-2d-matrix/
            glm::ivec3 target = glm::ivec3(m_player.m_position);
            vector<vector<int>> status(max_player_to_npc_dist * 2 + 1, vector<int>(max_player_to_npc_dist * 2 + 1, 0));
            vector<vector<glm::ivec2>> backtrack(max_player_to_npc_dist * 2 + 1, vector<glm::ivec2>(max_player_to_npc_dist * 2 + 1, glm::ivec2(INT_MAX, INT_MAX)));
            glm::ivec3 status_vector_offset = glm::ivec3(max_player_to_npc_dist, m_position.y, max_player_to_npc_dist) - source_int;
            // Status: -1: invalid, 0 unvisited, 1 visited
            status[max_player_to_npc_dist][max_player_to_npc_dist] = 1;
            glm::ivec2 current_cell = glm::ivec2((target + status_vector_offset).x, (target + status_vector_offset).z);
            int current_y = int(target.y);
            std::queue<glm::ivec3> bfs_queue;
            bfs_queue.push(source_int);
            while(!bfs_queue.empty())
            {
                if(backtrack[current_cell.x][current_cell.y] != glm::ivec2(INT_MAX, INT_MAX))
                    break;
                glm::ivec3 front = bfs_queue.front();
                glm::ivec3 status_front = front + status_vector_offset;
                bfs_queue.pop();
                for(int i = 0; i< directions.size(); i++)
                {
                    glm::ivec3 neighbor = front + directions[i];
                    glm::ivec3 status_neighbor = neighbor + status_vector_offset;
                    if(status_neighbor.x >= 0 && status_neighbor.x < max_player_to_npc_dist * 2 + 1
                       && status_neighbor.z >= 0 && status_neighbor.z < max_player_to_npc_dist * 2 + 1)
                    {
                        if(status[status_neighbor.x][status_neighbor.z] != -1)
                        {
                            if (m_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) == EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y + 1, neighbor.z) == EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y - 1, neighbor.z) != EMPTY)
                            {
                                if (status[status_neighbor.x][status_neighbor.z] == 0)
                                {
                                    bfs_queue.push(neighbor);
                                    status[status_neighbor.x][status_neighbor.z] = 1;
                                    backtrack[status_neighbor.x][status_neighbor.z] = glm::vec2(status_front.x, status_front.z);
                                }
                            }
                            else if (m_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) != EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y + 1, neighbor.z) == EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y + 2, neighbor.z) == EMPTY)
                            {
                                if (status[status_neighbor.x][status_neighbor.z] == 0)
                                {
                                    bfs_queue.push(glm::ivec3(neighbor.x, neighbor.y + 1, neighbor.z));
                                    status[status_neighbor.x][status_neighbor.z] = 1;
                                    backtrack[status_neighbor.x][status_neighbor.z] = glm::vec2(status_front.x, status_front.z);
                                }
                            }
                            else if (m_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) == EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y - 1, neighbor.z) == EMPTY
                                && m_terrain.getBlockAt(neighbor.x, neighbor.y - 2, neighbor.z) != EMPTY)
                            {
                                if (status[status_neighbor.x][status_neighbor.z] == 0)
                                {
                                    bfs_queue.push(glm::ivec3(neighbor.x, neighbor.y - 1, neighbor.z));
                                    status[status_neighbor.x][status_neighbor.z] = 1;
                                    backtrack[status_neighbor.x][status_neighbor.z] = glm::vec2(status_front.x, status_front.z);
                                }
                            }
                            else
                            {
                                status[status_neighbor.x][status_neighbor.z] = -1;
                            }
                        }
                    }
                }
            }
            if(backtrack[current_cell.x][current_cell.y] != glm::ivec2(INT_MAX, INT_MAX))
            {
                while(current_cell != glm::ivec2(max_player_to_npc_dist, max_player_to_npc_dist))
                {
                    glm::ivec2 dir = current_cell - backtrack[current_cell.x][current_cell.y];
                    glm::ivec3 prev_world_cell = glm::ivec3(backtrack[current_cell.x][current_cell.y].x-status_vector_offset.x, current_y, backtrack[current_cell.x][current_cell.y].y-status_vector_offset.z);
                    bool isJump = false;
                    if(checkOnGround(prev_world_cell.x, prev_world_cell.y - 1, prev_world_cell.z))
                    {
                        isJump = true;
                        current_y -= 1;
                    }
                    else if(checkOnGround(prev_world_cell.x, prev_world_cell.y + 1, prev_world_cell.z))
                    {
                        current_y += 1;
                    }
                    if(isJump)
                            pathToPlayer.push_front(STAY);
                    if(dir == glm::ivec2(0, 1))
                            pathToPlayer.push_front(BACKWARD);
                    else if(dir == glm::ivec2(0, -1))
                            pathToPlayer.push_front(FORWARD);
                    else if(dir == glm::ivec2(1, 0))
                            pathToPlayer.push_front(RIGHT);
                    else if(dir == glm::ivec2(-1, 0))
                            pathToPlayer.push_front(LEFT);
                    if(isJump || checkIsLiquid(prev_world_cell.x, current_y, prev_world_cell.z) || checkIsLiquid(prev_world_cell.x, current_y-1, prev_world_cell.z))
                            pathToPlayer.push_front(JUMP);
                    current_cell = glm::ivec2(backtrack[current_cell.x][current_cell.y]);
                }
                if(pathToPlayer.front() != JUMP && pathToPlayer.front() != STAY && pathToPlayer.front() != prev_dir)
                {
                    rotatePlayer(pathToPlayer.front());
                    prev_dir = pathToPlayer.front();
                }
            }
            else
            {
                doBFS = false;
            }
        }
    }
    if(doBFS && !pathToPlayer.empty())
    {
        NPCDirection dir = pathToPlayer.front();
        moveInGivenDirection(dir);
        if((dir == BACKWARD && source.z - m_prev_position.z >= 0.9f)
           ||(dir == FORWARD && source.z - m_prev_position.z <= -0.9f)
           ||(dir == LEFT && source.x - m_prev_position.x <= -0.9f)
           ||(dir == RIGHT && source.x - m_prev_position.x >= 0.9f)
           ||(!checkInLiquid() && dir == JUMP) || (dir == STAY && checkOnGround()) || (checkInLiquid() && dir == JUMP && source.y - m_prev_position.y >= 0.9f))
        {
            if(pathToPlayer.front() == JUMP || pathToPlayer.front() == STAY)
                m_prev_position = glm::vec3(m_prev_position.x, source.y, m_prev_position.z);
            else
                m_prev_position = source;
            pathToPlayer.pop_front();
            if(pathToPlayer.front() != JUMP && pathToPlayer.front() != STAY && pathToPlayer.front() != prev_dir)
            {
                rotatePlayer(pathToPlayer.front());
                prev_dir = pathToPlayer.front();
            }
        }
        if(pathToPlayer.size() == 1)
        {
            if(pathToPlayer.front() != JUMP && pathToPlayer.front() != STAY && pathToPlayer.front() != prev_dir)
            {
                rotatePlayer(pathToPlayer.front());
                prev_dir = pathToPlayer.front();
            }
            pathToPlayer.clear();
            moveInGivenDirection(STAY);
            doBFS = false;
        }
    }
}
void NPC::tick(float dT, InputBundle &input)
{
        moveNPC(dT);
        processInputs(input);
        computePhysics(dT);
}
void NPC::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    if (root != nullptr) {
        (static_cast<TranslateNode*>(root.get()))->translation = glm::vec3(m_position.x, m_position.y + 1.1f, m_position.z);
    }
}

void NPC::constructSceneGraph()
{
    // Construct Torso
    root = mkU<TranslateNode>(m_position.x, m_position.y + 1.1f, m_position.z);
    uPtr<Node> torsoRotate = mkU<RotateNode>(0.f, glm::vec3(0.f, 1.f, 0.f));
    torsoRotateRaw = torsoRotate.get();
    Node& torsoRotateRef = root->addChild(std::move(torsoRotate));    
    uPtr<Node> torsoScale = mkU<ScaleNode>(0.5f, 0.75f, 0.25f, &m_geomCube, true, TORSO);
    torsoScaleRaw = torsoScale.get();
    Node& torsoScaleRef = torsoRotateRef.addChild(std::move(torsoScale));

    // Construct head
    uPtr<Node> headTranslate = mkU<TranslateNode>(0.f, 0.625f, 0.f);
    Node& headTranslateRef = torsoRotateRef.addChild(std::move(headTranslate));
    uPtr<Node> headRotate = mkU<RotateNode>(0.f, glm::vec3(0.f, 1.f, 0.f));
    headRotateRaw = headRotate.get();
    Node& headRotateRef = headTranslateRef.addChild(std::move(headRotate));
    uPtr<Node> headScale = mkU<ScaleNode>(0.5f, 0.5f, 0.5f, &m_geomCube, true, HEAD);
    headScaleRaw = headScale.get();
    Node& headScaleRef = headRotateRef.addChild(std::move(headScale));

    // Construct Left Hand
    uPtr<Node> leftHandTranslate = mkU<TranslateNode>(-0.375f, 0.25f, 0.f);
    Node& leftHandTranslateRef = torsoRotateRef.addChild(std::move(leftHandTranslate));
    uPtr<Node> leftHandRotate = mkU<RotateNode>();
    leftHandRotateRaw = leftHandRotate.get();
    Node& leftHandRotateRef = leftHandTranslateRef.addChild(std::move(leftHandRotate));
    uPtr<Node> leftHandTranslate2 = mkU<TranslateNode>(0.f, -0.25f, 0.f);
    Node& leftHandTranslate2Ref = leftHandRotateRef.addChild(std::move(leftHandTranslate2));
    uPtr<Node> leftHandScale = mkU<ScaleNode>(0.25f, 0.75f, 0.25f, &m_geomCube, true, LEFT_HAND);
    leftHandScaleRaw = leftHandScale.get();
    Node& leftHandScaleRef = leftHandTranslate2Ref.addChild(std::move(leftHandScale));

    // Construct Right Hand
    uPtr<Node> rightHandTranslate = mkU<TranslateNode>(0.375f, 0.25f, 0.f);
    Node& rightHandTranslateRef = torsoRotateRef.addChild(std::move(rightHandTranslate));
    uPtr<Node> rightHandRotate = mkU<RotateNode>();
    rightHandRotateRaw = rightHandRotate.get();
    Node& rightHandRotateRef = rightHandTranslateRef.addChild(std::move(rightHandRotate));
    uPtr<Node> rightHandTranslate2 = mkU<TranslateNode>(0.f, -0.25f, 0.f);
    Node& rightHandTranslate2Ref = rightHandRotateRef.addChild(std::move(rightHandTranslate2));
    uPtr<Node> rightHandScale = mkU<ScaleNode>(0.25f, 0.75f, 0.25f, &m_geomCube, true, RIGHT_HAND);
    rightHandScaleRaw = rightHandScale.get();
    Node& rightHandScaleRef = rightHandTranslate2Ref.addChild(std::move(rightHandScale));


//     Construct Left Leg
    uPtr<Node> leftLegTranslate = mkU<TranslateNode>(-0.125f, -0.25f, 0.f);
    Node& leftLegTranslateRef = torsoRotateRef.addChild(std::move(leftLegTranslate));
    uPtr<Node> leftLegRotate = mkU<RotateNode>();
    leftLegRotateRaw = leftLegRotate.get();
    Node& leftLegRotateRef = leftLegTranslateRef.addChild(std::move(leftLegRotate));
    uPtr<Node> leftLegTranslate2 = mkU<TranslateNode>(0.f, -0.5f, 0.f);
    Node& leftLegTranslate2Ref = leftLegRotateRef.addChild(std::move(leftLegTranslate2));
    uPtr<Node> leftLegScale = mkU<ScaleNode>(0.25f, 0.75f, 0.25f, &m_geomCube, true, LEFT_LEG);
    leftLegScaleRaw = leftLegScale.get();
    Node& leftLegScaleRef = leftLegTranslate2Ref.addChild(std::move(leftLegScale));


    // Construct Right Leg
    uPtr<Node> rightLegTranslate = mkU<TranslateNode>(0.125f, -0.25f, 0.f);
    Node& rightLegTranslateRef = torsoRotateRef.addChild(std::move(rightLegTranslate));
    uPtr<Node> rightLegRotate = mkU<RotateNode>();
    rightLegRotateRaw = rightLegRotate.get();
    Node& rightLegRotateRef = rightLegTranslateRef.addChild(std::move(rightLegRotate));
    uPtr<Node> rightLegTranslate2 = mkU<TranslateNode>(0.f, -0.5f, 0.f);
    Node& rightLegTranslate2Ref = rightLegRotateRef.addChild(std::move(rightLegTranslate2));
    uPtr<Node> rightLegScale = mkU<ScaleNode>(0.25f, 0.75f, 0.25f, &m_geomCube, true, RIGHT_LEG);
    rightLegScaleRaw = rightLegScale.get();
    Node& rightLegScaleRef = rightLegTranslate2Ref.addChild(std::move(rightLegScale));
}
NPC::~NPC()
{}
