#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;

out vec4 finalColor;

void main() {
  vec4 renderColor = texture(texture0, fragTexCoord);
  vec4 texColor = texture(texture1, renderColor.rg);

  if (renderColor == vec4(0, 0, 0, 0)) {
    finalColor = vec4(0, 0, 0, 0);
  }
  else {
    finalColor = vec4(texColor.rgb * renderColor.a, 1);
  }
}
