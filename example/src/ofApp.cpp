#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	measurementListener =  goIO.newMeasurementEvent.newListener( [&](ofxGoIOMeasurement& m){
		measurement = m;
	});
	goIO.setup();
	goIO.startMeasurements();
}

//--------------------------------------------------------------
void ofApp::update(){

}
//--------------------------------------------------------------
void ofApp::exit(){
	goIO.close();
}
//--------------------------------------------------------------
void ofApp::draw(){
	float y = 30;
	ofBitmapFont bf;
	{
		stringstream ss;
		ss << goIO.getCurrentDevice();
		ofDrawBitmapStringHighlight(ss.str(), 30, y, ofColor::yellow, ofColor::black);
		y += bf.getBoundingBox(ss.str(), 0, 0).height + 30;
	}
	{
		auto state = ofxGoIO::stateToString(goIO.getState());
		ofDrawBitmapStringHighlight(state, 30, y, ofColor::magenta, ofColor::black);
		y += bf.getBoundingBox(state, 0, 0).height + 30;
	}
	{
		stringstream ss;
		ss << measurement;
		ofDrawBitmapStringHighlight(ss.str(), 30, y, ofColor::cyan, ofColor::black);
	}
	
	

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
