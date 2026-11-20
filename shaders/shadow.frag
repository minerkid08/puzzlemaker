#version 330 core

uniform sampler2D textures[15];

in vec2 iuv;
flat in int imatId;

out vec4 color;

void main()
{
  vec4 texColor = vec4(0, 0, 0, 1);
  switch (imatId)
  {
    case 0:
    texColor = texture(textures[0], iuv);
    break;
    case 1:
    texColor = texture(textures[1], iuv);
    break;
    case 2:
    texColor = texture(textures[2], iuv);
    break;
    case 3:
    texColor = texture(textures[3], iuv);
    break;
    case 4:
    texColor = texture(textures[4], iuv);
    break;
    case 5:
    texColor = texture(textures[5], iuv);
    break;
    case 6:
    texColor = texture(textures[6], iuv);
    break;
    case 7:
    texColor = texture(textures[7], iuv);
    break;
    case 8:
    texColor = texture(textures[8], iuv);
    break;
    case 9:
    texColor = texture(textures[9], iuv);
    break;
    case 10:
    texColor = texture(textures[10], iuv);
    break;
    case 11:
    texColor = texture(textures[11], iuv);
    break;
    case 12:
    texColor = texture(textures[12], iuv);
    break;
    case 13:
    texColor = texture(textures[13], iuv);
    break;
    case 14:
    texColor = texture(textures[14], iuv);
    break;
  }
  if (texColor.a < 1)
    discard;

  float nearPlane = 0.1;
  float farPlane = 20;
  float z = gl_FragCoord.z * 2.0 - 1.0;
  color = vec4((2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane)) / farPlane);
	color.a = 1;
}
