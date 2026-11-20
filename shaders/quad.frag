#version 330 core

uniform sampler2D textures[15];
uniform sampler2D shadowMap;
uniform int pass;
uniform vec3 camPos;
uniform int lightingPreview;

in VSOUT
{
  vec3 pos;
  vec2 uv;
  vec4 tint;
  vec3 normal;
  flat int materialId;
} fragIn;

out vec4 color;

struct Light
{
  vec3 position;
  float constant;
  float linear;
  float quadratic;
  float cutoff;
  int shadowMapIndex;
  vec3 ambient;
  float dirx;
  vec3 diffuse;
  float diry;
  vec3 specular;
  float dirz;
  mat4 transform;
};

layout(std140) uniform LightBlock
{
  Light lights[128];
};

float calcShadows(Light light, vec3 fragPos, vec3 lightDir, vec3 normal)
{
	if(light.shadowMapIndex == 0)
		return 0.0;

  vec4 pos = light.transform * vec4(fragPos, 1);
  vec3 projCoords = pos.xyz / pos.w;

  projCoords = projCoords * 0.5 + 0.5;
	float x = projCoords.x / 4;
	float y = projCoords.y / 4;
	x += ((light.shadowMapIndex - 1) % 4) * 0.25;
	y += ((light.shadowMapIndex - 1) / 4) * 0.25;
  float closestDepth = texture(shadowMap, vec2(x, y)).r;

  float nearPlane = 0.1;
  float farPlane = 50;
  float z = closestDepth * 2.0 - 1.0;

  closestDepth = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
  z = projCoords.z * 2.0 - 1.0;
  projCoords.z = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));

  float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01);

  float currentDepth = projCoords.z - bias;
  float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
  return shadow;
}

vec3 calcDirectionalLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(fragPos - light.position);

  vec3 dir = vec3(light.dirx, light.diry, light.dirz);
  float theta = dot(lightDir, dir);
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
    float diff = max(dot(normal, -lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.4);
    // attenuation
    // combine results
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = calcShadows(light, fragPos, -lightDir, normal);
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    return lighting;
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
  switch (fragIn.materialId)
  {
    case 0:
    texColor = texture(textures[0], fragIn.uv);
    break;
    case 1:
    texColor = texture(textures[1], fragIn.uv);
    break;
    case 2:
    texColor = texture(textures[2], fragIn.uv);
    break;
    case 3:
    texColor = texture(textures[3], fragIn.uv);
    break;
    case 4:
    texColor = texture(textures[4], fragIn.uv);
    break;
    case 5:
    texColor = texture(textures[5], fragIn.uv);
    break;
    case 6:
    texColor = texture(textures[6], fragIn.uv);
    break;
    case 7:
    texColor = texture(textures[7], fragIn.uv);
    break;
    case 8:
    texColor = texture(textures[8], fragIn.uv);
    break;
    case 9:
    texColor = texture(textures[9], fragIn.uv);
    break;
    case 10:
    texColor = texture(textures[10], fragIn.uv);
    break;
    case 11:
    texColor = texture(textures[11], fragIn.uv);
    break;
    case 12:
    texColor = texture(textures[12], fragIn.uv);
    break;
    case 13:
    texColor = texture(textures[13], fragIn.uv);
    break;
    case 14:
    texColor = texture(textures[14], fragIn.uv);
    break;
  }
  texColor.x *= fragIn.tint.x;
  texColor.y *= fragIn.tint.y;
  texColor.z *= fragIn.tint.z;
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
		color = vec4(fragIn.normal, 1);
		color.x = color.x * 0.5 + 0.5;
		color.y = color.y * 0.5 + 0.5;
		color.z = color.z * 0.5 + 0.5;
		return;
    color = vec4(0, 0, 0, 1);
    vec3 viewDir = normalize(camPos - fragIn.pos);

    vec3 result = vec3(0, 0, 0);
    for (int i = 0; i < 128; i++)
    {
      if (lights[i].position.x < 0)
        continue;
      if (lights[i].cutoff == 0)
        result += calcLight(lights[i], fragIn.normal, fragIn.pos, viewDir);
      else
        result += calcDirectionalLight(lights[i], fragIn.normal, fragIn.pos, viewDir);
    }

    color = vec4(result, 1.0);
    color *= texColor;
  }
  else
    color = texColor;
}
