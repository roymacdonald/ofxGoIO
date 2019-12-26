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
	
	ofBitmapFont bf;
	cout << "ofBitmapFont rect: " << ofToString(bf.getBoundingBox(" ", 0, 0)) << endl;
	
	
	gui.setup();
	
	gui.add(x);
	gui.add(y);
	gui.add(margin);
	gui.add(padding);
	
	cam.removeAllInteractions();
	cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);
	cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_Z, OF_MOUSE_BUTTON_RIGHT);
	cam.enableOrtho();
	cam.setNearClip(-1000000);
	cam.setFarClip(1000000);
	cam.setVFlip(true);
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
ofRectangle drawString(const string& str, const ofRectangle& refRect, bool bHorizontalStack, float margin, float padding, ofColor bgcolor, ofColor fgcolor = ofColor::black){
	ofBitmapFont bf;
	ofPushStyle();
	ofSetColor(bgcolor);
	glm::vec3 p;
	if(bHorizontalStack){
		p = refRect.getTopRight();
		p.x += margin;
		
	}else{
		p = refRect.getBottomLeft();
		p.y += margin;
	}
	p.x += padding;
	p.y += padding;
	p.y += 11;
	
	auto r = bf.getBoundingBox(str, p.x, p.y);
	r.x -= padding;
	r.y -= padding;
	r.height += padding*2;
	r.width += padding*2;
//	float h = r.height;
	ofDrawRectangle(r);
	
	ofSetColor(fgcolor);
	ofDrawBitmapString(str, p.x, p.y);
	
	ofPopStyle();
	
//	y += h + margin;
//	x = r.getMaxX() + margin;
	
	return r;
}
//--------------------------------------------------------------
void ofApp::draw(){
//	float y = 30;
//	float margin = 10;
//	float padding = 5;
//	float x = 30;
//
//	cam.begin();
	float mxy = 0;
	float mnx = x - padding;
	ofRectangle r(mnx, 0, 0,0);
	{
		stringstream ss;
		ss << goIO.getCurrentDevice();
		ss << endl;
		ss << "a"<< endl;
		ss << "Measurement period: " << endl;
		ss << "    current: " << currentPeriod << endl;
		ss << "    maximum: " << maxPeriod << endl;
		ss << "    minimum: " << minPeriod << endl;
		
		r = drawString(ss.str(), r, false, margin, padding, ofColor::yellow);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		auto state = ofxGoIO::stateToString(goIO.getState());
		r = drawString(state, r, true, margin, padding, ofColor::magenta);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		stringstream ss;
		ss << measurement;
		auto r2 = drawString(ss.str(), r, false, margin, padding, ofColor::cyan);
		mxy =  std::max(mxy, r2.getMaxY());
	}
	stringstream ss;
	ss << "App Framerate: " << ofGetFrameRate();
	r = drawString(ss.str(), r, true, margin, padding, ofColor::black, ofColor::white);
	mxy =  std::max(mxy, r.getMaxY());
//	if(bDraw){
	mxy += margin;
	waveRect.set( mnx, mxy, ofGetWidth() - 2*mnx,  ofGetHeight() - mxy - mnx );
	
//	renderer.draw(rendererBuffer, bufferIndex, waveRect, ofColor(40), ofColor(150));
	renderer.draw(goIO, waveRect, ofColor(40), ofColor(150));
//	}
//x	xxx
	
//	cam.end();
	
//	gui.draw();
	
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
