#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	measurementListener =  goIO.newMeasurementEvent.newListener( [&](ofxGoIOMeasurement& m){
		measurement = m;
	});
	if(goIO.setup()){
		goIO.startMeasurements();
		maxPeriod = goIO.getMaximumMeasurementPeriod();
		minPeriod = goIO.getMinimumMeasurementPeriod();
		currentPeriod = goIO.getMeasurementPeriod() ;

	}
}

//--------------------------------------------------------------
void ofApp::update(){

}
//--------------------------------------------------------------
void ofApp::exit(){
	goIO.close();
}
void drawString(const string& str, float x, float & y, float margin, ofColor bgcolor, ofColor fgcolor = ofColor::black){
	ofBitmapFont bf;
	ofPushStyle();
	ofSetColor(bgcolor);
	auto r = bf.getBoundingBox(str, x, y);
	r.x -= 5;
	r.y -= 5;
	r.height +=10;
	r.width +=10;
	float h = r.height;
	ofDrawRectangle(r);
	
	ofSetColor(fgcolor);
	ofDrawBitmapString(str, x, y);
	
	ofPopStyle();
	
	y += h + margin;
	
}
//--------------------------------------------------------------
void ofApp::draw(){
	float y = 30;
	float margin = 10;
	float x = 30;
	{
		stringstream ss;
		ss << goIO.getCurrentDevice();
		ss << endl;
		ss << "a"<< endl;
		ss << "Measurement period: " << endl;
		ss << "    current: " << currentPeriod << endl;
		ss << "    maximum: " << maxPeriod << endl;
		ss << "    minimum: " << minPeriod << endl;
		
		drawString(ss.str(), x, y,  margin, ofColor::yellow);
		
	}
	{
		auto state = ofxGoIO::stateToString(goIO.getState());
		drawString(state, x, y,  margin, ofColor::magenta);
		
	}
	{
		stringstream ss;
		ss << measurement;
		drawString(ss.str(), x, y,  margin, ofColor::cyan);
	}
	stringstream ss;
	ss << "App Framerate: " << ofGetFrameRate();
	drawString(ss.str(), x, y,  margin, ofColor::black, ofColor::white);

	if(bDraw){
		ofRectangle r( x, y, ofGetWidth() - 2*x,  ofGetHeight() - y - x );
		ofSetColor(80);
		ofDrawRectangle(r);
		ofSetColor(180);
		renderer.draw(goIO, r);
	}
	

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if(key == ' '){
		bDraw ^= true;
	}
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
