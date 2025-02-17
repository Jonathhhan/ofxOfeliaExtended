if type(window) ~= "userdata" then
window = ofWindow()
end

local a = ofelia
local clock = ofClock(this, "setup")
local projectM = ofxProjectM()
local array = ofArray("$0-stereo")
local send = ofSend("$0-update")
local sendQuit = ofSend("$0-quit")
local floatArray = ofxNew_floatArray(512)

function a.new()
ofWindow.addListener("setup", this)
ofWindow.addListener("update", this)
ofWindow.addListener("draw", this)
ofWindow.addListener("mousePressed", this)
ofWindow.addListener("exit", this)
window:setPosition(100, 100)
window:setSize(640, 360)
window:setGLVersion(4, 5)
if ofWindow.exists then
clock:delay(0)
else
window:create()
end
end

function a.free()
window:destroy()
ofWindow.removeListener("setup", this)
ofWindow.removeListener("update", this)
ofWindow.removeListener("draw", this)
ofWindow.removeListener("mousePressed", this)
ofWindow.removeListener("exit", this)
end

function a.setup()
--ofSetFrameRate(60)
projectM:load()
projectM:setWindowSize(1920, 1080)
ofSetWindowTitle("ofxProjectM")
ofBackground(150, 230, 255, 255)
end

function a.update()
send:sendBang()
for i = 1, 512 do
ofxFloatArray_setitem(floatArray, i, array:getAt(i))
end
projectM:audio(floatArray, 256, 2)
projectM:update()
end

function a.draw()
ofSetColor(255)
projectM:draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight())
ofDrawBitmapString(projectM:getPresetName(), 20, 20)
ofDrawBitmapString("fps: " .. ofGetFrameRate(), 20, 40)
end

function a.mousePressed()
ofToggleFullscreen()
end

function a.exit()
sendQuit:sendBang()
end

function a.randomPreset()
projectM:randomPreset()
end

function a.fullscreen()
ofToggleFullscreen()
end
