#### struct Theme
|Key|Type|
|---|----|
|voxel|VoxelDef|
|antline|AntlineDef|
|postProcessing|PostProcessingDef[]|

#### struct VoxelDef
|Key|Type|
|---|----|
|blackEditor|string|
|whiteEditor|string|
|blackMiniEditor|string|
|whiteMiniEditor|string|
|nodraw|string|
|backstage|string|
|blackFloor|string|
|blackWall|string|
|blackCeiling|string|
|whiteFloor|string|
|whiteWall|string|
|whiteCeiling|string|
|blackFloorMini|string|
|blackWallMini|string|
|blackCeilingMini|string|
|whiteFloorMini|string|
|whiteWallMini|string|
|whiteCeilingMini|string|

#### struct AntlineDef
|Key|Type|
|---|----|
|editorAntlineTex|string|
|editorAntlineActiveTex|string|
|editorCheckTex|string|
|editorCheckActiveTex|string|
|dotSize|int|
|checkSize|int|
|lineTex|string|
|cornerTex|string|
|checkTex|string|
|lineDotCountx|int|
|lineDotCounty|int|
|cornerDotCountx|int|
|cornerDotCounty|int|

#### struct PostProcessingDef
|Key|Type|
|---|----|
|name|string|
|tonemap|TonemapDef|
|fog|FogDef|
|colorCorrection|ColorCorrectionDef|

#### struct TonemapDef
|Key|Type|
|---|----|
|enabled|bool?|
|maxExposure|number|
|minExposure|number|
|tonemapRate|number|
|brightPixelPercent|number|

#### struct FogDef
|Key|Type|
|---|----|
|enabled|bool?|
|primaryColor|vec3|
|secondaryColor|vec3|
|start|number|
|end|number|
|density|number|

#### struct ColorCorrectionDef
|Key|Type|
|---|----|
|enabled|bool?|
|minFalloff|number|
|maxFalloff|number|
|maxWeight|number|
|fadeInDuration|number|
|fadeOutDuration|number|
|filename|string|
