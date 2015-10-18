----------------------------------------------------------------
-- Copyright (c) 2010-2011 Zipline Games, Inc. 
-- All Rights Reserved. 
-- http://getmoai.com
----------------------------------------------------------------

if MOAIEnvironment.osBrand ~= MOAIEnvironment.OS_BRAND_IOS then
	print ( 'iOS only sample - please run on an iOS device or simulator' )
	os.exit ()
end

MOAISim.openWindow ( "test", 320, 480 )

viewport = MOAIViewport.new ()
viewport:setSize ( 320, 480 )
viewport:setScale ( 320, 480 )

layer = MOAILayer2D.new ()
layer:setViewport ( viewport )
MOAISim.pushRenderPass ( layer )

gfxQuad = MOAIGfxQuad2D.new ()
gfxQuad:setTexture ( "moai.png" )
gfxQuad:setRect ( -64, -64, 64, 64 )

prop = MOAIProp2D.new ()
prop:setDeck ( gfxQuad )
layer:insertProp ( prop )

prop:moveRot ( 540, 2.0 )

timer = MOAITimer.new()
timer:setSpan(5)
timer:setListener(MOAITimer.EVENT_STOP,
    function()
        if MOAIAppIOS.canOpenURL("NG URL") then
            error("NG!")
        end
        if MOAIAppIOS.canOpenURL("https://www.google.co.jp/") then
            MOAIAppIOS.openURL("https://www.google.co.jp/")
        end
    end)
timer:start()

