local testchamberFrame = {
	[4] = "models/props_test_chamber/frame_4",
	[16] = "models/props_test_chamber/frame_16",
	[20] = "models/props_test_chamber/frame_20",
	[32] = "models/props_test_chamber/frame_32",
	[64] = "models/props_test_chamber/frame_64",
	[128] = "models/props_test_chamber/frame_128",
	corner = {
		name = "models/props_test_chamber/frame_128",
		size = 24
	},
	zOffset = 4
};

function addPanelFrame(config)
	config = config or testchamberFrame;
end
