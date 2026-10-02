struct ItemBase
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|||
|name|string|||
|exportScript|string||optional|
|inputs|Input[]||optional|
|outputs|Output[]||opional|
|kvs|Kv[]||optional|
|statickvs|table<string,string>||optional|
|snapMode|SnapMode|"corner"|
|deleteIntersectingVoxels|boolean|false|if the item should delete the voxels that it intersects with|
|genMissingVoxels|boolean|false|if the item should generate voxels so the map stays sealed|

enum SnapMode (string)  
corner, center, mini-corner, mini-center

struct Input
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|trueInput|string|||
|falseInput|string|||
|trueArg|string||optional|
|falseArg|string||optional|

struct Output
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|trueOutput|string|||
|falseOutput|string|||

struct Kv
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|name|string|||
|type|KvType||type of the kv, also the type for default value and options|
|defaultValue|type|||
|options|object<string, type>||only needs to be specified if using a dropdown type|

enum KvType (string)  
int, float, bool, drop-int, drop-string 

----------------------
struct EntityItem (ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"entity"||
|model|string||model to use in editor|
|mat|string||material to use in editor|
|bound1|vec3||point 1 for selection box|
|bound2|vec3||point 2 for selection box|
|transform|Transform||transform to apply to exported entity|
|entity|string||entity to spawn ingame, ignored if instance is specified|
|instance|string||instance to spawn ingame|

struct Transform
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|position|vec3|{0, 0, 0}||
|rotation|vec3|{0, 0, 0}|rotations happen in the yzx order|

----------------------
struct PanelItem (ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|type|string|"panel"||
|minSize|vec2|{0, 0}||
|maxSize|vec2|{99999, 99999}||
|defaultSize|vec2|{4, 4}||
|horizTile|bool||weather tiling horizontaly is enabled, optional|
|vertTile|bool||weather tiling vertically is enabled, optional|
|editorCenterTexture|string|||
|centerTexture|string|||
|texSize|string||size of the center texture|
|zTexture|string||texture used on the ends of the panel|
|thickness|number|||
|entity|string||name of the brush entity to tie this to, optional|
|borders|PanelBorder[]|||

struct PanelBorder
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|id|PanelBorderId|||
|editorTexture|string|||
|minSize|number|0||
|maxSize|number|9999||
|texture|string||texture used ingame|
|texSize|number||size of the texture used ingame|

enum PanelBorderId (string)  
bottom-left, bottom, bottom-right, left, right, top-left, top, top-right

----------------------
struct VolumeItem(ItemBase)
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|minSize|vec3|{0, 0, 0}||
|maxSize|vec3|{99999, 99999, 99999}||
|defaultSize|vec3|{4, 4, 4}||
|editorTexture|string|||
|texture|string\|VolumeTextures|||
|entity|string||brush entity to tie this brush to, optional|

struct VolumeTextures
|Key|Type|Default Value|Description|
|---|----|-------------|-----------|
|right|string||texture to use on the -x side|
|left|string||texture to use on the +x side|
|top|string||texture to use on the +z side|
|bottom|string||texture to use on the -z side|
|front|string||texture to use on the +y side|
|back|string||texture to use on the -y side|

