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
	rendererBuffer.resize(1000);
	for(size_t i = 0; i < rendererBuffer.size(); i++){
		rendererBuffer[i].rawData = ofMap(i, 0, rendererBuffer.size() - 1 ,-32767, 32767);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	rendererBuffer[bufferIndex].rawData = ofGetMouseY();
	++bufferIndex %= rendererBuffer.size();
}
//--------------------------------------------------------------
void ofApp::exit(){
	goIO.close();
}
ofRectangle drawString(const string& str, float &x, float & y, float margin, float padding, ofColor bgcolor, ofColor fgcolor = ofColor::black){
	ofBitmapFont bf;
	ofPushStyle();
	ofSetColor(bgcolor);
	auto r = bf.getBoundingBox(str, x, y);
	r.x -= padding;
	r.y -= padding;
	r.height += padding*2;
	r.width += padding*2;
//	float h = r.height;
	ofDrawRectangle(r);
	
	ofSetColor(fgcolor);
	ofDrawBitmapString(str, x, y);
	
	ofPopStyle();
	
//	y += h + margin;
	x = r.getMaxX() + margin;
	
	return r;
}
//--------------------------------------------------------------
void ofApp::draw(){
	float y = 30;
	float margin = 10;
	float padding = 5;
	float x = 30;
	float mxy = 0;
	float mnx = x - padding;
	ofRectangle r;
	{
		stringstream ss;
		ss << goIO.getCurrentDevice();
		ss << endl;
		ss << "a"<< endl;
		ss << "Measurement period: " << endl;
		ss << "    current: " << currentPeriod << endl;
		ss << "    maximum: " << maxPeriod << endl;
		ss << "    minimum: " << minPeriod << endl;
		
		r = drawString(ss.str(), x, y, margin, padding, ofColor::yellow);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		auto state = ofxGoIO::stateToString(goIO.getState());
		r = drawString(state, x, y, margin, padding, ofColor::magenta);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		stringstream ss;
		ss << measurement;
		r = drawString(ss.str(), x, y, margin, padding, ofColor::cyan);
		mxy =  std::max(mxy, r.getMaxY());
	}
	stringstream ss;
	ss << "App Framerate: " << ofGetFrameRate();
	r = drawString(ss.str(), x, y, margin, padding, ofColor::black, ofColor::white);
	mxy =  std::max(mxy, r.getMaxY());
//	if(bDraw){
	mxy += margin;
	waveRect.set( mnx, mxy, ofGetWidth() - 2*mnx,  ofGetHeight() - mxy - mnx );
	renderer.draw(rendererBuffer, bufferIndex, waveRect, ofColor(40), ofColor(150));
//	}
	

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
