#include "ofxVolumetrics.h"
#include "ofMain.h"

#define STRINGIFY(A) #A

using namespace glm;

ofxVolumetrics::~ofxVolumetrics() {
	destroy();
}

ofxVolumetrics::ofxVolumetrics() {
	quality = vec3(1.0);
	threshold = 1.0 / 255.0;
	density = 1.0;
	volWidth = renderWidth = 0;
	volHeight = renderHeight = 0;
	volDepth = 0;
	bIsInitialized = false;
	bDrawDebugVolume = false;

	vector<vec3> vertices {
		// front side
		vec3(1.0, 1.0, 1.0),
		vec3(0.0, 1.0, 1.0),
		vec3(1.0, 0.0, 1.0),
		vec3(0.0, 0.0, 1.0),
		// right side
		vec3(1.0, 1.0, 0.0),
		vec3(1.0, 1.0, 1.0),
		vec3(1.0, 0.0, 0.0),
		vec3(1.0, 0.0, 1.0),
		// top side
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 1.0, 1.0),
		vec3(1.0, 1.0, 0.0),
		vec3(1.0, 1.0, 1.0),		
		// left side
		vec3(0.0, 1.0, 1.0),
		vec3(0.0, 1.0, 0.0),		
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 0.0, 0.0),
		// bottom side
		vec3(1.0, 0.0, 1.0),
		vec3(0.0, 0.0, 1.0),
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 0.0, 0.0),		
		// back side
		vec3(0.0, 1.0, 0.0),
		vec3(1.0, 1.0, 0.0),	
		vec3(0.0, 0.0, 0.0),
		vec3(1.0, 0.0, 0.0)		
	};	
	vector<unsigned int> indices {
		// front side
		0, 2, 1, 1, 2, 3,
		// right side
		4, 6, 5, 5, 6, 7,
		// top side
		8, 10, 9, 9, 10, 11,
		// left side
		12, 14, 13, 13, 14, 15,
		// bottom side
		16, 18, 17, 17, 18, 19,
		// back side
		20, 22, 21, 21, 22, 23
	};
	volumeMesh.addVertices(vertices);
	for(int i = 0; i < indices.size(); i++){
		volumeMesh.addIndex(indices[i]);
	}
}

