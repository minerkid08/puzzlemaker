require("fizzlerUtils");
require("utils");

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

local transform = Item.getFloorTransform();

local p1 = transform:getPosition();
print(p1[1], p1[2], p1[3]);

local ent = Entity.new("trigger", "trigger_hurt", pos, { 0, 0, 0 });
ent:markAsIO();
ent:setKv("spawnflags", 1);
ent:setKv("startdisabled", 0);
ent:setKv("damagemodel", 0);
ent:setKv("damage", 1000);
ent:setKv("damagetype", 0);

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
  local laser = Brush.new({ 0, (i - 1) * 2, -1 / 64 }, { size[1], size[2] + (i - 1) * 2, 1 / 64 });
  laser:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
  laser:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
  laser:setTexture(Direction.POS_Y, "TOOLS/TOOLSNODRAW");
  laser:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
  laser:setTexture(Direction.POS_Z, "EFFECTS/LASERPLANE", { texSize = 512, fit = true });
  laser:setTexture(Direction.NEG_Z, "EFFECTS/LASERPLANE", { texSize = 512, fit = true });
  laser:transformMat(transform);
  laserEnt:attachBrush(laser);
end

addFizzlerProps(propConfig, false, ent, transform);

ent:addOutput("OnUser1", ent, "Disable");
ent:addOutput("OnUser1", laserEnt, "Disable");

ent:addOutput("OnUser2", ent, "Enable");
ent:addOutput("OnUser2", laserEnt, "Enable");
