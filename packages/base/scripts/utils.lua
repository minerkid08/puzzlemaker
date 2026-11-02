---@return mat4
function Item.getFloorTransform()
	local transform = Item.getTransform();
	local start = mat4.newRotate({ 90, 0, 0 });
	return transform:mul(start);
end

---@return mat4
function Item.getCeilingTransform()
	local transform = Item.getTransform();
	local start = mat4.newRotate({ -90, 0, 0 });
	return transform:mul(start);
end
