local pos = Item.getPosition();
local size = Item.getSize();
local transform = Item.getTransform();

pos[1] = pos[1] + size[1] / 2;
pos[2] = pos[2] + size[2] / 2;
pos[3] = pos[3] + size[3] / 2;

local hurt = Entity.new("hurt", "trigger_hurt", pos, { 0, 0, 0 });
local noportal = Entity.new("noportal", "func_noportal_volume", pos, { 0, 0, 0 });
local dissolve = Entity.new("dissolve", "trigger_multiple", pos, { 0, 0, 0 });
local filter = Entity.new("filter", "filter_activator_class", pos, { 0, 0, 0 });

local gooBrush = Brush.new({ 0, 0, 0 }, { size[1], size[2], size[3] });
local hurtBrush = Brush.new({ 0, 0, 0 }, { size[1], size[2], size[3] });
local noportalBrush = Brush.new({ 0, 0, 0 }, { size[1], size[2], size[3] });
local dissolveBrush = Brush.new({ 0, 0, 0 }, { size[1], size[2] - 1, size[3] });

gooBrush:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
gooBrush:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
gooBrush:setTexture(Direction.POS_Y, "NATURE/TOXICSLIME002A");
gooBrush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
gooBrush:setTexture(Direction.POS_Z, "TOOLS/TOOLSNODRAW");
gooBrush:setTexture(Direction.NEG_Z, "TOOLS/TOOLSNODRAW");

for i = 0, 5 do
	hurtBrush:setTexture(i, "TOOLS/TOOLSTRIGGER");
	noportalBrush:setTexture(i, "TOOLS/TOOLSINVISIBLE");
	dissolveBrush:setTexture(i, "TOOLS/TOOLSTRIGGER");
end

gooBrush:transformMat(transform);
noportalBrush:transformMat(transform);
hurtBrush:transformMat(transform);
dissolveBrush:transformMat(transform);

hurt:setKv("spawnflags", 1);
hurt:setKv("damagetype", 1048576);
hurt:setKv("damage", 100000);
hurt:setKv("nodmgforce", 1);
dissolve:setKv("spawnflags", 8);
dissolve:setKv("filtername", filter);
filter:setKv("filterclass", "prop_weighted_cube");

dissolve:addOutput("OnStartTouch", "!activator", "silentdissolve");

hurt:attachBrush(hurtBrush);
noportal:attachBrush(noportalBrush);
dissolve:attachBrush(dissolveBrush);