void ofxVolumetrics::setup(int w, int h, int d, vec3 voxelSize, bool usePowerOfTwoTexSize) {
#ifndef TARGET_EMSCRIPTEN
	if (ofIsGLProgrammableRenderer()) {
		string shaderProgramVert = STRINGIFY(
		// OF_GLSL_SHADER_HEADER is replaced by OF with the appropriate shader header
		OF_GLSL_SHADER_HEADER
		in vec4	position;

		out vec3 v_texcoord;
		out vec3 v_cameraPosition;

		uniform mat4 modelViewProjectionMatrix;
		uniform mat4 modelViewMatrixInverse;

		void main()
		{
			gl_Position = modelViewProjectionMatrix * position;

			// as our vertex coordinates are normalized (0..1)
			// we can reuse them as 3d texture coordinates
			v_texcoord = position.xyz;

			v_cameraPosition = (modelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
		});
		string shaderProgramFrag = STRINGIFY(
		// OF_GLSL_SHADER_HEADER is replaced by OF with the appropriate shader header
		OF_GLSL_SHADER_HEADER
		in vec3 v_texcoord;
		in vec3 v_cameraPosition;

		out vec4 out_color;

		uniform sampler3D volume_tex;
		uniform vec3 vol_d;
		uniform vec3 vol_d_pot;
		uniform vec2 bg_d;
		uniform float zoffset;
		uniform float quality;
		uniform float threshold;
		uniform float density;

		struct Ray
		{
			vec3 Origin;
			vec3 Dir;
		};

		struct BoundingBox
		{
			vec3 Min;
			vec3 Max;
		};

		bool IntersectBox(Ray r, BoundingBox box, out float t0, out float t1)
		{
			vec3 invR = 1.0 / r.Dir;
			vec3 tbot = invR * (box.Min - r.Origin);
			vec3 ttop = invR * (box.Max - r.Origin);
			vec3 tmin = min(ttop, tbot);
			vec3 tmax = max(ttop, tbot);
			vec2 t = max(tmin.xx, tmin.yz);
			t0 = max(t.x, t.y);
			t = min(tmax.xx, tmax.yz);
			t1 = min(t.x, t.y);
			return t0 <= t1;
		}

		void main()
		{
			vec3 minv = vec3(0.) + 1. / vol_d_pot;
			vec3 maxv = (vol_d / vol_d_pot) - 1. / vol_d_pot;
			vec3 vec;
			vec3 vold = (maxv - minv) * vol_d;
			float vol_l = length(vold);

			vec4 col_acc = vec4(0, 0, 0, 0);
			vec3 zOffsetVec = vec3(0.0, 0.0, zoffset / vol_d_pot.z);
			vec3 backPos = v_texcoord;
			vec3 lookVec = normalize(backPos - v_cameraPosition);

			Ray eye = Ray(v_cameraPosition, lookVec);
			BoundingBox box = BoundingBox(vec3(0.), vec3(1.));

			float tnear;
			float tfar;
			IntersectBox(eye, box, tnear, tfar);
			if (tnear < 0.15) tnear = 0.15;
			if (tnear > tfar) discard;

			vec3 rayStart = (eye.Origin + eye.Dir * tnear) * (maxv - minv) + minv; //vol_d/vol_d_pot;
			vec3 rayStop = (eye.Origin + eye.Dir * tfar) * (maxv - minv) + minv; //vol_d/vol_d_pot;

			vec3 dir = rayStop - rayStart; // starting position of the ray

			vec = rayStart;
			float dl = length(dir);
			if (dl == clamp(dl, 0., vol_l))
			{
				int steps = int(floor(length(vold * dir) * quality));
				vec3 delta_dir = dir / float(steps);
				vec4 color_sample;
				float aScale = density / quality;

				float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
				vec += delta_dir * random;

				//raycast
				for (int i = 0; i < steps; i++)
				{
					vec3 vecz = vec + zOffsetVec;
					if (vecz.z > maxv.z)
					{
						vecz.z -= maxv.z;
					}
					color_sample = texture(volume_tex, vecz);
					if (color_sample.a > threshold)
					{
						float oneMinusAlpha = 1. - col_acc.a;
						color_sample.a *= aScale;
						col_acc.rgb = mix(col_acc.rgb, color_sample.rgb * color_sample.a, oneMinusAlpha);
						col_acc.a += color_sample.a * oneMinusAlpha;
						col_acc.rgb /= col_acc.a;
						if (col_acc.a >= 1.0)
						{
							break; // terminate if opacity > 1
						}
					}
					vec += delta_dir;
				}
			}
			out_color = col_acc;
		});
		volumeShader.setupShaderFromSource(GL_VERTEX_SHADER, shaderProgramVert);
		volumeShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgramFrag);
		volumeShader.linkProgram();
	}
	else {
		string shaderProgramVert = STRINGIFY(
		// OF_GLSL_SHADER_HEADER is replaced by OF with the appropriate shader header
		OF_GLSL_SHADER_HEADER
		varying vec3 cameraPosition;

		void main()
		{
			gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

			// ofMesh / ofVboMesh only supports 2d texture coordinates
			// as our vertex coordinates are normalized (0..1) 
			// we can reuse them as 3d texture coordinates
			gl_TexCoord[0] = gl_Vertex;

			cameraPosition = (gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
		});
		string shaderProgramFrag = STRINGIFY(
		OF_GLSL_SHADER_HEADER
		varying vec3 cameraPosition;

		uniform sampler3D volume_tex;
		uniform vec3 vol_d;
		uniform vec3 vol_d_pot;
		uniform vec2 bg_d;
		uniform float zoffset;
		uniform float quality;
		uniform float threshold;
		uniform float density;

		struct Ray
		{
			vec3 Origin;
			vec3 Dir;
		};

		struct BoundingBox
		{
			vec3 Min;
			vec3 Max;
		};

		bool IntersectBox(Ray r, BoundingBox box, out float t0, out float t1)
		{
			vec3 invR = 1.0 / r.Dir;
			vec3 tbot = invR * (box.Min - r.Origin);
			vec3 ttop = invR * (box.Max - r.Origin);
			vec3 tmin = min(ttop, tbot);
			vec3 tmax = max(ttop, tbot);
			vec2 t = max(tmin.xx, tmin.yz);
			t0 = max(t.x, t.y);
			t = min(tmax.xx, tmax.yz);
			t1 = min(t.x, t.y);
			return t0 <= t1;
		}

		void main()
		{
			vec3 minv = vec3(0.) + 1. / vol_d_pot;
			vec3 maxv = (vol_d / vol_d_pot) - 1. / vol_d_pot;
			vec3 vec;
			vec3 vold = (maxv - minv) * vol_d;
			float vol_l = length(vold);

			vec4 col_acc = vec4(0, 0, 0, 0);
			vec3 zOffsetVec = vec3(0.0, 0.0, zoffset / vol_d_pot.z);
			vec3 backPos = gl_TexCoord[0].xyz;
			vec3 lookVec = normalize(backPos - cameraPosition);

			Ray eye = Ray(cameraPosition, lookVec);
			BoundingBox box = BoundingBox(vec3(0.), vec3(1.));

			float tnear;
			float tfar;
			IntersectBox(eye, box, tnear, tfar);
			if (tnear < 0.15) tnear = 0.15;
			if (tnear > tfar) discard;

			vec3 rayStart = (eye.Origin + eye.Dir * tnear) * (maxv - minv) + minv; //vol_d/vol_d_pot;
			vec3 rayStop = (eye.Origin + eye.Dir * tfar) * (maxv - minv) + minv; //vol_d/vol_d_pot;

			vec3 dir = rayStop - rayStart; // starting position of the ray

			vec = rayStart;
			float dl = length(dir);
			if (dl == clamp(dl, 0., vol_l))
			{
				int steps = int(floor(length(vold * dir) * quality));
				vec3 delta_dir = dir / float(steps);
				vec4 color_sample;
				float aScale = density / quality;

				float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
				vec += delta_dir * random;

				//raycast
				for (int i = 0; i < steps; i++)
				{
					vec3 vecz = vec + zOffsetVec;
					if (vecz.z > maxv.z)
					{
						vecz.z -= maxv.z;
					}
					color_sample = texture3D(volume_tex, vecz);
					if (color_sample.a > threshold)
					{
						float oneMinusAlpha = 1. - col_acc.a;
						color_sample.a *= aScale;
						col_acc.rgb = mix(col_acc.rgb, color_sample.rgb * color_sample.a, oneMinusAlpha);
						col_acc.a += color_sample.a * oneMinusAlpha;
						col_acc.rgb /= col_acc.a;
						if (col_acc.a >= 1.0)
						{
							break; // terminate if opacity > 1
						}
					}
					vec += delta_dir;
				}
			}
			gl_FragColor = col_acc;
		});
		volumeShader.setupShaderFromSource(GL_VERTEX_SHADER, shaderProgramVert);
		volumeShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgramFrag);
		volumeShader.linkProgram();
	};
#else
string shaderProgramVert = STRINGIFY(
// OF_GLSL_SHADER_HEADER is replaced by OF with the appropriate shader header
OF_GLSL_SHADER_HEADER
in vec4	position;

out vec3 v_texcoord;
out vec3 v_cameraPosition;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrixInverse;

void main()
{
	gl_Position = modelViewProjectionMatrix * position;

	// as our vertex coordinates are normalized (0..1)
	// we can reuse them as 3d texture coordinates
	v_texcoord = position.xyz;

	v_cameraPosition = (modelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
});
string shaderProgramFrag = STRINGIFY(
// OF_GLSL_SHADER_HEADER is replaced by OF with the appropriate shader header
OF_GLSL_SHADER_HEADER
precision lowp float;
precision lowp sampler3D;
//#extension GL_ARB_texture_rectangle : enable

in vec3 v_texcoord;
in vec3 v_cameraPosition;

out vec4 out_color;

uniform sampler3D volume_tex;
uniform vec3 vol_d;
uniform vec3 vol_d_pot;
uniform vec2 bg_d;
uniform float zoffset;
uniform float quality;
uniform float threshold;
uniform float density;

struct Ray
{
	vec3 Origin;
	vec3 Dir;
};

struct BoundingBox
{
	vec3 Min;
	vec3 Max;
};

bool IntersectBox(Ray r, BoundingBox box, out float t0, out float t1)
{
	vec3 invR = 1.0 / r.Dir;
	vec3 tbot = invR * (box.Min - r.Origin);
	vec3 ttop = invR * (box.Max - r.Origin);
	vec3 tmin = min(ttop, tbot);
	vec3 tmax = max(ttop, tbot);
	vec2 t = max(tmin.xx, tmin.yz);
	t0 = max(t.x, t.y);
	t = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);
	return t0 <= t1;
}

void main()
{
	vec3 minv = vec3(0.) + 1. / vol_d_pot;
	vec3 maxv = (vol_d / vol_d_pot) - 1. / vol_d_pot;
	vec3 vec;
	vec3 vold = (maxv - minv) * vol_d;
	float vol_l = length(vold);

	vec4 col_acc = vec4(0, 0, 0, 0);
	vec3 zOffsetVec = vec3(0.0, 0.0, zoffset / vol_d_pot.z);
	vec3 backPos = v_texcoord;
	vec3 lookVec = normalize(backPos - v_cameraPosition);

	Ray eye = Ray(v_cameraPosition, lookVec);
	BoundingBox box = BoundingBox(vec3(0.), vec3(1.));

	float tnear;
	float tfar;
	IntersectBox(eye, box, tnear, tfar);
	if (tnear < 0.15) tnear = 0.15;
	if (tnear > tfar) discard;

	vec3 rayStart = (eye.Origin + eye.Dir * tnear) * (maxv - minv) + minv; //vol_d/vol_d_pot;
	vec3 rayStop = (eye.Origin + eye.Dir * tfar) * (maxv - minv) + minv; //vol_d/vol_d_pot;

	vec3 dir = rayStop - rayStart; // starting position of the ray

	vec = rayStart;
	float dl = length(dir);
	if (dl == clamp(dl, 0., vol_l))
	{
		int steps = int(floor(length(vold * dir) * quality));
		vec3 delta_dir = dir / float(steps);
		vec4 color_sample;
		float aScale = density / quality;

		float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
		vec += delta_dir * random;

		//raycast
		for (int i = 0; i < steps; i++)
		{
			vec3 vecz = vec + zOffsetVec;
			if (vecz.z > maxv.z)
			{
				vecz.z -= maxv.z;
			}
			color_sample = texture(volume_tex, vecz);
			if (color_sample.a > threshold)
			{
				float oneMinusAlpha = 1. - col_acc.a;
				color_sample.a *= aScale;
				col_acc.rgb = mix(col_acc.rgb, color_sample.rgb * color_sample.a, oneMinusAlpha);
				col_acc.a += color_sample.a * oneMinusAlpha;
				col_acc.rgb /= col_acc.a;
				if (col_acc.a >= 1.0)
				{
					break; // terminate if opacity > 1
				}
			}
			vec += delta_dir;
		}
	}
	out_color = col_acc;
});

volumeShader.setupShaderFromSource(GL_VERTEX_SHADER, shaderProgramVert);
volumeShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgramFrag);
volumeShader.linkProgram();
#endif
	bIsPowerOfTwo = usePowerOfTwoTexSize;

	volWidthPOT = volWidth = renderWidth = w;
	volHeightPOT = volHeight = renderHeight = h;
	volDepthPOT = volDepth = d;

	if (bIsPowerOfTwo) {
		volWidthPOT = ofNextPow2(w);
		volHeightPOT = ofNextPow2(h);
		volDepthPOT = ofNextPow2(d);

		ofLogVerbose() << "ofxVolumetrics::setup(): Using power of two texture size. Requested: " << w << "x" << h << "x" << d << ". Actual: " << volWidthPOT << "x" << volHeightPOT << "x" << volDepthPOT << ".\n";
	}

	fboRender.allocate(w, h, GL_RGBA8);
	volumeTexture.allocate(volWidthPOT, volHeightPOT, volDepthPOT, GL_RGBA8);
	if (bIsPowerOfTwo) {
		// if using cropped power of two, blank out the extra memory
		unsigned char* d;
		d = new unsigned char[volWidthPOT * volHeightPOT * volDepthPOT * 4];
		memset(d, 0, volWidthPOT * volHeightPOT * volDepthPOT * 4);
		volumeTexture.loadData(d, volWidthPOT, volHeightPOT, volDepthPOT, 0, 0, 0, GL_RGBA);
	}
	voxelRatio = voxelSize;

	bIsInitialized = true;
}

