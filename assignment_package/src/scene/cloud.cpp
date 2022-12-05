#include "cloud.h"

Cloud::Cloud(OpenGLContext* context, Noise* N)
    : Drawable(context), m_offset({1000, 1000}), noise(N)
{}

Cloud::~Cloud()
{}

GLenum Cloud::drawMode() {
    return GL_TRIANGLES;
}

void Cloud::createVBOdata() {

    this->destroyVBOdata();

    std::vector<glm::vec4> pos, col;
    std::vector<GLuint> idx;

    int totalV = 0;

    for (int x = m_bounds[0]; x < m_bounds[1]; x++)
    {
        for (int z = m_bounds[2]; z < m_bounds[3]; z++)
        {
            int xo = x + m_offset.x;
            int zo = z + m_offset.y;

            int H = noise->computeHeight(xo, zo).first;

            if(H >= m_threshold)
            {

                float alpha = (float)(H - m_threshold)/(float)(256 - m_threshold);

                for (int i = x; i <= x+1; i++)
                {
                    for (int j = z; j <= z+1; j++)
                    {
                        col.push_back(glm::vec4(1.f, 1.f, 1.f, alpha)); // fill white cloud color
                        pos.push_back(glm::vec4(i, m_height, j, 1.f)); // populate vert pos
                    }
                }
                // Fan triangulate quad
                idx.push_back(totalV);
                idx.push_back(totalV + 3);
                idx.push_back(totalV + 1);
                idx.push_back(totalV);
                idx.push_back(totalV + 2);
                idx.push_back(totalV + 3);
                totalV += 4;
            }
        }
    }

    this->m_countOpq = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

void Cloud::setHeight(int h) {
    m_height = h;
}

void Cloud::setThreshold(int t) {
    m_threshold = t;
}

void Cloud::setBounds(int xmin, int xmax, int zmin, int zmax) {
    m_bounds = glm::ivec4(xmin, xmax, zmin, zmax);
}

void Cloud::incrementOffset(glm::ivec2 increment) {
    m_offset += increment;
}
