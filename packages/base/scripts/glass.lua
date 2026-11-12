require("frame");
require("utils");

local transform = Item.getFloorTransform();
local size = Item.getSize();

addPanelFrame(nil, size, transform);

local brush = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });
brush:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.POS_Z, "GLASS/GLASSWINDOW_FROSTED_002");
brush:setTexture(Direction.NEG_Z, "GLASS/GLASSWINDOW_FROSTED_002");
brush:transformMat(transform);
