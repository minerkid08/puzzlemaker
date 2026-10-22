## Item Base

#### struct ItemBase
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|||
|id|string|||
|name|string||display name of item, optional|
|exportScript|string||optional|
|inputs|Input[]||optional|
|outputs|Output[]||opional|
|kvs|Kv[]||optional|
|statickvs|table<string, string>||kv pairs to add to exported entity, does not work on instances. optional|
|snapMode|SnapMode|"corner"|how the item snaps when grabbed|
|rotationRestriction|string||which axis's should the item rotation be restricted on, ex "xyz" restricts rotation entirely and "xz" restricts only pitch and roll rotaiton|
|deleteIntersectingVoxels|boolean|false|if the item should delete the voxels that it intersects with|
|genMissingVoxels|boolean|true|if the item should generate voxels so the map stays sealed, only does something when deleteIntersectingVoxels is true|
|deleteIntersectingVoxelsEditor|boolean|false|same as deleteIntersectingVoxels but for the editor|
|requiresPaint|boolean|false||

#### enum SnapMode (string)  
corner, center, mini-corner, mini-center

#### struct Input
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|trueInput|string|||
|falseInput|string||oprional|
|trueArg|string||optional|
|falseArg|string||optional|

#### struct Output
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|trueOutput|string|||
|falseOutput|string||optional|

#### struct Kv
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|type|KvType||type of the kv, also the type for default value and options|
|defaultValue|type|||
|options|object<string, type>||only needs to be specified if using a dropdown type|

#### struct Transform
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|position|vec3|[0, 0, 0]||
|rotation|vec3|[0, 0, 0]|rotations happen in the yxz (yaw pitch roll) order|

#### enum KvType (string)  
int, float, bool, string, drop-int, drop-string

#### enum TextureAlignment (string)  
none, fit

#### struct Texture
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|filename|string|||
|alignment|TextureAlignment|"none"||
|size|number|512||

#### struct EditorTexture 
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|filename|string|||
|alignment|TextureAlignment|"fit"||
|size|number|1||

both texture structs can be a string

## Entity Item

#### struct EntityItem (ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"entity"||
|model|string||model to use in editor|
|mat|string||material to use in editor|
|bound1|vec3||point 1 for selection box|
|bound2|vec3||point 2 for selection box|
|transform|Transform||transform to apply to exported entity|
|editorTransform|Transform||transform to apply to mesh in the editor|
|entity|string||entity to spawn ingame, ignored if instance is specified|
|instance|string||instance to spawn ingame, can be templated|

#### Note
For snapping on walls to work best, build your instance in hammer so the side facing away from the wall faces +x  
Then set the rotation field in the transform to [0, 0, 0]  
For snapping on floors, set the rotation field in the transform to [90, 0, 0]  

## Panel Item

#### struct PanelItem (ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"panel"||
|minSize|vec2|[0, 0]||
|maxSize|vec2|[99999, 99999]||
|defaultSize|vec2|[4, 4]||
|horizTile|bool|false|weather tiling horizontaly is enabled|
|vertTile|bool|false|weather tiling vertically is enabled|
|editorCenterTexture|EditorTexture|||
|centerTexture|Texture|||
|zTexture|string||texture used on the ends of the panel|
|thickness|number|||
|entity|string||name of the brush entity to tie this to, optional|
|borders|PanelBorder[]|||
|props|Prop[]|||

#### struct Prop
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|id|PanelBorderId|||
|editorModel|string|||
|editorTexture|string|||
|model|string|||
|transform|Transform|||
|editorTransform|Transform|||

#### struct PanelBorder
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|id|PanelBorderId|||
|editorTexture|EditorTexture|||
|minSize|number|0||
|maxSize|number|9999||
|texture|Texture|||

#### enum PanelBorderId (string)  
bottom-left, bottom, bottom-right, left, right, top-left, top, top-right

## Volume Item

#### struct VolumeItem(ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"volume"||
|minSize|vec3|[0, 0, 0]||
|maxSize|vec3|[99999, 99999, 99999]||
|defaultSize|vec3|[4, 4, 4]||
|editorTexture|EditorTexture|||
|texture|Texture|either texture or textures has to be defined||
|textures|VolumeTextures|||
|entity|string||brush entity to tie this brush to, optional|

#### struct VolumeTextures
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|right|Texture||texture to use on the -x side|
|left|Texture||texture to use on the +x side|
|top|Texture||texture to use on the +z side|
|bottom|Texture||texture to use on the -z side|
|front|Texture||texture to use on the +y side|
|back|Texture||texture to use on the -y side|

## Overlay Item

#### struct OverlayItem(ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"overlay"||
|editorTexture|EditorTexture|||
|texture|string|texture to use, can be templated||

## Templated Strings

Some strings can be templated which means that they can use the values of kvs inside them. To do this put the kv name inside of square brackets or curly bracks in the string. For dropdown kvs square brackets use the name of the option, curly brackets use the value.
