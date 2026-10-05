#include "cglm/types.h"

void startGrab();
void updateGrab(vec3 cameraPos, vec3 mouseDir);
void startRotate(int mouseX);
void updateRotate(int mouseX);
void rotateSelection(float amount);
