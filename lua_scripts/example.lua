ConX.config({
  width = 800,
  height = 600,
  title = "ConX Example",
  fullscreen = true,
  vsync = true
})

package.path = "lua_scripts/?.lua;lua_scripts/?/init.lua;" .. package.path

local Game = require("game.main")

Game.init()

function update()
  Game.update()
end
