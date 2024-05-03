// addon MODULE
%module ofx


%import "../../ofxLua/swig/openFrameworks.i"
%import "../../../libs/openFrameworks/gl/ofGLBaseTypes.h"

%{
	#include "../../ofxVectorGraphics/src/ofxVectorGraphics.h"
	#include "../../ofxOpenCv/src/ofxOpenCv.h"
	#include "../../ofxAssimpModelLoader/src/ofxAssimpModelLoader.h"
	#include "../libs/ofxVolumetrics/src/ofxVolumetrics.h"
	#include "../libs/ofxStableDiffusion/include/stable-diffusion.h"
	#include "../libs/ofxImGui/src/Gui.h"
	using namespace ofxImGui;
	using namespace ns_creeps;
%}


// ofxImGui
%ignore operator bool;
%rename(beginGui) begin;
%rename(endGui) end;
%rename(ImGui) Gui;

// ofxOpenCv
%ignore operator &=;

// ofxVectorGraphics
%ignore operator |;


// ----- Renaming -----

// strip "ofx" prefix from classes
%rename("%(strip:[ofx])s", %$isclass) "";

// strip "ofx" prefix from global functions & make first char lowercase
%rename("%(regex:/ofx(.*)/\\l\\1/)s", %$isfunction) "";

// strip "OFX_" from constants & enums
%rename("%(strip:[OFX_])s", %$isconstant) "";
%rename("%(strip:[OFX_])s", %$isenumitem) "";


// ----- Bindings------
%include "../../ofxVectorGraphics/libs/CreEPS.hpp"
%include "../../ofxVectorGraphics/src/ofxVectorGraphics.h"
%include "../../ofxOpenCv/src/ofxCvBlob.h"
%include "../../ofxOpenCv/src/ofxCvHaarFinder.h"
%include "../../ofxOpenCv/src/ofxCvContourFinder.h"
%import "../../ofxOpenCv/src/ofxCvImage.h"
%include "../../ofxOpenCv/src/ofxCvColorImage.h"
%include "../../ofxOpenCv/src/ofxCvFloatImage.h"
%include "../../ofxOpenCv/src/ofxCvGrayscaleImage.h"
%include "../../ofxOpenCv/src/ofxCvShortImage.h"
%include "../../ofxAssimpModelLoader/src/ofxAssimpModelLoader.h"
%include "../../ofxAssimpModelLoader/src/ofxAssimpAnimation.h"
%include "../libs/ofxVolumetrics/src/ofxVolumetrics.h"
%include "../libs/ofxVolumetrics/src/ofxImageSequencePlayer.h"
%include "../libs/ofxStableDiffusion/include/stable-diffusion.h"
%include "../libs/ofxImGui/src/Gui.h"
