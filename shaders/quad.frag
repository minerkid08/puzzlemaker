#version 330 core

uniform sampler2D textures[16];
uniform int pass;
uniform vec3 camPos;
uniform int lightingPreview;

in vec2 iuv;
in vec3 fragPos;
in vec4 itint;
in vec3 inorm;
flat in int imatId;

out vec4 color;

struct Light
{
  vec3 position;
  float constant;
  float linear;
  float quadratic;
  float cutoff;
  vec3 ambient;
  float dirx;
  vec3 diffuse;
  float diry;
  vec3 specular;
  float dirz;
};

layout(std140) uniform LightBlock
{
  Light lights[128];
};

vec3 calcDirectionalLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);

  vec3 dir = vec3(light.dirx, light.diry, light.dirz);
  float theta = dot(lightDir, -dir);
  float distance = length(light.position - fragPos);

	float constant = light.constant;
	float linear = light.linear / distance;
	float quadratic = light.quadratic / (distance * distance);

  float attenuation = constant + linear + quadratic;

  vec3 ambient = light.ambient;
  ambient *= attenuation;
  if (theta > light.cutoff)
  {
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.4);
    // attenuation
    // combine results
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
  }
  else
    return ambient;
}

vec3 calcLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.4);
  // attenuation
  float distance = length(light.position - fragPos);

	float constant = light.constant;
	float linear = light.linear / distance;
	float quadratic = light.quadratic / (distance * distance);

  float attenuation = constant + linear + quadratic;
  // combine results
  vec3 ambient = light.ambient;
  vec3 diffuse = light.diffuse * diff;
  vec3 specular = light.specular * spec;
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}

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
    case 15:
    texColor = texture(textures[15], iuv);
    break;
  }
  texColor.x *= itint.x;
  texColor.y *= itint.y;
  texColor.z *= itint.z;
  if (pass == 0)
  {
    if (texColor.a < 1)
      discard;
  }
  else
  {
    if (texColor.a == 1)
      discard;
  }

  if (lightingPreview == 1)
  {
    vec3 viewDir = normalize(camPos - fragPos);

    vec3 result = vec3(0, 0, 0);
    for (int i = 0; i < 128; i++)
    {
      if (lights[i].position.x < 0)
        continue;
      if (lights[i].cutoff == 0)
        result += calcLight(lights[i], inorm, fragPos, viewDir);
      else
        result += calcDirectionalLight(lights[i], inorm, fragPos, viewDir);
    }

    color = vec4(result, 1.0);
    color *= texColor;
  }
  else
    color = texColor;
}
