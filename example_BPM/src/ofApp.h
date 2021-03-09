#pragma once

#include "ofMain.h"
#include "ofxGoIO.h"
#include "ofxGoIORenderer.h"
#include "ofxGui.h"

#include "heartRate.h"

#define RATE_SIZE 4 //Increase this for more averaging. 4 is good.

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
    
    float lastAquisitionTime;
    
    char rates[RATE_SIZE]; //Array of heart rates
    char rateSpot = 0;
    long lastBeat = 0; //Time at which the last beat occurred
    
    float beatsPerMinute;
    int beatAvg;
    bool detected;
    heartRate heart_object;
};
