---@enum Direction
Direction = {
	POS_X = 0,
	NEG_X = 1,
	POS_Y = 2,
	NEG_Y = 3,
	POS_Z = 4,
	NEG_Z = 5
};

---@alias vec2 number[]
---@alias vec3 number[]

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

---@class Entity
Entity = {}

---@param name string
---@param className string
---@param pos vec3
---@param rot vec3
---@return Entity
function Entity.new(name, className, pos, rot) end

---@param pos vec3
function Entity:setPosition(pos) end

---@param rot vec3
function Entity:setRotation(rot) end

---@param pos vec3
---@param rot vec3
function Entity:transform(pos, rot) end

---@param brush Brush
function Entity:attachBrush(brush) end

---@param key string
---@param value (number|string|boolean) 
function Entity:setKv(key, value) end

---@param output string
---@param entity (string|Entity)
---@param input string
---@param argument string?
---@param delay number?
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
---@param key string
---@return (number|boolean|string), (number|string)
function Item.getKv(key) end
---@return (vec2|vec3)
function Item.getSize() end
---@return vec2
function Item.getTile() end
