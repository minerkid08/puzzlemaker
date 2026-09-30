---@enum Direction
Direction = {
	POS_X = 0,
	NEG_X = 1,
	POS_Y = 2,
	NEG_Y = 3,
	POS_Z = 4,
	NEG_Z = 5
};

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
