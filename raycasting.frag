#version 330

in vec2 fragTexCoord;

uniform sampler2D mapTex;
uniform vec4 floorCeilColor;
uniform vec4 fogColor;
uniform vec2 pos;
uniform vec2 zBounds;
uniform float rot;
uniform float fov;
uniform float heightRatio;
uniform float stepSize;
uniform int numSteps;

out vec4 finalColor;

void main()
{
  float yaw = (fragTexCoord.x - 0.5) * fov + rot;
  float pitch = (fragTexCoord.y - 0.5) * (heightRatio * fov);

  float yawSin = sin(yaw);
  float yawCos = cos(yaw);
  float pitchSin = sin(pitch);
  float pitchCos = cos(pitch);

  mat3 yawMatrix = mat3(
    yawCos, yawSin, 0,
    -yawSin, yawCos, 0,
    0, 0, 1
  );

  mat3 pitchMatrix = mat3(
    pitchCos, 0, -pitchSin,
    0, 1, 0,
    pitchSin, 0, pitchCos
  );

  vec3 rayStep = pitchMatrix * vec3(1, 0, 0);
  rayStep = yawMatrix * rayStep * stepSize;
  vec3 rayPos = vec3(pos, 0);

  finalColor = vec4(0, 0, 0, 1);

  vec4 color = vec4(0, 0, 0, 1);
  
  for (float step = 0; step < numSteps; step += 1) {
    rayPos += rayStep;
    color = texture(mapTex, rayPos.xy);
    if (finalColor == vec4(0, 0, 0, 1)) {
      if ((rayPos.z < zBounds.x || rayPos.z > zBounds.y ) && color == vec4(0, 0, 0, 1)) {
        finalColor = mix(floorCeilColor, fogColor, step / numSteps);
        break;
      }
      else {
        finalColor = mix(color, fogColor, step / numSteps);
      }
    }
  }
}
