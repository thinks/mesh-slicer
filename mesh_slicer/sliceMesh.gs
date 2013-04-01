#version 420 core

layout(triangles) in;
layout(line_strip, max_vertices = 2) out;

layout(std140) uniform Camera {
  mat4 viewMatrix;
  mat4 projectionMatrix;
};

layout(std140) uniform Model {
  mat4 modelMatrix;
  mat4 normalMatrix;
};

float 
pointPlaneDistance(in vec3 point,
                   in vec3 planeNormal,
                   in float planeDistance) {
  return dot(planeNormal, point) + planeDistance;
}

vec3 
edgePlaneIntersection(in vec3 p1, in vec3 p2, in float d1, in float d2) {
  float t = d1/(d1 - d2);
  return p1 + t*(p2 - p1);
}
 
void 
main(void) {
  const vec3 VIEW_PLANE_NORMAL = vec3(0.0, 0.0, 1.0);
  const float VIEW_PLANE_DISTANCE = 0.0;

  mat4 viewModelMatrix = viewMatrix*modelMatrix;
  vec4 esx0 = viewModelMatrix*gl_in[0].gl_Position;
  vec4 esx1 = viewModelMatrix*gl_in[1].gl_Position;
  vec4 esx2 = viewModelMatrix*gl_in[2].gl_Position;
  float d0 = pointPlaneDistance(esx0.xyz, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE); 
  float d1 = pointPlaneDistance(esx1.xyz, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE);
  float d2 = pointPlaneDistance(esx2.xyz, VIEW_PLANE_NORMAL, VIEW_PLANE_DISTANCE);

  // All triangle vertices on the positive side of the plane.
  if (d0 > 0.0 && d1 > 0.0 && d2 > 0.0) {
    return;
  }

  // All triangle vertices on the negative side of the plane.
  if (d0 < 0.0 && d1 < 0.0 && d2 < 0.0) {
    return;
  }

  // All triangle vertices on the plane. Cannot output a line segment.
  if (d0 == 0.0 && d1 == 0.0 && d2 == 0.0) {
    return;
  }

  // Edge0 lies in plane. Output edge0 as a line.
  if (d0 == 0.0 && d1 == 0.0) {
	gl_Position = projectionMatrix*esx0;
    EmitVertex();
	gl_Position = projectionMatrix*esx1;
    EmitVertex();
    EndPrimitive();
    return;
  }

  // Edge1 lies in plane. Output edge1 as a line.
  if (d1 == 0.0 && d2 == 0.0) {
	gl_Position = projectionMatrix*esx1;
    EmitVertex();
	gl_Position = projectionMatrix*esx2;
    EmitVertex();
    EndPrimitive();
    return;
  }

  // Edge2 lies in plane. Output edge2 as a line.
  if (d2 == 0.0 && d0 == 0.0) {
	gl_Position = projectionMatrix*esx2;
    EmitVertex();
	gl_Position = projectionMatrix*esx0;
    EmitVertex();
    EndPrimitive();
    return;
  }

  // Vertex0 lies in the plane.
  if (d0 == 0.0) {
	gl_Position = projectionMatrix*esx0;
    EmitVertex();
    EmitVertex();
    EndPrimitive();
    return;
  }

  // Vertex1 lies in the plane.
  if (d1 == 0.0) {
	gl_Position = projectionMatrix*esx1;
    EmitVertex();
    EmitVertex();
    EndPrimitive();
    return;
  }

  // Vertex2 lies in the plane.
  if (d2 == 0.0) {
	gl_Position = projectionMatrix*esx2;
    EmitVertex();
    EmitVertex();
    EndPrimitive();
    return;
  }

  int lineIndex = 0;
  vec4 edgeIntersections[2];

  // Edge0 intersects plane.
  if (sign(d0) != sign(d1)) {
    edgeIntersections[lineIndex] = 
      vec4(edgePlaneIntersection(esx0.xyz, esx1.xyz, d0, d1), 1.0);
    ++lineIndex;
  }

  // Edge1 intersects plane.
  if (sign(d1) != sign(d2)) {
    edgeIntersections[lineIndex] = 
      vec4(edgePlaneIntersection(esx1.xyz, esx2.xyz, d1, d2), 1.0);
    ++lineIndex;
  }

  // Edge2 intersects plane.
  if (sign(d2) != sign(d0)) {
    edgeIntersections[lineIndex] = 
      vec4(edgePlaneIntersection(esx2.xyz, esx0.xyz, d2, d0), 1.0);
    ++lineIndex;
  }

  gl_Position = projectionMatrix*edgeIntersections[0];
  EmitVertex();
  gl_Position = projectionMatrix*edgeIntersections[1];
  EmitVertex();
  EndPrimitive();
}