void ofxVolumetrics::destroy() {
	volumeShader.unload();
	volumeTexture.clear();

	volWidth = renderWidth = 0;
	volHeight = renderHeight = 0;
	volDepth = 0;
	bIsInitialized = false;
}

void ofxVolumetrics::updateVolumeData(unsigned char* data, int w, int h, int d, int xOffset, int yOffset, int zOffset) {
	volumeTexture.loadData(data, w, h, d, xOffset, yOffset, zOffset, GL_RGBA);
}

void ofxVolumetrics::updateTexture(int xOffset, int yOffset, int zOffset, int x, int y, int width, int height) {
	volumeTexture.loadTexture(xOffset, yOffset, zOffset, x, y, width, height);
}

void ofxVolumetrics::drawVolume(float x, float y, float z, float size, int zTexOffset) {
	vec3 volumeSize = voxelRatio * vec3(volWidth, volHeight, volDepth);
	float maxDim = glm::max(glm::max(volumeSize.x, volumeSize.y), volumeSize.z);
	volumeSize = volumeSize * size / maxDim;

	drawVolume(x, y, z, volumeSize.x, volumeSize.y, volumeSize.z, zTexOffset);
}

void ofxVolumetrics::drawVolume(float x, float y, float z, float w, float h, float d, int zTexOffset) {
	updateRenderDimentions();

	// store current color
	GLint color[4];
	glGetIntegerv(GL_CURRENT_COLOR, color);

	// store current cull mode
	GLint cull_mode;
	glGetIntegerv(GL_FRONT_FACE, &cull_mode);

	// set fbo cull mode
	mat4 matModelview = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
	ofVec3f scale, t; ofQuaternion a, b;
	ofMatrix4x4(matModelview).decompose(t, a, scale, b);
	GLint cull_mode_fbo = (scale.x * scale.y * scale.z) > 0 ? GL_CCW : GL_CW;

	// raycasting pass
	fboRender.begin(OF_FBOMODE_NODEFAULTS);
	{
		// fix flipped y-axis
		ofSetMatrixMode(OF_MATRIX_PROJECTION);
		ofScale(1, -1, 1);
		ofSetMatrixMode(OF_MATRIX_MODELVIEW);
		ofScale(1, -1, 1);

		ofClear(0, 0);

		vec3 cubeSize(w, h, d);
		vec3 cubePos(x, y, z);
		ofTranslate(cubePos - cubeSize / 2.f);
		ofScale(cubeSize.x, cubeSize.y, cubeSize.z);

		volumeShader.begin();
		volumeShader.setUniformTexture("volume_tex", GL_TEXTURE_3D, volumeTexture.getTextureData().textureID, 0);
		volumeShader.setUniform3f("vol_d", vec3(volWidth, volHeight, volDepth)); //dimensions of the volume texture
		volumeShader.setUniform3f("vol_d_pot", vec3(volWidthPOT, volHeightPOT, volDepthPOT)); //dimensions of the volume texture power of two
		volumeShader.setUniform2f("bg_d", vec2(renderWidth, renderHeight)); // dimensions of the background texture
		volumeShader.setUniform1f("zoffset", zTexOffset); // used for animation so that we dont have to upload the entire volume every time
		volumeShader.setUniform1f("quality", quality.z); // 0..1
		volumeShader.setUniform1f("density", density); // 0..1
		volumeShader.setUniform1f("threshold", threshold);
		if (ofIsGLProgrammableRenderer()) {
			volumeShader.setUniformMatrix4f("modelViewMatrixInverse", inverse(ofGetCurrentMatrix(OF_MATRIX_MODELVIEW)));
		}

		glFrontFace(cull_mode_fbo);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		volumeMesh.drawFaces();
		glDisable(GL_CULL_FACE);
		glFrontFace(cull_mode);

		volumeShader.end();

		if (bDrawDebugVolume) {
			glFrontFace(cull_mode_fbo);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			volumeMesh.drawWireframe();
			glDisable(GL_CULL_FACE);
			glFrontFace(cull_mode);
		}		
	}
	fboRender.end();

	ofPushView();

	ofSetColor(255);
	ofSetupScreenOrtho();		
	fboRender.draw(0, 0, ofGetWidth(), ofGetHeight());

	ofPopView();
}

