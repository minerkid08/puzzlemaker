local pos = Item.getPosition();
local rot = Item.getRotation();
local size = Item.getSize();

local ent = Entity.new("trigger", "trigger_hurt", pos, { 0, 0, 0 });
ent:markAsIO();
ent:setKv("spawnflags", 4097);
ent:setKv("rendercolor", "255 255 255");
ent:setKv("solid", 6);
ent:setKv("damage", 100000);

local trigger = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });

trigger:setTexture(Direction.POS_X, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_X, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.POS_Y, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_Y, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.POS_Z, "TOOLS/TOOLSTRIGGER");
trigger:setTexture(Direction.NEG_Z, "TOOLS/TOOLSTRIGGER");
trigger:transform(pos, rot);
ent:attachBrush(trigger);

local laser = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });

local laserEnt = Entity.new("laser", "func_brush", pos, { 0, 0, 0 });

laserEnt:setKv("solidity", 0);
laserEnt:setKv("renderfx", 14);

laser:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
laser:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
laser:setTexture(Direction.POS_Y, "TOOLS/TOOLSNODRAW");
laser:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
laser:setTexture(Direction.POS_Z, "EFFECTS/LASERPLANE");
laser:setTexture(Direction.NEG_Z, "EFFECTS/LASERPLANE");
trigger:transform(pos, rot);
laserEnt:attachBrush(trigger);

local leftProp = Entity.new("leftProp", "prop_dynamic", { 0, 1, 0 }, { 0, 90, 0 });

leftProp:setKv("model", "models/props/fizzler_dynamic.mdl");
leftProp:setKv("holdanimation", true);
leftProp:setKv("skin", 2);
leftProp:transform(pos, rot);

local rightProp = Entity.new("rightProp", "prop_dynamic", { size[1], 1, 0 }, { 0, -90, 0 });

rightProp:setKv("model", "models/props/fizzler_dynamic.mdl");
rightProp:setKv("holdanimation", true);
rightProp:setKv("skin", 2);
rightProp:transform(pos, rot);

ent:addOutput("OnUser1", ent, "Disable");
ent:addOutput("OnUser1", laserEnt, "Disable");
ent:addOutput("OnUser1", leftProp, "SetAnimation", "open");
ent:addOutput("OnUser1", rightProp, "SetAnimation", "open");

ent:addOutput("OnUser2", ent, "Enable");
ent:addOutput("OnUser2", laserEnt, "Enable");
ent:addOutput("OnUser2", leftProp, "SetAnimation", "close");
ent:addOutput("OnUser2", rightProp, "SetAnimation", "close");
