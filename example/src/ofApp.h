#pragma once

#include "ofMain.h"
#include "ofxGoIO.h"
#include "ofxGoIORenderer.h"
#include "ofxGui.h"
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
	void exit();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
	ofxGoIO goIO;
	ofxGoIOMeasurement measurement;
	

	ofEventListener measurementListener;

	ofxGoIORenderer renderer;
	bool bDraw = false;
	double maxPeriod = 0;
	double minPeriod = 0;
	double currentPeriod = 0;
	
	ofxGoIOMeasurement rendererBuffer;
	size_t bufferIndex = 0;
	
	ofRectangle waveRect;

	
	ofParameter<float> x = {"x", 30, 0, 1000};
	ofParameter<float> y = {"y", 30, 0, 1000};
	ofParameter<float> margin = {"margin", 10, 0, 100};
	ofParameter<float> padding = {"padding", 5, 0, 100};
	

	ofxPanel gui;
	
	ofEasyCam cam;
};
