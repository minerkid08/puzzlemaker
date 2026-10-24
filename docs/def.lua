---@enum Direction
Direction = {
	POS_X = 0,
	NEG_X = 1,
	POS_Y = 2,
	NEG_Y = 3,
	POS_Z = 4,
	NEG_Z = 5
};

---@class vec2 : [number, number]
vec2 = {};
---@param x number
---@param y number
---@return vec2
function vec2.new(x, y) end

---@param a vec2
---@param b vec2
---@return vec2
function vec2.add(a, b) end

---@param a vec2
---@param b vec2
---@return vec2
function vec2.sub(a, b) end

---@param vec vec2
---@return number
function vec2.length(vec) end

---@param vec vec2
---@return vec2
function vec2.normalize(vec) end

---@param vec vec2
---@param s number
---@return vec2
function vec2.scale(vec, s) end

---@param a vec2
---@param b vec2
---@return number
function vec2.dot(a, b) end

---@param a vec2
---@param b vec2
---@return number
function vec2.cross(a, b) end

---@class vec3 : [number, number, number]
vec3 = {}
---@param x number
---@param y number
---@param z number
---@return vec3
function vec3.new(x, y, z) end

---@param a vec3
---@param b vec3
---@return vec3
function vec3.add(a, b) end

---@param a vec3
---@param b vec3
---@return vec3
function vec3.sub(a, b) end

---@param vec vec3
---@return number
function vec3.length(vec) end

---@param vec vec3
---@return vec3
function vec3.normalize(vec) end

---@param vec vec3
---@param s number
---@return vec3
function vec3.scale(vec, s) end

---@param a vec3
---@param b vec3
---@return number
function vec3.dot(a, b) end

---@param a vec3
---@param b vec3
---@return vec3
function vec3.cross(a, b) end

---@type vec3
vec3.forward = { 0, 0, 1 };
---@type vec3
vec3.up = { 0, 1, 0 };
---@type vec3
vec3.right = { -1, 0, 0 };

---@class mat4
mat4 = {}
---@return mat4
function mat4.new() end

---@param pos vec3
---@return mat4
function mat4.newTranslate(pos) end

---@param rot vec3
---@return mat4
function mat4.newRotate(rot) end

---@param mat mat4
---@param pos vec3
function mat4.translate(mat, pos) end

---@param mat mat4
---@param rot vec3
function mat4.rotate(mat, rot) end

---@param a mat4
---@param b mat4
---@return mat4
--- Transforms b by a
function mat4.mul(a, b) end

---@param mat mat4
---@param vec vec3
---@return vec3
function mat4.mulv3(mat, vec) end

---@param mat mat4
---@return vec3
function mat4.getPosition(mat) end

---@param mat mat4
---@return vec3
function mat4.getRotation(mat) end

---@param mat mat4
---@return vec3
function mat4.forward(mat) end

---@param mat mat4
---@return vec3
function mat4.up(mat) end

---@param mat mat4
---@return vec3
function mat4.right(mat) end

---@class BrushTexOpts
---@field texSize number?
---@field fit boolean?
---@field lightmapSize number?

---@class Brush
Brush = {}

---@param startPos vec3
---@param endPos vec3
---@return Brush
function Brush.new(startPos, endPos) end

---@param dir Direction
---@param texture string
---@param opts BrushTexOpts?
function Brush:setTexture(dir, texture, opts) end

---@param pos vec3
---@param rot vec3
function Brush:transform(pos, rot) end

---@param mat mat4
function Brush:transformMat(mat) end

---@class Entity
Entity = {}

---@param name string
---@param className string
---@param pos vec3
---@param rot vec3
---@return Entity
function Entity.new(name, className, pos, rot) end

---@param name string
---@param className string
---@param pos vec3
---@param rot vec3
---@return Entity
function Entity.newGlobal(name, className, pos, rot) end

---@param name string
---@return Entity
function Entity.getGlobal(name) end

---@param pos vec3
function Entity:setPosition(pos) end

---@param rot vec3
function Entity:setRotation(rot) end

---@param pos vec3
---@param rot vec3
function Entity:transform(pos, rot) end

---@param mat mat4
function Entity:transformMat(mat) end

---@param brush Brush
function Entity:attachBrush(brush) end

---@param key string
---@param value (number|string|boolean|Entity)
function Entity:setKv(key, value) end

---@param key string
---@param value (number|string|boolean|Entity)
function Entity:setInstanceKv(key, value) end

---@param key string
---@param value (number|string|boolean|Entity)
function Entity:updateKv(key, value) end

---@param key string
---@param value (number|string|boolean|Entity)
function Entity:updateInstaneKv(key, value) end

---@param output string
---@param entity (string|Entity)
---@param input string
---@param argument string?
---@param delay number?
--- entity names prefixed with @ or ! do not get prefixed, the @ does get removed from the final string
function Entity:addOutput(output, entity, input, argument, delay) end

function Entity:markAsIO() end

Item = {};

---@return string
function Item.getName() end

---@return string
function Item.getType() end

---@return vec3
function Item.getPosition() end

---@return vec3
function Item.getRotation() end

---@return mat4
function Item.getTransform() end

---@param key string
---@return (number|boolean|string), (number|string)?
function Item.getKv(key) end

---@return (vec2|vec3)
function Item.getSize() end

---@return vec2
function Item.getTile() end

---@param instance string
---@return string
function Item.instanceStr(instance) end

---@return Entity?, Brush[]?
function Item.defaultExport() end
