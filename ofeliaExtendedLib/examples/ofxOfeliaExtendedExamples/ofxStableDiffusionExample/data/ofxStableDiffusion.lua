if type(window) ~= "userdata" then
window = ofWindow()
end

local a = ofelia
local clock = ofClock(this, "setup")
local stableDiffusion = ofxStableDiffusion()
local modelName
local imgVec
local texture = ofTexture()
local pixels = ofPixels()
local gui = ofxImGui()
local boolArrayValue = ImGuiNew_BoolArray(1)
local intArrayValue = ImGuiNew_IntArray(1)
local prompt = "a skater in the woods, van gogh"
local negativePrompt = ""
local generate = false
local send = ofSend("$0-goo")

function a.new()
ofWindow.addListener("setup", this)
ofWindow.addListener("update", this)
ofWindow.addListener("draw", this)
ofWindow.addListener("exit", this)
window:setPosition(200, 100)
window:setSize(632, 900)
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
ofWindow.removeListener("exit", this)
end

function a.setup()
ofSetWindowTitle("ofxStableDiffusion")
ofBackground(150, 230, 255, 255)
ofSetDataPathRoot("")
texture:allocate(512, 512, GL_RGB)
ImGuiBoolArray_setitem(boolArrayValue, 0, true)
ImGuiIntArray_setitem(intArrayValue, 0, 5)
gui:setup()
modelName = "sd_turbo.safetensors"
print(stableDiffusion:getSystemInfo())
stableDiffusion:newSdCtx("sd_turbo.safetensors", "", "", "", "", "", "", true, false, false, 8, 1, 0, 0, false, false, false)
end

function a.update()
if stableDiffusion:isDiffused() then
imgVec = stableDiffusion:returnImages()
print("Width:", imgVec.width, "Height:", imgVec.height, "Channel:", imgVec.channel)
texture:loadData(imgVec.data, 512, 512, GL_RGB)
stableDiffusion:setDiffused(false)
end
if generate == true then
stableDiffusion:txt2img(prompt, negativePrompt, 0, 1, 512, 512, 0, 5, -1, 1, sd_image_t, 1, 1, false, "")
generate = false
end
end

function a.draw()
gui:beginGui()
--ImGuiShowDemoWindow_0()
ImGuiStyleColorsDark()
ImGuiPushStyleVar(ImGuiStyleVar_WindowMinSize, ImGuiImVec2(532, 120))
ImGuiPushStyleVar(ImGuiStyleVar_WindowPadding, ImGuiImVec2(10, 0))
ImGuiPushStyleVar(ImGuiStyleVar_IndentSpacing, 10)
ImGuiPushStyleVar(ImGuiStyleVar_ItemSpacing, ImGuiImVec2(0, 0))
ImGuiPushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImGuiImVec2(5, 0))
ImGuiSetNextWindowPos(ImGuiImVec2(50, 50), ImGuiCond_Once)
ImGuiBegin_3("ofxStableDiffusion", boolArrayValue, ImGuiWindowFlags_NoResize)
ImGuiDummy(ImGuiImVec2(0, 10))
ImGuiImage(texture:getTextureData().textureID, ImGuiImVec2(512, 512))
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiButton("Save")) then
texture:readToPixels(pixels)
ofSaveImage(pixels, ofGetTimestampString("output/ofxStableDiffusion-%Y-%m-%d-%H-%M-%S.png"))
end
ImGuiDummy(ImGuiImVec2(0, 10))
ImGuiPushItemWidth(420)
ImGuiText("The checkbox below is checked.")
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiSliderInt("Test", intArrayValue, 1, 16)) == true then
send:sendFloat(ImGuiIntArray_getitem(intArrayValue, 0))
end
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiCheckbox("Checkbox", boolArrayValue)) == true then
print("Toggle:", ImGuiBoolArray_getitem(boolArrayValue, 0))
end
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiRadioButton("Checked", true)) == true then
print("Button pressed!")
end
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiButton("Load Model")) == true then
local result = ofSystemLoadDialog("Load Model", false, "")
if (result.bSuccess) then
local modelPath = result:getPath()
modelName = result:getName()
local file = ofFile(result:getPath())
if(file:getExtension() == "safetensors") then
stableDiffusion:newSdCtx(modelPath, "", "", "", "", "", "", true, false, false, 8, 1, 0, 0, false, false, false)
end
end
end
ImGuiSameLine(0, 5)
ImGuiText(modelName)
ImGuiDummy(ImGuiImVec2(0, 10))
if (ImGuiButton("Generate")) == true then
generate = true
print("Prompt:", prompt)
end
ImGuiDummy(ImGuiImVec2(0, 10))
ImGuiInputText( "Prompt", prompt, 60)
ImGuiDummy(ImGuiImVec2(0, 10))
ImGuiInputText( "nPrompt", negativePrompt, 60)
ImGuiDummy(ImGuiImVec2(0, 20))
ImGuiEnd()
gui:endGui()
end

function a.exit()
stableDiffusion:freeSdCtx()
ImGuiDelete_BoolArray(boolArrayValue)
ImGuiDelete_IntArray(intArrayValue)
end
