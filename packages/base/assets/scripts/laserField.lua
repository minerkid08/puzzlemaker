require("fizzlerUtils");

---@type FizzlerProp
local propConfig = {
	static = false,
	model = "models/props/fizzler_dynamic",
	openAnim = "open",
	closeAnim = "close",
	skin = 2
};

local pos = Item.getPosition();
local size = Item.getSize();
local tile = Item.getTile();

local transform = Item.getTransform();

local rotation = mat4.newRotate({ 90, 0, 0 });
transform = mat4.mul(transform, rotation);

local ent = Entity.new("trigger", "trigger_hurt", pos, { 0, 0, 0 });
ent:markAsIO();
ent:setKv("spawnflags", 4097);
ent:setKv("rendercolor", "255 255 255");
ent:setKv("solid", 6);
ent:setKv("damage", 100000);

local trigger = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2] * tile[2], 1 / 64 });

trigger:setTexture(Direction.POS_X, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_X, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.POS_Y, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_Y, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.POS_Z, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_Z, "TOOLS/TOOLSTRIGGER");
trigger:transformMat(transform);
ent:attachBrush(trigger);

local laserEnt = Entity.new("laser", "func_brush", pos, { 0, 0, 0 });

laserEnt:setKv("solidity", 1);
laserEnt:setKv("renderfx", 14);

for i = 1, tile[2] do
	local laser = Brush.new({ 0, tile[2] * 2, -1 / 64 }, { size[1], size[2] + tile[2] * 2, 1 / 64 });
	laser:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
	laser:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
	laser:setTexture(Direction.POS_Y, "TOOLS/TOOLSNODRAW");
	laser:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
	laser:setTexture(Direction.POS_Z, "EFFECTS/LASERPLANE", { texSize = 512, fit = true });
	laser:setTexture(Direction.NEG_Z, "EFFECTS/LASERPLANE", { texSize = 512, fit = true });
trigger:transformMat(transform);
	laserEnt:attachBrush(laser);
end

addFizzlerProps(propConfig, false, ent, transform);

ent:addOutput("OnUser1", ent, "Disable");
ent:addOutput("OnUser1", laserEnt, "Disable");

ent:addOutput("OnUser2", ent, "Enable");
ent:addOutput("OnUser2", laserEnt, "Enable");
