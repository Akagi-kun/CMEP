local screen_size_x = 1100
local screen_size_y = 720

return {
	screen_size_x = screen_size_x,
	screen_size_y = screen_size_y,

	-- Pixel to screen-size conversion functions
	pxToScreenX = function(x)
		return x / screen_size_x
	end,
	
	pxToScreenY = function(y)
		return y / screen_size_y
	end,

	-- basic 2D box collision
	checkCollisions2DBox = function(x1, y1, w1, h1, x2, y2, w2, h2)
		if ((x1 < x2 + w2) and (x1 + w1 > x2) and (y1 < y2 + h2) and (y1 + h1 > y2)) then
			return true
		else
			return false
		end
	end,
}