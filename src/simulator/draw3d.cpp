//
// Created by Werner Kroneman on 27-01-16.
//

#include "draw3d.h"
#include <FL/gl.h>
#include <math.h>


/**
     *
     * Draw a sphere with radius r, using spherical coordinates as the texture coordinates.
     *
     * @param rings How many horizontal rings
     * @param segments How many vertical segments.
     */
void texturedSphere(float radius, int rings, int segments) {

    // Iterate over all the rings
    for (int ring = 0; ring < rings; ring++) {
        glBegin(GL_TRIANGLE_STRIP); // The rings are triangle strips.

        // Get the value between 0 and 1 how far along the curve we are
        float v1 = (float) ring / (float) rings;

        // Angle between the -Z axis and a vector on the current ring.
        float theta1 = M_PI * v1;
        // XY distance the normal from the Z axis
        float r1scale = sin(theta1);
        // XY distance of the vertex from the Z axis
        float r1 = radius * r1scale;
        // Z value of the normal
        float h1scale = -cos(theta1);
        // Z value of the vertex
        float h1 = radius * h1scale;

        // Repeat for the second parallel ring.
        float v2 = (float) (ring + 1) / (float) rings;

        float theta2 = M_PI * v2;
        float r2scale = sin(theta2);
        float r2 = radius * r2scale;
        float h2scale = -cos(theta2);
        float h2 = radius * h2scale;

        // For every segment, go back and forth between the first and second ring
        // so as to build up the triangle strip.
        for (int seg = 0; seg <= segments; seg++) {
            float u = (float) seg / (float) segments; // How far along the circle?
            float phi = M_PI * 2 * u; // Angle between the current point in the XY plane and +X axis

            glTexCoord2f(u, v1);
            // Normals are parralel to the vertex vectors in a sphere.
            glNormal3d(cos(phi) * r1scale, sin(phi) * r1scale, h1scale);
            // Simple sin/cos multiplied by radius
            glVertex3f(cos(phi) * r1, sin(phi) * r1, h1);

            // Same for upper ring.
            glTexCoord2f(u, v2);
            glNormal3d(cos(phi) * r2scale, sin(phi) * r2scale, h2scale);
            glVertex3d(cos(phi) * r2, sin(phi) * r2, h2);
        }

        glEnd();// End the triangle strip.
    }

}