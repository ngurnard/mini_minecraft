#include "cloud.h"

Cloud::Cloud(OpenGLContext* context)
    : Drawable(context), m_bounds(0, 16, 0, 16)
{}

Cloud::~Cloud()
{}

GLenum Cloud::drawMode() {
    return GL_TRIANGLES;
}

void Cloud::createVBOdata() {

    this->destroyVBOdata();
    std::cout << "in m_cloud createVBOdata()" << std::endl;
    std::vector<glm::vec4> pos, col;
    std::vector<GLuint> idx;

    int totalV = 0;

    for (int x = m_bounds[0]; x < m_bounds[1]; x++)
    {
        for (int z = m_bounds[2]; z < m_bounds[3]; z++)
        {
            for (int i = x; i <= x+1; i++)
            {
                for (int j = z; j <= z+1; j++)
                {
                    col.push_back(glm::vec4(1.f, 1.f, 1.f, 1.f)); // fill white cloud color
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

void Cloud::setBounds(int xmin, int xmax, int zmin, int zmax) {
    m_bounds = glm::ivec4(xmin, xmax, zmin, zmax);
}

