#pragma once

#include "../libs/imgui/src/imgui.h"

namespace ofxImGui
{
	class BaseTheme
	{
	public:
        BaseTheme()
        {
            
        }
        virtual ~BaseTheme()
        {
            
        }
        
		virtual void setup()=0;
	};
}
