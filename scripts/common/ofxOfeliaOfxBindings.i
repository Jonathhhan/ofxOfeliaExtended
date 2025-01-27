// addon MODULE
%module ofx


%import "../libs/ofxLua/swig/openFrameworks.i"
%import "../../../libs/openFrameworks/gl/ofGLBaseTypes.h"
%import "../../../libs/openFrameworks/utils/ofThread.h"
%import "../../../libs/openFrameworks/utils/ofThreadChannel.h"

%{
	#include "../../ofxVectorGraphics/src/ofxVectorGraphics.h"
	#include "../../ofxOpenCv/src/ofxOpenCv.h"
	#include "../../ofxAssimpModelLoader/src/ofxAssimpModelLoader.h"
	#include "../../ofxSvg/src/ofxSvg.h"
	#include "../libs/srtparser.h"
	#include "../libs/ofxVolumetrics/src/ofxVolumetrics.h"
	#include "../libs/ofxStableDiffusion/src/ofxStableDiffusion.h"
	#include "../libs/ofxStableDiffusion/libs/stable-diffusion/include/stable-diffusion.h"
	using namespace ns_creeps;
%}


// ofxOpenCv
%ignore ofxCvImage::operator &=;

// ofxStableDiffusion
%ignore ofxStableDiffusion::thread;
%extend ofxStableDiffusion {
    sd_image_t getImageAt(size_t index) const {
        return self->returnImages()[index];
    }
}

// ofxVectorGraphics
%ignore ns_creeps::CAt::operator |;

// srtparser
%typemap(throws) std::out_of_range {
  // custom exception handler 
}
%template(SubVector) std::vector<SubtitleItem*>;

// ofxSvg
%template(GlmVector) std::vector<glm::vec3>;


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
%include "../../ofxOpenCv/src/ofxCvImage.h"
%include "../../ofxOpenCv/src/ofxCvColorImage.h"
%include "../../ofxOpenCv/src/ofxCvFloatImage.h"
%include "../../ofxOpenCv/src/ofxCvGrayscaleImage.h"
%include "../../ofxOpenCv/src/ofxCvShortImage.h"
%include "../../ofxAssimpModelLoader/src/ofxAssimpModelLoader.h"
%include "../../ofxAssimpModelLoader/src/ofxAssimpAnimation.h"
%include "../../ofxSvg/src/ofxSvg.h"
%include "../libs/srtparser.h"
%include "../libs/ofxVolumetrics/src/ofxVolumetrics.h"
%include "../libs/ofxVolumetrics/src/ofxImageSequencePlayer.h"
%include "../libs/ofxStableDiffusion/src/ofxStableDiffusion.h"
%include "../libs/ofxStableDiffusion/libs/stable-diffusion/include/stable-diffusion.h"
%import "../libs/ofxStableDiffusion/src/ofxStableDiffusionThread.h"