void ofxVolumetrics::updateRenderDimentions() {
	if ((int)(ofGetWidth() * quality.x) != renderWidth) {
		renderWidth = ofGetWidth() * quality.x;
		renderHeight = ofGetHeight() * quality.x;
		fboRender.allocate(renderWidth, renderHeight, GL_RGBA);
	}
}

void ofxVolumetrics::setXyQuality(float q) {
	quality.x = MAX(q, 0.01);

	updateRenderDimentions();
}
void ofxVolumetrics::setZQuality(float q) {
	quality.z = MAX(q, 0.01);
}
void ofxVolumetrics::setThreshold(float t) {
	threshold = ofClamp(t, 0.0, 1.0);
}
void ofxVolumetrics::setDensity(float d) {
	density = MAX(d, 0.0);
}
void ofxVolumetrics::setRenderSettings(float xyQuality, float zQuality, float dens, float thresh) {
	setXyQuality(xyQuality);
	setZQuality(zQuality);
	setDensity(dens);
	setThreshold(thresh);
}

void ofxVolumetrics::setVolumeTextureFilterMode(GLint filterMode) {
	volumeTexture.setTextureMinMagFilter(filterMode);
}

void ofxVolumetrics::setDrawDebugVolume(bool b) {
	bDrawDebugVolume = b;
}

bool ofxVolumetrics::isInitialized() {
	return bIsInitialized;
}
int ofxVolumetrics::getVolumeWidth() {
	return volWidth;
}
int ofxVolumetrics::getVolumeHeight() {
	return volHeight;
}
int ofxVolumetrics::getVolumeDepth() {
	return volDepth;
}
int ofxVolumetrics::getRenderWidth() {
	return renderWidth;
}
int ofxVolumetrics::getRenderHeight() {
	return renderHeight;
}
float ofxVolumetrics::getXyQuality() {
	return quality.x;
}
float ofxVolumetrics::getZQuality() {
	return quality.z;
}
float ofxVolumetrics::getThreshold() {
	return threshold;
}
float ofxVolumetrics::getDensity() {
	return density;
}
const ofFbo& ofxVolumetrics::getFbo() const {
	return fboRender;
}

ofxTextureData3d ofxVolumetrics::getTextureData() {
	return volumeTexture.getTextureData();
}
