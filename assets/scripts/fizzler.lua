local pos = Item.getPosition();
local rot = Item.getRotation();
local size = Item.getSize();

local startDisabled = Item.getKv("startdisabled");

local ent = Entity.new("trigger", "trigger_portal_cleanser", pos, { 0, 0, 0 });
ent:setKv("visible", true);
ent:setKv("usescanline", true);
ent:setKv("spawnflags", 9);
ent:setKv("rendercolor", "255 255 255");
ent:setKv("viewhideflags", false);
ent:setKv("rendermode", 0);
ent:setKv("renderfx", 0);
ent:setKv("solid", 6);
ent:setKv("disablereceiveshadows", false);
ent:setKv("drawinfastreflection", false);
ent:setKv("startdisabled", startDisabled);

local leftBrush = Brush.new({ 0, 0, -1 / 64 }, { 1, size[2], 1 / 64 });
local centerBrush = Brush.new({ 1, 0, -1 / 64 }, { size[1] - 1, size[2], 1 / 64 });
local rightBrush = Brush.new({ size[1] - 1, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });

local brushes = { leftBrush, centerBrush, rightBrush };
ent:markAsIO();

leftBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_L", { texSize = 1024, fit = true });
leftBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_R", { texSize = 1024, fit = true });

centerBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_CENTER", { texSize = 1024, fit = true });
centerBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_CENTER", { texSize = 1024, fit = true });

rightBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_R", { texSize = 1024, fit = true });
rightBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_L", { texSize = 1024, fit = true });

for _, brush in pairs(brushes) do
	brush:setTexture(Direction.POS_X, "TOOLS/TOOLSTRIGGER");
	brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSTRIGGER");
	brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSTRIGGER");
	brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSTRIGGER");

	brush:transform(pos, rot);
	ent:attachBrush(brush);
end

local leftProp = Entity.new("leftProp", "prop_dynamic", { 0, 1, 0 }, { 0, 90, 0 });

leftProp:setKv("model", "models/props/fizzler_dynamic.mdl");
leftProp:setKv("holdanimation", true);
leftProp:transform(pos, rot);

local rightProp = Entity.new("rightProp", "prop_dynamic", { size[1], 1, 0 }, { 0, -90, 0 });

rightProp:setKv("model", "models/props/fizzler_dynamic.mdl");
rightProp:setKv("holdanimation", true);
rightProp:transform(pos, rot);

if(startDisabled) then
	leftProp:setKv("defaultanimation", "close");
	rightProp:setKv("defaultanimation", "close");
end

ent:addOutput("OnUser1", ent, "Disable");
ent:addOutput("OnUser1", leftProp, "SetAinimation", "open");
ent:addOutput("OnUser1", rightProp, "SetAinimation", "open");

ent:addOutput("OnUser2", ent, "Enable");
ent:addOutput("OnUser2", leftProp, "SetAinimation", "close");
ent:addOutput("OnUser2", rightProp, "SetAinimation", "close");
